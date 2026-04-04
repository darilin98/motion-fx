//
// Created by Darek Rudiš on 20.03.2025.
//
#include "controller.hpp"
#include "base/ustring.h"

#include "parameterdefaults.hpp"
#include "utils.hpp"
#include "base/source/fstreamer.h"
#include "vstgui/lib/vstguiinit.h"
#include "ui/motionfxeditor.hpp"
#include "video/features/colorfeatureextractor.hpp"
#include "video/features/motionfeatureextractor.hpp"
#include "video/features/brightnessfeatureextractor.hpp"
#include "video/features/depthfeatureextractor.hpp"
#include "video/features/saturationfeatureextractor.hpp"

tresult PLUGIN_API PluginController::initialize(FUnknown* context)
{
    auto moduleHandle = getPlatformModuleHandle();
    fprintf(stderr, "Module handle: %p\n", moduleHandle);
    tresult result = EditController::initialize(context);
    if (result != kResultOk)
        return result;
    parameters.removeAll();
    parameters.addParameter(STR16("UiControl"), nullptr, 1, 0.0,ParameterInfo::kCanAutomate | ParameterInfo::kIsList,kParamLoadVideo);
    parameters.addParameter(STR16("Play"), nullptr, 1, 0.0, ParameterInfo::kCanAutomate | ParameterInfo::kIsList, kParamPlay);
    parameters.addParameter(STR16("Reset"), nullptr, 1, 0.0, ParameterInfo::kCanAutomate | ParameterInfo::kIsList, kParamReset);
    parameters.addParameter(STR16("Pause"), nullptr, 1, 0.0, ParameterInfo::kCanAutomate | ParameterInfo::kIsList, kParamPause);
    parameters.addParameter(STR16("Bypass"), nullptr, 1, 0.0, ParameterInfo::kIsBypass | ParameterInfo::kCanAutomate | ParameterInfo::kIsList, kParamBypass);
    parameters.addParameter(STR16("Export"), nullptr, 1, 0.0, ParameterInfo::kIsHidden |ParameterInfo::kIsReadOnly, kParamExport);

    parameters.addParameter(STR16("BrightnessIntensity"), nullptr, 0, ParamDefaults::kIntensity, ParameterInfo::kCanAutomate, kParamBrightnessIntensity);
    parameters.addParameter(STR16("Brightness"), nullptr, 1, ParamDefaults::kBrightness, ParameterInfo::kIsHidden | ParameterInfo::kIsReadOnly,kParamBrightness);

    parameters.addParameter(STR16("SpaceIntensity"), nullptr, 0, ParamDefaults::kIntensity, ParameterInfo::kCanAutomate, kParamSpaceIntensity);
    parameters.addParameter(STR16("Space"), nullptr, 0, ParamDefaults::kDepth, ParameterInfo::kIsHidden | ParameterInfo::kIsReadOnly, kParamSpace);

    parameters.addParameter(STR16("MotionContIntensity"), nullptr, 0, ParamDefaults::kIntensity, ParameterInfo::kCanAutomate, kParamMotionContIntensity);
    parameters.addParameter(STR16("MotionBurstIntensity"), nullptr, 0, ParamDefaults::kIntensity, ParameterInfo::kCanAutomate, kParamMotionBurstIntensity);
    parameters.addParameter(STR16("MotionContinuous"), nullptr, 0, ParamDefaults::kMotion, ParameterInfo::kIsHidden | ParameterInfo::kIsReadOnly, kParamMotionContinuous);
    parameters.addParameter(STR16("MotionBurst"), nullptr, 0, ParamDefaults::kMotion, ParameterInfo::kIsHidden | ParameterInfo::kIsReadOnly, kParamMotionBurst);

    parameters.addParameter(STR16("ColorIntensity"), nullptr, 0, ParamDefaults::kIntensity, ParameterInfo::kCanAutomate, kParamColorIntensity);
    parameters.addParameter(STR16("ColorBaseFrequency"), STR16("Hz"), 0, ParamDefaults::kColorFreq, ParameterInfo::kCanAutomate, kParamColorFrequency);
    parameters.addParameter(STR16("ColorRed"), nullptr, 0, ParamDefaults::kColor, ParameterInfo::kIsHidden | ParameterInfo::kIsReadOnly, kParamColorRed);
    parameters.addParameter(STR16("ColorGreen"), nullptr, 0, ParamDefaults::kColor, ParameterInfo::kIsHidden | ParameterInfo::kIsReadOnly, kParamColorGreen);
    parameters.addParameter(STR16("ColorBlue"), nullptr, 0, ParamDefaults::kColor, ParameterInfo::kIsHidden | ParameterInfo::kIsReadOnly, kParamColorBlue);

    parameters.addParameter(STR16("SaturationIntensity"), nullptr, 0, ParamDefaults::kIntensity, ParameterInfo::kCanAutomate, kParamSaturationIntensity);
    parameters.addParameter(STR16("Saturation"), nullptr, 0, ParamDefaults::kSaturation, ParameterInfo::kIsHidden | ParameterInfo::kIsReadOnly, kParamSaturation);
    return kResultOk;
}

