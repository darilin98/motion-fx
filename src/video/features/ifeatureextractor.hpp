//
// Created by Darek Rudiš on 12.12.2025.
//

#ifndef IFEATUREEXTRACTOR_HPP
#define IFEATUREEXTRACTOR_HPP

#include "ifeaturesink.hpp"
#include "vstparameters.h"
#include "../videoframe.hpp"

using sink_t = IFeatureSink*;

struct FeatureParamUpdate {
	Steinberg::Vst::ParamID id;
	double normalized;
};

using param_arr_t = std::vector<FeatureParamUpdate>;

struct FeatureResult {
	param_arr_t params;
	double timestamp;
};

class IFeatureExtractor {
public:
	virtual ~IFeatureExtractor() = default;
	virtual void processFrame(const VideoFrame& videoFrame) {
		const auto result = extract(videoFrame);
		emitFeature(result);
	}
	virtual void setFeatureSink(const sink_t& sink) { sink_ = sink; }
protected:
	virtual FeatureResult extract(const VideoFrame& videoFrame) = 0;
	virtual void emitFeature(const FeatureResult& result) { if (sink_) sink_->onFeatureResult(result); }
	sink_t sink_ = nullptr;
};


#endif //IFEATUREEXTRACTOR_HPP
