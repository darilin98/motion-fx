//
// Created by Darek Rudiš on 13.12.2025.
//

#include "brightnessfeatureextractor.hpp"
#include <thread>

FeatureResult BrightnessFeatureExtractor::extract(const VideoFrame& videoFrame) {
	if (videoFrame.image.rgba_data.empty())
		return FeatureResult{};

	const auto& data = videoFrame.image.rgba_data;
	const size_t numPixels = data.size() / 4;

	if (numPixels == 0) return FeatureResult{};

	// Sum R,G,B channels
	uint64_t total = 0;
	for (size_t i = 0; i < data.size(); i += 4) {
		total += static_cast<uint64_t>(data[i]);     // R
		total += static_cast<uint64_t>(data[i + 1]); // G
		total += static_cast<uint64_t>(data[i + 2]); // B
	}

	const double normalized = static_cast<double>(total) / (numPixels * 3 * 255);

	auto param = FeatureParamUpdate { param_id_, normalized};
	param_arr_t parameters = { param };

	return {parameters, videoFrame.timestamp};
}

void BrightnessFeatureExtractor::onFrame(const VideoFrame& videoFrame) {
	frame_worker_->enqueueFrame(videoFrame);
}


