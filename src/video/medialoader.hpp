//
// Created by Darek Rudiš on 07.11.2025.
//

#pragma once

#ifndef MEDIALOADER_HPP
#define MEDIALOADER_HPP

#include <functional>
#include <thread>
#include <utility>

#include "videoframe.hpp"

/**
 * @brief Contract for decoding any media file.
 */
class IMediaDecoder {
public:
	virtual ~IMediaDecoder() = default;

	/**
	 * @brief Moves decoder head to given time.
	 * @param time Time in milliseconds.
	 * @return Status value based on whether the decoder head is now moved to the desired timestamp.
	 */
	virtual bool trySeekTo(int64_t time) = 0;

	/**
	 * @brief Sets up the decoder to work on a media file.
	 * @param path Absolute path to a media file.
	 * @return Status value based on whether the decoder was able to open said file.
	 */
	virtual bool tryOpen(const std::string& path) = 0;

	/**
	 * @brief Decodes next frame of media in line based on the position.
	 * @param outFrame A single frame of media. @see VideoFrame
	 * @return True if frame was decoded, false if decoder reached end of media file or other complications.
	 */
	virtual bool tryDecodeNext(VideoFrame& outFrame) = 0;

	// TODO: possibly for reverse: virtual bool decodePrev(VideoFrame& outFrame) = 0;

	/**
	 * @return Framerate of the media file being decoded.
	 */
	[[nodiscard]] virtual double frameRate() const = 0;
};

using decoder_t = std::unique_ptr<IMediaDecoder>;

/**
 * @brief Manages an IMediaDecoder and pushes results with an event.
 *
 * @warning Cannot be reused on multiple media files. Must be re-instantiated.
 */
class MediaLoader {
public:
	explicit MediaLoader(std::string path)
		: path_(std::move(path)) {
		decoder_ = makeDecoder(path_);
		if (decoder_)
			decoder_->tryOpen(path_);
	}
	explicit MediaLoader(decoder_t decoder)
		: decoder_(std::move(decoder)) {}
	~MediaLoader();

	/**
	 * @brief Starts exporting frames with onFrame.
	 * Starts the job on a worker thread.
	 */
	void startLoading();

	/**
	 * @brief Schedules a stop of the producing process.
	 * Joins the worker thread.
	 */
	void stopLoading();

	/**
	 * @brief Resets the position of the internal decoder back to zero.
	 * @return True if media is now
	 * @warning Must be called only after decoding has finished.
	 */
	bool tryRewindToStart();

	std::function<void(VideoFrame&&)> onFrame;
	std::function<void()> onVideoFinish;
private:
	/**
	 * @brief Loading job that runs on the worker_
	 * Calls onFrame
	 */
	void workerLoop();

	/**
	 * @brief Creates a decoder object from a file path.
	 * @param path Absolute path to a media file.
	 * @return Decoder if successful, @warning nullptr if the decoder could not be constructed.
	 */
	static decoder_t makeDecoder(const std::string& path);
	decoder_t decoder_; /// Decoder used for getting VideoFrames.
	std::string path_; /// Path to decoder_'s media file.

	std::thread worker_; /// Worker for the loading job
	std::atomic_bool running_ {false}; /// State of the worker_
	std::atomic_bool requested_stop_ {false}; /// Schedules a stop of the worker_
};

using loader_t = std::unique_ptr<MediaLoader>;

#endif //MEDIALOADER_HPP
