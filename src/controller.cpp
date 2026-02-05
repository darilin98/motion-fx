//
// Created by Darek Rudiš on 20.03.2025.
//
#include "controller.hpp"
#include "base/ustring.h"

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
        if (video_is_playing_)
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
    frame_queue_ = std::make_shared<FrameQueue>();

    auto ticker = std::make_unique<FrameTicker>();
    ticker->setQueue(frame_queue_);
    feature_extractor_ = std::make_unique<BrightnessFeatureExtractor>(kParamGain);
    feature_extractor_->setOutController(this);

    playback_controller_ = std::make_shared<PlaybackController>(path.data(), frame_queue_, std::move(ticker));
    playback_controller_->registerReceiver(feature_extractor_.get());
    video_is_playing_ = true;
    playback_controller_->startPipeline(1.0);
}

void PluginController::cleanUpPlayback() {
    playback_controller_->stopPipeline();
    feature_extractor_.reset();
    frame_queue_.reset();
    playback_controller_.reset();
    video_is_playing_ = false;
}

void PluginController::registerReceiver(IFrameReceiver* receiver) const {
    if (playback_controller_) {
        playback_controller_->registerReceiver(receiver);
   }
}

void PluginController::unregisterReceiver(IFrameReceiver *receiver) const {
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

    Steinberg::Vst::IMessage* msg = allocateMessage();
    if (!msg)
        return;

    msg->setMessageID("AddModulationPoint");

    auto* attr = msg->getAttributes();
    attr->setFloat("time", static_cast<float>(modPoint.timestamp));
    attr->setFloat("value", modPoint.values[kParamGain]);

    processorConnection_->notify(msg);
}





