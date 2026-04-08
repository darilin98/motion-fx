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
#include "effects/ringmodulator.hpp"
#include "effects/saturationexciter.hpp"
#include "effects/spatialecho.hpp"
#include "effects/noisemaker.hpp"
#include "effects/wavephaser.hpp"

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

    auto& noise_unit = effect_chain_.emplace_back();
    noise_unit.effect = std::make_unique<NoiseMaker>();
    noise_unit.intensity_param_id = kParamMotionBurstIntensity;

    auto& ring_unit = effect_chain_.emplace_back();
    ring_unit.effect = std::make_unique<RingModulator>();
    ring_unit.intensity_param_id = kParamColorIntensity;

    auto& exciter_unit = effect_chain_.emplace_back();
    exciter_unit.effect = std::make_unique<SaturationExciter>();
    exciter_unit.intensity_param_id = kParamSaturationIntensity;

    auto& phaser_unit = effect_chain_.emplace_back();
    phaser_unit.effect = std::make_unique<WavePhaser>();
    phaser_unit.intensity_param_id = kParamMotionContIntensity;

    auto& echo_unit = effect_chain_.emplace_back();
    echo_unit.effect = std::make_unique<SpatialEcho>();
    echo_unit.intensity_param_id = kParamSpaceIntensity;
}

tresult PLUGIN_API PluginProcessor::setupProcessing(ProcessSetup &setup) {
    tresult result = AudioEffect::setupProcessing(setup);
    if(result != kResultOk)
        return result;

    setupEffects(setup);

    return kResultOk;
}

tresult PluginProcessor::setProcessing(TBool state) {
    if (state) {
        is_video_playing_ = false;
        total_samples_ = 0;
        modulation_cursor_ = 0;
        resetParamsToDefault();
    }
    return AudioEffect::setProcessing(state);
}


void PluginProcessor::resetParamsToDefault() {
    for (auto& [id, defaultValue] : kParamDefaultsMap) {
        handleDspParam(id, defaultValue);
    }
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

        if (auto* noise = dynamic_cast<NoiseMaker*>(unit.effect.get())) {
            parameter_router_[kParamMotionBurst].push_back({
                [noise](float value) {
                    noise->setBurst(value);
                }
            });
        }
        if (auto* filter = dynamic_cast<MorphFilter*>(unit.effect.get())) {
            parameter_router_[kParamBrightness].push_back({
            [filter](float value) {
                    filter->setMorph(value);
                }
            });
        }
        if (auto* phaser = dynamic_cast<WavePhaser*>(unit.effect.get())) {
            parameter_router_[kParamMotionContinuous].push_back({
                [phaser](float value) {
                    phaser->setContinuous(value);
                }
            });
        }
        if (auto* echo = dynamic_cast<SpatialEcho*>(unit.effect.get())) {
            parameter_router_[kParamSpace].push_back({
                [echo](float value) {
                    echo->setSpaceLevel(value);
                }
            });
        }
        if (auto* ring = dynamic_cast<RingModulator*>(unit.effect.get())) {
            parameter_router_[kParamColorRed].push_back({
                [ring](float value) {
                    ring->setR(value);
                }
            });
            parameter_router_[kParamColorGreen].push_back({
                [ring](float value) {
                    ring->setG(value);
                }
            });
            parameter_router_[kParamColorBlue].push_back({
                [ring](float value) {
                    ring->setB(value);
                }
            });
            parameter_router_[kParamColorFrequency].push_back({
                [ring](float value) {
                    ring->setBase(value);
                }
            });
        }
        if (auto* exciter = dynamic_cast<SaturationExciter*>(unit.effect.get())) {
            parameter_router_[kParamSaturation].push_back({
                [exciter](float value) {
                    exciter->setSaturation(value);
                }
            });
        }
    }
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

tresult PluginProcessor::setState(IBStream* state) {
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

    if (numPoints != 0)
        modulation_cache_ready_ = true;

    if (!effect_chain_.empty()) {
        for (auto& effect_unit : effect_chain_) {
            double value;
            if (!streamer.readDouble(value))
                return kResultFalse;
            effect_unit.intensity = value;
        }
    }

    return kResultOk;
}

tresult PluginProcessor::getState(IBStream* state) {
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

    if (!effect_chain_.empty()) {
        for (auto& effect_unit : effect_chain_) {
            double value = effect_unit.intensity;
            if (!streamer.writeDouble(value))
                return kResultFalse;
        }
    }
    return kResultOk;
}

