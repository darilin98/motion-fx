//
// Created by Darek Rudiš on 08.11.2025.
//


#include "playbackcontroller.hpp"
#include "../controller.hpp"
#include "videoparamlistener.hpp"
#include "vstgui/lib/tasks.h"
#include "../parameterdefaults.hpp"

PlaybackController::~PlaybackController() = default;

void PlaybackController::shutdown() {
	++generation_;

	is_decoding_.store(false);
	if (loader_) {
		loader_->onFrame = nullptr;
	}
	if (frame_ticker_) {
		frame_ticker_->stopConsuming();
		frame_ticker_->clearReceivers();
	}

	play_listener_.reset();
	reset_listener_.reset();
}

void PlaybackController::startPipeline(const double playbackRate) {
	++generation_;
	is_decoding_ = true;
	rate_ = playbackRate;
	setupCallbacks();
	if (frame_ticker_) {
		frame_ticker_->resetTimer();
		frame_ticker_->startConsumingAt(25);
		is_playing_ = true;
	}
	scheduleNextFrame();
}

void PlaybackController::stopPipeline() {
	++generation_;
	if (frame_ticker_) {
		frame_ticker_->stopConsuming();
		is_playing_ = false;
	}
	is_decoding_.store(false);
}

void PlaybackController::scheduleNextFrame() {
	if (!is_decoding_) return;

	const uint64_t task_generation = generation_.load();
	// TODO: Move this to regular work thread - needs to function without GUI
	VSTGUI::Tasks::schedule(VSTGUI::Tasks::backgroundQueue(), [self = shared_from_this(), task_generation]() {
		if (self->generation_ != task_generation) return;
		if (!self->is_decoding_) return;

		// TODO: manage rate of requests based on speed setting
		if(self->loader_->requestNextFrame()) {
			self->scheduleNextFrame();
		} else {
			self->is_decoding_ = false;
		}
	});
}

void PlaybackController::setupCallbacks() {
	if (loader_) {
		loader_->onFrame = [this](VideoFrame&& frame) {
			frame_queue_->push(std::move(frame));
		};
	}

	if (frame_ticker_) {
		frame_ticker_->setOnQueueEmptyCallback(nullptr);
		frame_ticker_->setOnQueueEmptyCallback([self = shared_from_this()] {
			VSTGUI::Tasks::schedule(VSTGUI::Tasks::mainQueue(), [self] {
				if (self->is_decoding_)
					return;

				self->onVideoFinished();

				if (self->looping_) {
					if (self->loader_->tryRewindToStart()) {
						self->is_decoding_.store(true);
						self->scheduleNextFrame();
						if (self->frame_ticker_) self->frame_ticker_->resetTimer();
						return;
					}
				}

				fprintf(stderr, "Playback finished, static last frame displayed (scheduled stop)\n");
				self->stopPipeline();
			});
		});
	}
}

void PlaybackController::registerReceiver(IFrameReceiver* receiver) const {
	if (frame_ticker_) frame_ticker_->addReceiver(receiver);
}


void PlaybackController::unregisterReceiver(IFrameReceiver* receiver) const {
	if (frame_ticker_) frame_ticker_->removeReceiver(receiver);
}

void PlaybackController::setParamListeners(controller_t controller) {
	this->controller_ = controller;
	play_listener_ = std::make_unique<VideoParamListener>(
		controller_->getParameterObject(kParamPlay), shared_from_this(), controller_);
	reset_listener_ = std::make_unique<VideoParamListener>(
		controller_->getParameterObject(kParamReset), shared_from_this(), controller_);
}


void PlaybackController::onParamChanged(Steinberg::Vst::ParamID paramId, float paramValue) {
	switch (paramId) {
		case kParamPlay:
			if (paramValue > 0.5 && !is_playing_) {
				startPipeline(1.0);
			}
			break;
		case kParamReset:
			if (paramValue > 0.5 && is_playing_) {
				if (loader_->tryRewindToStart()) {
					stopPipeline();
					frame_queue_->clearAsync();
				}
			}
		default:
			break;
	}
}

void PlaybackController::onVideoFinished() const {
	if (controller_) {
		if (auto* pcont = dynamic_cast<PluginController*>(controller_)) {
			pcont->onVideoFinished();
		}
	}
}




