//
// Created by Darek Rudiš on 18.03.2026.
//
#include <catch2/catch_test_macros.hpp>
#include "audio/effects/wavephaser.hpp"
#include "random"

TEST_CASE("Processing stays in normalized range") {
	WavePhaser phaser;

	Steinberg::Vst::ProcessSetup setup{};
	setup.sampleRate = 48000.0;
	phaser.init(setup);

	constexpr int bufferSize = 256;
	float buffer[bufferSize];

	static std::mt19937 rng(std::random_device{}());
	static std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

	for (int i = 0; i < bufferSize; ++i) {
		buffer[i] = dist(rng);
	}

	phaser.setContinuous(1.0f);

	phaser.process(buffer, bufferSize, 0);

	// Check all samples are within [-1, 1]
	for (int i = 0; i < bufferSize; ++i) {

		fprintf(stderr, "value: %f\n", buffer[i]);
		REQUIRE(buffer[i] <= 1.0f);
		REQUIRE(buffer[i] >= -1.0f);
	}
}