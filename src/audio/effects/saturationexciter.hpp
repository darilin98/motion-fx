//
// Created by Darek Rudiš on 24.03.2026.
//

/**
 * @file saturationexciter.hpp
 * @brief Declares the SaturationExciter effect.
 */

/**
 * @file saturationexciter.hpp
 * @brief Declares the SaturationExciter effect.
 */

#ifndef SATURATIONEXCITER_HPP
#define SATURATIONEXCITER_HPP
#include "ieffect.hpp"
#include "daisysp.h"

constexpr float kExciterHighpassFreq = 3000.0f; /// Declares high pass region
constexpr float kExciterResonance = 0.3f; /// Boost around high pass pivot
constexpr float kExciterDriveMax = 0.5f; /// Overdrive intensity

using filter_t = daisysp::Svf;
using overdrive_t = daisysp::Overdrive;

/**
 * @brief Adds an overdrive boost in the high frequency range.
 *
 * Results in a bright tone without distorting the core sound of the lower frequencies.
 */
class SaturationExciter : public IEffect {
public:
	void init(Steinberg::Vst::ProcessSetup setup) override;
	void process(float* buffer, int32_t numSamples, int32_t channel) override;
	void setSaturation(float saturation);
private:
	/**
	 * @brief Adjusts effect, to comply to a new number of channels.
	 * @param size New number of channels
	 */
	void channelResizeTo(size_t size);
	struct ChannelState {
		filter_t filter;
		overdrive_t overdrive;
	};
	std::vector<ChannelState> channels_; /// Filter and overdrive for each channel.

	Smoothed saturation_; /// Intensity of overdrive applied.
	double sample_rate_ = kSampleRateDefault; /// Assumed sample rate in processing
};



#endif //SATURATIONEXCITER_HPP
