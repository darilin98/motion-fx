//
// Created by Darek Rudiš on 20.03.2025.
//

#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

#include <mutex>
#include "base/source/fstring.h"
#include "public.sdk/source/vst/vsteditcontroller.h"
#include "vstgui4/vstgui/plugin-bindings/vst3editor.h"
#include "public.sdk/source/main/moduleinit.h"
#include "video/playbackcontroller.hpp"
#include "audio/modulationcurve.hpp"
#include "video/features/ifeatureextractor.hpp"


using namespace Steinberg::Vst;
using namespace Steinberg;

using extractors_t = std::vector<std::unique_ptr<IFeatureExtractor>>;

/**
 * @brief Custom implementation of the VSTSDK @ref EditController
 *
 * Manages plugin states and parameter values. Communicates with a VSTGUI instance.
 */
class PluginController : public EditController, IFeatureSink {
public:
	PluginController() = default;

	~PluginController() override = default;

	static FUnknown* createInstance(void*) { return static_cast<IEditController*>(new PluginController()); }
	/**
	 * @brief Initializes a custom root view as part of VSTGUI defined in: @file resources/viewGUI.uidesc
	 * @return Newly created view
	 */
	IPlugView* PLUGIN_API createView(FIDString name) SMTG_OVERRIDE;

	/**
	 * @brief Creates a controller instance and defines all of its parameters
	 * @return State based on the validity of controller creation
	 */
	tresult PLUGIN_API initialize(FUnknown* context) SMTG_OVERRIDE;

	/**
	 * @brief Tears down all artefacts in a safe manner.
	 * @return Method sucess status.
	 */
	tresult PLUGIN_API terminate() SMTG_OVERRIDE;

	/**
	 * @brief Tries to update internal bypass state based on the values of @kParamBypass
	 */
	tresult PLUGIN_API getState(IBStream* state) SMTG_OVERRIDE;

	/**
	 * @brief Tries to update @kParamBypass by the state of the stream
	 */
	tresult PLUGIN_API setState(IBStream*) SMTG_OVERRIDE;

	tresult PLUGIN_API setParamNormalized(ParamID tag, ParamValue value) SMTG_OVERRIDE;

	tresult PLUGIN_API connect(IConnectionPoint* other) SMTG_OVERRIDE;

	tresult PLUGIN_API disconnect(IConnectionPoint* other) SMTG_OVERRIDE;

	tresult PLUGIN_API getParamStringByValue(ParamID id, ParamValue valueNormalized, String128 string) SMTG_OVERRIDE;

	tresult PLUGIN_API getParamValueByString(ParamID id, TChar* string, ParamValue& valueNormalized) SMTG_OVERRIDE;

	/**
	 * @brief Instantiates all objects required for the video pipeline to run.
	 * @param path Input media file path.
	 */
	void setupPlayback(const VSTGUI::UTF8String& path);

	/**
	 * @brief Destroys all members of the video pipeline.
	 */
	void cleanUpPlayback();

	void registerReceiver(IFrameReceiver* receiver) const;

	void unregisterReceiver(IFrameReceiver* receiver) const;

	/**
	 * @brief Sends out cache of a fully processed video.
	 */
	void onVideoFinished();

	/**
	 * @brief Callback for
	 * @param result Parameter outputs coupled with a frame timestamp.
	 */
	void onFeatureResult(const FeatureResult& result) override;

private:
	/**
	 * @brief Simplifies the process of adding a new FeatureExtractor to the extractors_ list.
	 * @tparam T Extractor type.
	 * @tparam Args Any constructor arguments.
	 * @param args Extractor constructor arguments.
	 */
	template <typename T, typename... Args>
	void addExtractor(Args&&... args);

	/**
	 * @brief Task sending out automation from the pending_params_ buffer.
	 */
	void flushPendingParams();

	/**
	 * @brief Sets all feature extraction related parameters to their default values.
	 *
	 * Calls automation and local controller parameter changes.
	 */
	void resetInternalParams();

	/**
	 * @brief Clears and fills the extractors_ list with all configured extractors.
	 */
	void instantiateExtractors();

	/**
	 * @brief Adds each member of the extractors_ list to the video pipeline mechanism.
	 */
	void registerExtractors() const;

	/**
	 * @brief Serializes modulation_cache_ and sends it via processor_connection_.
	 */
	void sendModulationCacheChunked();

	/**
	 * @brief Invalidates stale extractor tasks.
	 *
	 * Called during plugin destruction.
	 * Prevents crashes related to stale tasks in VSTGUI::Tasks::mainQueue
	 */
	void invalidateScheduledTasks();

	std::mutex pending_mutex_; /// Guards automation call buffer.
	std::vector<FeatureParamUpdate> pending_params_; /// Automation call buffer.
	std::atomic<bool> flush_scheduled_{false}; /// Limits amount of automation calls scheduled.
	std::shared_ptr<std::atomic<bool>> schedule_token_ = std::make_shared<std::atomic<bool>>(true); /// Invalidates stale extractor tasks.
	std::atomic<bool> vstgui_scheduling_active_ { true }; /// Toggles extractor output processing.

	std::mutex cache_mutex_; /// Mutex guarding modulation_cache_ edits.
	modulation_time_curve_t modulation_cache_; /// Cache of parameter changes in the time domain.
	static constexpr size_t kChunkSize = 512 * 1024; /// Size of an IMessage chunk.
	static constexpr size_t kMaxCacheSize = 100'000; /// Limit to parameter change cache size.

	IConnectionPoint* processor_connection_{nullptr}; /// IMessage connection to the PluginProcessor.
	bool is_video_preview_mode_ = false; /// State of the GUI.
	VSTGUI::UTF8String video_path_ = ""; /// Location of a selected media file.
	pcont_t playback_controller_ = nullptr; /// Video pipeline controller.

	extractors_t extractors_; /// List of visual feature extractors.
};

using controller_t = Steinberg::Vst::EditController*;

#endif //CONTROLLER_HPP
