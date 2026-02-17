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

class IMediaDecoder {
public:
	virtual ~IMediaDecoder() = default;
	virtual bool seekTo(int64_t time) = 0;
	virtual bool open(const std::string& path) = 0;
	virtual bool decodeNext(VideoFrame& outFrame) = 0;
	// TODO: possibly for reverse: virtual bool decodePrev(VideoFrame& outFrame) = 0;
	[[nodiscard]] virtual double frameRate() const = 0;
};

using decoder_t = std::unique_ptr<IMediaDecoder>;

class MediaLoader {
public:
	explicit MediaLoader(std::string path)
		: path_(std::move(path)) {}
	explicit MediaLoader(decoder_t decoder)
		: decoder_(std::move(decoder)) {}
	~MediaLoader();

	void startLoading();
	void stopLoading();
	bool tryRewindToStart();

	std::function<void(VideoFrame&&)> onFrame;
	std::function<void()> onVideoFinish;
private:
	void workerLoop();
	static decoder_t makeDecoder(const std::string& path);
	decoder_t decoder_;
	std::string path_;

	std::thread worker_;
	std::atomic_bool running_ {false};
	std::atomic_bool requested_stop_ {false};

	std::mutex seek_lock_;
};

using loader_t = std::unique_ptr<MediaLoader>;

#endif //MEDIALOADER_HPP
