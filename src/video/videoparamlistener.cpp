//
// Created by Darek Rudiš on 08.02.2026.
//

#include "videoparamlistener.hpp"
#include "playbackcontroller.hpp"

VideoParamListener::VideoParamListener(param_t param, pcont_t playbackController, controller_t editController)
	: param_(param), playback_controller_(playbackController), edit_controller_(editController) {
	if (param)
		param->addDependent(this);
}
VideoParamListener::~VideoParamListener() {
	if (param_)
		param_->removeDependent(this);
}

void VideoParamListener::update(Steinberg::FUnknown* changedUnknown, Steinberg::int32 message) {
	if (message == IDependent::kChanged && playback_controller_ && edit_controller_ && param_) {
		auto normalizedValue = static_cast<float>(edit_controller_->getParamNormalized(param_->getInfo().id));
		playback_controller_->onParamChanged(param_->getInfo().id, normalizedValue);
	}
}
