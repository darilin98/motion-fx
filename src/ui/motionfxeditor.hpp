//
// Created by Darek Rudiš on 24.10.2025.
//
#ifndef MOTIONFXEDITOR_HPP
#define MOTIONFXEDITOR_HPP

#include "buttonactioncontroller.hpp"
#include "public.sdk/source/vst/vsteditcontroller.h"
#include "vstgui4/vstgui/plugin-bindings/vst3editor.h"
#include "fileselectcontroller.hpp"
#include "../controller.hpp"

class MediaView;

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

				auto pcont = dynamic_cast<PluginController*>(controller.get());

				pcont->setupPlayback(path);

				this->exchangeView("AudioProcessing");
			};
			return ctrl;
		}
		if (strcmp(name, "ButtonActionController") == 0) {
			fprintf(stderr, "Creating controller of name %s\n", name);
			auto* ctrl = new ButtonActionController(this);
			ctrl->action = [this]() {
				auto pcont = dynamic_cast<PluginController*>(controller.get());
				pcont->cleanUpPlayback();
				fprintf(stderr, "Switching back to  InputSelectView\n");
				this->exchangeView("InputSelect");
			};
			return ctrl;
		}
		return nullptr;
	}
};
#endif //MOTIONFXEDITOR_HPP
