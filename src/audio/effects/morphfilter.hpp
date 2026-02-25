//
// Created by Darek Rudiš on 25.02.2026.
//

#ifndef MORPHFILTER_HPP
#define MORPHFILTER_HPP

#include <Filters/svf.h>

#include "ieffect.hpp"

constexpr float pivotFreq = 1000.0f;
constexpr float res = 0.7f;

class MorphFilter : public IEffect {
public:
	void init(Steinberg::Vst::ProcessSetup setup) override;
	void process(float* buffer, int32_t numSamples) override;
	void setMorph(float morph);
private:
	daisysp::Svf filter_;
	float morph_target_ = 0.0f;
	float morph_smoothed_ = 0.0f;
	float smoothing_ = 0.001f;
};



#endif //MORPHFILTER_HPP
