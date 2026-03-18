//
// Created by Darek Rudiš on 17.03.2026.
//

#ifndef WAVEPHASER_HPP
#define WAVEPHASER_HPP
#include <Effects/phaser.h>

#include "ieffect.hpp"



constexpr float kContSmooth = 0.05f;
constexpr float kBurstSmooth = 0.1f;

constexpr float kMinLfo = 0.1f;
constexpr float kMaxLfo = 5.0f;

class WavePhaser : public IEffect{
public:
	void init(Steinberg::Vst::ProcessSetup setup) override;
	void process(float* buffer, int32_t numSamples, int32_t channel) override;
	void setContinuous(float value);
	void setBurst(float value);
private:
	void channelResizeTo(size_t size);

	struct ChannelState {
		daisysp::Phaser phaser;
		float feedback_lp = 0.0f;
	};
	std::vector<ChannelState> channels_;
	float continuous_target_ = 0;
	float continuous_smoothed_ = 0;

	float burst_target_ = 0;
	float burst_smoothed_ = 0;

	double sample_rate_ = 44100.0;
};



#endif //WAVEPHASER_HPP
