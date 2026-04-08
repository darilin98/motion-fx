//
// Created by Darek Rudiš on 24.03.2026.
//


/**
 * @file ringmodulator.hpp
 * @brief Declares the RingModulator effect.
 */

#ifndef RINGMODULATOR_HPP
#define RINGMODULATOR_HPP
#include "ieffect.hpp"
#include "daisysp.h"
#include "parameterdefaults.hpp"

constexpr float kRatioR = 1.5f; /// R Oscillator interval - perfect fifth
constexpr float kRatioG = 1.25f; /// G Oscillator interval - major third
constexpr float kRatioB = 2.0f; /// B Oscillator interval - octave

using oscillator_t = daisysp::Oscillator;

/**
 * @brief Sums oscillators to the input signal.
 *
 * Results in the input signal shaped in the pitch of the oscillators used.
 */
class RingModulator : public IEffect {
public:
	void init(Steinberg::Vst::ProcessSetup setup) override;
	void process(float* buffer, int32_t numSamples, int32_t channel) override;

	void setR(float r);
	void setG(float g);
	void setB(float b);
	void setBase(float base);
private:
	/**
	 * @brief Adjusts effect, to comply to a new number of channels.
	 * @param size New number of channels
	 */
	void channelResizeTo(size_t size);
	struct ChannelState {
		oscillator_t osc_r;
		oscillator_t osc_g;
		oscillator_t osc_b;
	};
	std::vector<ChannelState> channels_; /// Pack of oscillators for each channel

	double base_freq_ = getHzFromNormalized(ParamDefaults::kColorFreq); /// Default base frequency
	bool freq_dirty_ = false; /// Signal to change base frequency
	Smoothed r_, g_, b_; /// Oscillator intensity amounts
	double sample_rate_ = kSampleRateDefault; /// Assumed sample rate in processing
};



#endif //RINGMODULATOR_HPP
