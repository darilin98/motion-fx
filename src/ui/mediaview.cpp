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

	const auto w = static_cast<uint32_t>(img.width);
	const auto h = static_cast<uint32_t>(img.height);

	// TODO: Rescaling for rect size
	auto* newBmp = new VSTGUI::CBitmap(static_cast<VSTGUI::CCoord>(w), static_cast<VSTGUI::CCoord>(h));
	VSTGUI::CBitmapPixelAccess* access = VSTGUI::CBitmapPixelAccess::create(newBmp, true);
	if (!access) {
		delete newBmp;
		return;
	}
	const uint8_t* src = img.rgba_data.data();

	for (uint32_t y = 0; y < h; ++y)
	{
		if (!access->setPosition(0u, y))
			continue;

		for (uint32_t x = 0; x < w; ++x)
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
	bmp_width_ = w;
	bmp_height_ = h;

	VSTGUI::Tasks::schedule(VSTGUI::Tasks::mainQueue(), [this] {
		this->invalid();
	});
}
