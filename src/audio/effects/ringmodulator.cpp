//
// Created by Darek Rudiš on 24.03.2026.
//

#include "ringmodulator.hpp"

void RingModulator::setR(float r) { r_.target = std::clamp(r, 0.f, 1.f); }
void RingModulator::setG(float g) { g_.target = std::clamp(g, 0.f, 1.f); }
void RingModulator::setB(float b) { b_.target = std::clamp(b, 0.f, 1.f); }

void RingModulator::init(Steinberg::Vst::ProcessSetup setup) {
    sample_rate_ = setup.sampleRate;
    channelResizeTo(kChannelCountDefault);
}

void RingModulator::channelResizeTo(size_t size) {
    channels_.resize(size);

    auto initOsc = [&](oscillator_t& osc, float ratio) {
        osc.Init(sample_rate_);
        osc.SetWaveform(daisysp::Oscillator::WAVE_SIN);
        osc.SetFreq(kBaseFreq * ratio);
        osc.SetAmp(1.0f);
    };

    for (auto& ch : channels_) {
        initOsc(ch.osc_r, kRatioR);
        initOsc(ch.osc_g, kRatioG);
        initOsc(ch.osc_b, kRatioB);
    }
}

void RingModulator::process(float* buffer, int32_t numSamples, int32_t channel) {
    if (channel >= channels_.size()) {
        channelResizeTo(channel + 1);
    }

    auto& osc_r = channels_[channel].osc_r;
    auto& osc_g = channels_[channel].osc_g;
    auto& osc_b = channels_[channel].osc_b;

    for (int32_t i = 0; i < numSamples; i++) {
        r_.value += kSmoothing * (r_.target - r_.value);
        g_.value += kSmoothing * (g_.target - g_.value);
        b_.value += kSmoothing * (b_.target - b_.value);

        const float dry = buffer[i];

        // ring mod signal scaled by its colour weight
        float mod_r = dry * osc_r.Process() * r_.value;
        float mod_g = dry * osc_g.Process() * g_.value;
        float mod_b = dry * osc_b.Process() * b_.value;

        // normalize
        // TODO: possibly single color might be too quiet due to this
        float wet = (mod_r + mod_g + mod_b) / 3.f;

        // Dry/Wet blend, black = dry signal
        float energy = (r_.value + g_.value + b_.value) / 3.f;
        float a = cosf(energy * 0.5f * M_PI);
        float b_coef = sinf(energy * 0.5f * M_PI);
        buffer[i] = a * dry + b_coef * wet;
    }
}