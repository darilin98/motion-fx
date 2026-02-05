//
// Created by Darek Rudiš on 13.12.2025.
//

#ifndef BRIGHTNESS_FEATURE_EXTRACTOR_HPP
#define BRIGHTNESS_FEATURE_EXTRACTOR_HPP

#include "ifeatureextractor.hpp"
#include "asyncframeworker.hpp"
#include "../iframereceiver.hpp"
#include "utility/audiobuffers.h"
#include "../../audio/modulationcurve.hpp"

class BrightnessFeatureExtractor : public IFrameReceiver, public IFeatureExtractor {
public:
	explicit BrightnessFeatureExtractor(const Steinberg::Vst::ParamID pid) : param_id_(pid) {
		frame_worker_ = std::make_unique<AsyncFrameWorker>(
			[this](const VideoFrame& f) { this->processFrame(f); }
		);
	}
	void processFrame(const VideoFrame& videoFrame) override;
	void onFrame(const VideoFrame& videoFrame) override;
	void setOutController(const econt_t &controller) override;
private:
	void emitModulation(ModulationPoint point) override;
	std::atomic<bool> busy_{false};
	std::unique_ptr<AsyncFrameWorker> frame_worker_;
	Steinberg::Vst::ParamID param_id_;
	econt_t out_controller_;
};

#endif //BRIGHTNESS_FEATURE_EXTRACTOR_HPP
