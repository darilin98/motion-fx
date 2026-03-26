//
// Created by Darek Rudiš on 24.03.2026.
//

#ifndef WAVEPANNER_HPP
#define WAVEPANNER_HPP
#include "ieffect.hpp"

constexpr float kFlutterFreqMax = 17.0f;
constexpr float kFlutterFreqMin = 1.0f;
constexpr float kFlutterDecay = 0.998f;
constexpr float kFlutterSensitivity = 7.0f;

class WavePanner : public IEffect {
public:
	void init(Steinberg::Vst::ProcessSetup setup) override;
	void process(float* buffer, int32_t numSamples, int32_t channel) override;
	void setBurst(float burst);

private:
	float prev_burst_ = 0.0f;
	float envelope_ = 0.0f;
	float lfo_phase_ = 0.0f;

	std::vector<float> cache_l_;
	bool cache_l_filled_ = false;

	Smoothed burst_;
	double sample_rate_ = kSampleRateDefault;
};

#endif //WAVEPANNER_HPP
