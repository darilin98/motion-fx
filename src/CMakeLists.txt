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
# Find local ffmpeg installation (temporary during dev time)
#-------------------------------------------------------------------------------

if(DEFINED ENV{HOMEBREW_PREFIX})
    set(HOMEBREW_PREFIX $ENV{HOMEBREW_PREFIX})
else()
    set(HOMEBREW_PREFIX /opt/homebrew)
endif()

set(ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:${HOMEBREW_PREFIX}/lib/pkgconfig:${HOMEBREW_PREFIX}/share/pkgconfig")
set(ENV{PATH} "$ENV{PATH}:${HOMEBREW_PREFIX}/bin")

find_package(PkgConfig REQUIRED)

pkg_check_modules(AVCODEC REQUIRED libavcodec)
pkg_check_modules(AVFORMAT REQUIRED libavformat)
pkg_check_modules(AVUTIL REQUIRED libavutil)
pkg_check_modules(SWSCALE REQUIRED libswscale)

set(FFMPEG_INCLUDES
        ${AVCODEC_INCLUDE_DIRS}
        ${AVFORMAT_INCLUDE_DIRS}
        ${AVUTIL_INCLUDE_DIRS}
        ${SWSCALE_INCLUDE_DIRS}
)

set(FFMPEG_LIBS_PKG
        ${AVCODEC_LIBRARIES}
        ${AVFORMAT_LIBRARIES}
        ${AVUTIL_LIBRARIES}
        ${SWSCALE_LIBRARIES}
)

set(FFMPEG_LIB_DIRS
        ${AVCODEC_LIBRARY_DIRS}
        ${AVFORMAT_LIBRARY_DIRS}
        ${AVUTIL_LIBRARY_DIRS}
        ${SWSCALE_LIBRARY_DIRS}
)

message(STATUS "FFmpeg include dirs: ${FFMPEG_INCLUDES}")
message(STATUS "FFmpeg library dirs: ${FFMPEG_LIB_DIRS}")
message(STATUS "FFmpeg pkg-config libs: ${FFMPEG_LIBS_PKG}")

#-------------------------------------------------------------------------------
# Include stb_image headers
#-------------------------------------------------------------------------------

FetchContent_Declare(
        stb_image
        GIT_REPOSITORY https://github.com/nothings/stb.git
        GIT_TAG master
)

FetchContent_MakeAvailable(stb_image)

include_directories(${stb_image_SOURCE_DIR})

#-------------------------------------------------------------------------------
# Include DaisySP
#-------------------------------------------------------------------------------

FetchContent_Declare(
        daisysp
        GIT_REPOSITORY https://github.com/electro-smith/DaisySP.git
        GIT_TAG master
)

FetchContent_MakeAvailable(daisysp)

#-------------------------------------------------------------------------------
# Declare sources and ext. library directories
#-------------------------------------------------------------------------------

add_library(motion_fx_dsp
        audio/effects/ieffect.hpp
        audio/effects/morphfilter.cpp
        audio/effects/morphfilter.hpp
        audio/effects/spatialecho.cpp
        audio/effects/spatialecho.hpp
        audio/effects/wavephaser.cpp
        audio/effects/wavephaser.hpp
)

target_include_directories(motion_fx_dsp PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${VST3_SDK_PATH}
        ${VST3_SDK_PATH}/pluginterfaces
        ${VST3_SDK_PATH}/public.sdk/source/vst/
)

target_link_libraries(motion_fx_dsp PUBLIC DaisySP)

set(SOURCES
        controller.cpp
        controller.hpp
        pluginfactory.cpp
        parameterdefaults.hpp
        audio/processor.cpp
        audio/processor.hpp
        audio/modulationcurve.hpp
        audio/parameterextraction.hpp
        ui/fileselectcontroller.hpp
        ui/motionfxeditor.hpp
        ui/mediaview.cpp
        ui/mediaview.hpp
        ui/mediaviewcreator.cpp
        ui/buttonactioncontroller.hpp
        video/videoframe.hpp
        video/framequeue.hpp
        video/medialoader.cpp
        video/medialoader.hpp
        video/playbackcontroller.cpp
        video/playbackcontroller.hpp
        video/frameticker.cpp
        video/frameticker.hpp
        video/iframereceiver.hpp
        video/videoparamlistener.cpp
        video/videoparamlistener.hpp
        video/decoders/videodecoder.cpp
        video/decoders/videodecoder.hpp
        video/decoders/imagedecoder.cpp
        video/decoders/imagedecoder.hpp
        video/features/asyncframeworker.hpp
        video/features/ifeatureextractor.hpp
        video/features/ifeaturesink.hpp
        video/features/brightnessfeatureextractor.cpp
        video/features/brightnessfeatureextractor.hpp
        video/features/depthfeatureextractor.cpp
        video/features/depthfeatureextractor.hpp
        video/features/motionfeatureextractor.cpp
        video/features/motionfeatureextractor.hpp
        utils.hpp
)

smtg_add_vst3plugin(motion-fx ${SOURCES})

target_link_directories(motion-fx PRIVATE
        ${FFMPEG_LIB_DIRS}
)

target_include_directories(motion-fx PRIVATE
        ${VST3_SDK_PATH}
        ${VST3_SDK_PATH}/pluginterfaces
        ${VST3_SDK_PATH}/public.sdk/source/vst/
        ${VST3_SDK_PATH}/vstgui4
        ${FFMPEG_INCLUDES}
)

target_link_libraries(motion-fx PRIVATE
        motion_fx_dsp
        sdk
        vstgui_support
        ${FFMPEG_LIBS_PKG}
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

#-------------------------------------------------------------------------------
# Bundle FFmpeg dylibs + fix rpaths (macOS Release)
#-------------------------------------------------------------------------------

if(APPLE AND CMAKE_BUILD_TYPE STREQUAL "Release")

    message(STATUS "Bundling FFmpeg libraries into VST3")

    set(PLUGIN_FRAMEWORK_DIR
            $<TARGET_BUNDLE_DIR:motion-fx>/Contents/Frameworks)

    # Ensure Frameworks folder exists
    add_custom_command(TARGET motion-fx POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory ${PLUGIN_FRAMEWORK_DIR}
    )

    set(FFMPEG_DYLIBS
            /opt/homebrew/opt/ffmpeg/lib/libavcodec.62.dylib
            /opt/homebrew/opt/ffmpeg/lib/libavformat.62.dylib
            /opt/homebrew/opt/ffmpeg/lib/libavutil.60.dylib
            /opt/homebrew/opt/ffmpeg/lib/libswscale.9.dylib
    )

    foreach(lib ${FFMPEG_DYLIBS})

        get_filename_component(lib_name ${lib} NAME)

        # Copy dylib into plugin bundle
        add_custom_command(TARGET motion-fx POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                ${lib}
                ${PLUGIN_FRAMEWORK_DIR}/${lib_name}
        )

        # Change the internal ID of the dylib itself
        add_custom_command(TARGET motion-fx POST_BUILD
                COMMAND install_name_tool -id @rpath/${lib_name} ${PLUGIN_FRAMEWORK_DIR}/${lib_name}
        )

        # Update plugin binary to use bundled dylib
        add_custom_command(TARGET motion-fx POST_BUILD
                COMMAND install_name_tool -change ${lib} @loader_path/../Frameworks/${lib_name} $<TARGET_FILE:motion-fx>
        )

        # Optionally ad-hoc codesign the dylib to avoid Gatekeeper warnings
        add_custom_command(TARGET motion-fx POST_BUILD
                COMMAND codesign --force --sign - ${PLUGIN_FRAMEWORK_DIR}/${lib_name}
        )

    endforeach()

    # Ensure plugin knows where to find Frameworks at runtime
    add_custom_command(TARGET motion-fx POST_BUILD
            COMMAND install_name_tool -add_rpath @loader_path/../Frameworks $<TARGET_FILE:motion-fx>
    )

    add_custom_command(TARGET motion-fx POST_BUILD
            COMMAND codesign --force --deep --sign "-" $<TARGET_FILE:motion-fx>
            COMMENT "Code-signing the VST3 bundle for Release"
    )
endif()