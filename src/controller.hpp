//
// Created by Darek Rudiš on 20.03.2025.
//

#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

#include "base/source/fstring.h"
#include "public.sdk/source/vst/vsteditcontroller.h"
#include "vstgui4/vstgui/plugin-bindings/vst3editor.h"
#include "public.sdk/source/main/moduleinit.h"
#include "video/playbackcontroller.hpp"
#include "video/features/brightnessfeatureextractor.hpp"
#include "audio/modulationcurve.hpp"
//#include "parameterdefaults.hpp"


using namespace Steinberg::Vst;
using namespace Steinberg;


/**
 * @brief Custom implementation of the VSTSDK @ref EditController
 *
 * Manages plugin states and parameter values. Communicates with a VSTGUI instance.
 */
class PluginController : public EditController {
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
    tresult PLUGIN_API setParamNormalized(ParamID tag, ParamValue value) SMTG_OVERRIDE;

    void setupPlayback(const VSTGUI::UTF8String& path);
    void cleanUpPlayback();

    [[nodiscard]] frame_queue_t getFrameQueue() const { return frame_queue_; }
    void registerReceiver(IFrameReceiver* receiver) const;
    void unregisterReceiver(IFrameReceiver* receiver) const;

    void addModulation(ModulationPoint modPoint) const;
    tresult PLUGIN_API connect(IConnectionPoint* other) override;

    ParamValue bypassState = 0.0;
private:
    IConnectionPoint* processorConnection_ {nullptr};
    bool is_video_preview_mode_ = false;
    pcont_t playback_controller_ = nullptr;
    frame_queue_t frame_queue_ = nullptr;
    std::unique_ptr<BrightnessFeatureExtractor> feature_extractor_ = nullptr;
};

using controller_t = Steinberg::Vst::EditController*;

#endif //CONTROLLER_HPP