tresult PLUGIN_API PluginController::terminate()
{
    if (playback_controller_) {
        playback_controller_->shutdown();
        playback_controller_.reset();
    }
    extractors_.clear();
    return EditController::terminate();
}

tresult PluginController::disconnect(IConnectionPoint* other) {
    if (playback_controller_) { playback_controller_->stopPipeline(); }
    processor_connection_ = nullptr;
    return EditController::disconnect(other);
}

tresult PLUGIN_API PluginController::setState(IBStream *state)
{
    resetInternalParams();

    if (!state)
        return kResultFalse;

    IBStreamer streamer(state, kLittleEndian);

    int32 version;
    if (!streamer.readInt32(version))
        return kResultFalse;

    double bypass_state = 0.0;
    if (!streamer.readDouble(bypass_state))
        return kResultFalse;
    setParamNormalized(kParamBypass, bypass_state);

    double export_state = 0.0;
    if (!streamer.readDouble(export_state))
        return kResultFalse;
    setParamNormalized(kParamExport, export_state);

    if (char* path = streamer.readStr8()) {
        video_path_ = VSTGUI::UTF8String(path);
        delete[] path;
    } else {
        video_path_.clear();
    }

    if(!streamer.readBool(is_video_preview_mode_))
        return kResultFalse;

    if (!video_path_.empty() && is_video_preview_mode_)
        setupPlayback(video_path_);

    for (auto& id : kParamIntensities) {
        double value;
        if (!streamer.readDouble(value))
            return kResultFalse;
        setParamNormalized(id, value);
    }

    return kResultOk;
}

tresult PLUGIN_API PluginController::getState(IBStream *state)
{
    // Ugly hack, but it fixes weird Waveform exit behavior
    auto now = std::chrono::steady_clock::now();
    auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_get_state_time_).count();
    last_get_state_time_ = now;

    if (delta < 100 && playback_controller_) {
        log_debug("Rapid getState detected, stopping pipeline");
        playback_controller_->stopPipeline();
    }

    if (!state)
        return kResultFalse;

    IBStreamer streamer(state, kLittleEndian);
    if(!streamer.writeInt32(1)) // version
        return kResultFalse;

    auto bypass_state = getParamNormalized(kParamBypass);
    if (!streamer.writeDouble(bypass_state))
        return kResultFalse;

    double export_state = getParamNormalized(kParamExport);
    if (!streamer.writeDouble(export_state))
        return kResultFalse;

    const std::string pathUtf8 = video_path_.getString();
    if (!streamer.writeStr8(pathUtf8.c_str()))
        return kResultFalse;

    if (!streamer.writeBool(is_video_preview_mode_))
        return kResultFalse;

    for (auto& id : kParamIntensities) {
        double value = getParamNormalized(id);
        if (!streamer.writeDouble(value))
            return kResultFalse;
    }


    return kResultOk;
}

