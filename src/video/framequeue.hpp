//
// Created by Darek Rudiš on 08.11.2025.
//

#ifndef FRAMEQUEUE_HPP
#define FRAMEQUEUE_HPP

#include <mutex>
#include <queue>

#include "videoframe.hpp"

class IFrameQueue {
public:
	virtual ~IFrameQueue() = default;
	virtual bool tryPop(VideoFrame& outFrame) = 0;
	virtual void push(VideoFrame&& frame) = 0;
	virtual void clear() = 0;
};

class FrameQueue final : public IFrameQueue {
public:
	FrameQueue() = default;

	bool tryPop(VideoFrame &outFrame) override {
		std::lock_guard lock(mutex_);
		if(queue_.empty()) return false;
		outFrame = std::move(queue_.front());
		queue_.pop();
		return true;
	}

	void push(VideoFrame &&frame) override {
		std::lock_guard lock(mutex_);
		queue_.push(std::move(frame));
	}

	void clear() override {
		std::lock_guard lock(mutex_);
		std::queue<VideoFrame> empty;
		queue_.swap(empty);
	}

private:
	std::queue<VideoFrame> queue_;
	std::mutex mutex_;
};

using frame_queue_t = std::shared_ptr<IFrameQueue>;

#endif //FRAMEQUEUE_HPP
