//
// Created by Darek Rudiš on 07.11.2025.
//

#include "medialoader.hpp"

void MediaLoader::loadMedia(const std::string& path) {
	return;
}

decoder_t MediaLoader::makeDecoder(const std::string &path) {
	const std::string ext = path.substr(path.find_last_of('.') + 1);

	if (ext == "mp4" || ext == "mov" || ext == "avi" || ext == "gif") {
		return nullptr; // TODO: Decoder implementations
	}
	if (ext == "jpg" || ext == "jpeg" || ext == "png") {
		return nullptr;
	}
	return nullptr;
}
