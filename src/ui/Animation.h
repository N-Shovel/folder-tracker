#pragma once
#include <algorithm>
#include <chrono>
#include <cmath>

// A number that moves toward a target over time (0 = closed, 1 = open).
// It moves at a steady speed; easing is applied when drawing.
class Animation {
public:
    void animateTo(float target, int fullDurationMs) {
        from_ = value_;
        to_ = target;
        // Reversing halfway only takes half the time.
        durationMs_ = fullDurationMs * std::abs(to_ - from_);
        start_ = Clock::now();
        running_ = durationMs_ > 0;
        if (!running_) value_ = to_;
    }

    // Moves the value forward; returns true while still animating.
    bool update() {
        if (!running_) return false;
        const float elapsed = std::chrono::duration<float, std::milli>(Clock::now() - start_).count();
        const float t = std::min(elapsed / durationMs_, 1.0f);
        value_ = from_ + (to_ - from_) * t;
        running_ = t < 1.0f;
        return running_;
    }

    float value() const { return value_; }
    bool running() const { return running_; }

private:
    using Clock = std::chrono::steady_clock;
    float value_ = 0, from_ = 0, to_ = 0, durationMs_ = 0;
    Clock::time_point start_;
    bool running_ = false;
};

// Starts fast, slows down at the end.
inline float easeOutCubic(float t) {
    const float u = 1 - t;
    return 1 - u * u * u;
}
