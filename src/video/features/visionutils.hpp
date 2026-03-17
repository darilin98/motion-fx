//
// Created by Darek Rudiš on 15.03.2026.
//

#ifndef VISIONUTILS_HPP
#define VISIONUTILS_HPP

inline float luminance(uint8_t r, uint8_t g, uint8_t b) {
    return 0.2126f * r + 0.7152f * g + 0.0722f * b;
}

inline uint8_t max_rgb(uint8_t r, uint8_t g, uint8_t b) {
    return std::max(r, std::max(g, b));
}

inline float safeDiv(float a, float b) {
    return b > 1e-6f ? a / b : 0.0f;
}

#endif //VISIONUTILS_HPP
