//
// Created by Darek Rudiš on 08.11.2025.
//


#include "playbackcontroller.hpp"
#include "../controller.hpp"
#include "videoparamlistener.hpp"
#include "vstgui/lib/tasks.h"
#include "../parameterdefaults.hpp"

PlaybackController::~PlaybackController() = default;

constexpr int kMaxQueueSize = 120;

void PlaybackController::shutdown() {
	stopPipeline();

	frame_queue_->clear();

	{
		std::lock_guard lock(completion_mutex_);
		completion_signal_.store(CompletionSignal::Shutdown);
	}
	completion_cv_.notify_all();

	if (completion_thread_.joinable())
		completion_thread_.join();

	if (frame_ticker_)
		frame_ticker_->clearReceivers();

	play_listener_.reset();
	reset_listener_.reset();
	pause_listener_.reset();
}

void PlaybackController::startPipeline(const double playbackRate) {
	rate_ = playbackRate;

	if (!completion_thread_.joinable()) {
		completion_thread_ = std::thread([this] { completionLoop(); });
	}

	setupCallbacks();

	if (loader_) {
		is_decoding_.store(true);
		loader_->startLoading();
	}

	if (frame_ticker_) {
		frame_ticker_->resetTimer();
		is_playing_.store(true);
		frame_ticker_->startConsumingAt(30);
	}
}

void PlaybackController::stopPipeline() {
	if (frame_ticker_) {
        frame_ticker_->setOnQueueEmptyCallback(nullptr);
    }

	if (loader_) {
		is_decoding_.store(false);
		loader_->stopLoading();
		loader_->onFrame = nullptr;
	}

	if (frame_ticker_) {
		frame_ticker_->stopConsuming();
		is_playing_.store(false);
	}
}

void PlaybackController::setupCallbacks() {
	if (loader_) {
		loader_->onFrame = [this](VideoFrame&& frame) {
			while (!frame_queue_->tryPush(std::move(frame))) {
				std::this_thread::sleep_for(std::chrono::milliseconds(2));
				if (!is_decoding_) return;
			}
		};
		loader_->onVideoFinish = [this]() {
			is_decoding_.store(false);
		};
	}

	if (frame_ticker_) {
		frame_ticker_->setOnQueueEmptyCallback([this] {
			if (!is_decoding_.load()) {
				std::lock_guard lock(completion_mutex_);
				completion_signal_.store(CompletionSignal::Handle);
				completion_cv_.notify_one();
			}
		});
	}
}

void PlaybackController::completionLoop() {
	while (true) {
		std::unique_lock lock(completion_mutex_);
		completion_cv_.wait(lock, [this] {
			return completion_signal_.load() != CompletionSignal::None;
		});

		auto signal = completion_signal_.exchange(CompletionSignal::None);
		lock.unlock();

		if (signal == CompletionSignal::Shutdown) return;

		onVideoFinished();

		if (looping_ && loader_ && loader_->tryRewindToStart()) {
			is_decoding_.store(true);
			loader_->startLoading();
			if (frame_ticker_) frame_ticker_->resetTimer();
			continue;
		}

		stopPipeline();
	}
}

void PlaybackController::registerReceiver(IFrameReceiver* receiver) const {
	if (frame_ticker_) frame_ticker_->addReceiver(receiver);
}


void PlaybackController::unregisterReceiver(IFrameReceiver* receiver) const {
	if (frame_ticker_) frame_ticker_->removeReceiver(receiver);
}

void PlaybackController::setParamListeners(controller_t controller) {
	this->controller_ = controller;
	play_listener_ = std::make_unique<VideoParamListener>(
		controller_->getParameterObject(kParamPlay), shared_from_this(), controller_);
	reset_listener_ = std::make_unique<VideoParamListener>(
		controller_->getParameterObject(kParamReset), shared_from_this(), controller_);
	pause_listener_ = std::make_unique<VideoParamListener>(
		controller->getParameterObject(kParamPause), shared_from_this(), controller_);
}


void PlaybackController::onParamChanged(Steinberg::Vst::ParamID paramId, float paramValue) {
	switch (paramId) {
		case kParamPlay:
			if (paramValue > 0.5 && !is_playing_) {
				startPipeline(1.0);
			}
			break;
		case kParamReset:
			if (paramValue > 0.5 && is_playing_) {
				if (loader_->tryRewindToStart()) {
					stopPipeline();
					frame_queue_->clear();
				}
			}
		case kParamPause:
			if (paramValue > 0.5 && is_playing_) {
				stopPipeline();
			}
		default:
			break;
	}
}

void PlaybackController::onVideoFinished() const {
	if (controller_) {
		if (auto* pcont = dynamic_cast<PluginController*>(controller_)) {
			pcont->onVideoFinished();
		}
	}
}




