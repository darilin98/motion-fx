//
// Created by Darek Rudiš on 25.02.2026.
//

#ifndef MORPHFILTER_HPP
#define MORPHFILTER_HPP

#include <Filters/svf.h>

#include "ieffect.hpp"

constexpr float kCutoffFreq = 800.0f;
constexpr float kResonance = 0.7f;

using filter_t = daisysp::Svf;

class MorphFilter : public IEffect {
public:
	void init(Steinberg::Vst::ProcessSetup setup) override;
	void process(float* buffer, int32_t numSamples, int32_t channel) override;
	void setMorph(float morph);
private:
	void channelResizeTo(size_t size);
	struct ChannelState {
		filter_t filter;
	};
	std::vector<ChannelState> channels_;

	Smoothed morph_;
	double sample_rate_ = kSampleRateDefault;
};



#endif //MORPHFILTER_HPP
