//
// Created by Darek Rudiš on 24.03.2026.
//

#include "colorfeatureextractor.hpp"

FeatureResult ColorFeatureExtractor::extract(const VideoFrame& videoFrame) {
	const auto& data = videoFrame.image.rgba_data;
	const size_t numPixels = data.size() / 4;

	if (numPixels == 0)
		return FeatureResult{};

	uint64_t r_sum = 0;
	uint64_t g_sum = 0;
	uint64_t b_sum = 0;

	for (size_t i = 0; i < data.size(); i += 4) {
		r_sum += data[i];
		g_sum += data[i + 1];
		b_sum += data[i + 2];
	}

	float r = static_cast<float>(r_sum) / (numPixels * 255.0f);
	float g = static_cast<float>(g_sum) / (numPixels * 255.0f);
	float b = static_cast<float>(b_sum) / (numPixels * 255.0f);

	param_arr_t params = {
		FeatureParamUpdate{ red_id_, r },
		FeatureParamUpdate{ green_id_, g },
		FeatureParamUpdate{ blue_id_, b }
	};

	return { params, videoFrame.timestamp };
}

void ColorFeatureExtractor::onFrame(const VideoFrame& videoFrame) {
	frame_worker_->enqueueFrame(videoFrame);
}