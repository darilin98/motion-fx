#-------------------------------------------------------------------------------
# Fetch Steinberg VST3 SDK
#-------------------------------------------------------------------------------

set(SMTG_ENABLE_VST3_PLUGIN_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SMTG_ENABLE_VST3_HOSTING_EXAMPLES ON CACHE BOOL "" FORCE)
set(SMTG_CREATE_PLUGIN_LINK ON CACHE BOOL "" FORCE)

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
# Find local ffmpeg installation
#-------------------------------------------------------------------------------

if(WIN32)
    set(VCPKG_APPLOCAL_DEPS OFF)
    # vcpkg path — find_package works when CMAKE_TOOLCHAIN_FILE points to vcpkg
    find_package(FFMPEG REQUIRED COMPONENTS avcodec avformat avutil swscale)
    set(FFMPEG_INCLUDES  ${FFMPEG_INCLUDE_DIRS})
    set(FFMPEG_LIB_DIRS  ${FFMPEG_LIBRARY_DIRS})
    set(FFMPEG_LIBS_PKG  ${FFMPEG_LIBRARIES})

else()
    # macOS / Linux: use pkg-config via Homebrew or system install ---

    if(APPLE)
        if(DEFINED ENV{HOMEBREW_PREFIX})
            set(HOMEBREW_PREFIX $ENV{HOMEBREW_PREFIX})
        else()
            set(HOMEBREW_PREFIX /opt/homebrew)
        endif()

        set(ENV{PKG_CONFIG_PATH}
                "$ENV{PKG_CONFIG_PATH}:${HOMEBREW_PREFIX}/lib/pkgconfig:${HOMEBREW_PREFIX}/share/pkgconfig")
        set(ENV{PATH} "$ENV{PATH}:${HOMEBREW_PREFIX}/bin")
    endif()

    find_package(PkgConfig REQUIRED)

    pkg_check_modules(AVCODEC  REQUIRED libavcodec)
    pkg_check_modules(AVFORMAT REQUIRED libavformat)
    pkg_check_modules(AVUTIL   REQUIRED libavutil)
    pkg_check_modules(SWSCALE  REQUIRED libswscale)

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
endif()

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

add_library(visimusic_dsp
        audio/effects/ieffect.hpp
        audio/effects/morphfilter.cpp
        audio/effects/morphfilter.hpp
        audio/effects/spatialecho.cpp
        audio/effects/spatialecho.hpp
        audio/effects/wavephaser.cpp
        audio/effects/wavephaser.hpp
        audio/effects/ringmodulator.cpp
        audio/effects/ringmodulator.hpp
        audio/effects/saturationexciter.cpp
        audio/effects/saturationexciter.hpp
        audio/effects/NoiseMaker.cpp
        audio/effects/NoiseMaker.hpp
)

target_include_directories(visimusic_dsp PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${VST3_SDK_PATH}
        ${VST3_SDK_PATH}/pluginterfaces
        ${VST3_SDK_PATH}/public.sdk/source/vst/
)

target_link_libraries(visimusic_dsp PUBLIC DaisySP)

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
        ui/VisiMusicEditor.hpp
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
        video/features/colorfeatureextractor.cpp
        video/features/colorfeatureextractor.hpp
        video/features/saturationfeatureextractor.cpp
        video/features/saturationfeatureextractor.hpp
        utils.hpp
)

smtg_add_vst3plugin(visimusic ${SOURCES})

target_link_directories(visimusic PRIVATE
        ${FFMPEG_LIB_DIRS}
)

target_include_directories(visimusic PRIVATE
        ${VST3_SDK_PATH}
        ${VST3_SDK_PATH}/pluginterfaces
        ${VST3_SDK_PATH}/public.sdk/source/vst/
        ${VST3_SDK_PATH}/vstgui4
        ${FFMPEG_INCLUDES}
)

target_link_libraries(visimusic PRIVATE
        visimusic_dsp
        sdk
        vstgui_support
        ${FFMPEG_LIBS_PKG}
)

#-------------------------------------------------------------------------------
# Configure plugin output format
#-------------------------------------------------------------------------------

smtg_target_configure_version_file(visimusic)

# Specify the plugin format, version, etc.
if (APPLE)
    smtg_target_set_bundle(visimusic
            BUNDLE_IDENTIFIER "github.com/darilin98/visimusic"
            COMPANY_NAME "Darek Rudiš"
    )
elseif (WIN32)
    target_sources(visimusic
            PRIVATE
            resources/win32resource.rc
    )
    if(MSVC)
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT visimusic)

        smtg_target_set_debug_executable(visimusic
                "$(ProgramW6432)/Steinberg/VST3PluginTestHost/VST3PluginTestHost.exe"
                "--pluginfolder \"$(OutDir)/\""
        )
    endif()

endif()

#-------------------------------------------------------------------------------
# Auto-copy view files
#-------------------------------------------------------------------------------

smtg_target_add_plugin_resources(visimusic
        RESOURCES
        "resources/button_bypass_37x37.png"
        "resources/on_off_button_20x20.png"
        "resources/viewGUI.uidesc"
)

#-------------------------------------------------------------------------------
# Bundle FFmpeg dylibs
#-------------------------------------------------------------------------------

if(WIN32)
    if(MSVC)
        target_sources(visimusic PRIVATE windllload.cpp)
        target_link_libraries(visimusic PRIVATE delayimp.lib)
        target_link_options(visimusic PRIVATE
                /DELAYLOAD:avcodec-62.dll
                /DELAYLOAD:avformat-62.dll
                /DELAYLOAD:avutil-60.dll
                /DELAYLOAD:swscale-9.dll
                /DELAYLOAD:swresample-6.dll
        )
    endif()

    list(GET FFMPEG_LIB_DIRS 0 FFMPEG_LIB_DIR_RELEASE)
    get_filename_component(VCPKG_BIN "${FFMPEG_LIB_DIR_RELEASE}/../bin" ABSOLUTE)
    message(STATUS "VCPKG bin dir: ${VCPKG_BIN}")

    file(GLOB FFMPEG_DLLS
            "${VCPKG_BIN}/*.dll"
    )
    message(STATUS "FFmpeg DLLs found: ${FFMPEG_DLLS}")

    foreach(dll ${FFMPEG_DLLS})
        add_custom_command(TARGET visimusic POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:visimusic>/../../Frameworks"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${dll} "$<TARGET_FILE_DIR:visimusic>/../../Frameworks"
                COMMENT "Copying ${dll} to Frameworks"
        )
    endforeach()
endif()

if(APPLE AND CMAKE_BUILD_TYPE STREQUAL "Release")
    add_custom_command(TARGET visimusic POST_BUILD
            COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/../release.sh
            $<TARGET_BUNDLE_DIR:visimusic>
            COMMENT "Bundling dylibs, signing, and creating DMG..."
    )
endif()