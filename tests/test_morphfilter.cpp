//
// Created by Darek Rudiš on 04.03.2026.
//
#include <catch2/catch_test_macros.hpp>
#include "audio/effects/morphfilter.hpp"
#include "testinghelpers.hpp"

TEST_CASE("MorphFilter — morph=0 passes low frequencies, attenuates high") {
    MorphFilter fx;
    fx.init(makeSetup());
    fx.setMorph(0.f);

    // High freq content buffer
    constexpr int N = 1024;
    std::vector<float> buffer(N);
    for (int i = 0; i < N; i++)
        buffer[i] = sinf(2.f * M_PI * 10000.f * i / 48000.f);

    for (int block = 0; block < 8; block++)
        fx.process(buffer.data(), N, 0);

    float highFreqRms = rms(buffer);

    // Compare test low frequency
    for (int i = 0; i < N; i++)
        buffer[i] = sinf(2.f * M_PI * 200.f * i / 48000.f);
    for (int block = 0; block < 8; block++)
        fx.process(buffer.data(), N, 0);

    float lowFreqRms = rms(buffer);

    REQUIRE(lowFreqRms > highFreqRms);
}

TEST_CASE("MorphFilter — morph=1 passes high frequencies, attenuates low") {
    MorphFilter fx;
    fx.init(makeSetup());
    fx.setMorph(1.f);

    constexpr int N = 1024;
    std::vector<float> buffer(N);

    auto noise = makeNoise(N);
    for (int i = 0; i < 8; i++) fx.process(noise.data(), N, 0);

    for (int i = 0; i < N; i++)
        buffer[i] = sinf(2.f * M_PI * 200.f * i / 48000.f);
    for (int block = 0; block < 8; block++)
        fx.process(buffer.data(), N, 0);
    float lowFreqRms = rms(buffer);

    for (int i = 0; i < N; i++)
        buffer[i] = sinf(2.f * M_PI * 10000.f * i / 48000.f);
    for (int block = 0; block < 8; block++)
        fx.process(buffer.data(), N, 0);
    float highFreqRms = rms(buffer);

    REQUIRE(highFreqRms > lowFreqRms);
}

TEST_CASE("MorphFilter — morph smoothing does not produce discontinuity") {
    MorphFilter fx;
    fx.init(makeSetup());

    constexpr int N = 256;
    auto buffer = makeNoise(N);

    fx.setMorph(0.f);
    fx.process(buffer.data(), N, 0);
    fx.setMorph(1.f);  // large jump
    buffer = makeNoise(N);
    fx.process(buffer.data(), N, 0);

    REQUIRE(withinRange(buffer));
    for (float sample : buffer)
        REQUIRE_FALSE(std::isnan(sample));
}