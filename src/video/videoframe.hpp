//
// Created by Darek Rudiš on 07.11.2025.
//
#pragma once

#ifndef VIDEOFRAME_HPP
#define VIDEOFRAME_HPP

#include <vector>

/**
 * @brief Size and pixel values in rgba format for a single frame.
 */
struct ImageData {
	size_t width = 0;
	size_t height = 0;
	std::vector<uint8_t> rgba_data;
};

/**
 * @brief Core data type of the video pipeline.
 *
 * Holds image and time information for a single frame of video.
 */
struct VideoFrame {
	ImageData image;
	double timestamp = 0;
};

#endif //VIDEOFRAME_HPP
