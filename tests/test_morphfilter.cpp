//
// Created by Darek Rudiš on 04.03.2026.
//
#include <catch2/catch_test_macros.hpp>
#include "audio/effects/morphfilter.hpp"

TEST_CASE("Processing modifies signal")
{
	MorphFilter filter;

	Steinberg::Vst::ProcessSetup setup{};
	setup.sampleRate = 48000.0;
	filter.init(setup);

	float buffer[64];
	for (float& i : buffer)
		i = 1.0f;

	filter.process(buffer, 64, 0);

	REQUIRE(buffer[0] != 1.0f);
}