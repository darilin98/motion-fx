//
// Created by Darek Rudiš on 24.03.2026.
//

#ifndef SATURATIONEXCITER_HPP
#define SATURATIONEXCITER_HPP
#include "ieffect.hpp"
#include "daisysp.h"

constexpr float kExciterHighpassFreq = 3000.0f;
constexpr float kExciterResonance = 0.3f;
constexpr float kExciterDriveMax = 0.7f;

using filter_t = daisysp::Svf;
using overdrive_t = daisysp::Overdrive;

class SaturationExciter : public IEffect {
public:
	void init(Steinberg::Vst::ProcessSetup setup) override;
	void process(float* buffer, int32_t numSamples, int32_t channel) override;
	void setSaturation(float saturation);
private:
	void channelResizeTo(size_t size);
	struct ChannelState {
		filter_t filter;
		overdrive_t overdrive;
	};
	std::vector<ChannelState> channels_;

	Smoothed saturation_;
	double sample_rate_ = kSampleRateDefault;
};



#endif //SATURATIONEXCITER_HPP
