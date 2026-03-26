//
// Created by Darek Rudiš on 17.03.2026.
//

#include "wavephaser.hpp"

void WavePhaser::setContinuous(float value) {
	continuous_.target = std::clamp(value, 0.0f, 1.0f);
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
		c.phaser.SetLfoDepth(0.3f);
		c.phaser.SetLfoFreq(0.1f);
		c.phaser.SetFeedback(0.0f);
	}
}

void WavePhaser::process(float* buffer, int32_t numSamples, int32_t channel) {
	if (channel >= channels_.size()) {
		channelResizeTo(channel + 1);
	}

	auto& ph = channels_[channel].phaser;

	auto softClip = [](float x) -> float {
		if (x > 1.0f) return 2.0f/3.0f;
		if (x < -1.0f) return -2.0f/3.0f;
		return x - (x*x*x)/3.0f;
	};

	for (int32_t i = 0; i < numSamples; i++) {
		continuous_.value += kContSmooth * (continuous_.target - continuous_.value);

		const float lfoFreq  = kMinLfo + continuous_.value * (kMaxLfo - kMinLfo);
		const float phaserMix = std::clamp(continuous_.value, 0.0f, 1.0f);

		ph.SetLfoFreq(lfoFreq);

		float dry = buffer[i];
		float wet = ph.Process(dry);
		buffer[i] = softClip(dry * (1.0f - phaserMix) + wet * phaserMix);
	}
}