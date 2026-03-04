//
// Created by Darek Rudiš on 25.02.2026.
//

#include "morphfilter.hpp"

void MorphFilter::setMorph(float morph) {
    morph_target_ = std::clamp(morph, 0.0f, 1.0f);
}

void MorphFilter::init(Steinberg::Vst::ProcessSetup setup) {
	sample_rate_ = setup.sampleRate;
	channelResizeTo(kChannelCountDefault);
}

void MorphFilter::channelResizeTo(size_t size) {
	filters_.resize(size);
	for (auto& filter : filters_) {
		filter.Init(sample_rate_);
		filter.SetFreq(kCutoffFreq);
		filter.SetRes(kResonance);
	}
}

void MorphFilter::process(float* buffer, int32_t numSamples, int32_t channel) {
	if (channel >= filters_.size()) {
		channelResizeTo(channel + 1);
	}

	auto& filter = filters_[channel];

	for (int32_t i = 0; i < numSamples; i++) {
		morph_smoothed_ += kSmoothing * (morph_target_ - morph_smoothed_);
		const float dry = buffer[i];

		filter.Process(dry);
		float low_pass  = filter.Low();
		float high_pass = filter.High();

		if(morph_smoothed_ < 0.5f) {
			float crossfade = morph_smoothed_ * 2.0f;
			float a = cosf(crossfade * 0.5f * M_PI);
			float b = sinf(crossfade * 0.5f * M_PI);
			buffer[i] = a * low_pass + b * dry;
		} else {
			float crossfade = (morph_smoothed_ - 0.5f) * 2.0f;
			float a = cosf(crossfade * 0.5f * M_PI);
			float b = sinf(crossfade * 0.5f * M_PI);
			buffer[i] = a * dry + b * high_pass;
		}
	}
}


