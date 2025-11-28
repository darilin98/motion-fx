//
// Created by Darek Rudiš on 08.11.2025.
//

#pragma once
#ifndef PLAYBACKCONTROLLER_HPP
#define PLAYBACKCONTROLLER_HPP

#include <utility>
#include "../video/framequeue.hpp"
#include "../video/medialoader.hpp"
#include "../ui/mediaview.hpp"

class MediaView;

class PlaybackController : public std::enable_shared_from_this<PlaybackController> {
public:
	explicit PlaybackController(const std::string& path, frame_queue_t frameQueue)
		: loader_(std::make_unique<MediaLoader>(path)), frame_queue_(std::move(frameQueue)) {
		loader_->onFrame = [this](VideoFrame&& frame) {
			frame_queue_->push(std::move(frame));
		};
	}

	explicit PlaybackController(loader_t loader, frame_queue_t frameQueue)
		: loader_(std::move(loader)), frame_queue_(std::move(frameQueue)) {
		loader_->onFrame = [this](VideoFrame&& frame) {
			frame_queue_->push(std::move(frame));
		};
	}

	void startPipeline(double playbackRate);
	// void pausePipeline();
	void stopPipeline();
	// void setPlaybackRate(double playbackRate);

	void setMediaView(MediaView* view);

private:
	void scheduleNextFrame();
	bool is_running_ = false;
	bool looping_ = true;
	double rate_ = 0.0;
	loader_t loader_;
	frame_queue_t frame_queue_;

	MediaView* view_ = nullptr;
};

using pcont_t = std::shared_ptr<PlaybackController>;


#endif //PLAYBACKCONTROLLER_HPP
