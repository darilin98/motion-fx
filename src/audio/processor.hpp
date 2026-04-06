//
// Created by Darek Rudiš on 20.03.2025.
//

#ifndef PROCESSOR_HPP
#define PROCESSOR_HPP

#include <map>
#include <mutex>
#include "modulationcurve.hpp"
#include "parameterextraction.hpp"
#include "effects/ieffect.hpp"
#include "effects/morphfilter.hpp"
#include "public.sdk/source/vst/vstaudioeffect.h"
#include "pluginterfaces/base/ustring.h"

using namespace Steinberg::Vst;
using namespace Steinberg;

static const FUID PluginProcessorUID(0xd07f0f74, 0x71df4221, 0x98a7dc36, 0x845c4cf5);
static const FUID PluginControllerUID(0x16616cb1, 0xdace4b43, 0xb29ea085, 0xa674bb2d);

constexpr int32 kStateVersion = 1;

/**
 * @brief Main class for the audio processing part of the plugin.
 *
 * Called by the host. Works with provided samples, outputs them for further actions.
 */
class PluginProcessor : public AudioEffect {
public:
	PluginProcessor() = default;

	~PluginProcessor() override = default;

	static FUnknown* createInstance(void*) { return static_cast<IAudioProcessor*>(new PluginProcessor()); }

	tresult PLUGIN_API getControllerClassId(TUID classId) SMTG_OVERRIDE;

	tresult PLUGIN_API initialize(FUnknown* context) SMTG_OVERRIDE;

	tresult PLUGIN_API terminate() SMTG_OVERRIDE;

	/**
	 * @brief Main processing method.
	 *
	 * Called periodically by the host when the plugin is not bypassing.
	 */
	tresult PLUGIN_API process(ProcessData& data) SMTG_OVERRIDE;

	/**
	 * @brief Sets up internal pipeline processor objects and buffers.
	 */
	tresult PLUGIN_API setupProcessing(ProcessSetup& setup) SMTG_OVERRIDE;

	tresult PLUGIN_API setProcessing(TBool state) SMTG_OVERRIDE;

	tresult PLUGIN_API setState(IBStream* state) SMTG_OVERRIDE;

	tresult PLUGIN_API getState(IBStream* state) SMTG_OVERRIDE;

	tresult PLUGIN_API notify(IMessage* message) SMTG_OVERRIDE;

private:
	void createEffects();
	void setupEffects(ProcessSetup& setup);
	void resetParamsToDefault();

	void updateDspParamValues(const ProcessData& data);
	void handleDspParam(ParamID id, ParamValue value);

	void updateControlParamValues(const ProcessData& data);
	void handleControlParam(ParamID id, ParamValue value, const ProcessData& data, int32 sampleOffset);

	void deserializeModulationCache(const uint8_t* data, uint32 size);
	void updateOfflineDspParamValues(const ProcessData& data);

	static tresult bypassProcessing(const ProcessData& data, int32_t numChannels, int32_t numSamples);

	[[nodiscard]] tresult processSamples(const ProcessData& data, int32_t numChannels, int32_t numSamples);

	parameter_router_t parameter_router_ {};
	effect_chain_t effect_chain_ {};

	bool is_offline_ = false;
	bool is_video_playing_ = false;
	TSamples epoch_start_sample_ = -1;
	TSamples total_samples_ = 0;
	ParamValue bypass_state_ = 0.0f;

	bool modulation_cache_ready_ = false;
	int  modulation_cursor_ = 0;

	std::map<int, std::vector<uint8_t>> chunk_map_;
	int expected_total_chunks_ = 0;
	size_t expected_total_bytes_ = 0;

	modulation_curve_t modulation_curve_ = {};
};
#endif //PROCESSOR_HPP
