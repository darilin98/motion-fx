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
    parameters.addParameter(STR16("UiControl"), nullptr, 1, 0.0,ParameterInfo::kCanAutomate | ParameterInfo::kIsList,
     kParamLoadVideo);
    parameters.addParameter(STR16("Bypass"), nullptr, 1, 0.0, ParameterInfo::kIsBypass | ParameterInfo::kCanAutomate | ParameterInfo::kIsList, kParamBypass);
    parameters.addParameter(STR16("Gain"), nullptr, 1, 0.5, ParameterInfo::kIsHidden | ParameterInfo::kIsReadOnly,kParamGain);
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
        return new MotionFxEditor (this, "InputSelect", "viewGUI.uidesc");
    }
    return nullptr;
}

tresult PLUGIN_API PluginController::setParamNormalized(ParamID tag, ParamValue value)
{
    if (tag == kParamGain)
    {
        fprintf(stderr, ">>> setParamNormalized(kParamGain, %f)\n", value);
    }
    return EditController::setParamNormalized(tag, value);
}
