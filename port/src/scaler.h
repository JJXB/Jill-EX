#pragma once

// Shared non-integer presentation scaler.
//
// The game frame is always 320x200 (or smaller during a resolution change).
// Every backend except the desktop SDL renderer upscales it on the CPU, so the
// sample tables live here and are reused instead of recomputed per frame.
//
// Modes:
//   0 INTEGER    largest integer scale that fits (the original behaviour)
//   1 FRACTIONAL nearest per axis, non-integer uniform scale
//   2 VERTICAL   nearest horizontally, bilinear vertically
//   3 BILINEAR   bilinear on both axes
//
// The fit is always uniform (aspect preserving), so fractional/vertical modes
// fill the height and pillarbox on 16:9 targets.  320x200 is 1.6:1, so a
// 1920x1080 output gets a 1728x1080 picture instead of the old 1600x1000.

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace jill_scale {

enum Mode {
    kInteger = 0,
    kFractional = 1,
    kVertical = 2,
    kFull = 3,
};

struct Fit {
    int w = 0;
    int h = 0;
    int x = 0;
    int y = 0;
};

inline Fit compute_fit(int src_w, int src_h, int dst_w, int dst_h, int mode) {
    Fit f{};
    if (src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) {
        return f;
    }
    if (mode == kInteger) {
        int s = std::min(dst_w / src_w, dst_h / src_h);
        if (s < 1) {
            s = 1;
        }
        f.w = src_w * s;
        f.h = src_h * s;
    } else {
        const float sx = static_cast<float>(dst_w) / static_cast<float>(src_w);
        const float sy = static_cast<float>(dst_h) / static_cast<float>(src_h);
        const float s = std::min(sx, sy);
        f.w = std::max(1, static_cast<int>(src_w * s + 0.5f));
        f.h = std::max(1, static_cast<int>(src_h * s + 0.5f));
        // Never overflow the destination when rounding up.
        if (f.w > dst_w) {
            f.w = dst_w;
        }
        if (f.h > dst_h) {
            f.h = dst_h;
        }
    }
    f.x = (dst_w - f.w) / 2;
    f.y = (dst_h - f.h) / 2;
    return f;
}

// 0xFFRRGGBB/0xAARRGGBB blend with an 8-bit weight (0..256) for `b`.
inline uint32_t blend(uint32_t a, uint32_t b, int t) {
    const int it = 256 - t;
    const uint32_t rb =
        (((a & 0x00FF00FFu) * it + (b & 0x00FF00FFu) * t) >> 8) & 0x00FF00FFu;
    const uint32_t g =
        (((a & 0x0000FF00u) * it + (b & 0x0000FF00u) * t) >> 8) & 0x0000FF00u;
    return 0xFF000000u | rb | g;
}

class Scaler {
public:
    void setup(int src_w, int src_h, int out_w, int out_h, int mode) {
        if (src_w == src_w_ && src_h == src_h_ && out_w == out_w_ &&
            out_h == out_h_ && mode == mode_) {
            return;
        }
        src_w_ = src_w;
        src_h_ = src_h;
        out_w_ = out_w;
        out_h_ = out_h;
        mode_ = mode;

        xs_.resize(static_cast<size_t>(out_w));
        x1_.resize(static_cast<size_t>(out_w));
        wx_.resize(static_cast<size_t>(out_w));
        const bool bilinear_x = (mode == kFull);
        for (int x = 0; x < out_w; ++x) {
            if (bilinear_x) {
                sample_axis(x, src_w, out_w, &xs_[x], &x1_[x], &wx_[x]);
            } else {
                const int sx = static_cast<int>(static_cast<int64_t>(x) * src_w /
                                                out_w);
                xs_[x] = std::min(sx, src_w - 1);
                x1_[x] = xs_[x];
                wx_[x] = 0;
            }
        }

        ys_.resize(static_cast<size_t>(out_h));
        y1_.resize(static_cast<size_t>(out_h));
        wy_.resize(static_cast<size_t>(out_h));
        const bool bilinear_y = (mode == kVertical || mode == kFull);
        for (int y = 0; y < out_h; ++y) {
            if (bilinear_y) {
                sample_axis(y, src_h, out_h, &ys_[y], &y1_[y], &wy_[y]);
            } else {
                const int sy = static_cast<int>(static_cast<int64_t>(y) * src_h /
                                                out_h);
                ys_[y] = std::min(sy, src_h - 1);
                y1_[y] = ys_[y];
                wy_[y] = 0;
            }
        }
    }

    // Sample the declared source frame into the destination rectangle using
    // `store(x, y, 0xAARRGGBB)`.  The caller is responsible for clearing the
    // letterbox area first.
    template <typename Store>
    void blit(const uint32_t* src, Store&& store) const {
        if (src == nullptr || out_w_ <= 0 || out_h_ <= 0) {
            return;
        }
        for (int y = 0; y < out_h_; ++y) {
            const uint32_t* r0 = src + static_cast<size_t>(ys_[y]) * src_w_;
            const uint32_t* r1 = src + static_cast<size_t>(y1_[y]) * src_w_;
            const int wy = wy_[y];
            for (int x = 0; x < out_w_; ++x) {
                const int x0 = xs_[x];
                const int x1 = x1_[x];
                const int wx = wx_[x];
                uint32_t c = r0[x0];
                if (wx != 0) {
                    c = blend(c, r0[x1], wx);
                }
                if (wy != 0) {
                    const uint32_t lo = r1[x0];
                    const uint32_t hi = (wx != 0) ? r1[x1] : lo;
                    const uint32_t row = (wx != 0) ? blend(lo, hi, wx) : lo;
                    c = blend(c, row, wy);
                }
                store(x, y, c | 0xFF000000u);
            }
        }
    }

    int out_w() const { return out_w_; }
    int out_h() const { return out_h_; }

private:
    // Bilinear tap pair and 8-bit weight for one axis.
    static void sample_axis(int d, int src, int dst, int* lo, int* hi,
                            int* weight) {
        float f = ((static_cast<float>(d) + 0.5f) * static_cast<float>(src) /
                   static_cast<float>(dst)) -
                  0.5f;
        if (f < 0.0f) {
            f = 0.0f;
        }
        const int i0 = static_cast<int>(f);
        const int i1 = std::min(i0 + 1, src - 1);
        int t = static_cast<int>((f - static_cast<float>(i0)) * 256.0f + 0.5f);
        if (t < 0) {
            t = 0;
        } else if (t > 256) {
            t = 256;
        }
        *lo = i0;
        *hi = i1;
        *weight = t;
    }

    int src_w_ = 0;
    int src_h_ = 0;
    int out_w_ = 0;
    int out_h_ = 0;
    int mode_ = -1;
    std::vector<int> xs_;
    std::vector<int> x1_;
    std::vector<int> wx_;
    std::vector<int> ys_;
    std::vector<int> y1_;
    std::vector<int> wy_;
};

}  // namespace jill_scale
