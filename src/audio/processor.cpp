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
#include "vst/ivstprocesscontext.h"
#include "parameterextraction.hpp"
#include "../parameterdefaults.hpp"

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
    is_offline_ = setup.processMode == kOffline;
    tresult result = AudioEffect::setupProcessing(setup);
    if(result != kResultOk)
        return result;
    return kResultOk;
}

tresult PLUGIN_API PluginProcessor::process(ProcessData& data)
{
    // Ensure call is not empty
    if (!(data.numInputs > 0 && data.inputs[0].numChannels > 0))
        return kResultOk;

    const int32 num_channels = data.inputs[0].numChannels;
    const int32 num_samples = data.numSamples;
    const float sample_rate = this->processSetup.sampleRate;

    updateControlParamValues(data);

    if (is_video_playing_) {
        if (is_offline_ || (data.processMode == kOffline)) {
            // Update params using modulation vector indexed by time
        } else {
            updateDspParamValues(data);
        }
    }

    // Choose to either apply DSP or bypass
    if (tryGetBypassState(data))
        return bypassProcessing(data, num_channels, num_samples);

    return processSamples(data, num_channels, num_samples);
}

bool PluginProcessor::tryGetBypassState(const ProcessData &data) const
{
    if (!data.inputParameterChanges)
        return bypass_state_ == 1.0f;

    for (int32 i = 0; i < data.inputParameterChanges->getParameterCount(); ++i)
    {
        auto* queue = data.inputParameterChanges->getParameterData(i);
        if (!queue || queue->getParameterId() != kParamBypass)
            continue;

        int32 sample_offset;
        ParamValue value;

        if(queue->getPoint(queue->getPointCount() - 1, sample_offset, value) != kResultTrue)
            continue;

        bool new_bypass_state = value == 1.0f;

        if (!(bypass_state_ == 1.0f) && new_bypass_state) {
            // Flush
        }

        const_cast<PluginProcessor*>(this)->bypass_state_ = value;
        return new_bypass_state;
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

tresult PLUGIN_API PluginProcessor::notify(Steinberg::Vst::IMessage* message)
{
    if (!message)
        return kInvalidArgument;

    return kResultOk;
}

void PluginProcessor::updateControlParamValues(const ProcessData& data) {
    forEachLastParamChange(data,
        [&](ParamID id, ParamValue value, int32)
        {
            handleControlParam(id, value, data);
        }
    );
}

void PluginProcessor::handleControlParam(ParamID id, ParamValue value, const ProcessData& data) {
    switch (id) {
        case kParamPlay:
            if (value > 0.5) {
                if (!is_video_playing_)
                    epoch_start_sample_ = data.processContext ? data.processContext->projectTimeSamples : total_samples_;

                is_video_playing_ = !is_video_playing_;
            }
            break;
        case kParamReset:
            if (value > 0.5) {
                modulation_curve_.clear();
                epoch_start_sample_ = -1;
            }
            break;
        default:
            break;
    }
}


void PluginProcessor::updateDspParamValues(const ProcessData& data) {
    forEachLastParamChange(data,
        [&](ParamID id, ParamValue value, int32)
        {
            handleDspParam(id, value);
            captureModulation(id, value, data);
        }
    );
}

void PluginProcessor::handleDspParam(ParamID id, ParamValue value) {
    switch (id) {
        case kParamGain:
            gain_ = static_cast<float>(value);
            break;
        default:
            break;
    }
}

void PluginProcessor::captureModulation(ParamID id, ParamValue value, const ProcessData& data) {
    if (!is_video_playing_ || epoch_start_sample_ < 0)
        return;

    const auto nowSamples =
        data.processContext ? data.processContext->projectTimeSamples : total_samples_;

    const auto relativeSamples = nowSamples - epoch_start_sample_;

    if (modulation_curve_.empty() || modulation_curve_.back().timestamp != relativeSamples)
    {
        modulation_curve_.push_back({relativeSamples, {}});
    }

    modulation_curve_.back().values.emplace_back(id, value);
}




tresult PluginProcessor::bypassProcessing(const ProcessData& data, const int32_t numChannels, const int32_t numSamples) {
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

tresult PluginProcessor::processSamples(const ProcessData& data, int32_t numChannels, int32_t numSamples) const {
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

