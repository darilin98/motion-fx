//
// Created by Darek Rudiš on 08.04.2026.
//

#include <catch2/catch_test_macros.hpp>
#include "audio/effects/saturationexciter.hpp"
#include "testinghelpers.hpp"
#include "catch2/matchers/catch_matchers_vector.hpp"

TEST_CASE("SaturationExciter — saturation=0 passes signal unchanged") {
	SaturationExciter fx;
	fx.init(makeSetup());
	fx.setSaturation(0.f);

	auto warmup = makeNoise(256);
	for (int i = 0; i < 16; i++) fx.process(warmup.data(), 256, 0);

	auto buffer= makeNoise(256, 0.5f, 11);
	auto expected = buffer;
	fx.process(buffer.data(), 256, 0);

	for (int i = 0; i < 256; i++)
		REQUIRE(buffer[i] == Catch::Approx(expected[i]).margin(1e-4f));
}

TEST_CASE("SaturationExciter — high saturation increases high frequency energy") {
	SaturationExciter fxDry, fxWet;
	fxDry.init(makeSetup());
	fxWet.init(makeSetup());
	fxDry.setSaturation(0.f);
	fxWet.setSaturation(1.f);

	constexpr int N = 1024;

	for (int i = 0; i < 16; i++) {
		auto w = makeNoise(N);
		fxDry.process(w.data(), N, 0);
		fxWet.process(w.data(), N, 0);
	}

	std::vector<float> bufDry(N), bufWet(N);
	for (int i = 0; i < N; i++)
		bufDry[i] = bufWet[i] = sinf(2.f * M_PI * 1000.f * i / 48000.f) * 0.5f;

	fxDry.process(bufDry.data(), N, 0);
	fxWet.process(bufWet.data(), N, 0);

	// Wet should have more total energy due to added drive
	REQUIRE(rms(bufWet) > rms(bufDry));
}

TEST_CASE("SaturationExciter — does not add energy to silence") {
	SaturationExciter fx;
	fx.init(makeSetup());
	fx.setSaturation(1.f);

	auto buffer = makeSilence(256);
	
	for (int i = 0; i < 16; i++)
		fx.process(buffer.data(), 256, 0);

	fx.process(buffer.data(), 256, 0);
	REQUIRE(allZero(buffer));
}
