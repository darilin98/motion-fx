//
// Created by Darek Rudiš on 24.02.2026.
//

#ifndef IFEATURESINK_HPP
#define IFEATURESINK_HPP

struct FeatureResult;

/**
 * @brief A common point for all IFeatureExtractor implementing objects to publish their results.
 */
class IFeatureSink {
public:
	virtual ~IFeatureSink() = default;

	/**
	 * An event to-be called when a FeatureExtractor has finished a job on a VideoFrame.
	 * @param result Result of a processed VideoFrame by an object implementing IFeatureExtractor.
	 */
	virtual void onFeatureResult(const FeatureResult& result) = 0;
};

#endif //IFEATURESINK_HPP
