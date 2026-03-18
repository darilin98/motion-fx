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
	virtual bool push(VideoFrame&& frame) = 0;

	/**
	 * @brief Clears all entries in the queue asynchronously.
	 */
	virtual void clearAsync() = 0;

	virtual size_t size() = 0;

	virtual bool tryPeek(VideoFrame& outFrame) = 0;

};

/**
 * @brief A thread-safe implementation of the IFrameQueue contract.
 *
 * Supports simultaneous access from a producer and consumer.
 */
class FrameQueue final : public IFrameQueue {
public:
	explicit FrameQueue(size_t capacity = 128)
		: capacity_(capacity + 1), buffer_(capacity_),
		  head_(0), tail_(0) {}

	bool push(VideoFrame&& frame) override {
		size_t nextTail = (tail_.load(std::memory_order_relaxed) + 1) % capacity_;
		if (nextTail == head_.load(std::memory_order_acquire))
			return false;

		buffer_[tail_.load(std::memory_order_relaxed)] = std::move(frame);
		tail_.store(nextTail, std::memory_order_release);
		return true;
	}

	bool tryPop(VideoFrame& outFrame) override {
		size_t head = head_.load(std::memory_order_relaxed);
		if (head == tail_.load(std::memory_order_acquire))
			return false;

		outFrame = std::move(buffer_[head]);
		head_.store((head + 1) % capacity_, std::memory_order_release);
		return true;
	}

	bool tryPeek(VideoFrame& outFrame) override {
		size_t head = head_.load(std::memory_order_acquire);
		if (head == tail_.load(std::memory_order_acquire))
			return false;

		outFrame = buffer_[head];
		return true;
	}

	size_t size() override {
		size_t head = head_.load(std::memory_order_acquire);
		size_t tail = tail_.load(std::memory_order_acquire);
		if (tail >= head)
			return tail - head;

		return capacity_ - head + tail;
	}

	void clearAsync() override {
		head_.store(tail_.load(std::memory_order_acquire), std::memory_order_release);
	}

private:
	const size_t capacity_;
	std::vector<VideoFrame> buffer_;
	std::atomic<size_t> head_;
	std::atomic<size_t> tail_;
};

using frame_queue_t = std::shared_ptr<IFrameQueue>;

#endif //FRAMEQUEUE_HPP
