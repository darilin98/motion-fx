//
// Created by Darek Rudiš on 20.03.2025.
//

#ifndef PROCESSOR_HPP
#define PROCESSOR_HPP

#include "public.sdk/source/vst/vstaudioeffect.h"
#include "pluginterfaces/base/ustring.h"

using namespace Steinberg::Vst;
using namespace Steinberg;

static const FUID PluginProcessorUID(0xd07f0f74, 0x71df4221, 0x98a7dc36, 0x845c4cf5);
static const FUID PluginControllerUID(0x16616cb1, 0xdace4b43, 0xb29ea085, 0xa674bb2d);

constexpr int FFT_SIZE = 1024;

/**
 * @brief Main class for the audio processing part of the plugin.
 *
 * Called by the host. Works with provided samples, outputs them for further actions.
 */
class PluginProcessor : public AudioEffect
{
public:
    PluginProcessor() = default;
    ~PluginProcessor() override = default;

    static FUnknown* createInstance(void*) { return static_cast<IAudioProcessor *>(new PluginProcessor()); }

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
    tresult PLUGIN_API setupProcessing(ProcessSetup &setup) SMTG_OVERRIDE;
    tresult PLUGIN_API setState(IBStream* state) SMTG_OVERRIDE;
    tresult PLUGIN_API getState(IBStream* state) SMTG_OVERRIDE;
private:
    /**
     * @brief Helper to safely get the plugin's bypass state.
     * @param data Parameter data
     * @return State boolean
     */
    [[nodiscard]] bool getBypassState(const ProcessData& data) const;
    ParamValue bypass_state_ = 0.0f;
    ParamValue gain_ = 1.0f;
};
#endif //PROCESSOR_HPP
