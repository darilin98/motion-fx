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
        ui/mediaview.cpp
        ui/mediaview.hpp
        video/videoframe.hpp
        video/medialoader.cpp
        video/medialoader.hpp
        video/decoders/videodecoder.cpp
        video/decoders/videodecoder.hpp
        video/decoders/imagedecoder.cpp
        video/decoders/imagedecoder.cpp
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
