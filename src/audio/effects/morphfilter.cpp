//
// Created by Darek Rudiš on 25.02.2026.
//

#include "morphfilter.hpp"

void MorphFilter::init(Steinberg::Vst::ProcessSetup setup) {
	filter_.Init(setup.sampleRate);
	filter_.SetFreq(pivotFreq);
	filter_.SetRes(res);
}

void MorphFilter::process(float* buffer, int32_t numSamples) {

	for (int32_t i = 0; i < numSamples; i++) {
		morph_smoothed_ += smoothing_ * (morph_target_ - morph_smoothed_);
		const float dry = buffer[i];
		filter_.Process(dry);

		float low_pass = filter_.Low();
		float high_pass = filter_.High();

		if (morph_smoothed_ < 0.5) {
			float crossfade = morph_smoothed_ * 2.0f;
			buffer[i] = (1.0f - crossfade) * low_pass + crossfade * dry;
		} else {
			float crossfade = (morph_smoothed_ - 0.5f) * 2.0f;
			buffer[i] = (1.0f - crossfade) * dry + crossfade * high_pass;
		}
	}
}

void MorphFilter::setMorph(float morph) {
    morph_target_ = std::clamp(morph, 0.0f, 1.0f);
}

