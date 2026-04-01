//
// Created by Darek Rudiš on 17.03.2026.
//

#ifndef WAVEPHASER_HPP
#define WAVEPHASER_HPP
#include <Effects/phaser.h>

#include "ieffect.hpp"

constexpr float kContSmooth = 0.05f;

constexpr float kMinLfo = 0.1f;
constexpr float kMaxLfo = 5.0f;

using phaser_t = daisysp::Phaser;

class WavePhaser : public IEffect {
public:
	void init(Steinberg::Vst::ProcessSetup setup) override;
	void process(float* buffer, int32_t numSamples, int32_t channel) override;
	void setContinuous(float value);
private:
	void channelResizeTo(size_t size);

	struct ChannelState {
		phaser_t phaser;
		float last_lfo_freq = 0.0f;
	};
	std::vector<ChannelState> channels_;

	Smoothed continuous_;
	double sample_rate_ = kSampleRateDefault;
};



#endif //WAVEPHASER_HPP
