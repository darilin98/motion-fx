//
// Created by Darek Rudiš on 24.03.2026.
//

#include "saturationfeatureextractor.hpp"

FeatureResult SaturationFeatureExtractor::extract(const VideoFrame& videoFrame) {
	if (videoFrame.image.rgba_data.empty())
		return FeatureResult{};

	const auto& data = videoFrame.image.rgba_data;
	const size_t numPixels = data.size() / 4;

	if (numPixels == 0)
		return FeatureResult{};

	double saturation_sum = 0.0;

	for (size_t i = 0; i < data.size(); i += 4)
	{
		float r = (data[i]);
		float g = (data[i + 1]);
		float b = (data[i + 2]);

		float maxc = std::max({r, g, b});
		float minc = std::min({r, g, b});

		float sat = 0.0f;

		if (maxc > 0.0f)
			sat = (maxc - minc) / maxc;

		saturation_sum += sat;
	}

	float avg_saturation = static_cast<float>(saturation_sum / numPixels);

	avg_saturation = std::clamp(avg_saturation, 0.0f, 1.0f);

	param_arr_t params = {
		FeatureParamUpdate{ param_id_, avg_saturation }
	};

	return { params, videoFrame.timestamp };
}

void SaturationFeatureExtractor::onFrame(const VideoFrame& videoFrame) {
	frame_worker_->enqueueFrame(videoFrame);
}