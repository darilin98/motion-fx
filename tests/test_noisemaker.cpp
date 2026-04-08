//
// Created by Darek Rudiš on 08.04.2026.
//
#include <catch2/catch_test_macros.hpp>
#include "audio/effects/noisemaker.hpp"
#include "testinghelpers.hpp"
#include "catch2/matchers/catch_matchers_vector.hpp"

TEST_CASE("NoiseMaker — no burst produces near silence") {
	NoiseMaker fx;
	fx.init(makeSetup());
	fx.setBurst(0.f);

	constexpr int N = 256;

	for (int i = 0; i < 64; i++) {
		auto buffer = makeSilence(N);
		fx.process(buffer.data(), N, 0);
	}

	auto buffer = makeSilence(N);
	fx.process(buffer.data(), N, 0);

	REQUIRE(peak(buffer) < 1e-4f);
}

TEST_CASE("NoiseMaker — burst adds audible noise") {
	NoiseMaker fx;
	fx.init(makeSetup());
	fx.setBurst(1.f);

	constexpr int N = 256;
	auto buffer = makeSilence(N);

	// Process immediately after burst
	fx.process(buffer.data(), N, 0);

	REQUIRE(rms(buffer) > 0.01f);
}

TEST_CASE("NoiseMaker — noise decays after burst") {
	NoiseMaker fx;
	fx.init(makeSetup());

	constexpr int N = 256;

	fx.setBurst(1.f);
	auto buffer = makeSilence(N);
	fx.process(buffer.data(), N, 0);
	float rmsImmediately = rms(buffer);

	// Decay phase
	fx.setBurst(0.f);
	for (int i = 0; i < 32; i++) {
		buffer = makeSilence(N);
		fx.process(buffer.data(), N, 0);
	}
	float rmsAfterDecay = rms(buffer);

	REQUIRE(rmsImmediately > rmsAfterDecay);
}
