//
// Created by Darek Rudiš on 03.03.2026.
//

#include "spatialecho.hpp"

void SpatialEcho::setSpaceLevel(float spaceLevel) {
	space_level_.target = std::clamp(spaceLevel, 0.0f, 1.0f);
}

void SpatialEcho::init(Steinberg::Vst::ProcessSetup setup) {
	sample_rate_ = setup.sampleRate;
	channelResizeTo(kChannelCountDefault);
}

void SpatialEcho::channelResizeTo(size_t size) {
	channels_.resize(size);
	for (auto &c : channels_) {
		c.delay.Init();
		c.delay.SetDelay((0.05f) * static_cast<float>(sample_rate_));
	}
}

void SpatialEcho::process(float* buffer, int32_t numSamples, int32_t channel) {
	if (channel >= channels_.size()) {
		channelResizeTo(channel + 1);
	}

	auto& state = channels_[channel];

	space_level_.value += kSmoothing * (space_level_.target - space_level_.value);

	float delay_time = 0.02f + 0.08f * space_level_.value; // Delay range: 20–100 ms
	float delay_samples = delay_time * sample_rate_;
	state.delay.SetDelay(delay_samples);

	float feedback = 0.5f * space_level_.value;
	float wet_amount = space_level_.value;
	float dry_amount = 1.0f - wet_amount;

	for (int32_t i = 0; i < numSamples; ++i) {
		float in = buffer[i];

		float d1 = state.delay.Read();
		float d2 = state.delay.Read(delay_samples * 0.6f); 

		state.feedback_lp += 0.3f * (d1 - state.feedback_lp);

		state.delay.Write(in + feedback * state.feedback_lp);

		float reverb = 0.6f * d1 + 0.4f * d2;

		buffer[i] = dry_amount * in + wet_amount * reverb;
	}
}



