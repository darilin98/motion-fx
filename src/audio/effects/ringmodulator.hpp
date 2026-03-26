//
// Created by Darek Rudiš on 24.03.2026.
//

#ifndef RINGMODULATOR_HPP
#define RINGMODULATOR_HPP
#include "ieffect.hpp"
#include "daisysp.h"
#include "parameterdefaults.hpp"

constexpr float kRatioR = 1.5f; // perfect fifth
constexpr float kRatioG = 1.25f; // major third
constexpr float kRatioB = 2.0f; // octave

using oscillator_t = daisysp::Oscillator;

class RingModulator : public IEffect {
public:
	void init(Steinberg::Vst::ProcessSetup setup) override;
	void process(float* buffer, int32_t numSamples, int32_t channel) override;

	void setR(float r);
	void setG(float g);
	void setB(float b);
	void setBase(float base);
private:
	void channelResizeTo(size_t size);
	struct ChannelState {
		oscillator_t osc_r;
		oscillator_t osc_g;
		oscillator_t osc_b;
	};
	std::vector<ChannelState> channels_;

	double base_freq_ = getHzFromNormalized(ParamDefaults::kColorFreq);
	bool freq_dirty_ = false;
	Smoothed r_, g_, b_;
	double sample_rate_ = kSampleRateDefault;
};



#endif //RINGMODULATOR_HPP