IPlugView* PLUGIN_API PluginController::createView (FIDString name)
{
    if (strcmp (name, ViewType::kEditor) == 0)
    {
        if (is_video_preview_mode_)
            return new MotionFxEditor (this, "AudioProcessing", "viewGUI.uidesc");
        return new MotionFxEditor (this, "InputSelect", "viewGUI.uidesc");
    }
    return nullptr;
}

tresult PLUGIN_API PluginController::setParamNormalized(ParamID tag, ParamValue value)
{
    if (tag == kParamBrightness)
    {
        // fprintf(stderr, ">>> setParamNormalized(kParamGain, %f)\n", value);
    }
    return EditController::setParamNormalized(tag, value);
}

template<typename T, typename... Args>
void PluginController::addExtractor(Args&&... args) {
    extractors_.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
    extractors_.back()->setFeatureSink(this);
}

void PluginController::instantiateExtractors() {
    extractors_.clear();

    addExtractor<BrightnessFeatureExtractor>(kParamBrightness);
    addExtractor<DepthFeatureExtractor>(kParamSpace);
    addExtractor<MotionFeatureExtractor>(kParamMotionContinuous, kParamMotionBurst);
    addExtractor<ColorFeatureExtractor>(kParamColorRed, kParamColorGreen, kParamColorBlue);
    addExtractor<SaturationFeatureExtractor>(kParamSaturation);
}

void PluginController::registerExtractors() const {
    if (!playback_controller_)
        return;

    for (const auto& extractor : extractors_) {
        if (auto* receiver = dynamic_cast<IFrameReceiver*>(extractor.get())) {
            playback_controller_->registerReceiver(receiver);
        }
    }
}

void PluginController::setupPlayback(const VSTGUI::UTF8String& path) {
    auto frame_queue = std::make_shared<FrameQueue>();

    auto ticker = std::make_unique<FrameTicker>();
    ticker->setQueue(frame_queue);

    instantiateExtractors();

    playback_controller_ = std::make_shared<PlaybackController>(path.data(), std::move(frame_queue), std::move(ticker));

    registerExtractors();
    playback_controller_->setParamListeners(this);

    video_path_ = path;
    is_video_preview_mode_ = true;
}

void PluginController::cleanUpPlayback() {
    if (playback_controller_) {
        playback_controller_->shutdown();
        playback_controller_.reset();
    }

    extractors_.clear();

    resetInternalParams();

    video_path_.clear();
    is_video_preview_mode_ = false;

    double export_invalid = 0.0;
    setParamNormalized(kParamExport, export_invalid);
}

void PluginController::registerReceiver(IFrameReceiver* receiver) const {
    if (playback_controller_) {
        playback_controller_->registerReceiver(receiver);
   }
}

void PluginController::unregisterReceiver(IFrameReceiver* receiver) const {
    if (playback_controller_) {
        playback_controller_->unregisterReceiver(receiver);
    }
}

tresult PLUGIN_API PluginController::connect(IConnectionPoint* other) {
    processor_connection_ = other;
    return kResultOk;
}

void PluginController::onVideoFinished() {
    if (!processor_connection_)
        return;

    Steinberg::Vst::IMessage* msg = allocateMessage();

    if (!msg)
        return;

    msg->setMessageID("VideoFinished");
    processor_connection_->notify(msg);

    sendModulationCacheChunked();

    // not using bool here in order to communicate the fact through a parameter
    double export_ready = 1.0;
    setParamNormalized(kParamExport, export_ready);
}


