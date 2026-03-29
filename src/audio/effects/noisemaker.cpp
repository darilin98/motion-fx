//
// Created by Darek Rudiš on 24.03.2026.
//

#include "noisemaker.hpp"

void NoiseMaker::setBurst(float burst) {
	burst_.target = std::clamp(burst, 0.f, 1.f);
}

void NoiseMaker::init(Steinberg::Vst::ProcessSetup setup) {
	sample_rate_ = setup.sampleRate;
	envelope_curve_.resize(setup.maxSamplesPerBlock);
	strike_curve_.resize(setup.maxSamplesPerBlock);
	channelResizeTo(kChannelCountDefault);
}

void NoiseMaker::channelResizeTo(size_t size) {
	channels_.resize(size);
	for (auto& ch : channels_) {
		ch.noise.Init();
		ch.filter.Init(sample_rate_);
		ch.filter.SetFreq(kSwishHighpassFreq);
		ch.filter.SetRes(kSwishResonance);
	}
}

void NoiseMaker::process(float* buffer, int32_t numSamples, int32_t channel) {
	if (channel >= channels_.size()) {
		channelResizeTo(channel + 1);
	}

	if (channel == 0) {
		for (int32_t i = 0; i < numSamples; i++) {
			burst_.value += kSmoothing * (burst_.target - burst_.value);

			float burst_delta = burst_.value - prev_burst_;
			if (burst_delta > 0.f) {
				// background, slow for sustain
				envelope_ = std::min(envelope_ + burst_delta * kNoiseSensitivity,  1.f);

				// strike, fast decay
				strike_envelope_ = std::min(strike_envelope_ + burst_delta * kStrikeSensitivity, 1.f);
			}
			prev_burst_ = burst_.value;

			envelope_ *= kNoiseDecay;
			strike_envelope_ *= kStrikeDecay;

			envelope_curve_[i] = envelope_;
			strike_curve_[i] = strike_envelope_;
		}
	}

	auto& ch = channels_[channel];

	for (int32_t i = 0; i < numSamples; i++) {
		float raw = ch.noise.Process();

		// highpass only
		ch.filter.Process(raw);
		float swish = ch.filter.High();

		float env = envelope_curve_[i];
		float strike = strike_curve_[i];

		// tune background vs momentary strike ratios
		buffer[i] += swish * (env * kNoiseMaxGain + powf(strike, 2.0f) * kStrikeGain);
	}
}