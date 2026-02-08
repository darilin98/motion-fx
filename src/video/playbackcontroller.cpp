//
// Created by Darek Rudiš on 08.11.2025.
//


#include "playbackcontroller.hpp"
#include "videoparamlistener.hpp"
#include "vstgui/lib/tasks.h"
#include "../parameterdefaults.hpp"

PlaybackController::~PlaybackController() = default;

void PlaybackController::startPipeline(const double playbackRate) {
	is_running_ = true;
	rate_ = playbackRate;
	setupCallbacks();
	if (frame_ticker_) {
		frame_ticker_->resetTimer();
		frame_ticker_->startConsumingAt(25);
	}
	scheduleNextFrame();
}

void PlaybackController::stopPipeline() {
	if (frame_ticker_) frame_ticker_->stopConsuming();
	is_running_.store(false);
}

void PlaybackController::scheduleNextFrame() {
	if (!is_running_) return;

	// TODO: Move this to regular work thread - needs to function without GUI
	VSTGUI::Tasks::schedule(VSTGUI::Tasks::backgroundQueue(), [self = shared_from_this()] {
		if (!self->is_running_) return;

		// TODO: manage rate of requests based on speed setting
		if(self->loader_->requestNextFrame()) {
			self->scheduleNextFrame();
		} else {
			self->is_running_ = false;
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
		frame_ticker_->setOnQueueEmptyCallback([self = shared_from_this()] {
			if (self->is_running_)
				return;
			if (self->looping_) {
				if (self->loader_->tryRewindToStart()) {
					self->is_running_.store(true);
					self->scheduleNextFrame();
					self->frame_ticker_->resetTimer();
					return;
				}
			}
			fprintf(stderr, "Playback finished, static last frame displayed\n");
			self->stopPipeline();
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
}


void PlaybackController::onParamChanged(Steinberg::Vst::ParamID paramId, float paramValue) {
	switch (paramId) {
		case kParamPlay:
			if (paramValue > 0.5) {
				is_running_ ? stopPipeline() : startPipeline(1.0);
			}
			break;
		default:
			break;
	}
}




