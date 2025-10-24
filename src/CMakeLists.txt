#-------------------------------------------------------------------------------
# Fetch Steinberg VST3 SDK
#-------------------------------------------------------------------------------

set(SMTG_ENABLE_VST3_PLUGIN_EXAMPLES OFF CACHE BOOL "" FORCE)

# Fetch the VST3 SDK
FetchContent_Declare(
        vst3sdk
        GIT_REPOSITORY https://github.com/steinbergmedia/vst3sdk.git
        GIT_TAG master
)
FetchContent_MakeAvailable(vst3sdk)

set(VST3_SDK_PATH ${vst3sdk_SOURCE_DIR})
smtg_enable_vst3_sdk()

#-------------------------------------------------------------------------------
# Declare sources and ext. library directories
#-------------------------------------------------------------------------------

set(SOURCES
        controller.cpp
        controller.hpp
        pluginfactory.cpp
        audio/processor.cpp
        audio/processor.hpp
        ui/fileselectcontroller.hpp
        ui/motionfxeditor.hpp
        utils.hpp
)

smtg_add_vst3plugin(motion-fx ${SOURCES})

target_include_directories(motion-fx PRIVATE
        ${VST3_SDK_PATH}
        ${VST3_SDK_PATH}/pluginterfaces
        ${VST3_SDK_PATH}/public.sdk/source/vst/
        ${VST3_SDK_PATH}/vstgui4
)


target_link_libraries(motion-fx PRIVATE sdk
                                         vstgui_support
)

#-------------------------------------------------------------------------------
# Configure plugin output format
#-------------------------------------------------------------------------------

smtg_target_configure_version_file(motion-fx)

# Specify the plugin format, version, etc.
if (APPLE)
    smtg_target_set_bundle(motion-fx
            BUNDLE_IDENTIFIER "github.com/darilin98/motion-fx"
            COMPANY_NAME "Darek Rudiš"
    )
elseif (WIN32)
    target_sources(motion-fx
            PRIVATE
            resources/win32resource.rc
    )
    if(MSVC)
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT motion-fx)

        smtg_target_set_debug_executable(motion-fx
                "$(ProgramW6432)/Steinberg/VST3PluginTestHost/VST3PluginTestHost.exe"
                "--pluginfolder \"$(OutDir)/\""
        )
    endif()

endif()

#-------------------------------------------------------------------------------
# Auto-copy view files
#-------------------------------------------------------------------------------

smtg_target_add_plugin_resources(motion-fx
        RESOURCES
        "resources/viewGUI.uidesc"
)
