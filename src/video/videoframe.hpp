//
// Created by Darek Rudiš on 07.11.2025.
//
#pragma once

#ifndef VIDEOFRAME_HPP
#define VIDEOFRAME_HPP

#include <vector>

struct ImageData {
	size_t width = 0;
	size_t height = 0;
	std::vector<uint8_t> rgba_data;
};

struct VideoFrame {
	ImageData image;
	double timestamp = 0;
};

#endif //VIDEOFRAME_HPP
