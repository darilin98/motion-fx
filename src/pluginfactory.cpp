//
// Created by Darek Rudiš on 20.03.2025.
//

/**
 * @file pluginfactory.cpp
 * @brief States essential plugin metadata for the host.
 */

#include "public.sdk/source/main/pluginfactory.h"
#include "public.sdk/source/vst/utility/stringconvert.h"
#include "public.sdk/source/vst/vstcomponent.h"
#include "public.sdk/source/vst/vsteditcontroller.h"
#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/base/ustring.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "processor.hpp"
#include "controller.hpp"

using namespace Steinberg::Vst;

BEGIN_FACTORY_DEF("Darek Rudiš",
                  "https://github.com/darilin98",
                  "darekrudis@email.cz")

    // Register the processor
    DEF_CLASS2(INLINE_UID_FROM_FUID(PluginProcessorUID),
               PClassInfo::kManyInstances, // multiple instances allowed
               kVstAudioEffectClass,       // Component category
               "Motion Fx",      // Plugin name
               Vst::kDistributable,        // Distributable attribute
               "Fx|Synth",                 // Subcategories
               "1.0.0",                     // Version
               "Darek Rudiš",                // Vendor
               PluginProcessor::createInstance)

    // Register the controller
    DEF_CLASS2(INLINE_UID_FROM_FUID(PluginControllerUID),
               PClassInfo::kManyInstances,
               kVstComponentControllerClass,
               "Motion Fx Controller",
               0,
               "",
               "1.0.0",
               "Darek Rudiš",
               PluginController::createInstance)

END_FACTORY

