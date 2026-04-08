//
// Created by Darek Rudiš on 03.03.2026.
//

/**
 * @file spatialecho.hpp
 * @brief Declares the SpatialEcho effect.
 */

#ifndef SPATIALECHO_HPP
#define SPATIALECHO_HPP
#include <Utility/delayline.h>

#include "ieffect.hpp"

constexpr size_t kDelayMax = 192000; /// Limit on delay size in samples.

using delay_t = daisysp::DelayLine<float, kDelayMax>;

/**
 * @brief Produces a simple hall echo inspired delay.
 */
class SpatialEcho : public IEffect {
public:
	void init(Steinberg::Vst::ProcessSetup setup) override;
	void process(float* buffer, int32_t numSamples, int32_t channel) override;
	void setSpaceLevel(float spaceLevel);
private:
	/**
	 * @brief Adjusts effect, to comply to a new number of channels.
	 * @param size New number of channels
	 */
	void channelResizeTo(size_t size);
	struct ChannelState {
		delay_t delay;
		float feedback_lp = 0.0f;
	};
	std::vector<ChannelState> channels_; /// Delay buffers and amount of feedback separate for each channel.

	Smoothed space_level_; /// Amount of desired delay
	double sample_rate_ = kSampleRateDefault; /// Assumed sample rate in processing
};



#endif //SPATIALECHO_HPP
