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

	tresult PLUGIN_API connect(IConnectionPoint* other) override;

	tresult PLUGIN_API disconnect(IConnectionPoint* other) override;

	tresult PLUGIN_API getParamStringByValue(ParamID id, ParamValue valueNormalized, String128 string) override;

	tresult PLUGIN_API getParamValueByString(ParamID id, TChar* string, ParamValue& valueNormalized) override;

	void setupPlayback(const VSTGUI::UTF8String& path);

	void cleanUpPlayback();

	void registerReceiver(IFrameReceiver* receiver) const;

	void unregisterReceiver(IFrameReceiver* receiver) const;

	void onVideoFinished();

	void onFeatureResult(const FeatureResult& result) override;

private:
	template <typename T, typename... Args>
	void addExtractor(Args&&... args);

	void flushPendingParams();
	void resetInternalParams();
	void instantiateExtractors();
	void registerExtractors() const;
	void sendModulationCacheChunked();

	std::mutex pending_mutex_;
	std::vector<FeatureParamUpdate> pending_params_;
	std::atomic<bool> flush_scheduled_{false};

	std::mutex cache_mutex_;
	modulation_time_curve_t modulation_cache_;
	static constexpr size_t kChunkSize = 512 * 1024;
	static constexpr size_t kMaxCacheSize = 100'000;

	IConnectionPoint* processor_connection_{nullptr};
	bool is_video_preview_mode_ = false;
	VSTGUI::UTF8String video_path_ = "";
	pcont_t playback_controller_ = nullptr;

	std::chrono::time_point<std::chrono::steady_clock> last_get_state_time_;
	extractors_t extractors_;
};

using controller_t = Steinberg::Vst::EditController*;

#endif //CONTROLLER_HPP
