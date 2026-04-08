//
// Created by Darek Rudiš on 08.04.2026.
//

#include "testinghelpers.hpp"
#include "audio/effects/morphfilter.hpp"
#include "audio/effects/noisemaker.hpp"
#include "audio/effects/ringmodulator.hpp"
#include "audio/effects/saturationexciter.hpp"
#include "catch2/catch_test_macros.hpp"

template<typename T>
void testInterfaceContract(T& fx) {
	auto setup = makeSetup();
	fx.init(setup);

	SECTION("silence in silence out") {
		auto buffer = makeSilence(256);
		
		if constexpr (!std::is_same_v<T, NoiseMaker>) {
			fx.process(buffer.data(), 256, 0);
			REQUIRE(allZero(buffer));
		}
	}

	SECTION("output stays in -1..1 with full amplitude input") {
		auto buffer = makeNoise(256, 1.f);
		fx.process(buffer.data(), 256, 0);
		REQUIRE(withinRange(buffer));
	}

	SECTION("channel index beyond stereo does not crash") {
		auto buffer = makeNoise(256);
		REQUIRE_NOTHROW(fx.process(buffer.data(), 256, 5));
	}

	SECTION("zero length buffer does not crash") {
		std::vector<float> buffer;
		REQUIRE_NOTHROW(fx.process(buffer.data(), 0, 0));
	}

	SECTION("multiple blocks process consistently") {
		auto b1 = makeNoise(256, 1.f, 1);
		auto b2 = makeNoise(256, 1.f, 2);
		fx.process(b1.data(), 256, 0);
		fx.process(b2.data(), 256, 0);
		for (float s : b2) REQUIRE_FALSE(std::isnan(s));
	}

	SECTION("varying block size does not crash") {
		for (int size : {64, 128, 256}) {
			auto buffer = makeNoise(size);
			REQUIRE_NOTHROW(fx.process(buffer.data(), size, 0));
		}
	}
}

TEST_CASE("IEffect interface contract — MorphFilter") {
	MorphFilter fx; testInterfaceContract(fx);
}
TEST_CASE("IEffect interface contract — RingModulator") {
	RingModulator fx; testInterfaceContract(fx);
}
TEST_CASE("IEffect interface contract — Exciter") {
	SaturationExciter fx; testInterfaceContract(fx);
}
TEST_CASE("IEffect interface contract — NoiseMaker") {
	NoiseMaker fx; testInterfaceContract(fx);
}