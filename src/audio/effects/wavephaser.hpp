//
// Created by Darek Rudiš on 17.03.2026.
//

/**
 * @file wavephaser.hpp
 * @brief Declares the WavePhaser effect.
 */

#ifndef WAVEPHASER_HPP
#define WAVEPHASER_HPP
#include <Effects/phaser.h>

#include "ieffect.hpp"

constexpr float kContSmooth = 0.05f; /// Smoothing of long-term phasing
constexpr float kMinLfo = 0.1f; /// Minimum speed of phaser
constexpr float kMaxLfo = 5.0f; /// Maximum speed of phaser

using phaser_t = daisysp::Phaser;

class WavePhaser : public IEffect {
public:
	void init(Steinberg::Vst::ProcessSetup setup) override;
	void process(float* buffer, int32_t numSamples, int32_t channel) override;
	void setContinuous(float value);
private:
	/**
	 * @brief Adjusts effect, to comply to a new number of channels.
	 * @param size New number of channels
	 */
	void channelResizeTo(size_t size);

	struct ChannelState {
		phaser_t phaser;
		float last_lfo_freq = 0.0f;
	};
	std::vector<ChannelState> channels_; /// Phaser and its frequency separate for each channel.

	Smoothed continuous_; /// Speed of phaser oscillation
	double sample_rate_ = kSampleRateDefault; /// Assumed sample rate in processing
};



#endif //WAVEPHASER_HPP
