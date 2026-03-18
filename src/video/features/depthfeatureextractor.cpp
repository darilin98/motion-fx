//
// Created by Darek Rudiš on 03.03.2026.
//

#include "depthfeatureextractor.hpp"
#include "visionutils.hpp"

FeatureResult DepthFeatureExtractor::extract(const VideoFrame& videoFrame) {
    if (videoFrame.image.rgba_data.empty())
        return FeatureResult{};

    float depth_raw = calculateDetailRatio(videoFrame);

    depth_raw = std::clamp(depth_raw, 0.0f, 1.0f);
    depth_raw = std::pow(depth_raw, 0.8f); // Boost mid values
    smoothed_depth_ = 0.95f * smoothed_depth_ + 0.05f * depth_raw;

    auto param = FeatureParamUpdate{ param_id_, smoothed_depth_ };
    param_arr_t parameters = { param };

    return { parameters, videoFrame.timestamp };
}

float DepthFeatureExtractor::calculateDetailRatio(const VideoFrame& videoFrame) {
    const auto& rgba = videoFrame.image.rgba_data;
    const size_t width  = videoFrame.image.width;
    const size_t height = videoFrame.image.height;

    if (width < 3 || height < 3)
        return 0;

    const size_t third = height / 3;
    const size_t top_third = third;
    const size_t bottom_third = third * 2;

    float edge_bottom = 0.0f;
    float edge_top = 0.0f;

    size_t count_bottom = 0;
    size_t count_top = 0;

    // Compute simple vertical+horizontal gradient energy
    for (size_t y = 1; y < height - 1; ++y) {
        for (size_t x = 1; x < width - 1; ++x) {
            const size_t idx = (y * width + x) * 4;

            const float curr_pixel_lumi = luminance(rgba[idx], rgba[idx+1], rgba[idx+2]);

            const size_t right_idx = idx + 4;
            const size_t down_idx  = idx + width * 4;

            const float right_pixel_lumi = luminance(rgba[right_idx], rgba[right_idx + 1], rgba[right_idx + 2]);
            const float down_pixel_lumi = luminance(rgba[down_idx],  rgba[down_idx + 1],  rgba[down_idx + 2]);

            const float edge = std::abs(curr_pixel_lumi - right_pixel_lumi) + std::abs(curr_pixel_lumi - down_pixel_lumi);

            if (y >= bottom_third) {
                edge_bottom += edge;
                ++count_bottom;
            }
            else if (y < top_third) {
                edge_top += edge;
                ++count_top;
            }
        }
    }

    edge_bottom = safeDiv(edge_bottom, count_bottom);
    edge_top = safeDiv(edge_top, count_top);

    float depth_raw = safeDiv(edge_bottom - edge_top, edge_bottom + 1.0f);

    return depth_raw;
}


void DepthFeatureExtractor::onFrame(const VideoFrame& videoFrame) {
	frame_worker_->enqueueFrame(videoFrame);
}

