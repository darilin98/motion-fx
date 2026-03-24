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

    parameters.addParameter(STR16("BrightnessIntensity"), nullptr, 1, ParamDefaults::kIntensity, ParameterInfo::kCanAutomate, kParamBrightnessIntensity);
    parameters.addParameter(STR16("Brightness"), nullptr, 1, ParamDefaults::kBrightness, ParameterInfo::kIsHidden | ParameterInfo::kIsReadOnly,kParamBrightness);

    parameters.addParameter(STR16("DepthIntensity"), nullptr, 1, ParamDefaults::kIntensity, ParameterInfo::kCanAutomate, kParamDepthIntensity);
    parameters.addParameter(STR16("Depth"), nullptr, 1, ParamDefaults::kDepth, ParameterInfo::kIsHidden | ParameterInfo::kIsReadOnly, kParamDepth);

    parameters.addParameter(STR16("MotionIntensity"), nullptr, 1, ParamDefaults::kIntensity, ParameterInfo::kCanAutomate, kParamMotionIntensity);
    parameters.addParameter(STR16("MotionContinuous"), nullptr, 1, ParamDefaults::kMotion, ParameterInfo::kIsHidden | ParameterInfo::kIsReadOnly, kParamMotionContinuous);
    parameters.addParameter(STR16("MotionBurst"), nullptr, 1, ParamDefaults::kMotion, ParameterInfo::kIsHidden | ParameterInfo::kIsReadOnly, kParamMotionBurst);

    parameters.addParameter(STR16("ColorIntensity"), nullptr, 1, ParamDefaults::kColorIntensity, ParameterInfo::kCanAutomate, kParamColorIntensity);
    parameters.addParameter(STR16("ColorRed"), nullptr, 1, ParamDefaults::kColor, ParameterInfo::kIsHidden | ParameterInfo::kIsReadOnly, kParamColorRed);
    parameters.addParameter(STR16("ColorGreen"), nullptr, 1, ParamDefaults::kColor, ParameterInfo::kIsHidden | ParameterInfo::kIsReadOnly, kParamColorGreen);
    parameters.addParameter(STR16("ColorBlue"), nullptr, 1, ParamDefaults::kColor, ParameterInfo::kIsHidden | ParameterInfo::kIsReadOnly, kParamColorBlue);

    parameters.addParameter(STR16("SaturationIntensity"), nullptr, 1, ParamDefaults::kIntensity, ParameterInfo::kCanAutomate, kParamSaturationIntensity);
    parameters.addParameter(STR16("Saturation"), nullptr, 1, ParamDefaults::kSaturation, ParameterInfo::kIsHidden | ParameterInfo::kIsReadOnly, kParamSaturation);
    return kResultOk;
}

tresult PLUGIN_API PluginController::terminate()
{
    return EditController::terminate();
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

    return kResultOk;
}

tresult PLUGIN_API PluginController::getState(IBStream *state)
{
    if(playback_controller_) playback_controller_->stopPipeline();

    if (!state)
        return kResultFalse;

    IBStreamer streamer(state, kLittleEndian);
    if(!streamer.writeInt32(1)) // version
        return kResultFalse;

    auto bypass_state = getParamNormalized(kParamBypass);
    if (!streamer.writeDouble(bypass_state))
        return kResultFalse;

    const std::string pathUtf8 = video_path_.getString();
    if (!streamer.writeStr8(pathUtf8.c_str()))
        return kResultFalse;

    if (!streamer.writeBool(is_video_preview_mode_))
        return kResultFalse;

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
    addExtractor<DepthFeatureExtractor>(kParamDepth);
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
    processorConnection_ = other;
    return kResultOk;
}

void PluginController::onVideoFinished() const {
    if (!processorConnection_)
        return;

    Steinberg::Vst::IMessage* msg = allocateMessage();

    if (!msg)
        return;

    msg->setMessageID("VideoFinished");

    processorConnection_->notify(msg);
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

    // TODO: Here we can cache the results
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

    if (auto* handler = getComponentHandler()) {
        for (auto& param : local) {
            handler->beginEdit(param.id);
            handler->performEdit(param.id, param.normalized);
            handler->endEdit(param.id);
        }
    } else {
        for (auto& param : local)
            setParamNormalized(param.id, param.normalized);
    }
}

void PluginController::resetInternalParams() {
    {
        std::lock_guard lock(pending_mutex_);
        pending_params_.clear();
        flush_scheduled_ = false;
    }

    std::list<std::pair<Steinberg::Vst::ParamID, double>> params {
        {kParamBrightness, ParamDefaults::kBrightness},
        {kParamDepth, ParamDefaults::kDepth},
        {kParamMotionContinuous, ParamDefaults::kMotion},
        {kParamMotionBurst, ParamDefaults::kMotion},
        {kParamColorRed, ParamDefaults::kColor},
        {kParamColorGreen, ParamDefaults::kColor},
        {kParamColorBlue, ParamDefaults::kColor},
        {kParamSaturation, ParamDefaults::kSaturation},
    };

    if (auto* handler = getComponentHandler()) {
        for (auto& param : params) {
            handler->beginEdit(param.first);
            handler->performEdit(param.first, param.second);
            handler->endEdit(param.first);
        }
    } else {
        for (auto& param : params)
            setParamNormalized(param.first, param.second);
    }
}








