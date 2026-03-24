//
// Created by Darek Rudiš on 25.02.2026.
//

#include "morphfilter.hpp"

void MorphFilter::setMorph(float morph) {
    morph_.target = std::clamp(morph, 0.0f, 1.0f);
}

void MorphFilter::init(Steinberg::Vst::ProcessSetup setup) {
	sample_rate_ = setup.sampleRate;
	channelResizeTo(kChannelCountDefault);
}

void MorphFilter::channelResizeTo(size_t size) {
	channels_.resize(size);
	for (auto& ch : channels_) {
		ch.filter.Init(sample_rate_);
		ch.filter.SetFreq(kCutoffFreq);
		ch.filter.SetRes(kResonance);
	}
}

void MorphFilter::process(float* buffer, int32_t numSamples, int32_t channel) {
	if (channel >= channels_.size()) {
		channelResizeTo(channel + 1);
	}

	auto& filter = channels_[channel].filter;

	for (int32_t i = 0; i < numSamples; i++) {
		morph_.value += kSmoothing * (morph_.target - morph_.value);
		const float dry = buffer[i];

		filter.Process(dry);
		float low_pass  = filter.Low();
		float high_pass = filter.High();

		if(morph_.value < 0.5f) {
			float crossfade = morph_.value * 2.0f;
			float a = cosf(crossfade * 0.5f * M_PI);
			float b = sinf(crossfade * 0.5f * M_PI);
			buffer[i] = a * low_pass + b * dry;
		} else {
			float crossfade = (morph_.value - 0.5f) * 2.0f;
			float a = cosf(crossfade * 0.5f * M_PI);
			float b = sinf(crossfade * 0.5f * M_PI);
			buffer[i] = a * dry + b * high_pass;
		}
	}
}


