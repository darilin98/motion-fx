//
// Created by Darek Rudiš on 25.02.2026.
//

#ifndef MORPHFILTER_HPP
#define MORPHFILTER_HPP

#include <Filters/svf.h>

#include "ieffect.hpp"

constexpr float cutoffFreq = 800.0f;
constexpr float res = 0.7f;
constexpr float filterCountDefault = 2;

using filters_t = std::vector<daisysp::Svf>;

class MorphFilter : public IEffect {
public:
	void init(Steinberg::Vst::ProcessSetup setup) override;
	void process(float* buffer, int32_t numSamples, int32_t channel) override;
	void setMorph(float morph);
private:
	filters_t filters_;
	double sample_rate_ = 44100.0;
	float morph_target_ = 0.0f;
	float morph_smoothed_ = 0.0f;
	float smoothing_ = 0.001f;
};



#endif //MORPHFILTER_HPP
