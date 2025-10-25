//
// Created by Darek Rudiš on 24.10.2025.
//

#ifndef MOTIONFXEDITOR_HPP
#define MOTIONFXEDITOR_HPP

#include "public.sdk/source/vst/vsteditcontroller.h"
#include "vstgui4/vstgui/plugin-bindings/vst3editor.h"
#include "fileselectcontroller.hpp"

class MotionFxEditor : public VSTGUI::VST3Editor
{
public:
	MotionFxEditor(Steinberg::Vst::EditController* controller,
			 const char* templateName, const char* xmlFile)
		: VST3Editor(controller, templateName, xmlFile) {}

	VSTGUI::IController* createSubController(VSTGUI::UTF8StringPtr name, const VSTGUI::IUIDescription* description) override
	{
		if (strcmp(name, "FileSelectController") == 0) {
			auto* ctrl = new FileSelectController(this);
			ctrl->onFileSelected = [this](const VSTGUI::UTF8String& path){
				fprintf(stderr, "Switching to AudioProcessingView for %s\n", path.data());
				this->exchangeView("AudioProcessing");
			};
			return ctrl;
		}
		return nullptr;
	}
};
#endif //MOTIONFXEDITOR_HPP
