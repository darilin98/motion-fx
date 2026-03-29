//
// Created by Darek Rudiš on 24.03.2026.
//

#ifndef WAVEPANNER_HPP
#define WAVEPANNER_HPP

#include <Filters/svf.h>
#include <Noise/whitenoise.h>
#include <Noise/dust.h>

#include "ieffect.hpp"

constexpr float kNoiseDecay = 0.99984f;
constexpr float kNoiseSensitivity = 5.0f;
constexpr float kNoiseMaxGain = 0.6f;

constexpr float kStrikeDecay = 0.9980f;
constexpr float kStrikeGain = 1.2f;
constexpr float kStrikeSensitivity = 11.0f;

constexpr float kSwishHighpassFreq = 3000.0f;  // airy swish character
constexpr float kSwishResonance = 0.3f;

class NoiseMaker : public IEffect {
public:
	void init(Steinberg::Vst::ProcessSetup setup) override;
	void process(float* buffer, int32_t numSamples, int32_t channel) override;
	void setBurst(float burst);

private:
	void channelResizeTo(size_t size);
	struct ChannelState {
		daisysp::Dust noise;
		daisysp::Svf filter;
	};
	std::vector<ChannelState> channels_;

	std::vector<float> envelope_curve_;
	std::vector<float> strike_curve_;

	float prev_burst_ = 0.0f;
	float envelope_ = 0.0f;
	float strike_envelope_ = 0.0f;

	Smoothed burst_;
	double sample_rate_ = kSampleRateDefault;
};

#endif //WAVEPANNER_HPP
