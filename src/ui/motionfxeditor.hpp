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
#include "../video/playbackcontroller.hpp"
#include "../video/framequeue.hpp"
#include "../video/frameticker.hpp"
#include "../video/features/brightnessfeatureextractor.hpp"

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

				// Allocate playback
				frame_queue_ = std::make_shared<FrameQueue>();

				auto ticker = std::make_unique<FrameTicker>();
				ticker->setQueue(frame_queue_);

				feature_extractor_ = std::make_unique<BrightnessFeatureExtractor>(kParamGain);
				feature_extractor_->setOutController(controller);
				playback_controller_ = std::make_shared<PlaybackController>(path.data(), frame_queue_, std::move(ticker));

				this->exchangeView("AudioProcessing");
			};
			return ctrl;
		}
		if (strcmp(name, "ButtonActionController") == 0) {
			fprintf(stderr, "Creating controller of name %s\n", name);
			auto* ctrl = new ButtonActionController(this);
			ctrl->action = [this]() {
				playback_controller_->stopPipeline();
				feature_extractor_.reset();
				frame_queue_.reset();
				playback_controller_.reset();

				fprintf(stderr, "Switching back to  InputSelectView\n");
				this->exchangeView("InputSelect");
			};
			return ctrl;
		}
		return nullptr;
	}
	[[nodiscard]] frame_queue_t getFrameQueue() const { return frame_queue_; }
	void setMediaView(MediaView* view) const {
		if (playback_controller_) {
			playback_controller_->registerReceiver(view);
			playback_controller_->registerReceiver(feature_extractor_.get());
			playback_controller_->startPipeline(1.0);
		}
	}
private:
	std::unique_ptr<BrightnessFeatureExtractor> feature_extractor_ = nullptr;
	frame_queue_t frame_queue_ = nullptr;
	pcont_t playback_controller_ = nullptr;
};
#endif //MOTIONFXEDITOR_HPP
