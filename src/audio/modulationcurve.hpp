//
// Created by Darek Rudiš on 04.02.2026.
//

#ifndef MODULATIONCURVE_HPP
#define MODULATIONCURVE_HPP
#include <vector>

#include "vst/vsttypes.h"

using param_kv_list_t = std::vector<std::pair<Steinberg::Vst::ParamID, Steinberg::Vst::ParamValue>>;

struct ModulationPointTime {
	double timestamp;
	param_kv_list_t values;
};
struct ModulationPoint {
	Steinberg::Vst::TSamples timestamp;
	param_kv_list_t values;
};

using modulation_time_curve_t = std::vector<ModulationPointTime>;

using modulation_curve_t = std::vector<ModulationPoint>;

#endif //MODULATIONCURVE_HPP
