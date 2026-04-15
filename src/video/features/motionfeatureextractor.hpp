//
// Created by Darek Rudiš on 15.03.2026.
//

#ifndef MOTIONFEATUREEXTRACTOR_HPP
#define MOTIONFEATUREEXTRACTOR_HPP
#include "asyncframeworker.hpp"
#include "ifeatureextractor.hpp"
#include "video/iframereceiver.hpp"


/**
 * @brief Extracts types of motion detected between VideoFrames.
 *
 * Motion is represented as the difference in energy between two consecutive frames.
 * Motion average is a long-term value, resistant to small spikes in energy.
 * Motion burst calculates the delta of motion, boosts it but decays fast. This leads to dynamic value changes.
 */
class MotionFeatureExtractor : public IFrameReceiver, public IFeatureExtractor {
public:
	explicit MotionFeatureExtractor(const Steinberg::Vst::ParamID continuous_pid,
									const Steinberg::Vst::ParamID burst_pid)
		: continuous_param_id_(continuous_pid),burst_param_id_(burst_pid) {
		frame_worker_ = std::make_unique<AsyncFrameWorker>(
			[this](const VideoFrame& f) { this->processFrame(f); }
		);
	}
	void onFrame(const VideoFrame& videoFrame) override;

private:
	FeatureResult extract(const VideoFrame& videoFrame) override;

	std::vector<uint8_t> prev_frame_;

	float previous_raw_ = 0.0f;
	float previous_energy_ = 0.0f;
	float continuous_motion_ = 0.0f;
	float burst_motion_ = 0.0f;

	std::unique_ptr<AsyncFrameWorker> frame_worker_;
	Steinberg::Vst::ParamID continuous_param_id_;
	Steinberg::Vst::ParamID burst_param_id_;
};



#endif //MOTIONFEATUREEXTRACTOR_HPP
