//
// Created by Darek Rudiš on 24.03.2026.
//

#ifndef COLORFEATUREEXTRACTOR_HPP
#define COLORFEATUREEXTRACTOR_HPP
#include "asyncframeworker.hpp"
#include "ifeatureextractor.hpp"
#include "video/iframereceiver.hpp"
#include "vst/vsttypes.h"

/**
 * @brief Extracts the RGB color composition out of a VideoFrame.
 *
 * Sums and normalizes color values independently. Exports them as three individual parameters.
 */
class ColorFeatureExtractor : public IFrameReceiver, public IFeatureExtractor {
public:
	explicit ColorFeatureExtractor(const Steinberg::Vst::ParamID red, const Steinberg::Vst::ParamID green, const Steinberg::Vst::ParamID blue)
		: red_id_(red), green_id_(green), blue_id_(blue) {
		frame_worker_ = std::make_unique<AsyncFrameWorker>(
			[this](const VideoFrame& f) { this->processFrame(f); }
		);
	}
	void onFrame(const VideoFrame& videoFrame) override;
private:
	FeatureResult extract(const VideoFrame& videoFrame) override;

	std::unique_ptr<AsyncFrameWorker> frame_worker_;
	Steinberg::Vst::ParamID red_id_;
	Steinberg::Vst::ParamID green_id_;
	Steinberg::Vst::ParamID blue_id_;
};



#endif //COLORFEATUREEXTRACTOR_HPP
