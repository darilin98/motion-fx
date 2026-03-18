//
// Created by Darek Rudiš on 13.12.2025.
//

#include "frameticker.hpp"

#include <thread>

FrameTicker::~FrameTicker() {
	FrameTicker::stopConsuming();
}

void FrameTicker::setQueue(const frame_queue_t& queue) {
	frame_queue_ = queue;
}

void FrameTicker::addReceiver(IFrameReceiver* receiver) {
	std::lock_guard lock(receivers_mutex_);
	receivers_.emplace_back(receiver);
}

void FrameTicker::removeReceiver(IFrameReceiver* receiver) {
	std::lock_guard lock(receivers_mutex_);
	auto it = std::remove(receivers_.begin(), receivers_.end(), receiver);
	receivers_.erase(it, receivers_.end());
}

void FrameTicker::clearReceivers() {
	std::lock_guard lock(receivers_mutex_);
	receivers_.clear();
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
	if (!consumer_running_.exchange(false))
		return;

	if (consumer_thread_.joinable()) {
		if (std::this_thread::get_id() != consumer_thread_.get_id()) {
			consumer_thread_.join();
		}
	}
}

void FrameTicker::resetTimer() {
	std::lock_guard lock(time_mutex_);
	const auto now = std::chrono::steady_clock::now();
	consumer_start_ = now;
	next_tick_ = now;
}

void FrameTicker::consumerLoop() {
	if (!consumer_running_) return;

	// TODO: Let decoder run ahead in a cleaner way
	//		 Queue size checking might make images and gifs not work (queue always small)
	std::this_thread::sleep_for(std::chrono::milliseconds(15));

	const double periodSec = 1.0 / fps_;
	const auto period = std::chrono::duration<double>(periodSec);
	{
		std::lock_guard lock(time_mutex_);
		const auto now = std::chrono::steady_clock::now();
		consumer_start_ = now;
		next_tick_ = now;
	}

	bool firstFrame = true;
	while (consumer_running_.load(std::memory_order_acquire)) {

		std::this_thread::sleep_until(next_tick_);
		const auto tickStart = std::chrono::steady_clock::now();

		double elapsed;
		{
			std::lock_guard lock(time_mutex_);
			elapsed = std::chrono::duration<double>(tickStart - consumer_start_).count();
		}

		VideoFrame latest;
		bool gotFrame = false;
		VideoFrame tmp;

		while (frame_queue_->tryPop(tmp)) {
			if (tmp.timestamp >= elapsed || firstFrame) {
				latest = std::move(tmp);
				gotFrame = true;
				firstFrame = false;
				break;
			}
		}

		if (gotFrame) {
			std::vector<IFrameReceiver*> snapshot;
			{
				std::lock_guard lock(receivers_mutex_);
				snapshot = receivers_;
			}

			for (auto* r : snapshot) {
				r->onFrame(latest);
			}
		} else {
			onQueueEmpty();
		}

		next_tick_ += std::chrono::duration_cast<std::chrono::steady_clock::duration>(period);

		if (std::chrono::steady_clock::now() - next_tick_ > period * 2) {
			std::lock_guard lock(time_mutex_);
			const auto now = std::chrono::steady_clock::now();
			consumer_start_ = now;
			next_tick_ = now;
		}
	}
}
