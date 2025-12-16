//
// Created by Darek Rudiš on 13.12.2025.
//

#include "frameticker.hpp"

#include <thread>

void FrameTicker::setQueue(const frame_queue_t& queue) {
	frame_queue_ = queue;
}

void FrameTicker::addReceiver(IFrameReceiver* receiver) {
	receivers_.emplace_back(receiver);
}


void FrameTicker::startConsumingAt(double fps) {
	stopConsuming();

	if (!frame_queue_) return;

	fps_ = fps > 0 ? fps : 25.0;
	consumer_running_.store(true, std::memory_order_release);

	consumer_thread_ = std::thread([this] {
		consumerLoop();
	});
}

void FrameTicker::stopConsuming() {
	if (!consumer_running_.load()) return;

	consumer_running_.store(false, std::memory_order_release);
	if (consumer_thread_.joinable()) {
		try {
			consumer_thread_.join();
		} catch (...) { }
	}

	frame_queue_.reset();
}

void FrameTicker::resetTimer() {
	std::lock_guard lock(time_mutex_);
	consumer_start_ = std::chrono::steady_clock::now();
}

void FrameTicker::consumerLoop() {
	if (!consumer_running_) return;

	const double periodSec = 1.0 / fps_;
	const auto period = std::chrono::duration<double>(periodSec);
	{
		std::lock_guard lock(time_mutex_);
		consumer_start_ = std::chrono::steady_clock::now();
	}

	bool firstFrame = true;
	while (consumer_running_.load(std::memory_order_acquire)) {
		const auto tickStart = std::chrono::steady_clock::now();

		double elapsed;
		{
			std::lock_guard lock(time_mutex_);
			elapsed = std::chrono::duration<double>(tickStart - consumer_start_).count();
		}

		VideoFrame latest;
		bool gotFrame = false;
		VideoFrame tmp;

		// Dropping any outdated frames to prevent slo-mo
		while (frame_queue_->tryPop(tmp)) {
			if (tmp.timestamp >= elapsed || firstFrame) {
				latest = std::move(tmp);
				gotFrame = true;
				firstFrame = false;
				break;
			}
		}

		if (gotFrame) {
			for (auto* receiver : receivers_) {
				receiver->onFrame(latest);
			}
		} else {
			onQueueEmpty();
		}
		std::this_thread::sleep_until(tickStart + period);
	}
}
