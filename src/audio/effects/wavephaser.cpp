//
// Created by Darek Rudiš on 17.03.2026.
//

#include "wavephaser.hpp"

void WavePhaser::setContinuous(float value) {
	continuous_target_ = std::clamp(value, 0.0f, 1.0f);
}

void WavePhaser::setBurst(float value) {
	burst_target_ = std::clamp(value, 0.0f, 1.0f);
}

void WavePhaser::init(Steinberg::Vst::ProcessSetup setup) {
	sample_rate_ = setup.sampleRate;
	channelResizeTo(kChannelCountDefault);
}

void WavePhaser::channelResizeTo(size_t size) {
	channels_.resize(size);
	for (auto& c : channels_) {
		c.phaser.Init(sample_rate_);
		c.phaser.SetPoles(4);
		c.phaser.SetFreq(1000.0f);
		c.phaser.SetLfoDepth(0.0f);
		c.phaser.SetLfoFreq(0.1f);
		c.phaser.SetFeedback(0.0f);
	}
}

void WavePhaser::process(float* buffer, int32_t numSamples, int32_t channel) {
	if (channel >= channels_.size()) {
		channelResizeTo(channel + 1);
	}

	auto& ph = channels_[channel].phaser;


	const float lfoFreq  = kMinLfo + continuous_smoothed_ * (kMaxLfo - kMinLfo);
	const float lfoDepth = std::clamp(continuous_smoothed_ + burst_smoothed_, 0.0f, 1.0f);
	const float feedback = burst_smoothed_ * 0.25f;

	ph.SetLfoFreq(lfoFreq);
	ph.SetLfoDepth(lfoDepth);
	ph.SetFeedback(feedback);

	for (int32_t i = 0; i < numSamples; i++) {
		continuous_smoothed_ += kContSmooth * (continuous_target_ - continuous_smoothed_);
		burst_smoothed_ += kBurstSmooth * (burst_target_ - burst_smoothed_);

		float dry = buffer[i];
		float wet = ph.Process(dry);
		buffer[i] = dry * (1.0f - lfoDepth) + wet * lfoDepth;
	}
}