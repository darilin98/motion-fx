//
// Created by Darek Rudiš on 20.03.2025.
//
#include "controller.hpp"
#include "base/ustring.h"

#include "parameterdefaults.hpp"
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

    if (!streamer.readDouble(bypassState))
        return kResultFalse;

    setParamNormalized(kParamBypass, bypassState);

    return kResultOk;
}

tresult PLUGIN_API PluginController::getState(IBStream *state)
{
    if (!state)
        return kResultFalse;

    IBStreamer streamer(state, kLittleEndian);

    bypassState = getParamNormalized(kParamBypass);

    if (!streamer.writeDouble(bypassState))
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
    is_video_preview_mode_ = true;
}

void PluginController::cleanUpPlayback() {
    if (playback_controller_) {
        playback_controller_->shutdown();
        playback_controller_.reset();
    }
    // TODO: Reset param call?
    feature_extractor_.reset();
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





