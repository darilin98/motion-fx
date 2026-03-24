//
// Created by Darek Rudiš on 24.03.2026.
//

#ifndef SATURATIONFEATUREEXTRACTOR_HPP
#define SATURATIONFEATUREEXTRACTOR_HPP
#include "asyncframeworker.hpp"
#include "ifeatureextractor.hpp"
#include "video/iframereceiver.hpp"
#include "vst/vsttypes.h"


class SaturationFeatureExtractor : public IFrameReceiver, public IFeatureExtractor {
public:
	explicit SaturationFeatureExtractor(const Steinberg::Vst::ParamID pid) : param_id_(pid) {
		frame_worker_ = std::make_unique<AsyncFrameWorker>(
			[this](const VideoFrame& f) { this->processFrame(f); }
		);
	}
	void onFrame(const VideoFrame& videoFrame) override;
private:
	FeatureResult extract(const VideoFrame& videoFrame) override;

	std::unique_ptr<AsyncFrameWorker> frame_worker_;
	Steinberg::Vst::ParamID param_id_;
};



#endif //SATURATIONFEATUREEXTRACTOR_HPP
