//
// Created by Darek Rudiš on 08.02.2026.
//

#ifndef VIDEOPARAMLISTENER_HPP
#define VIDEOPARAMLISTENER_HPP

#include "vstgui/plugin-bindings/vst3editor.h"

class PlaybackController;
using pcont_t = std::shared_ptr<PlaybackController>;
using param_t = Steinberg::Vst::Parameter*;
using controller_t = Steinberg::Vst::EditController*;

/**
 * @brief Listens for changes made by the PluginController to a selected parameter.
 */
class VideoParamListener : public Steinberg::FObject {
public:
	VideoParamListener(param_t param, pcont_t playbackController, controller_t editController);

	~VideoParamListener() override;

	/**
	 * Event triggered when a change to the dependent parameter is registered.
	 * @param changedUnknown Unused.
	 * @param message The type of call.
	 */
	void PLUGIN_API update(Steinberg::FUnknown* changedUnknown, Steinberg::int32 message) override;

private:
	param_t param_; /// Parameter being listened for.
	pcont_t playback_controller_; /// PlaybackController that needs to be called when a change is detected.
	controller_t edit_controller_; /// The PluginController making changes to the parameters.
};

using listener_t = std::unique_ptr<VideoParamListener>;


#endif //VIDEOPARAMLISTENER_HPP
