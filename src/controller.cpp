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
    parameters.addParameter(STR16("Bypass"), nullptr, 1, 0.0, ParameterInfo::kIsBypass | ParameterInfo::kCanAutomate | ParameterInfo::kIsList, kParamBypass);
    parameters.addParameter(STR16("Gain"), nullptr, 1, 1, ParameterInfo::kIsHidden | ParameterInfo::kIsReadOnly,kParamGain);

    return kResultOk;
}

tresult PLUGIN_API PluginController::terminate()
{
    return EditController::terminate();
}

tresult PLUGIN_API PluginController::setState(IBStream *state)
{
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
    if (tag == kParamGain)
    {
        // fprintf(stderr, ">>> setParamNormalized(kParamGain, %f)\n", value);
    }
    return EditController::setParamNormalized(tag, value);
}

void PluginController::setupPlayback(const VSTGUI::UTF8String& path) {
    auto frame_queue = std::make_shared<FrameQueue>();

    auto ticker = std::make_unique<FrameTicker>();
    ticker->setQueue(frame_queue);

    feature_extractor_ = std::make_unique<BrightnessFeatureExtractor>(kParamGain);
    feature_extractor_->setOutController(this);

    playback_controller_ = std::make_shared<PlaybackController>(path.data(), std::move(frame_queue), std::move(ticker));
    playback_controller_->registerReceiver(feature_extractor_.get());
    playback_controller_->setParamListeners(this);

    video_path_ = path;
    is_video_preview_mode_ = true;
}

void PluginController::cleanUpPlayback() {
    if (playback_controller_) {
        playback_controller_->shutdown();
        playback_controller_.reset();
    }
    // TODO: Reset param call?
    feature_extractor_.reset();

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

void PluginController::addModulation(ModulationPoint modPoint) const {
    if (!processorConnection_)
        return;
}





