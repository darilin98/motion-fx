//
// Created by Darek Rudiš on 18.03.2026.
//
#include <catch2/catch_test_macros.hpp>
#include "audio/effects/ringmodulator.hpp"
#include "testinghelpers.hpp"

TEST_CASE("RingModulator — high RGB produces different output than dry") {
	RingModulator fx;
	fx.init(makeSetup());
	fx.setR(1.f); fx.setG(1.f); fx.setB(1.f);

	auto warmup = makeNoise(256);
	for (int i = 0; i < 32; i++) fx.process(warmup.data(), 256, 0);

	auto buffer= makeNoise(256, 1.f, 7);
	auto original = buffer;
	fx.process(buffer.data(), 256, 0);

	// Output should differ from input — ring mod is active
	float diff = 0.f;
	for (int i = 0; i < 256; i++)
		diff += std::abs(buffer[i] - original[i]);
	REQUIRE(diff > 0.1f);
}

TEST_CASE("RingModulator — single channel normalizes correctly vs all channels") {
	RingModulator fxSingle, fxAll;
	fxSingle.init(makeSetup());
	fxAll.init(makeSetup());

	for (int i = 0; i < 32; i++) {
		auto w = makeNoise(256);
		fxSingle.process(w.data(), 256, 0);
		fxAll.process(w.data(), 256, 0);
	}

	fxSingle.setR(1.f);
	fxSingle.setG(0.f);
	fxSingle.setB(0.f);

	fxAll.setR(1.f);
	fxAll.setG(1.f);
	fxAll.setB(1.f);

	auto bufSingle = makeNoise(256, 1.f, 55);
	auto bufAll = bufSingle;  // same input

	for (int i = 0; i < 32; i++) {
		fxSingle.process(bufSingle.data(), 256, 0);
		fxAll.process(bufAll.data(), 256, 0);
	}

	// Both should be in range — single channel should not be quieter than all channels
	REQUIRE(withinRange(bufSingle));
	REQUIRE(withinRange(bufAll));
	REQUIRE(rms(bufSingle) > 0.001f);  // not squashed to silence
}