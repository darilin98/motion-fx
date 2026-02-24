//
// Created by Darek Rudiš on 24.02.2026.
//

#ifndef IFEATURESINK_HPP
#define IFEATURESINK_HPP

struct FeatureResult;

class IFeatureSink {
public:
	virtual ~IFeatureSink() = default;

	virtual void onFeatureResult(const FeatureResult& result) = 0;
};

#endif //IFEATURESINK_HPP
