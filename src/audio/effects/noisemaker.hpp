//
// Created by Darek Rudiš on 24.03.2026.
//

/**
 * @file noisemaker.hpp
 * @brief Declares the NoiseMaker effect.
 */

#ifndef WAVEPANNER_HPP
#define WAVEPANNER_HPP

#include <Filters/svf.h>
#include <Noise/whitenoise.h>
#include <Noise/dust.h>

#include "ieffect.hpp"

constexpr float kNoiseDecay = 0.99984f; /// Long-term noise longevity
constexpr float kNoiseSensitivity = 5.0f; /// Sensitivity of long-term noise
constexpr float kNoiseMaxGain = 0.6f; /// Limit gain of long-term noise

constexpr float kStrikeDecay = 0.9980f; /// Sudden noise burst longevity
constexpr float kStrikeGain = 1.2f; /// Limit gain of sudden burst noise
constexpr float kStrikeSensitivity = 11.0f; /// Sensitivity of sudden change impacts

constexpr float kSwishHighpassFreq = 3000.0f; /// Sets amount of airy swish
constexpr float kSwishResonance = 0.3f; /// Boosts swish

/**
 * @class NoiseMaker
 * @brief Creates layers of noise.
 *
 * Mixes in layers of white noise inside the input signal.
 * Reacts to sudden changes of the input parameter.
 */
class NoiseMaker : public IEffect {
public:
	void init(Steinberg::Vst::ProcessSetup setup) override;
	void process(float* buffer, int32_t numSamples, int32_t channel) override;

	/**
	 * @brief Sets noise amount parameter.
	 * @param burst Normalized 0-1 parameter
	 */
	void setBurst(float burst);

private:
	/**
	 * @brief Adjusts effect, to comply to a new number of channels.
	 * @param size New number of channels
	 */
	void channelResizeTo(size_t size);
	struct ChannelState {
		daisysp::Dust noise; /// Source of white noise
		daisysp::Svf filter; /// Filter for noise adjustment
	};
	std::vector<ChannelState> channels_; /// Filters and noise sources for each channel.

	std::vector<float> envelope_curve_; /// Stores long-term noise calculations from channel 0, lets other channels copy.
	std::vector<float> strike_curve_; /// Stores sudden noise calculations from channel 0, lets other channels copy.

	float prev_burst_ = 0.0f; /// Last param value for determining rate of change
	float envelope_ = 0.0f; /// Gate for long-term noise
	float strike_envelope_ = 0.0f; /// Gate for sudden noise

	Smoothed burst_; /// Amount of noise value
	double sample_rate_ = kSampleRateDefault; /// Assumed sample rate inside calculations
};

#endif //WAVEPANNER_HPP
