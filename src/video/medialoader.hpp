//
// Created by Darek Rudiš on 07.11.2025.
//

#pragma once

#ifndef MEDIALOADER_HPP
#define MEDIALOADER_HPP

#include <functional>
#include "videoframe.hpp"

class IMediaDecoder {
public:
	virtual ~IMediaDecoder() = default;
	virtual bool seekTo(int64_t time) = 0;
	virtual bool open(const std::string& path) = 0;
	virtual bool decodeNext(VideoFrame& outFrame) = 0;
	// TODO: possibly for reverse: virtual bool decodePrev(VideoFrame& outFrame) = 0;
	[[nodiscard]] virtual double frameRate() const = 0;
	[[nodiscard]] virtual bool isStillImage() const = 0;
};

using decoder_t = std::shared_ptr<IMediaDecoder>;

class MediaLoader {
public:
	void loadMedia(const std::string& path);
	std::function<void(VideoFrame&&)> onFrame;
private:
	static decoder_t makeDecoder(const std::string& path);
	decoder_t decoder_;
};
#endif //MEDIALOADER_HPP
