//
// Created by Darek Rudiš on 20.03.2025.
//
#include "processor.hpp"
#include "../controller.hpp"

#if defined(__APPLE__)
    #include <sys/proc.h>
#elif defined(_WIN32)
    #include <windows.h>
    #include <tlhelp32.h>
    #include <psapi.h>
#endif

#include "base/source/fstreamer.h"
#include "vst/ivstparameterchanges.h"

tresult PLUGIN_API PluginProcessor::initialize(FUnknown *context)
{
    tresult result = AudioEffect::initialize(context);
    if (result != kResultOk) {
        return result;
    }
    addAudioInput(USTRING("Stereo In"), SpeakerArr::kStereo);
    addAudioOutput(USTRING("Stereo Out"), SpeakerArr::kStereo);
    return kResultOk;
}
tresult PLUGIN_API PluginProcessor::setupProcessing(ProcessSetup &setup)
{
    tresult result = AudioEffect::setupProcessing(setup);
    if(result != kResultOk)
        return result;
    return kResultOk;
}

tresult PLUGIN_API PluginProcessor::process(ProcessData& data)
{
    if (!(data.numInputs > 0 && data.inputs[0].numChannels > 0))
        return kResultOk;

    bool isBypass = getBypassState(data);

    const int32 numSamples = data.numSamples;
    const float sampleRate = this->processSetup.sampleRate;
    const int32 numChannels = data.inputs[0].numChannels;

    if (data.inputParameterChanges)
    {
        for (int32 i = 0; i < data.inputParameterChanges->getParameterCount(); ++i)
        {
            auto* queue = data.inputParameterChanges->getParameterData(i);
            if (!queue) continue;

            if (queue->getParameterId() == kParamGain)
            {
                int32 sampleOffset;
                ParamValue value;
                // Take the last point in the queue for current block
                if (queue->getPoint(queue->getPointCount() - 1, sampleOffset, value) == kResultTrue)
                {
                    gain_ = static_cast<float>(value);
                }
            }
        }
    }

    if (isBypass)
    {
        for (int32 ch = 0; ch < numChannels; ++ch)
        {
            float* input = data.inputs[0].channelBuffers32[ch];
            float* output = data.outputs[0].channelBuffers32[ch];

            if (input && output) {
                std::copy(input, input + numSamples, output);
            }
        }
        return kResultOk;
    }
    for (int32 ch = 0; ch < numChannels; ++ch)
    {
        float* input = data.inputs[0].channelBuffers32[ch];
        float* output = data.outputs[0].channelBuffers32[ch];

        if (input && output) {
            for (int32 i = 0; i < numSamples; ++i) {
                output[i] = input[i] * gain_;
            }
        }
    }
    return kResultOk;
}

bool PluginProcessor::getBypassState(const ProcessData &data) const
{
    if (!data.inputParameterChanges)
        return bypass_state_ == 1.0f;

    for (int32 i = 0; i < data.inputParameterChanges->getParameterCount(); ++i)
    {
        auto* queue = data.inputParameterChanges->getParameterData(i);
        if (!queue || queue->getParameterId() != kParamBypass)
            continue;

        int32 sampleOffset;
        ParamValue value;

        if(queue->getPoint(queue->getPointCount() - 1, sampleOffset, value) != kResultTrue)
            continue;

        bool newBypassState = value == 1.0f;

        if (!(bypass_state_ == 1.0f) && newBypassState) {
            // Flush
        }

        const_cast<PluginProcessor*>(this)->bypass_state_ = value;
        return newBypassState;
    }

    return bypass_state_ == 1.0f;
}

tresult PLUGIN_API PluginProcessor::getControllerClassId(TUID classId)
{
    if (!classId)
        return kInvalidArgument;
    // Copy the controller's FUID into cid
    PluginControllerUID.toTUID(classId);
    return kResultOk;
}

tresult PLUGIN_API PluginProcessor::terminate()
{
    return AudioEffect::terminate();
}

tresult PluginProcessor::setState(IBStream *state)
{
    if (!state)
        return kResultFalse;

    IBStreamer streamer(state, kLittleEndian);

    if (!streamer.readDouble(bypass_state_))
        return kResultFalse;
    return kResultOk;
}

tresult PluginProcessor::getState(IBStream *state)
{
    if (!state)
        return kResultFalse;

    IBStreamer streamer(state, kLittleEndian);

    if (!streamer.writeDouble(bypass_state_))
        return kResultFalse;

    return kResultOk;
}

