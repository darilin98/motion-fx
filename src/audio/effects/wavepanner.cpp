//
// Created by Darek Rudiš on 24.03.2026.
//

#include "wavepanner.hpp"

void WavePanner::setBurst(float burst) {
	burst_.target = std::clamp(burst, 0.f, 1.f);
}

void WavePanner::init(Steinberg::Vst::ProcessSetup setup) {
	sample_rate_ = setup.sampleRate;
	cache_l_.resize(setup.maxSamplesPerBlock, 0.0f);
	cache_l_filled_ = false;
}

void WavePanner::process(float* buffer, int32_t numSamples, int32_t channel) {
	if (channel > 1) return;

	if (channel == 0) {
		for (int32_t i = 0; i < numSamples; i++) {
			float incoming = buffer[i];
			if (cache_l_filled_) {
				buffer[i] = cache_l_[i];
			}
			cache_l_[i] = incoming;
			cache_l_filled_ = true;
		}
		return;
	}

	const float dt = 1.f / static_cast<float>(sample_rate_);

	for (int32_t i = 0; i < numSamples; i++) {
		burst_.value += kSmoothing * (burst_.target - burst_.value);

		float burst_delta = burst_.value - prev_burst_;
		if (burst_delta > 0.f) {
			envelope_ = std::min(envelope_ + burst_delta * kFlutterSensitivity, 1.f);
		}
		prev_burst_ = burst_.value;

		envelope_ *= kFlutterDecay;

		float freq  = kFlutterFreqMin + envelope_ * (kFlutterFreqMax - kFlutterFreqMin);
		lfo_phase_ += freq * dt;
		if (lfo_phase_ > 1.f) lfo_phase_ -= 1.f;

		float lfo = sinf(lfo_phase_ * 2.f * M_PI) * envelope_;
		float angle = (lfo + 1.f) * 0.5f * 0.5f * M_PI;
		float gain_l = cosf(angle);
		float gain_r = sinf(angle);

		float left_ch = cache_l_[i];
		float right_ch = buffer[i];

		cache_l_[i] = left_ch * gain_l + right_ch * (1.f - gain_r);  // processed left, written next ch0 call
		buffer[i] = right_ch * gain_r + left_ch * (1.f - gain_l);  // processed right, written immediately
	}
}