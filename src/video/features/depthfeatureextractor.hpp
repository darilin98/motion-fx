//
// Created by Darek Rudiš on 03.03.2026.
//

#ifndef DEPTHFEATUREEXTRACTOR_HPP
#define DEPTHFEATUREEXTRACTOR_HPP

#include "asyncframeworker.hpp"
#include "ifeatureextractor.hpp"
#include "../iframereceiver.hpp"


class DepthFeatureExtractor : public IFrameReceiver, public IFeatureExtractor {
public:
	explicit DepthFeatureExtractor(const Steinberg::Vst::ParamID pid) : param_id_(pid) {
		frame_worker_ = std::make_unique<AsyncFrameWorker>(
			[this](const VideoFrame& f) { this->processFrame(f); }
		);
	}
	void onFrame(const VideoFrame& videoFrame) override;

private:
	FeatureResult extract(const VideoFrame& videoFrame) override;

	float calculateDetailRatio(const VideoFrame& videoFrame);

	double smoothed_depth_ = 0;
	std::atomic<bool> busy_{false};
	std::unique_ptr<AsyncFrameWorker> frame_worker_;
	Steinberg::Vst::ParamID param_id_;
};



#endif //DEPTHFEATUREEXTRACTOR_HPP
