//
// Created by Darek Rudiš on 08.11.2025.
//

#ifndef FRAMEQUEUE_HPP
#define FRAMEQUEUE_HPP

#include <mutex>
#include <queue>
#include <thread>

#include "videoframe.hpp"

/**
 * @brief Data structure for storing VideoFrames.
 *
 * Stores frames and removes them FIFO.
 */
class IFrameQueue {
public:
	virtual ~IFrameQueue() = default;

	/**
	 * @brief Tries to remove a VideoFrame from the queue.
	 * @param outFrame Out value VideoFrame if operation succeeded.
	 * @return Status of operation success.
	 */
	virtual bool tryPop(VideoFrame& outFrame) = 0;

	/**
	 * @brief Stores a VideoFrame in the queue.
	 * @param frame VideoFrame to be stored in the queue.
	 */
	virtual void push(VideoFrame&& frame) = 0;

	/**
	 * @brief Clears all entries in the queue asynchronously.
	 */
	virtual void clearAsync() = 0;
};

/**
 * @brief A thread-safe implementation of the IFrameQueue contract.
 *
 * Supports simultaneous access from a producer and consumer.
 */
class FrameQueue final : public IFrameQueue {
public:
	FrameQueue() = default;

	bool tryPop(VideoFrame& outFrame) override {
		std::lock_guard lock(mutex_);
		if(queue_.empty()) return false;
		outFrame = std::move(queue_.front());
		queue_.pop();
		return true;
	}

	void push(VideoFrame&& frame) override {
		std::lock_guard lock(mutex_);
		queue_.push(std::move(frame));
	}

	/**
	 * @brief Clears contents on a separate thread.
	 *
	 * Does not slow down the caller.
	 */
	void clearAsync() override {
		std::queue<VideoFrame> old_queue;
		{
			std::lock_guard lock(mutex_);
			std::swap(queue_, old_queue);
		}

		// Spawning a thread for de-allocation in order to not slow down the main thread
		std::thread([oldQueue = std::move(old_queue)]() mutable {}).detach();

		std::lock_guard lock(mutex_);
		std::queue<VideoFrame> empty;
		queue_.swap(empty);
	}

private:
	std::queue<VideoFrame> queue_; /// Stores VideoFrames
	std::mutex mutex_; /// Guards the queue
};

using frame_queue_t = std::shared_ptr<IFrameQueue>;

#endif //FRAMEQUEUE_HPP
