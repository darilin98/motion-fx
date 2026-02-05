//
// Created by Darek Rudiš on 12.12.2025.
//

#ifndef IFEATUREEXTRACTOR_HPP
#define IFEATUREEXTRACTOR_HPP

#include "vsteditcontroller.h"
#include "../videoframe.hpp"
#include "base/smartpointer.h"
#include "../../audio/modulationcurve.hpp"

using econt_t = Steinberg::IPtr<Steinberg::Vst::EditController>;
class IFeatureExtractor {
public:
	virtual ~IFeatureExtractor() = default;
	virtual void processFrame(const VideoFrame& videoFrame) = 0;
	virtual void setOutController(const econt_t& controller) = 0;
private:
	virtual void emitModulation(ModulationPoint point) = 0;
};


#endif //IFEATUREEXTRACTOR_HPP
