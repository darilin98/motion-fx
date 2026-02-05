//
// Created by Darek Rudiš on 04.02.2026.
//

#ifndef MODULATIONCURVE_HPP
#define MODULATIONCURVE_HPP
#include <unordered_map>

#include "vst/vsttypes.h"

struct ModulationPoint {
	double timestamp;
    std::unordered_map<Steinberg::Vst::ParamID, double> values;
};

using modulation_curve_t = std::vector<ModulationPoint>;

#endif //MODULATIONCURVE_HPP
