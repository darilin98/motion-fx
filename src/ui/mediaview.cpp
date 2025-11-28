//
// Created by Darek Rudiš on 07.11.2025.
//

#include "mediaview.hpp"

#include <iostream>
#include <thread>

MediaView::~MediaView() {
	fprintf(stderr, "Killing media view\n");
	stopConsuming();
	if (bmp_) {
		auto bmp = std::move(bmp_);
		auto mq = VSTGUI::Tasks::mainQueue();
		schedule(mq, [bmp = std::move(bmp)]() mutable{});
	}
}

bool MediaView::removed(CView *parent) {
	stopConsuming();
	fprintf(stderr, "Removed media view\n");
	return CView::removed(parent);
}

void MediaView::draw(VSTGUI::CDrawContext* dc) {

	if (!bmp_) {
		setDirty(false);
		return;
	}

	const VSTGUI::CRect viewRect = getViewSize();
	if (bmp_width_ == 0 || bmp_height_ == 0) {
		setDirty(false);
		return;
	}

	bmp_->draw(dc, viewRect);
	setDirty(false);
}

void MediaView::setQueue(const frame_queue_t& queue) {
	queue_ = queue;
}

void MediaView::updateFromQueue() {
	if (!queue_)
		return;

	VideoFrame newFrame;
	if(queue_->tryPop(newFrame)) {
		frameToBitmap(std::move(newFrame));
	}
}

void MediaView::startConsumingAt(double fps) {
	stopConsuming();

	if (!queue_) return;

	fps_ = fps > 0 ? fps : 25.0;
	consumerRunning_.store(true);

	consumerQueue_ = std::make_unique<VSTGUI::Tasks::Queue>(VSTGUI::Tasks::makeSerialQueue("frame-consumer"));
	VSTGUI::Tasks::schedule(*consumerQueue_, [this] {
		this->consumerLoop();
	});
}

void MediaView::consumerLoop() {
	if (!consumerRunning_) return;

	// TODO: Timing is broken on re-entry
	using clock = std::chrono::system_clock;
	const double periodSec = 1.0 / fps_;
	const auto period = std::chrono::duration<double>(periodSec);
	static auto start = clock::now();

	const auto now = clock::now();
	const double elapsed = std::chrono::duration<double>(now - start).count();

	VideoFrame latest;
	bool gotFrame = false;
	VideoFrame tmp;

	fprintf(stderr, "Elapsed: %f\n", elapsed);

	// Dropping any outdated frames to prevent slo-mo
	while (queue_->tryPop(tmp)) {
		if (tmp.timestamp >= elapsed || !bmp_) {
			latest = std::move(tmp);
			gotFrame = true;
			break;
		}
	}


	if (gotFrame) {
		frameToBitmap(std::move(latest));

		// Sleep the successful worker thread so we don't rush ahead
		std::this_thread::sleep_for(period);
	}

	if (consumerRunning_.load()) {
		VSTGUI::Tasks::schedule(*consumerQueue_, [this] {
			this->consumerLoop();
		});
	}
}

void MediaView::stopConsuming() {
	if (!consumerQueue_) return;

	VSTGUI::Tasks::releaseSerialQueue(*consumerQueue_);
	consumerQueue_.reset();

	consumerRunning_ = false;
	queue_.reset();
}

void MediaView::frameToBitmap(VideoFrame&& frame)
{
	const auto& img = frame.image;
	if (img.width == 0 || img.height == 0 || img.rgba_data.empty())
		return;

	const auto bitmapWidth = static_cast<uint32_t>(img.width);
	const auto bitmapHeight = static_cast<uint32_t>(img.height);

	const VSTGUI::CRect viewRect = getViewSize();
	const uint32_t viewWidth = viewRect.getWidth();
	const uint32_t viewHeight = viewRect.getHeight();

	std::vector<uint8_t> resized;
	const uint8_t* srcPtr = img.rgba_data.data();
	if (bitmapWidth == viewWidth && bitmapHeight == viewHeight) {
		resized = img.rgba_data;
	} else {
		resized = resizeNearestRGBA(srcPtr, bitmapWidth, bitmapHeight, viewWidth, viewHeight);
	}

	VSTGUI::Tasks::schedule(VSTGUI::Tasks::mainQueue(), [this, resized, viewWidth, viewHeight]() {
		auto* newBmp = new VSTGUI::CBitmap(static_cast<VSTGUI::CCoord>(viewWidth), static_cast<VSTGUI::CCoord>(viewHeight));
		VSTGUI::CBitmapPixelAccess* access = VSTGUI::CBitmapPixelAccess::create(newBmp, true);
		if (!access) {
			delete newBmp;
			return;
		}
		const uint8_t* src = resized.data();

		for (uint32_t y = 0; y < viewHeight; ++y)
		{
			if (!access->setPosition(0u, y))
				continue;

			for (uint32_t x = 0; x < viewWidth; ++x)
			{
				const uint8_t r = *src++;
				const uint8_t g = *src++;
				const uint8_t b = *src++;
				const uint8_t a = *src++;

				const uint32_t val =
					static_cast<uint32_t>(b) << 24 |
					static_cast<uint32_t>(g) << 16 |
					static_cast<uint32_t>(r) << 8  |
					static_cast<uint32_t>(a);

				access->setValue(val);

				++(*access);
			}
		}

		access->forget();

		this->bmp_ = VSTGUI::owned(newBmp);
		this->bmp_width_ = viewWidth;
		this->bmp_height_ = viewHeight;
		this->invalid();
	});
}


static std::vector<uint8_t> resizeNearestRGBA(const uint8_t* src, const int originWidth, const int originHeight, const int destinationWidth, const int destinationHeight) {
    std::vector<uint8_t> out(static_cast<size_t>(destinationWidth) * destinationHeight * 4);
    for (int y = 0; y < destinationHeight; ++y) {
        const int sy = std::min(originHeight - 1, (y * originHeight) / destinationHeight);
        for (int x = 0; x < destinationWidth; ++x) {
            const int sx = std::min(originWidth - 1, (x * originWidth) / destinationWidth);
            const uint8_t* s = src + (sy * originWidth + sx) * 4;
            uint8_t* d = out.data() + (y * destinationWidth + x) * 4;
            d[0] = s[0]; d[1] = s[1]; d[2] = s[2]; d[3] = s[3];
        }
    }
    return out;
}
