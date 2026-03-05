//
// Created by Darek Rudiš on 08.02.2026.
//

#ifndef PARAMETERDEFAULTS_HPP
#define PARAMETERDEFAULTS_HPP

#include "vst/vsttypes.h"

/**
 * @brief Defines parameters used by the plugin
 */
enum AudioParamID : Steinberg::Vst::ParamID{
    kParamBypass = 101,
    kParamBrightness,
    kParamBrightnessIntensity,
    kParamDepth,
    kParamDepthIntensity
};

enum ControlParamID : Steinberg::Vst::ParamID {
    kParamPlay = 501,
	kParamReset,
	kParamLoop,
};

enum ViewParamID : Steinberg::Vst::ParamID {
    kParamLoadVideo = 1001,
};

inline bool isControlParam(Steinberg::Vst::ParamID id) noexcept {
    switch (id) {
        case kParamPlay:
        case kParamReset:
        case kParamLoop:
            return true;
        default:
            return false;
    }
}

namespace ParamDefaults {
    constexpr double kBrightness = 0.5;
    constexpr double kDepth = 0.0;
    constexpr double kIntensity = 0.5;
}

#endif //PARAMETERDEFAULTS_HPP
