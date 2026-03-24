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

class SaturationExciter : public IEffect {
public:
	void init(Steinberg::Vst::ProcessSetup setup) override;
	void process(float* buffer, int32_t numSamples, int32_t channel) override;
	void setSaturation(float saturation);
private:
	void channelResizeTo(size_t size);

	std::vector<daisysp::Svf> filters_;
	std::vector<daisysp::Overdrive> overdrives_;

	double sample_rate_ = 44100.0;

	struct Smoothed { float target = 0.f; float value = 0.f; };
	Smoothed saturation_;
};



#endif //SATURATIONEXCITER_HPP
