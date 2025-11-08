//
// Created by Darek Rudiš on 08.11.2025.
//
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "imagedecoder.hpp"

bool ImageDecoder::open(const std::string &path) {
	int width, height, channels;

	stbi_uc* stbi_data = stbi_load(path.c_str(), &width, &height, &channels, 4);
	if (!stbi_data) {
		return false;
	}

	cached_frame_.image.width = width;
	cached_frame_.image.height = height;
	cached_frame_.image.rgba_data.assign(
		stbi_data,
		stbi_data + width * height * 4
	);
	cached_frame_.timestamp = 0.0;
	stbi_image_free(stbi_data);
	loaded_ = true;
	return true;
}

bool ImageDecoder::seekTo(int64_t time) {
	// Does not make sense to call on image decoder
	return loaded_;
}

bool ImageDecoder::decodeNext(VideoFrame &outFrame) {
	if (!loaded_) {
		return false;
	}
	outFrame = cached_frame_;
	return true;
}


