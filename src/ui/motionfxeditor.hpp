//
// Created by Darek Rudiš on 24.10.2025.
//

#ifndef MOTIONFXEDITOR_HPP
#define MOTIONFXEDITOR_HPP

#include "public.sdk/source/vst/vsteditcontroller.h"
#include "vstgui4/vstgui/plugin-bindings/vst3editor.h"
#include "fileselectcontroller.hpp"
#include "../video/medialoader.hpp"

using loader_t = std::shared_ptr<MediaLoader>;

class MotionFxEditor : public VSTGUI::VST3Editor
{
public:
	MotionFxEditor(EditController* controller,
			 const char* templateName, const char* xmlFile)
		: VST3Editor(controller, templateName, xmlFile) {}

	IController* createSubController(const VSTGUI::UTF8StringPtr name, const VSTGUI::IUIDescription* description)
	 override
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
private:
	loader_t loader_;
};
#endif //MOTIONFXEDITOR_HPP
