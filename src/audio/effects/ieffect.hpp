//
// Created by Darek Rudiš on 25.02.2026.
//

#ifndef IEFFECT_HPP
#define IEFFECT_HPP
#include <memory>
#include <vector>

#include "vst/vsttypes.h"
#include "vst/ivstaudioprocessor.h"

constexpr int32_t kChannelCountDefault = 2; /// Default count of channels (stereo).
constexpr float kSmoothing = 0.001f; /// Default smoothing of parameter values.
constexpr double kSampleRateDefault = 44100.0;

/**
 * @brief A contract for processing buffers of audio samples.
 */
class IEffect {
public:
	virtual ~IEffect() = default;

	/**
	 * Initializes the effect.
	 * @param setup Processing data provided by the plugin.
	 */
	virtual void init(Steinberg::Vst::ProcessSetup setup) = 0;

	/**
	 * @brief Applies processing to a buffer of audio samples.
	 * Processes on a single channel.
	 * @param buffer Sample buffer.
	 * @param numSamples Buffer size.
	 * @param channel Number of the channel being processed.
	 */
	virtual void process(float* buffer, int32_t numSamples, int32_t channel) = 0;
};

/**
 * @brief Data type coupling an IEffect with a mix ratio.
 *
 * Allows to independently mix the outputs of an IEffect
 */
struct EffectMixingUnit {
	std::unique_ptr<IEffect> effect; /// Effect applying processing to samples.
	Steinberg::Vst::ParamID intensity_param_id; /// ID of the parameter controlling the mix ratio.

	float intensity = 1.0f; /// Dry/wet mix ratio.
	std::vector<float> wet_buffer; /// Holds the completely processed result of the effect.
};

using effect_chain_t = std::vector<EffectMixingUnit>;

/**
 * @brief Mixes signal in respect to the intensity amount.
 *
 * @param dry Buffer containing clean input samples.
 * @param wet Buffer containing pure processed samples.
 * @param out Resulting mixed sample buffer.
 * @param intensity Normalized value deciding the mix ratio.
 * @param n Number of samples.
 */
inline void mixDryWet(float* dry, float* wet, float* out, float intensity, int n) {
	for (int i = 0; i < n; ++i)
		out[i] = dry[i] + intensity * (wet[i] - dry[i]);
}

struct Smoothed {
	float target = 0.f;
	float value = 0.f;
};

#endif //IEFFECT_HPP