tresult PLUGIN_API PluginProcessor::notify(Steinberg::Vst::IMessage* message) {
    if (!message)
        return kInvalidArgument;

    if (strcmp(message->getMessageID(), "ModulationCacheChunk") == 0) {
        int64 chunkIndex, totalChunks, totalBytes;
        message->getAttributes()->getInt("chunkIndex",chunkIndex);
        message->getAttributes()->getInt("totalChunks",totalChunks);
        message->getAttributes()->getInt("totalBytes",totalBytes);

        const void* data; uint32 size;
        message->getAttributes()->getBinary("data", data, size);

        chunk_map_[chunkIndex] = std::vector(
            static_cast<const uint8_t*>(data),
            static_cast<const uint8_t*>(data) + size);

        expected_total_chunks_ = totalChunks;
        expected_total_bytes_ = totalBytes;

        if ((int)chunk_map_.size() == expected_total_chunks_) {
            // All chunks arrived
            std::vector<uint8_t> blob;
            blob.reserve(expected_total_bytes_);
            for (auto& [idx, chunk] : chunk_map_)
                blob.insert(blob.end(), chunk.begin(), chunk.end());

            chunk_map_.clear();
            deserializeModulationCache(blob.data(), blob.size());
            modulation_cache_ready_ = true;
            modulation_cursor_ = 0;
        }
        return kResultOk;
    }

    return kResultOk;
}

void PluginProcessor::deserializeModulationCache(const uint8_t* data, uint32 size) {
    modulation_curve_.clear();
    const uint8_t* p = data;

    uint32_t count;
    memcpy(&count, p, sizeof(count)); p += sizeof(count);
    modulation_curve_.reserve(count);

    for (uint32_t i = 0; i < count; ++i) {
        double seconds;
        memcpy(&seconds, p, sizeof(seconds)); p += sizeof(seconds);

        uint32_t vcount;
        memcpy(&vcount, p, sizeof(vcount)); p += sizeof(vcount);

        ModulationPoint point;
        point.timestamp = static_cast<TSamples>(seconds * processSetup.sampleRate);
        point.values.reserve(vcount);

        for (uint32_t j = 0; j < vcount; ++j) {
            ParamID id;
            ParamValue value;
            memcpy(&id, p, sizeof(id)); p += sizeof(id);
            memcpy(&value, p, sizeof(value)); p += sizeof(value);
            point.values.emplace_back(id, value);
        }
        modulation_curve_.push_back(std::move(point));
    }
}

void PluginProcessor::updateControlParamValues(const ProcessData& data) {
    forEachLastParamChange(data,
        [&](ParamID id, ParamValue value, int32 sampleOffset)
        {
            handleControlParam(id, value, data, sampleOffset);
        }
    );
}

void PluginProcessor::handleControlParam(ParamID id, ParamValue value, const ProcessData& data, const int32 sampleOffset) {
    switch (id) {
        case kParamPlay:
            if (!is_video_playing_) {
                epoch_start_sample_ = data.processContext ?
                    data.processContext->projectTimeSamples + sampleOffset
                    : total_samples_ + sampleOffset;
                is_video_playing_ = true;
            }
            break;
        case kParamReset:
            if (is_video_playing_) {
                epoch_start_sample_ = -1;
                is_video_playing_ = false;
                modulation_cursor_ = 0;
            }
            break;
        case kParamPause:
            if (is_video_playing_) {
                is_video_playing_ = false;
            }
            break;
        case kParamBypass:
            bypass_state_ = value;
            break;
        case kParamLoadVideo:
            modulation_cache_ready_ = false;
            is_video_playing_ = false;
            modulation_curve_.clear();
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
    if (!modulation_cache_ready_ || modulation_curve_.empty())
        return;

    const auto nowSamples = data.processContext
        ? data.processContext->projectTimeSamples
        : total_samples_;

    // Reset cursor on rewind/loop
    if (modulation_cursor_ > 0 && nowSamples < modulation_curve_[modulation_cursor_ - 1].timestamp + epoch_start_sample_) {
        modulation_cursor_ = 0;
    }

    while (modulation_cursor_ < static_cast<int>(modulation_curve_.size())) {
        const auto& point = modulation_curve_[modulation_cursor_];
        const auto absSample = point.timestamp + epoch_start_sample_;

        if (absSample >= nowSamples + data.numSamples)
            break;

        for (auto& [id, value] : point.values)
            handleDspParam(id, value);

        ++modulation_cursor_;
    }
}

tresult PLUGIN_API PluginProcessor::process(ProcessData& data) {
    // Ensure call is not empty
    if (!(data.numInputs > 0 && data.inputs[0].numChannels > 0))
        return kResultOk;

    is_offline_ = (data.processMode == kOffline);

    const int32 num_channels = data.inputs[0].numChannels;
    const int32 num_samples = data.numSamples;

    updateControlParamValues(data);

    if (is_video_playing_) {
        if (is_offline_) {
            updateOfflineDspParamValues(data);
        }
    }

    if (!is_offline_)
        updateDspParamValues(data);

    // Choose to either apply DSP or bypass
    if (bypass_state_ == 1.0f)
        return bypassProcessing(data, num_channels, num_samples);

    total_samples_ += num_samples;

    return processSamples(data, num_channels, num_samples);
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

        std::copy(input, input + numSamples, output);

        for (auto& unit : effect_chain_) {
            const float intensity = unit.intensity;

            if (intensity < 0.0001f)
                continue;

            std::copy(output, output + numSamples, unit.wet_buffer.data());

            unit.effect->process(unit.wet_buffer.data(), numSamples, ch);
            mixDryWet(output, unit.wet_buffer.data(), output, intensity, numSamples);
        }
    }
    return kResultOk;
}

