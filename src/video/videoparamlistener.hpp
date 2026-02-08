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

class VideoParamListener : public Steinberg::FObject {
public:
	VideoParamListener(param_t param, pcont_t playbackController, controller_t editController);

	~VideoParamListener() override;

	void PLUGIN_API update(Steinberg::FUnknown* changedUnknown, Steinberg::int32 message) override;

private:
	param_t param_;
	pcont_t playback_controller_;
	controller_t edit_controller_;
};

using listener_t = std::unique_ptr<VideoParamListener>;


#endif //VIDEOPARAMLISTENER_HPP
