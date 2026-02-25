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

#include "daisysp.h"

tresult PLUGIN_API PluginProcessor::initialize(FUnknown *context) {
    tresult result = AudioEffect::initialize(context);
    if (result != kResultOk)
        return result;

    addAudioInput(USTRING("Stereo In"), SpeakerArr::kStereo);
    addAudioOutput(USTRING("Stereo Out"), SpeakerArr::kStereo);

    createEffects();

    return kResultOk;
}

void PluginProcessor::createEffects() {
    auto& filter_unit = effect_chain_.emplace_back();
    filter_unit.effect = std::make_unique<MorphFilter>();
    filter_unit.intensity_param_id = kParamBrightnessIntensity;
}

tresult PLUGIN_API PluginProcessor::setupProcessing(ProcessSetup &setup) {
    tresult result = AudioEffect::setupProcessing(setup);
    if(result != kResultOk)
        return result;

    is_offline_ = setup.processMode == kOffline;

    setupEffects(setup);

    return kResultOk;
}

void PluginProcessor::setupEffects(ProcessSetup& setup) {
    for(auto& unit : effect_chain_) {
        unit.wet_buffer.resize(setup.maxSamplesPerBlock);

        unit.effect->init(setup);

        // Bind intensity for each effect unit first (all effects have this)
        parameter_router_[unit.intensity_param_id].push_back({
            [unit_ptr = &unit](float value) {
                unit_ptr->intensity = value;
            }
        });

        if (auto* filter = dynamic_cast<MorphFilter*>(unit.effect.get())) {
            registerFilterBindings(filter);
        }
    }
}

void PluginProcessor::registerFilterBindings(MorphFilter* filter) {
    parameter_router_[kParamGain].push_back({
    [filter](float value) {
            filter->setMorph(value);
        }
    });
}

tresult PLUGIN_API PluginProcessor::process(ProcessData& data) {
    // Ensure call is not empty
    if (!(data.numInputs > 0 && data.inputs[0].numChannels > 0))
        return kResultOk;

    const int32 num_channels = data.inputs[0].numChannels;
    const int32 num_samples = data.numSamples;
    const float sample_rate = this->processSetup.sampleRate;

    updateControlParamValues(data);

    if (is_video_playing_) {
        if (is_offline_ || (data.processMode == kOffline)) {
            updateOfflineDspParamValues(data);
        } else {
            updateDspParamValues(data);
        }
    }

    // Choose to either apply DSP or bypass
    if (bypass_state_ == 1.0f)
        return bypassProcessing(data, num_channels, num_samples);

    if (data.processContext)
        last_project_sample_ = data.processContext->projectTimeSamples;

    return processSamples(data, num_channels, num_samples);
}

tresult PLUGIN_API PluginProcessor::getControllerClassId(TUID classId) {
    if (!classId)
        return kInvalidArgument;
    // Copy the controller's FUID into cid
    PluginControllerUID.toTUID(classId);
    return kResultOk;
}

tresult PLUGIN_API PluginProcessor::terminate() {
    return AudioEffect::terminate();
}

tresult PluginProcessor::setState(IBStream *state) {
    if (!state)
        return kResultFalse;

    IBStreamer streamer(state, kLittleEndian);

    int32 version = 0;
    if (!streamer.readInt32(version))
        return kResultFalse;

    if (version > kStateVersion)
        return kResultFalse;

    if (!streamer.readDouble(bypass_state_))
        return kResultFalse;

    if (!streamer.readBool(is_video_playing_))
        return kResultFalse;

    // Modulation vector de-serialization
    modulation_curve_.clear();
    uint32 numPoints = 0;
    if (!streamer.readInt32u(numPoints))
        return kResultFalse;

    modulation_curve_.reserve(numPoints);
    for (uint32 i = 0; i < numPoints; ++i) {
        ModulationPoint point{};

        if (!streamer.readInt64(point.timestamp))
            return kResultFalse;

        uint32 numValues = 0;
        if (!streamer.readInt32u(numValues))
            return kResultFalse;

        point.values.reserve(numValues);

        for (int32 j = 0; j < numValues; ++j) {
            uint32 id;
            double value;

            if (!streamer.readInt32u(id))
                return kResultFalse;
            if (!streamer.readDouble(value))
                return kResultFalse;

            point.values.emplace_back(id, value);
        }

        modulation_curve_.push_back(std::move(point));
    }
    return kResultOk;
}

