//
// Created by Darek Rudiš on 07.11.2025.
//

#include "mediaview.hpp"

#include "vstgui/lib/tasks.h"

void MediaView::draw(VSTGUI::CDrawContext* dc) {
	updateFromQueue();

	if (!bmp_) {
		setDirty(false);
		return;
	}

	const VSTGUI::CRect viewRect = getViewSize();
	if (bmp_width_ == 0 || bmp_height_ == 0) {
		bmp_->draw(dc, viewRect);
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

void MediaView::frameToBitmap(VideoFrame&& frame)
{
	const auto& img = frame.image;
	if (img.width == 0 || img.height == 0 || img.rgba_data.empty())
		return;

	const auto bitmapWidth = static_cast<uint32_t>(img.width);
	const auto bitmapHeight = static_cast<uint32_t>(img.height);

	VSTGUI::CRect viewRect = getViewSize();
	const uint32_t viewWidth = viewRect.getWidth();
	const uint32_t viewHeight = viewRect.getHeight();

	std::vector<uint8_t> resized;
	const uint8_t* srcPtr = img.rgba_data.data();
	if (bitmapWidth == viewWidth && bitmapHeight == viewHeight) {
		resized = img.rgba_data;
	} else {
		resized = resizeNearestRGBA(srcPtr, bitmapWidth, bitmapHeight, viewWidth, viewHeight);
	}

	auto* newBmp = new VSTGUI::CBitmap(static_cast<VSTGUI::CCoord>(bitmapWidth), static_cast<VSTGUI::CCoord>(bitmapHeight));
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

	bmp_ = VSTGUI::owned(newBmp);
	bmp_width_ = bitmapWidth;
	bmp_height_ = bitmapHeight;

	VSTGUI::Tasks::schedule(VSTGUI::Tasks::mainQueue(), [this] {
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
