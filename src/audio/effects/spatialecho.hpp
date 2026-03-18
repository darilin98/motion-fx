//
// Created by Darek Rudiš on 03.03.2026.
//

#ifndef SPATIALECHO_HPP
#define SPATIALECHO_HPP
#include <Utility/delayline.h>

#include "ieffect.hpp"

constexpr size_t kDelayMax = 192000;

class SpatialEcho : public IEffect {
public:
	void init(Steinberg::Vst::ProcessSetup setup) override;
	void process(float* buffer, int32_t numSamples, int32_t channel) override;
	void setSpaceLevel(float spaceLevel);
private:
	void channelResizeTo(size_t size);
	struct ChannelState {
		daisysp::DelayLine<float, kDelayMax> delay;
		float feedback_lp = 0.0f;
	};

	std::vector<ChannelState> channels_;
	float space_level_target_ = 0;
	float space_level_smoothed_ = 0;
	double sample_rate_ = 44100.0;
};



#endif //SPATIALECHO_HPP
