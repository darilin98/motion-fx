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
#include "frameticker.hpp"
#include "features/ifeatureextractor.hpp"

#include "videoparamlistener.hpp"

class PlaybackController : public std::enable_shared_from_this<PlaybackController> {
public:
	explicit PlaybackController(const std::string& path, frame_queue_t frameQueue, frame_ticker_t ticker)
		: loader_(std::make_unique<MediaLoader>(path)), frame_queue_(std::move(frameQueue)), frame_ticker_(std::move(ticker)) {}

	explicit PlaybackController(loader_t loader, frame_queue_t frameQueue, frame_ticker_t ticker)
		: loader_(std::move(loader)), frame_queue_(std::move(frameQueue)), frame_ticker_(std::move(ticker)) {}

	~PlaybackController();

	void startPipeline(double playbackRate);
	// void pausePipeline();
	void stopPipeline();
	// void setPlaybackRate(double playbackRate);

	void shutdown();

	void registerReceiver(IFrameReceiver* receiver) const;
	void unregisterReceiver(IFrameReceiver* receiver) const;

	void setParamListeners(controller_t controller);
	void onParamChanged(Steinberg::Vst::ParamID paramId, float paramValue);

private:
	void setupCallbacks();
	void scheduleNextFrame();
	std::atomic<bool> is_decoding_ = false;
	std::atomic<bool> is_playing_ = false;
	std::atomic<uint64_t> generation_ { 0 };
	bool looping_ = true;
	double rate_ = 0.0;
	loader_t loader_;
	frame_queue_t frame_queue_;
	frame_ticker_t frame_ticker_;
	controller_t controller_ = nullptr;

	listener_t play_listener_;
	listener_t reset_listener_;
};

using pcont_t = std::shared_ptr<PlaybackController>;

#endif //PLAYBACKCONTROLLER_HPP
