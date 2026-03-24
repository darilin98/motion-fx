//
// Created by Darek Rudiš on 24.03.2026.
//

#ifndef RINGMODULATOR_HPP
#define RINGMODULATOR_HPP
#include "ieffect.hpp"
#include "daisysp.h"

constexpr float kRatioR = 1.5f; // perfect fifth
constexpr float kRatioG = 1.25f; // major third
constexpr float kRatioB = 2.0f; // octave

// TODO: Might be worth to expose this? Determines the chord
constexpr float kBaseFreq = 220.0f;

using oscillators_t = std::vector<daisysp::Oscillator>;
class RingModulator : public IEffect {
public:
	void init(Steinberg::Vst::ProcessSetup setup) override;
	void process(float* buffer, int32_t numSamples, int32_t channel) override;

	void setR(float r);
	void setG(float g);
	void setB(float b);
private:
	void channelResizeTo(size_t size);

	oscillators_t oscs_r_;
	oscillators_t oscs_g_;
	oscillators_t oscs_b_;

	double sample_rate_ = 44100.0;

	struct Smoothed { float target = 0.f; float value = 0.f; };
	Smoothed r_, g_, b_;

};



#endif //RINGMODULATOR_HPP
