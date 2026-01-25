//
// Created by Darek Rudiš on 25.01.2026.
//

#ifndef ASYNCFRAMEWORKER_HPP
#define ASYNCFRAMEWORKER_HPP

#include "../videoframe.hpp"
#include <functional>
#include <queue>
#include <thread>

class AsyncFrameWorker {
public:
	using FrameCallback = std::function<void(const VideoFrame&)>;

	explicit AsyncFrameWorker(FrameCallback cb) : callback_(std::move(cb)), running_(true) {
		worker_ = std::thread([this]{ runLoop(); });
	}

	~AsyncFrameWorker() {
		running_.store(false);
		cv_.notify_all();
		if (worker_.joinable())
			worker_.join();
	}

	void enqueueFrame(const VideoFrame& frame) {
		std::lock_guard lock(frame_mutex_);

		// If worker is still processing, we just overwrite the buffered frame
		latest_frame_ = frame;
		new_frame_enqueued_.store(true, std::memory_order_release);
		cv_.notify_one();
	}

private:
	void runLoop() {
		while (running_.load()) {
			VideoFrame frame;
			{
				std::unique_lock lock(frame_mutex_);
				cv_.wait(lock, [this]{ return new_frame_enqueued_.load() || !running_.load(); });
				if (!running_) break;

				frame = latest_frame_;
				new_frame_enqueued_.store(false);
			}

			callback_(frame);
		}
	}

	std::atomic<bool> running_;
	std::thread worker_;

	VideoFrame latest_frame_;
	std::atomic<bool> new_frame_enqueued_ {false};
	std::mutex frame_mutex_;

	std::condition_variable cv_;
	FrameCallback callback_;
};


#endif //ASYNCFRAMEWORKER_HPP
