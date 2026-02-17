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
	if (loader_) {
		is_decoding_.store(false);
		loader_->stopLoading();
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
	rate_ = playbackRate;
	setupCallbacks();

	if (loader_) {
		is_decoding_.store(true);
		loader_->startLoading();
	}

	if (frame_ticker_) {
		frame_ticker_->resetTimer();
		is_playing_.store(true);
		frame_ticker_->startConsumingAt(25);
	}
}

void PlaybackController::stopPipeline() {
	if (loader_) {
		loader_->stopLoading();
		is_decoding_.store(false);
	}
	if (frame_ticker_) {
		frame_ticker_->stopConsuming();
		is_playing_.store(false);
	}
}

void PlaybackController::setupCallbacks() {
	if (loader_) {
		loader_->onFrame = [this](VideoFrame&& frame) {
			// TODO: Here we could add framerate limiting for pushing
			frame_queue_->push(std::move(frame));
		};
		loader_->onVideoFinish = [this]() {
			is_decoding_.store(false);
		};
	}

	if (frame_ticker_) {
		frame_ticker_->setOnQueueEmptyCallback(nullptr);
		frame_ticker_->setOnQueueEmptyCallback([self = shared_from_this()] {
			// TODO: the GUI task dependency could be removed by just requesting a restart higher up
			VSTGUI::Tasks::schedule(VSTGUI::Tasks::mainQueue(), [self] {
				if (self->is_decoding_)
					return;

				self->onVideoFinished();

				if (self->looping_) {
					if (self->loader_->tryRewindToStart()) {
						self->is_decoding_.store(true);
						self->loader_->startLoading();
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




