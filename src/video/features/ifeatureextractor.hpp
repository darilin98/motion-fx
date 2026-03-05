//
// Created by Darek Rudiš on 12.12.2025.
//

#ifndef IFEATUREEXTRACTOR_HPP
#define IFEATUREEXTRACTOR_HPP

#include "ifeaturesink.hpp"
#include "vstparameters.h"
#include "../videoframe.hpp"

using sink_t = IFeatureSink*;

/**
 * @brief Data type for an update to a single parameter.
 */
struct FeatureParamUpdate {
	Steinberg::Vst::ParamID id;
	double normalized;
};

using param_arr_t = std::vector<FeatureParamUpdate>; /// Combines any amount of updates to parameters.

/**
 * @brief Complete IFeatureExtractor result data type.
 * Attaches a timestamp to a set of updates to parameters.
 */
struct FeatureResult {
	param_arr_t params;
	double timestamp;
};

/**
 * @brief Contract for objects processing features from VideoFrames
 */
class IFeatureExtractor {
public:
	virtual ~IFeatureExtractor() = default;

	/**
	 * @brief Schedules the processing of a VideoFrame.
	 * @param videoFrame Incoming unprocessed VideoFrame.
	 * @warning Must be thread-safe.
	 */
	virtual void processFrame(const VideoFrame& videoFrame) {
		const auto result = extract(videoFrame);
		emitFeature(result);
	}

	/**
	 * Sets the target for extracted results.
	 * @param sink Target for export of FeatureResults
	 */
	virtual void setFeatureSink(const sink_t& sink) { sink_ = sink; }

protected:
	/**
	 * @brief Extracts features from a VideoFrame
	 * @param videoFrame Incoming unprocessed VideoFrame.
	 * @return Resulting changes to parameters.
	 */
	virtual FeatureResult extract(const VideoFrame& videoFrame) = 0;

	/**
	 * @brief Exports the results of extract() to the sink_.
	 * @param result Changes to parameters from extract().
	 */
	virtual void emitFeature(const FeatureResult& result) { if (sink_) sink_->onFeatureResult(result); }
	sink_t sink_ = nullptr;
};


#endif //IFEATUREEXTRACTOR_HPP
