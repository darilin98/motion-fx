//
// Created by Darek Rudiš on 08.11.2025.
//


#include "playbackcontroller.hpp"
#include "vstgui/lib/tasks.h"

void PlaybackController::startPipeline(const double playbackRate) {
	is_running_ = true;
	rate_ = playbackRate;
	if (view_) view_->startConsumingAt(25);
	scheduleNextFrame();
}

void PlaybackController::stopPipeline() {
	if (view_) view_->stopConsuming();
	is_running_ = false;
}

void PlaybackController::scheduleNextFrame() {
	if (!is_running_) return;

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

void PlaybackController::setMediaView(MediaView* view) {
	view_ = view;
}



