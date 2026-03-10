//
// Created by Darek Rudiš on 07.11.2025.
//

#include "medialoader.hpp"
#include "decoders/videodecoder.hpp"
#include "decoders/imagedecoder.hpp"

MediaLoader::~MediaLoader() {
	stopLoading();
}

decoder_t MediaLoader::makeDecoder(const std::string &path) {
	const std::string ext = path.substr(path.find_last_of('.') + 1);

	if (ext == "mp4" || ext == "mov" || ext == "avi" || ext == "gif") {
		return std::make_unique<VideoDecoder>();
	}
	if (ext == "jpg" || ext == "jpeg" || ext == "png") {
		return std::make_unique<ImageDecoder>();
	}
	return nullptr;
}

void MediaLoader::startLoading() {
	stopLoading();

	requested_stop_ = false;
	running_ = true;

	worker_ = std::thread([this] { workerLoop(); });
}

void MediaLoader::stopLoading() {
	requested_stop_ = true;

	if (worker_.joinable())
		worker_.join();

	running_ = false;
}

void MediaLoader::workerLoop() {
	if (!decoder_)
		return;

	while (!requested_stop_) {
		VideoFrame frame;
		if (!decoder_->tryDecodeNext(frame))
			break;

		if (onFrame)
			onFrame(std::move(frame));
	}

	running_ = false;

	if (!requested_stop_ && onVideoFinish)
		onVideoFinish();
}

bool MediaLoader::tryRewindToStart() {
	stopLoading();

	if (!decoder_)
		return false;

	return decoder_->trySeekTo(0);
}
