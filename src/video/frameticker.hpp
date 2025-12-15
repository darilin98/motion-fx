//
// Created by Darek Rudiš on 13.12.2025.
//

#ifndef FRAMETICKER_HPP
#define FRAMETICKER_HPP

#include "framequeue.hpp"
#include "iframereceiver.hpp"

#include <thread>

// Interface for ease of mock-ability
class IFrameTicker {
public:
	virtual ~IFrameTicker() = default;
	virtual void resetTimer() = 0;
	virtual void startConsumingAt(double fps) = 0;
	virtual void stopConsuming() = 0;
	virtual void setQueue(const frame_queue_t& queue) = 0;
	virtual void onQueueEmpty() = 0;
};

class FrameTicker : public IFrameTicker {
public:
	~FrameTicker() override { FrameTicker::stopConsuming(); }
	void resetTimer() override;
	void startConsumingAt(double fps) override;
	void stopConsuming() override;
	void setQueue(const frame_queue_t& queue) override;
	void addReceiver(IFrameReceiver* receiver);

	void onQueueEmpty() override {
		if (on_queue_empty_callback_) on_queue_empty_callback_();
	}
	void setOnQueueEmptyCallback(std::function<void()> fn) {
		on_queue_empty_callback_ = std::move(fn);
	}
private:
	void consumerLoop();
	receiver_list_t receivers_;

	std::thread consumer_thread_;
	std::atomic<bool> consumer_running_ {false};

    std::mutex time_mutex_;
	std::chrono::steady_clock::time_point consumer_start_;

    double fps_ = 25.0;
	frame_queue_t frame_queue_;

    std::function<void()> on_queue_empty_callback_;
};

using frame_ticker_t = std::unique_ptr<FrameTicker>;

#endif //FRAMETICKER_HPP
