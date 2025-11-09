//
// Created by Darek Rudiš on 07.11.2025.
//

#include "medialoader.hpp"
#include "decoders/videodecoder.hpp"
#include "decoders/imagedecoder.hpp"

MediaLoader::MediaLoader(const std::string& path) {
	decoder_ = makeDecoder(path);
	if (decoder_)
		decoder_->open(path); // Should be a task
}

decoder_t MediaLoader::makeDecoder(const std::string &path) {
	const std::string ext = path.substr(path.find_last_of('.') + 1);

	if (ext == "mp4" || ext == "mov" || ext == "avi" || ext == "gif") {
		return nullptr;
		//return std::make_shared<VideoDecoder>();
	}
	if (ext == "jpg" || ext == "jpeg" || ext == "png") {
		return std::make_shared<ImageDecoder>();
	}
	return nullptr;
}

bool MediaLoader::requestNextFrame() const {
	if (!decoder_)
		return false;
	if (VideoFrame frame; decoder_->decodeNext(frame)) {
		if (onFrame)
			onFrame(std::move(frame));
		return true;
	}
	return false;
}
