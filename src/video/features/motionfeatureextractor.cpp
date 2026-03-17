//
// Created by Darek Rudiš on 15.03.2026.
//

#include "motionfeatureextractor.hpp"
#include "visionutils.hpp"

FeatureResult MotionFeatureExtractor::extract(const VideoFrame& videoFrame) {
    const auto& rgba = videoFrame.image.rgba_data;
    const size_t width  = videoFrame.image.width;
    const size_t height = videoFrame.image.height;

    if (rgba.empty())
        return FeatureResult{};

    if (prev_frame_.size() != rgba.size())
        prev_frame_ = rgba;

    constexpr int stride = 2;

    int samples = 0;
    int active_pixels = 0;

    for (size_t y = 0; y < height; y += stride)
    {
        for (size_t x = 0; x < width; x += stride)
        {
            size_t idx = (y * width + x) * 4;

            uint8_t r0 = prev_frame_[idx];
            uint8_t g0 = prev_frame_[idx + 1];
            uint8_t b0 = prev_frame_[idx + 2];

            uint8_t r1 = rgba[idx];
            uint8_t g1 = rgba[idx + 1];
            uint8_t b1 = rgba[idx + 2];

            float l0 = luminance(r0, g0, b0);
            float l1 = luminance(r1, g1, b1);

            float diff = std::abs(l1 - l0);

            // ignore tiny noise
            constexpr float threshold = 3.0f;
            if (diff > threshold) {
                active_pixels++;
            }

            samples++;
        }
    }

    prev_frame_ = rgba;

    float raw_motion = float(active_pixels) / float(samples);
    raw_motion *= 2.0f;
    raw_motion = std::clamp(raw_motion, 0.0f, 1.0f);

    const float cont_alpha = 0.1f;
    continuous_motion_ = (1.0f - cont_alpha) * continuous_motion_ + cont_alpha * raw_motion;

    float delta = raw_motion - previous_raw_;
    previous_raw_ = raw_motion;

    float burst = std::max(0.0f, delta) * 7.0f; // boost small spikes
    const float burst_alpha = 0.15f;
    burst_motion_ = (1.0f - burst_alpha) * burst_motion_ + burst_alpha * burst;

    burst_motion_ = std::max(0.0f, burst_motion_ * 0.96f); // burst decay

    burst_motion_ = std::clamp(burst_motion_, 0.0f, 1.0f);
    continuous_motion_ = std::clamp(continuous_motion_, 0.0f, 1.0f);

    param_arr_t params = {
        FeatureParamUpdate{ continuous_param_id_, continuous_motion_ },
        FeatureParamUpdate{ burst_param_id_, burst_motion_ }
    };

    return { params, videoFrame.timestamp };
}

void MotionFeatureExtractor::onFrame(const VideoFrame& videoFrame) {
	frame_worker_->enqueueFrame(videoFrame);
}


