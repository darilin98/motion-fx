//
// Created by Darek Rudiš on 04.02.2026.
//

/**
 * @file modulationcurve.hpp
 * @brief Declares types used for transmitting parameter change cache.
 *
 */

#ifndef MODULATIONCURVE_HPP
#define MODULATIONCURVE_HPP
#include <vector>

#include "vst/vsttypes.h"

using param_kv_list_t = std::vector<std::pair<Steinberg::Vst::ParamID, Steinberg::Vst::ParamValue>>; /// All param-value pairs for a given moment in time.

/**
 * @brief Video-time parameter values datatype
 */
struct ModulationPointTime {
	double timestamp;
	param_kv_list_t values;
};

/**
 * @brief Sample-time parameter values datatype
 */
struct ModulationPoint {
	Steinberg::Vst::TSamples timestamp;
	param_kv_list_t values;
};

using modulation_time_curve_t = std::vector<ModulationPointTime>; /// Full video-time parameter changes representation

using modulation_curve_t = std::vector<ModulationPoint>; /// Full sample-time parameter changes representation

#endif //MODULATIONCURVE_HPP
