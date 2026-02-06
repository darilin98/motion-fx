//
// Created by Darek Rudiš on 06.02.2026.
//

#ifndef PARAMETEREXTRACTION_HPP
#define PARAMETEREXTRACTION_HPP

#include "vst/ivstaudioprocessor.h"
#include "vst/ivstparameterchanges.h"
#include "vst/vsttypes.h"

template <typename Fn>
void forEachLastParamChange(const Steinberg::Vst::ProcessData& data, Fn&& fn)
{
	if (!data.inputParameterChanges)
		return;

	const Steinberg::int32 numParams = data.inputParameterChanges->getParameterCount();
	for (Steinberg::int32 i = 0; i < numParams; ++i)
	{
		auto* queue = data.inputParameterChanges->getParameterData(i);
		if (!queue || queue->getPointCount() == 0)
			continue;

		Steinberg::int32 sample_offset = 0;
		Steinberg::Vst::ParamValue value = 0.0;

		if (queue->getPoint(queue->getPointCount() - 1, sample_offset, value) != Steinberg::kResultTrue)
			continue;

		fn(queue->getParameterId(), value, sample_offset);
	}
}
#endif //PARAMETEREXTRACTION_HPP