tresult PluginProcessor::getState(IBStream *state) {
    if (!state)
        return kResultFalse;

    IBStreamer streamer(state, kLittleEndian);

    if (!streamer.writeInt32(kStateVersion))
        return kResultFalse;

    if (!streamer.writeDouble(bypass_state_))
        return kResultFalse;

    if (!streamer.writeBool(is_video_playing_))
        return kResultFalse;

    // Modulation vector serialization
    auto numPoints = static_cast<uint32>(modulation_curve_.size());
    if (!streamer.writeInt32u(numPoints))
        return kResultFalse;

    for (const auto& point : modulation_curve_) {
        if (!streamer.writeInt64(point.timestamp))
            return kResultFalse;

        auto numValues = static_cast<uint32>(point.values.size());
        if (!streamer.writeInt32u(numValues))
            return kResultFalse;

        for (const auto& [id, value] : point.values) {
            if (!streamer.writeInt32u(id))
                return kResultFalse;
            if (!streamer.writeDouble(value))
                return kResultFalse;
        }
    }

    return kResultOk;
}

tresult PLUGIN_API PluginProcessor::notify(Steinberg::Vst::IMessage* message) {
    if (!message)
        return kInvalidArgument;

    if (strcmp(message->getMessageID(), "VideoFinished") == 0) {
        if (capture_state_ == CaptureState::Recording) {
            capture_state_ = CaptureState::Complete;
        }
    }

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
            if (!is_video_playing_) {
                epoch_start_sample_ = data.processContext ? data.processContext->projectTimeSamples : total_samples_;
                is_video_playing_ = true;
            }
            break;
        case kParamReset:
            if (is_video_playing_) {
                epoch_start_sample_ = -1;
                is_video_playing_ = false;
            }
            break;
        case kParamBypass:
            bypass_state_ = value;
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
        }
    );
}

void PluginProcessor::handleDspParam(ParamID id, ParamValue value) {
    const auto it = parameter_router_.find(id);
    if (it == parameter_router_.end())
        return;

    const auto v = static_cast<float>(value);

    for (auto& binding : it->second)
        binding.apply(v);
}

void PluginProcessor::updateOfflineDspParamValues(const ProcessData& data) {
    if (capture_state_ != CaptureState::Complete)
        return;

    const auto nowSamples = data.processContext ? data.processContext->projectTimeSamples : total_samples_;

    static int last_index = 0;

    if (!modulation_curve_.empty() && nowSamples <= modulation_curve_[last_index].timestamp + epoch_start_sample_) {
        last_index = 0;
        return;
    }

    // TODO: Here it should wrap around mod vector size, not just end
    while (last_index < modulation_curve_.size()) {
        const auto& point = modulation_curve_[last_index];
        const auto absSample = point.timestamp + epoch_start_sample_;
        if (absSample >= nowSamples + data.numSamples)
            break;

        for (auto& [id, value] : point.values) {
            handleDspParam(id, value);
        }

        ++last_index;
    }
}


tresult PluginProcessor::bypassProcessing(const ProcessData& data, const int32_t numChannels, const int32_t numSamples) {
    for (int32 ch = 0; ch < numChannels; ++ch) {
        float* input = data.inputs[0].channelBuffers32[ch];
        float* output = data.outputs[0].channelBuffers32[ch];

        if (input && output) {
            std::copy(input, input + numSamples, output);
        }
    }
    return kResultOk;
}

tresult PluginProcessor::processSamples(const ProcessData& data, int32_t numChannels, int32_t numSamples) {
    for (int32 ch = 0; ch < numChannels; ++ch) {
        float* input = data.inputs[0].channelBuffers32[ch];
        float* output = data.outputs[0].channelBuffers32[ch];

        if(!input || !output)
            continue;

        for (auto& unit : effect_chain_) {
            const float intensity = unit.intensity;

            if (intensity < 0.0001f)
                continue;

            std::copy(output, output + numSamples, unit.wet_buffer.data());

            unit.effect->process(unit.wet_buffer.data(), numSamples);
            mixDryWet(output, unit.wet_buffer.data(), output, intensity, numSamples);
        }
    }
    return kResultOk;
}

