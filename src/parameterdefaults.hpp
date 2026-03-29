//
// Created by Darek Rudiš on 08.02.2026.
//

#ifndef PARAMETERDEFAULTS_HPP
#define PARAMETERDEFAULTS_HPP

#include "vst/vsttypes.h"
#include <vector>

/**
 * @brief Audio processing parameters used by the plugin.
 */
enum AudioParamID : Steinberg::Vst::ParamID {
    kParamBypass = 101,
    kParamBrightness,
    kParamBrightnessIntensity,
    kParamDepth,
    kParamDepthIntensity,
    kParamMotionContinuous,
    kParamMotionBurst,
    kParamMotionIntensity,
    kParamColorRed,
    kParamColorGreen,
    kParamColorBlue,
    kParamColorIntensity,
    kParamSaturation,
    kParamSaturationIntensity,
    kParamColorFrequency,
};

/**
 * @brief Video control parameters used by the plugin.
 */
enum ControlParamID : Steinberg::Vst::ParamID {
    kParamPlay = 501,
	kParamReset,
	kParamLoop,
    kParamPause,
};

/**
 * @brief Utility parameters used by the plugin.
 */
enum ViewParamID : Steinberg::Vst::ParamID {
    kParamLoadVideo = 1001,
};

inline bool isControlParam(Steinberg::Vst::ParamID id) noexcept {
    switch (id) {
        case kParamPlay:
        case kParamReset:
        case kParamLoop:
        case kParamPause:
            return true;
        default:
            return false;
    }
}

constexpr double kHzLowerBound = 55.0;
constexpr double kHzUpperBound = 10000.0;

inline double getHzFromNormalized(const double normalized) {
    return std::exp(std::log(kHzLowerBound) + normalized * (std::log(kHzUpperBound) - std::log(kHzLowerBound)));
}

inline double getNormalizedFromHz(const double hz) {
    return (std::log(hz) - std::log(kHzLowerBound)) / (std::log(kHzUpperBound) - std::log(kHzLowerBound));
}

/**
 * @brief Encapsulates the default values of plugin parameters.
 */
namespace ParamDefaults {
    constexpr double kBrightness = 0.5;
    constexpr double kDepth = 0.0;
    constexpr double kIntensity = 0.5;
    constexpr double kColorIntensity = 0.0;
    constexpr double kMotion = 0.0;
    constexpr double kColor = 0.0;
    constexpr double kSaturation = 0.0;
    const double kColorFreq = getNormalizedFromHz(440.0);
}

static const std::vector<std::pair<Steinberg::Vst::ParamID, double>> kParamDefaultsMap {
            {kParamBrightness, ParamDefaults::kBrightness},
            {kParamDepth, ParamDefaults::kDepth},
            {kParamMotionContinuous, ParamDefaults::kMotion},
            {kParamMotionBurst, ParamDefaults::kMotion},
            {kParamColorRed, ParamDefaults::kColor},
            {kParamColorGreen, ParamDefaults::kColor},
            {kParamColorBlue, ParamDefaults::kColor},
            {kParamSaturation, ParamDefaults::kSaturation},
};

#endif //PARAMETERDEFAULTS_HPP
