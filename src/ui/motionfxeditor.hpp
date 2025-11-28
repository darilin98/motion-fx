//
// Created by Darek Rudiš on 24.10.2025.
//
#ifndef MOTIONFXEDITOR_HPP
#define MOTIONFXEDITOR_HPP

#include "buttonactioncontroller.hpp"
#include "public.sdk/source/vst/vsteditcontroller.h"
#include "vstgui4/vstgui/plugin-bindings/vst3editor.h"
#include "fileselectcontroller.hpp"
#include "../video/playbackcontroller.hpp"
#include "../video/framequeue.hpp"

class MotionFxEditor : public VSTGUI::VST3Editor
{
public:
	MotionFxEditor(Steinberg::Vst::EditController* controller,
			 const char* templateName, const char* xmlFile)
		: VST3Editor(controller, templateName, xmlFile) {}

	VSTGUI::IController* createSubController(const VSTGUI::UTF8StringPtr name, const VSTGUI::IUIDescription* description)
	 override
	{
		if (strcmp(name, "FileSelectController") == 0) {
			auto* ctrl = new FileSelectController(this);
			ctrl->onFileSelected = [this](const VSTGUI::UTF8String& path) {
				fprintf(stderr, "Switching to AudioProcessingView for %s\n", path.data());

				// Allocate playback
				// TODO: Remove refs on viewTemplate switch
				frame_queue_ = std::make_shared<FrameQueue>();
				playback_controller_ = std::make_shared<PlaybackController>(path.data(), frame_queue_);

				this->exchangeView("AudioProcessing");
				playback_controller_->startPipeline(1.0);
			};
			return ctrl;
		}
		if (strcmp(name, "ButtonActionController") == 0) {
			fprintf(stderr, "Creating controller of name %s\n", name);
			auto* ctrl = new ButtonActionController(this);
			ctrl->action = [this]() {
				fprintf(stderr, "Switching back to  InputSelectView\n");

				playback_controller_->stopPipeline();
				frame_queue_ = nullptr;
				playback_controller_ = nullptr;

				this->exchangeView("InputSelect");
			};
			return ctrl;
		}
		return nullptr;
	}
	[[nodiscard]] frame_queue_t getFrameQueue() const { return frame_queue_; }
private:
	frame_queue_t frame_queue_ = nullptr;
	pcont_t playback_controller_ = nullptr;
};
#endif //MOTIONFXEDITOR_HPP
