//
// Created by Darek Rudiš on 20.03.2025.
//

#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

#include "base/source/fstring.h"
#include "public.sdk/source/vst/vsteditcontroller.h"
#include "vstgui4/vstgui/plugin-bindings/vst3editor.h"
#include "public.sdk/source/main/moduleinit.h"


using namespace Steinberg::Vst;
using namespace Steinberg;

/**
 * @brief Defines parameters used by the plugin
 */
enum AudioParamID : ParamID{
    kParamBypass = 101,
};

enum ViewParamID : ParamID {
    kParamLoadVideo = 1001,
};
/**
 * @brief Custom implementation of the VSTSDK @ref EditController
 *
 * Manages plugin states and parameter values. Communicates with a VSTGUI instance.
 */
class PluginController : public EditController
{
public:
    PluginController() = default;
    ~PluginController() override = default;

    static FUnknown* createInstance(void*) { return static_cast<IEditController *>(new PluginController()); }
    /**
     * @brief Initializes a custom root view as part of VSTGUI defined in: @file resources/viewGUI.uidesc
     * @return Newly created view
     */
    IPlugView* PLUGIN_API createView (FIDString name) SMTG_OVERRIDE;
    /**
     * @brief Creates a controller instance and defines all of its parameters
     * @return State based on the validity of controller creation
     */
    tresult PLUGIN_API initialize(FUnknown* context) SMTG_OVERRIDE;
    tresult PLUGIN_API terminate() SMTG_OVERRIDE;
    /**
     * @brief Tries to update internal bypass state based on the values of @kParamBypass
     */
    tresult PLUGIN_API getState(IBStream *state) SMTG_OVERRIDE;
    /**
     * @brief Tries to update @kParamBypass by the state of the stream
     */
    tresult PLUGIN_API setState(IBStream*) SMTG_OVERRIDE;
    /**
     * @brief Handles translation between parameter float values and their name representations inside selector lists.
     * @param id Type of parameter being handled
     * @param valueNormalized Current value of the parameter
     * @param string String representing the translated name
     * @return
     */

    ParamValue bypassState = 0.0;
};
#endif //CONTROLLER_HPP