void PluginController::sendModulationCacheChunked() {
    std::vector<uint8_t> blob;
    {
        std::lock_guard lock(cache_mutex_);

        uint32_t count = modulation_cache_.size();
        auto append = [&](const void* data, size_t n) {
            auto* p = static_cast<const uint8_t*>(data);
            blob.insert(blob.end(), p, p + n);
        };

        append(&count, sizeof(count));
        for (auto& point : modulation_cache_) {
            append(&point.timestamp, sizeof(point.timestamp));
            uint32_t vcount = point.values.size();
            append(&vcount, sizeof(vcount));
            for (auto& [id, value] : point.values) {
                append(&id, sizeof(id));
                append(&value, sizeof(value));
            }
        }
    }

    int totalChunks = (blob.size() + kChunkSize - 1) / kChunkSize;

    for (int i = 0; i < totalChunks; ++i) {
        size_t offset = i * kChunkSize;
        size_t len = std::min(kChunkSize, blob.size() - offset);

        auto* msg = allocateMessage();
        msg->setMessageID("ModulationCacheChunk");
        msg->getAttributes()->setInt("chunkIndex",  i);
        msg->getAttributes()->setInt("totalChunks", totalChunks);
        msg->getAttributes()->setInt("totalBytes",  (int64)blob.size());
        msg->getAttributes()->setBinary("data", blob.data() + offset, len);
        processor_connection_->notify(msg);
        msg->release();
    }

    modulation_cache_.clear();
}

void PluginController::onFeatureResult(const FeatureResult& result) {
    auto params = result.params;
    auto timestamp = result.timestamp;

    {
        std::lock_guard lock(pending_mutex_);
        pending_params_.insert(
            pending_params_.end(),
            result.params.begin(),
            result.params.end()
        );
    }

    if (!flush_scheduled_.exchange(true)) {
        VSTGUI::Tasks::schedule(VSTGUI::Tasks::mainQueue(), [this]() { flushPendingParams(); });
    }

    {
        std::lock_guard lock(cache_mutex_);

        if (modulation_cache_.size() < kMaxCacheSize) {
            ModulationPointTime point;
            point.timestamp = result.timestamp;
            for (auto& [id, value] : result.params)
                point.values.emplace_back(id, value);
            modulation_cache_.push_back(std::move(point));
        }
    }
}

void PluginController::flushPendingParams() {
    std::vector<FeatureParamUpdate> local;

    {
        std::lock_guard lock(pending_mutex_);
        local.swap(pending_params_);
        flush_scheduled_ = false;
    }

    if (local.empty())
        return;

    for (auto& param : local)
            setParamNormalized(param.id, param.normalized);

    if (auto* handler = getComponentHandler()) {
        for (auto& param : local) {
            handler->beginEdit(param.id);
            handler->performEdit(param.id, param.normalized);
            handler->endEdit(param.id);
        }
    }
}

void PluginController::resetInternalParams() {
    {
        std::lock_guard lock(pending_mutex_);
        pending_params_.clear();
        flush_scheduled_ = false;
    }

    for (auto& param : kParamDefaultsMap)
            setParamNormalized(param.first, param.second);

    if (auto* handler = getComponentHandler()) {
        for (auto& param : kParamDefaultsMap) {
            handler->beginEdit(param.first);
            handler->performEdit(param.first, param.second);
            handler->endEdit(param.first);
        }
    }
}

tresult PLUGIN_API PluginController::getParamStringByValue(const ParamID id, const ParamValue valueNormalized, String128 string)
{
    if (id == kParamColorFrequency) {
        double hz = getHzFromNormalized(valueNormalized);

        char buf[64];
        if (hz >= 1000.0)
            snprintf(buf, sizeof(buf), "%.2f kHz", hz / 1000.0);
        else
            snprintf(buf, sizeof(buf), "%.1f Hz", hz);

        Steinberg::UString(string, 128).fromAscii(buf);
        return kResultTrue;
    }
    return EditController::getParamStringByValue(id, valueNormalized, string);
}

tresult PluginController::getParamValueByString(ParamID id, TChar* string, ParamValue& valueNormalized) {
    if (id == kParamColorFrequency) {
        char buf[64];
        Steinberg::UString(string, 128).toAscii(buf, sizeof(buf));

        try {
            double hz = std::stod(buf);
            hz = std::clamp(hz, kHzLowerBound, kHzUpperBound);

            valueNormalized = (std::log(hz) - std::log(kHzLowerBound)) / (std::log(kHzUpperBound) - std::log(kHzLowerBound));

            return kResultTrue;
        } catch (...) {
            return kResultFalse;
        }
    }
    return EditController::getParamValueByString(id, string, valueNormalized);
}









