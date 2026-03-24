//
// Created by Darek Rudiš on 24.03.2026.
//

#include "saturationexciter.hpp"

void SaturationExciter::setSaturation(float saturation) {
	saturation_.target = std::clamp(saturation, 0.f, 1.f);
}

void SaturationExciter::init(Steinberg::Vst::ProcessSetup setup) {
	sample_rate_ = setup.sampleRate;
	channelResizeTo(kChannelCountDefault);
}

void SaturationExciter::channelResizeTo(size_t size) {
	channels_.resize(size);
	for (auto& ch : channels_) {
		ch.filter.Init(sample_rate_);
		ch.filter.SetFreq(kExciterHighpassFreq);
		ch.filter.SetRes(kExciterResonance);

		ch.overdrive.Init();
	}
}

void SaturationExciter::process(float* buffer, int32_t numSamples, int32_t channel) {
	if (channel >= channels_.size()) {
		channelResizeTo(channel + 1);
	}

	auto& filter = channels_[channel].filter;
	auto& overdrive = channels_[channel].overdrive;

	for (int32_t i = 0; i < numSamples; i++) {
		saturation_.value += kSmoothing * (saturation_.target - saturation_.value);

		const float dry = buffer[i];

		// Select only high freqs
		filter.Process(dry);
		float high = filter.High();

		// Saturate them
		overdrive.SetDrive(saturation_.value * kExciterDriveMax);
		float excited = overdrive.Process(high);

		// reconstruct
		float wet_amount = sinf(saturation_.value * 0.5f * M_PI);
		buffer[i] = dry + excited * wet_amount;
	}
}