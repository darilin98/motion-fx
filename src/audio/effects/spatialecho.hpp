//
// Created by Darek Rudiš on 03.03.2026.
//

#ifndef SPATIALECHO_HPP
#define SPATIALECHO_HPP
#include <Utility/delayline.h>

#include "ieffect.hpp"

constexpr size_t kDelayMax = 192000;

using delay_t = daisysp::DelayLine<float, kDelayMax>;

class SpatialEcho : public IEffect {
public:
	void init(Steinberg::Vst::ProcessSetup setup) override;
	void process(float* buffer, int32_t numSamples, int32_t channel) override;
	void setSpaceLevel(float spaceLevel);
private:
	void channelResizeTo(size_t size);
	struct ChannelState {
		delay_t delay;
		float feedback_lp = 0.0f;
	};
	std::vector<ChannelState> channels_;

	Smoothed space_level_;
	double sample_rate_ = kSampleRateDefault;
};



#endif //SPATIALECHO_HPP
