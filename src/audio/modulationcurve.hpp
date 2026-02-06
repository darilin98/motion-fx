//
// Created by Darek Rudiš on 04.02.2026.
//

#ifndef MODULATIONCURVE_HPP
#define MODULATIONCURVE_HPP
#include <vector>

#include "vst/vsttypes.h"

struct ModulationPoint {
	Steinberg::Vst::TSamples timestamp;
    std::vector<std::pair<Steinberg::Vst::ParamID, Steinberg::Vst::ParamValue>> values;
};

using modulation_curve_t = std::vector<ModulationPoint>;

#endif //MODULATIONCURVE_HPP
