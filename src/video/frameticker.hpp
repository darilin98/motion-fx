//
// Created by Darek Rudiš on 13.12.2025.
//

#ifndef FRAMETICKER_HPP
#define FRAMETICKER_HPP

#include "framequeue.hpp"
#include "iframereceiver.hpp"

#include <thread>
#include <mutex>
#include <functional>

/**
 * @brief The driving consumer of the video pipeline.
 *
 * Consumes VideoFrames from an IFrameQueue at a steady fps.
 */
class IFrameTicker {
public:
	virtual ~IFrameTicker() = default;

	/**
	 * @brief Resets internal timer.
	 */
	virtual void resetTimer() = 0;

	/**
	 * @brief Starts the ticking process.
	 * @param fps Speed of VideoFrame consumption in FPS
	 */
	virtual void startConsumingAt(double fps) = 0;

	/**
	 * @brief Safely stops the consumption process. Must retain the ability to be started again.
	 */
	virtual void stopConsuming() = 0;

	/**
	 * @brief Sets the source of frames. Must be called before @ref startConsumingAt().
	 * @param queue IFrameQueue data structure that serves as a source of VideoFrames to be consumed.
	 */
	virtual void setQueue(const frame_queue_t& queue) = 0;

	/**
	 * @brief Handler signalling that there is nothing left to consume.
	 */
	virtual void onQueueEmpty() = 0;
};

/**
 * @brief Real-time friendly implementation of the IFrameTicker contract.
 *
 * Consumes VideoFrames and passes each frame to any n subscribed IFrameReceiver objects.
 */
class FrameTicker : public IFrameTicker {
public:
	/**
	 * @brief Terminates consuming and destroys FrameTicker.
	 */
	~FrameTicker() override;

	/**
	 * @brief Resets internal timer. Must be called between @ref stopConsuming() and @ref startConsumingAt() calls.
	 */
	void resetTimer() override;

	/**
	 * @brief Creates a thread for the consumption process and starts the job.
	 * @param fps Speed of consumption in FPS.
	 */
	void startConsumingAt(double fps) override;

	/**
	 * @brief Schedules the termination of the consumption process. Joins consumption thread.
	 */
	void stopConsuming() override;
	void setQueue(const frame_queue_t& queue) override;

	/**
	 * @brief Adds an object implementing IFrameReceiver to the list of VideoFrame subscribers.
	 * @param receiver An object that wishes to subscribe to VideoFrames.
	 */
	void addReceiver(IFrameReceiver* receiver);

	/**
	 * @brief Removes an object from the VideoFrame subscriber list.
	 * @param receiver An object that wishes to be removed from the VideoFrame subscriber list.
	 */
	void removeReceiver(IFrameReceiver* receiver);

	/**
	 * @breif Removes all objects from the VideoFrame subscriber list.
	 */
	void clearReceivers();

	/**
	 * @brief Triggers when the ticker encounters an IFrameQueue containing no VideoFrames.
	 */
	void onQueueEmpty() override {
		if (on_queue_empty_callback_) on_queue_empty_callback_();
	}

	/**
	 * @brief Allows injection of the method for handling an OnQueueEmpty event.
	 * @param fn External handler of the OnQueueEmpty event.
	 */
	void setOnQueueEmptyCallback(std::function<void()> fn) {
		on_queue_empty_callback_ = std::move(fn);
	}
private:
	void consumerLoop(); /// Core consumption loop for the worker thread.
	receiver_list_t receivers_; /// VideoFrame subscriber list.
	std::mutex receivers_mutex_; /// Guards changes to the VideoFrame subscriber list.

	std::thread consumer_thread_; /// Consumption worker thread.
	std::atomic<bool> consumer_running_ {false}; /// State of the worker thread.

    std::mutex time_mutex_; /// Guards changes to timing.
	std::chrono::steady_clock::time_point consumer_start_; /// Frame synchronization time point
	std::chrono::steady_clock::time_point next_tick_;

    double fps_ = 25.0; /// Determines the FrameTicker's consumption speed.
	frame_queue_t frame_queue_; /// Source of VideoFrames

    std::function<void()> on_queue_empty_callback_;
};

using frame_ticker_t = std::unique_ptr<FrameTicker>;

#endif //FRAMETICKER_HPP
