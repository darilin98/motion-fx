//
// Created by Darek Rudiš on 25.02.2026.
//

/**
 * @file morphfilter.hpp
 * @brief Declares the MorphFilter effect.
 */

#ifndef MORPHFILTER_HPP
#define MORPHFILTER_HPP

#include <Filters/svf.h>

#include "ieffect.hpp"

constexpr float kCutoffFreq = 800.0f; /// Filter pivot frequency
constexpr float kResonance = 0.7f; /// Boost around cutoff frequency

using filter_t = daisysp::Svf; /// Type of filter used

/**
 * @class MorphFilter
 * @brief A low/high pass filter implementation.
 *
 * Changes what frequencies get cut based on the value of a parameter.
 */
class MorphFilter : public IEffect {
public:
	void init(Steinberg::Vst::ProcessSetup setup) override;
	void process(float* buffer, int32_t numSamples, int32_t channel) override;

	/**
	 * Changes low to highpass filter configuration.
	 * @param morph Normalized 0-1 param. 0 for pure low pass; 1 for pure high pass
	 */
	void setMorph(float morph);
private:
	/**
	 * @brief Adjusts effect, to comply to a new number of channels.
	 * @param size New number of channels
	 */
	void channelResizeTo(size_t size);
	struct ChannelState {
		filter_t filter;
	};
	std::vector<ChannelState> channels_; /// Filter for each individual channel.

	Smoothed morph_; /// Normalized filter setting parameter.
	double sample_rate_ = kSampleRateDefault; /// Effect's assumed sample rate
};



#endif //MORPHFILTER_HPP
