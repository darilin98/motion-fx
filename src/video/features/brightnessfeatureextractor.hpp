//
// Created by Darek Rudiš on 13.12.2025.
//

#ifndef BRIGHTNESS_FEATURE_EXTRACTOR_HPP
#define BRIGHTNESS_FEATURE_EXTRACTOR_HPP

#include "ifeatureextractor.hpp"
#include "asyncframeworker.hpp"
#include "vstparameters.h"
#include "../iframereceiver.hpp"

class BrightnessFeatureExtractor : public IFrameReceiver, public IFeatureExtractor {
public:
	explicit BrightnessFeatureExtractor(const Steinberg::Vst::ParamID pid) : param_id_(pid) {
		frame_worker_ = std::make_unique<AsyncFrameWorker>(
			[this](const VideoFrame& f) { this->processFrame(f); }
		);
	}
	void onFrame(const VideoFrame& videoFrame) override;
private:
	FeatureResult extract(const VideoFrame& videoFrame) override;

	std::atomic<bool> busy_{false};
	std::unique_ptr<AsyncFrameWorker> frame_worker_;
	Steinberg::Vst::ParamID param_id_;
};

#endif //BRIGHTNESS_FEATURE_EXTRACTOR_HPP
