//
// Created by Darek Rudiš on 13.12.2025.
//

#include "brightnessfeatureextractor.hpp"
#include "vstgui/lib/tasks.h"
#include "../../controller.hpp"
#include <thread>

void BrightnessFeatureExtractor::processFrame(const VideoFrame& videoFrame) {
	if (videoFrame.image.rgba_data.empty())
		return;

	const auto& data = videoFrame.image.rgba_data;
	const size_t numPixels = data.size() / 4;

	if (numPixels == 0) return;

	// Sum R,G,B channels
	uint64_t total = 0;
	for (size_t i = 0; i < data.size(); i += 4) {
		total += static_cast<uint64_t>(data[i]);     // R
		total += static_cast<uint64_t>(data[i + 1]); // G
		total += static_cast<uint64_t>(data[i + 2]); // B
	}

	double normalized = static_cast<double>(total) / (numPixels * 3 * 255);

	VSTGUI::Tasks::schedule(VSTGUI::Tasks::mainQueue(), [normalized, paramId = param_id_, controller = out_controller_]() {
		if (!controller) return;

		if (auto* handler = controller->getComponentHandler()) {
			handler->beginEdit(paramId);
			handler->performEdit(paramId, normalized);
			handler->endEdit(paramId);
		} else {
			controller->setParamNormalized(paramId, normalized);
		}
	});
}

void BrightnessFeatureExtractor::onFrame(const VideoFrame& videoFrame) {
	frame_worker_->enqueueFrame(videoFrame);
}

void BrightnessFeatureExtractor::setOutController(const econt_t &controller) {
	out_controller_ = controller;
}

void BrightnessFeatureExtractor::emitModulation(ModulationPoint point) {
	if (out_controller_) {
		if (auto* pcont = dynamic_cast<PluginController*>(out_controller_.get())) {
			pcont->addModulation(point);
		}
	}
}

