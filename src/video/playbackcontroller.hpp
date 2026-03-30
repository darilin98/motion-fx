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

enum class CompletionSignal { None, Handle, Shutdown };

/**
 * @brief Controls the state of the video pipeline.
 *
 * Two states exist in the pipeline, and they are not mutually exclusive.
 * If the producer is running, the pipeline is_playing_.
 * If the decoder is running, the pipeline is_decoding_.
 * Both can be true at a given time, but the pipeline cannot be decoding and not playing.
 */
class PlaybackController : public std::enable_shared_from_this<PlaybackController> {
public:
	/**
	 * @brief Initializes the controller with a loader created from a file path.
	 * @param path Absolute path to a media file.
	 * @param frameQueue Data structure connecting producer and consumer.
	 * @param ticker Consumer of VideoFrames from frameQueue.
	 */
	explicit PlaybackController(const std::string& path, frame_queue_t frameQueue, frame_ticker_t ticker)
		: loader_(std::make_unique<MediaLoader>(path)), frame_queue_(std::move(frameQueue)), frame_ticker_(std::move(ticker)) {}

	/**
	 * @brief Initializes the controller with an injected loader.
	 * @param loader An initialized MediaLoader.
	 * @param frameQueue Data structure connecting producer and consumer.
	 * @param ticker Consumer of VideoFrames from frameQueue.
	 */
	explicit PlaybackController(loader_t loader, frame_queue_t frameQueue, frame_ticker_t ticker)
		: loader_(std::move(loader)), frame_queue_(std::move(frameQueue)), frame_ticker_(std::move(ticker)) {}

	~PlaybackController();

	/**
	 * @brief Starts the full video pipeline.
	 * @param playbackRate Rate of playback as percentage.
	 */
	void startPipeline(double playbackRate);

	// void pausePipeline();

	/**
	 * @brief Stops the full video pipeline.
	 * The video pipeline can be restarted by calling startPipeline().
	 */
	void stopPipeline();

	// void setPlaybackRate(double playbackRate);

	/**
	 * @brief Cleans up after the video pipeline.
	 * @warning Cannot be restarted after this call.
	 */
	void shutdown();

	void registerReceiver(IFrameReceiver* receiver) const;
	void unregisterReceiver(IFrameReceiver* receiver) const;

	void setParamListeners(controller_t controller);
	void onParamChanged(Steinberg::Vst::ParamID paramId, float paramValue);

	void completionLoop();

private:
	/**
	 * @brief Sets all necessary callbacks for loader_ and frame_ticker_
	 */
	void setupCallbacks();

	/**
	 * @brief Event triggered when a video has been fully decoded and consumed.
	 */
	void onVideoFinished() const;

	std::atomic<CompletionSignal> completion_signal_{ CompletionSignal::None };
	std::condition_variable completion_cv_;
	std::thread completion_thread_;
	std::mutex completion_mutex_;

	std::atomic<bool> is_decoding_ = false; /// loader_ is active.
	std::atomic<bool> is_playing_ = false; /// frame_ticker_ is active.
	bool looping_ = true; /// Setting, whether decoding should restart onVideoFinished.
	double rate_ = 0.0; /// Rate of playback of the video pipeline.
	loader_t loader_; /// MediaLoader acting as the producer in the pipeline.
	frame_queue_t frame_queue_; /// Shared medium between producer and consumer.
	frame_ticker_t frame_ticker_; /// FrameTicker acting as the consumer in the pipeline.
	controller_t controller_ = nullptr; /// Main plugin EditController

	listener_t play_listener_; /// Listener for changes to kParamPlay
	listener_t reset_listener_; /// Listener for changes to kParamReset
	listener_t pause_listener_; /// Listener for changes to kParamPause
};

using pcont_t = std::shared_ptr<PlaybackController>;

#endif //PLAYBACKCONTROLLER_HPP
