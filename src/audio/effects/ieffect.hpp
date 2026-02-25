//
// Created by Darek Rudiš on 25.02.2026.
//

#ifndef IEFFECT_HPP
#define IEFFECT_HPP
#include <memory>
#include <vector>

#include "vst/vsttypes.h"
#include "vst/ivstaudioprocessor.h"

class IEffect {
public:
	virtual ~IEffect() = default;
	virtual void init(Steinberg::Vst::ProcessSetup setup) = 0;
	virtual void process(float* buffer, int32_t numSamples) = 0;
};

struct EffectMixingUnit {
	std::unique_ptr<IEffect> effect;
	Steinberg::Vst::ParamID intensity_param_id;

	float intensity = 1.0f;
	std::vector<float> wet_buffer;
};

using effect_chain_t = std::vector<EffectMixingUnit>;

inline void mixDryWet(float* dry, float* wet, float* out, float intensity, int n) {
	for (int i = 0; i < n; ++i)
		out[i] = dry[i] + intensity * (wet[i] - dry[i]);
}
#endif //IEFFECT_HPP
