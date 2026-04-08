//
// Created by Darek Rudiš on 20.03.2025.
//

/**
 * @file processor.hpp
 * @brief Declares the PluginProcessor
 */

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

	/**
	 * @brief Initializes processing and all individual effects.
	 * @param context Processing context
	 * @return Method success status.
	 */
	tresult PLUGIN_API initialize(FUnknown* context) SMTG_OVERRIDE;

	/**
	 * @brief Destroys plugin processor.
	 * @return Method success status.
	 */
	tresult PLUGIN_API terminate() SMTG_OVERRIDE;

	/**
	 * @brief Main sample processing method.
	 *
	 * Calls for updates of parameter states.
	 * Applies processing of IEffect implementations to input samples.
	 * @param data Data containing samples and processing context.
	 */
	tresult PLUGIN_API process(ProcessData& data) SMTG_OVERRIDE;

	/**
	 * @brief Sets up internal pipeline processor objects and buffers.
	 */
	tresult PLUGIN_API setupProcessing(ProcessSetup& setup) SMTG_OVERRIDE;

	/**
	 * @brief Sets between processing and non-processing state.
	 * @param state Processing state to switch to.
	 * @return Method success status.
	 */
	tresult PLUGIN_API setProcessing(TBool state) SMTG_OVERRIDE;

	/**
	 * @brief Restores the saved state of the processor.
	 * @param state Input state stream.
	 * @return Method success status.
	 */
	tresult PLUGIN_API setState(IBStream* state) SMTG_OVERRIDE;

	/**
	 * @brief Saves the state of the processor.
	 * @param state Output state stream.
	 * @return Method success status.
	 */
	tresult PLUGIN_API getState(IBStream* state) SMTG_OVERRIDE;

	/**
	 * @brief Handles an IMessage.
	 * Called when PluginController sends an IMessage.
	 * @param message IMessage to-be handled.
	 * @return Method success status.
	 */
	tresult PLUGIN_API notify(IMessage* message) SMTG_OVERRIDE;

private:
	/**
	 * @brief Instantiate all effects of the effect_chain_.
	 */
	void createEffects();

	/**
	 * @brief Initialize all effect_chain_ effects with the current processing context.
	 * @param setup Processing context.
	 */
	void setupEffects(ProcessSetup& setup);

	/**
	 * @brief Reset effect-related params to their defaults.
	 */
	void resetParamsToDefault();

	/**
	 * @brief Updates processing parameter changes in inputParameterChanges.
	 * @param data ProcessData containing inputParameterChanges.
	 */
	void updateDspParamValues(const ProcessData& data);

	/**
	 * @brief Matches the parameter ID to its related update functions.
	 * @param id Parameter key.
	 * @param value Parameter value.
	 */
	void handleDspParam(ParamID id, ParamValue value);

	/**
	 * @brief Updates control parameter changes in inputParameterChanges.
	 * @param data ProcessData containing inputParameterChanges.
	 */
	void updateControlParamValues(const ProcessData& data);

	/**
	 * @brief Changes video state based on the control parameters being called.
	 * @param id Control parameter ID.
	 * @param value Control parameter value.
	 * @param data Processing context.
	 * @param sampleOffset Sample-accurate parameter update timestamp.
	 */
	void handleControlParam(ParamID id, ParamValue value, const ProcessData& data, int32 sampleOffset);

	/**
	 * @brief Parses the incoming IMessage containing modulation cache.
	 * @param data Message data blob.
	 * @param size Size of data.
	 */
	void deserializeModulationCache(const uint8_t* data, uint32 size);

	/**
	 * @brief Updates parameter values from the cached modulation_curve_.
	 * @param data ProcessData containing inputParameterChanges.
	 */
	void updateOfflineDspParamValues(const ProcessData& data);

	/**
	 * @brief Performs a clean pass of the audio samples.
	 * @param data Audio samples.
	 * @param numChannels Channel count.
	 * @param numSamples Block sample count.
	 * @return Method success status.
	 */
	static tresult bypassProcessing(const ProcessData& data, int32_t numChannels, int32_t numSamples);

	/**
	 * @brief Applies processing of each IEffect contained in effect_chain_.
	 * @param data Audio samples.
	 * @param numChannels Channel count.
	 * @param numSamples Block sample count.
	 * @return Method success status.
	 */
	[[nodiscard]] tresult processSamples(const ProcessData& data, int32_t numChannels, int32_t numSamples);

	parameter_router_t parameter_router_ {}; /// Map of parameter IDs to update functions.
	effect_chain_t effect_chain_ {}; /// All instantiated IEffect implementations taking part in processing.

	bool is_offline_ = false; /// Indicates intent to process from modulation_curve_.
	bool is_video_playing_ = false; /// Follows the state of video inside PluginController.
	TSamples epoch_start_sample_ = -1; /// Sample time start
	TSamples total_samples_ = 0; /// Backup sample time tracking
	ParamValue bypass_state_ = 0.0f; /// Bypass plugin state

	bool modulation_cache_ready_ = false; /// Cache received from PluginController state.
	int modulation_cursor_ = 0; /// Position in cache video representation.

	std::map<int, std::vector<uint8_t>> chunk_map_; /// Cache chunk map
	int expected_total_chunks_ = 0; /// Cache total messages count
	size_t expected_total_bytes_ = 0; /// Cache total bytes count

	modulation_curve_t modulation_curve_ = {}; /// Modulation cache
};
#endif //PROCESSOR_HPP
