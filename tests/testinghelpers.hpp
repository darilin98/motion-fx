//
// Created by Darek Rudiš on 08.04.2026.
//

#ifndef TESTINGHELPERS_HPP
#define TESTINGHELPERS_HPP
#include <random>

#include "audio/effects/ieffect.hpp"
#include "vst/ivstaudioprocessor.h"

static Steinberg::Vst::ProcessSetup makeSetup(double sampleRate = 48000.0, int32_t maxBlock  = 256) {
	Steinberg::Vst::ProcessSetup setup {};
	setup.sampleRate = sampleRate;
	setup.maxSamplesPerBlock = maxBlock;
	return setup;
}

static std::vector<float> makeSilence(int n) {
	return std::vector(n, 0.f);
}

static std::vector<float> makeNoise(int n, float amplitude = 1.f, unsigned seed = 8829) {
	std::mt19937 rng(seed);
	std::uniform_real_distribution dist(-amplitude, amplitude);
	std::vector<float> buffer(n);
	
	for (auto& sample : buffer)
		sample = dist(rng);
	
	return buffer;
}

static std::vector<float> makeDC(int n, float value) {
	return std::vector(n, value);
}

static float rms(const std::vector<float>& buffer) {
	float sum = 0.f;
	for (float sample : buffer)
		sum += sample * sample;
	return std::sqrt(sum / buffer.size());
}

static float peak(const std::vector<float>& buffer) {
	float p = 0.f;
	for (float sample : buffer)
		p = std::max(p, std::abs(sample));
	return p;
}

static bool allZero(const std::vector<float>& buffer, float thresh = 1e-9f) {
	for (float sample : buffer)
		if (std::abs(sample) > thresh) return false;
	return true;
}

static bool withinRange(const std::vector<float>& buffer, float lo = -1.f, float hi = 1.f) {
	for (float sample : buffer)
		if (sample < lo || sample > hi) return false;
	return true;
}

static void processStereo(IEffect& fx, std::vector<float>& left, std::vector<float>& right) {
	fx.process(left.data(),  left.size(),  0);
	fx.process(right.data(), right.size(), 1);
}

#endif //TESTINGHELPERS_HPP
