// PS4 native presentation: sceVideoOut with triple buffering.
//
// SDL2's PS4 video driver waits for the previous flip to complete before it
// starts building the next frame, which pins the loop to 30 Hz on hardware.
// This backend submits a flip and keeps two frames in flight, so the CPU builds
// the next frame while the previous one is scanned out.

#include "../src/screen.h"

#include <orbis/VideoOut.h>
#include <orbis/libkernel.h>

#include <algorithm>
#include <cstdint>
#include <cstring>

#include "../src/platform_paths.h"

namespace {

constexpr int kBufferCount = 3;
// One extra registered buffer that is never drawn into.  Flipping to it puts
// the display on a buffer that is not part of the presentation group, which is
// what lets the live group be unregistered on quit.
constexpr int kBlankIndex = kBufferCount;
constexpr int kTotalBuffers = kBufferCount + 1;

class ScreenVideoOut final : public Screen {
public:
    ~ScreenVideoOut() override { shutdown(); }

    bool init(int pixel_w, int pixel_h, int mode, std::string& err) override {
        (void)pixel_w;
        (void)pixel_h;
        (void)mode;

        handle_ = sceVideoOutOpen(ORBIS_VIDEO_USER_MAIN,
                                  ORBIS_VIDEO_OUT_BUS_MAIN, 0, nullptr);
        if (handle_ <= 0) {
            err = "sceVideoOutOpen failed";
            return false;
        }

        OrbisVideoOutResolutionStatus res{};
        if (sceVideoOutGetResolutionStatus(handle_, &res) == 0 &&
            res.width > 0 && res.height > 0) {
            width_ = static_cast<int>(res.width);
            height_ = static_cast<int>(res.height);
        } else {
            width_ = 1920;
            height_ = 1080;
        }
        jill_paths::trace("videoout: %dx%d handle=%d", width_, height_, handle_);

        std::memset(&attr_, 0, sizeof(attr_));
        attr_.format = ORBIS_VIDEO_OUT_PIXEL_FORMAT_A8B8G8R8_SRGB;
        attr_.tmode = ORBIS_VIDEO_OUT_TILING_MODE_LINEAR;
        attr_.aspect = ORBIS_VIDEO_OUT_ASPECT_RATIO_16_9;
        attr_.width = static_cast<uint32_t>(width_);
        attr_.height = static_cast<uint32_t>(height_);
        attr_.pixelPitch = static_cast<uint32_t>(width_);

        const size_t pixel_count =
            static_cast<size_t>(width_) * static_cast<size_t>(height_);
        buffer_size_ = pixel_count * 4;
        const size_t align = 1024 * 1024;
        const size_t aligned = (buffer_size_ + align - 1) & ~(align - 1);
        total_size_ = aligned * kTotalBuffers;
        const size_t total = total_size_;

        int32_t rc = sceKernelAllocateDirectMemory(
            0, ORBIS_KERNEL_MAIN_DMEM_SIZE, total, align,
            ORBIS_KERNEL_WC_GARLIC, &phys_);
        if (rc != 0) {
            jill_paths::trace("videoout: alloc failed 0x%x", rc);
            err = "sceKernelAllocateDirectMemory failed";
            return false;
        }
        rc = sceKernelMapDirectMemory(
            reinterpret_cast<void**>(&map_), total,
            ORBIS_KERNEL_PROT_CPU_RW | ORBIS_KERNEL_PROT_GPU_RW, 0, phys_,
            align);
        if (rc != 0) {
            jill_paths::trace("videoout: map failed 0x%x", rc);
            err = "sceKernelMapDirectMemory failed";
            return false;
        }
        void* ptrs[kBufferCount];
        for (int i = 0; i < kBufferCount; ++i) {
            buffers_[i] = map_ + aligned * static_cast<size_t>(i);
            ptrs[i] = buffers_[i];
        }
        buffers_[kBlankIndex] =
            map_ + aligned * static_cast<size_t>(kBlankIndex);
        std::memset(buffers_[kBlankIndex], 0, buffer_size_);

        rc = sceVideoOutRegisterBuffers(handle_, 0, ptrs, kBufferCount,
                                        &attr_);
        if (rc < 0) {
            jill_paths::trace("videoout: register failed 0x%x", rc);
            err = "sceVideoOutRegisterBuffers failed";
            return false;
        }
        group_ = rc;
        void* blank = buffers_[kBlankIndex];
        rc = sceVideoOutRegisterBuffers(handle_, kBlankIndex, &blank, 1, &attr_);
        if (rc < 0) {
            jill_paths::trace("videoout: blank register failed 0x%x", rc);
            blank_group_ = -1;
        } else {
            blank_group_ = rc;
        }

        rc = sceKernelCreateEqueue(&flip_queue_, "jillFlip");
        if (rc != 0) {
            jill_paths::trace("videoout: equeue failed 0x%x", rc);
            err = "sceKernelCreateEqueue failed";
            return false;
        }
        rc = sceVideoOutAddFlipEvent(flip_queue_, handle_, nullptr);
        if (rc != 0) {
            jill_paths::trace("videoout: add flip event failed 0x%x", rc);
            err = "sceVideoOutAddFlipEvent failed";
            return false;
        }
        sceVideoOutSetFlipRate(handle_, ORBIS_VIDEO_OUT_FLIP_60HZ);
        jill_paths::trace("videoout: ready (buffers=%d, %.1f MB each)",
                          kBufferCount,
                          static_cast<double>(buffer_size_) / (1024.0 * 1024.0));
        // Lets a log tell this build from releases that unregistered the group
        // while it was still being scanned out (the source of CE-34878-0).
        jill_paths::trace("videoout: quit returns the display to the system");
        return true;
    }

    void apply(int pixel_w, int pixel_h, int mode, int display_w,
               int display_h) override {
        (void)pixel_w;
        (void)pixel_h;
        (void)mode;
        (void)display_w;
        (void)display_h;
    }

    void present(const uint32_t* argb, int w, int h) override {
        if (map_ == nullptr || w <= 0 || h <= 0) {
            return;
        }
        uint8_t* dst_base = buffers_[write_];
        // A8B8G8R8 in memory is R,G,B,A.
        std::memset(dst_base, 0, buffer_size_);

        int scale = std::min(width_ / w, height_ / h);
        if (scale < 1) {
            scale = 1;
        }
        const int out_w = w * scale;
        const int out_h = h * scale;
        const int off_x = (width_ - out_w) / 2;
        const int off_y = (height_ - out_h) / 2;

        for (int sy = 0; sy < h; ++sy) {
            const uint32_t* src = argb + static_cast<size_t>(sy) * w;
            for (int sx = 0; sx < w; ++sx) {
                const uint32_t c = src[sx];
                const uint32_t r = (c >> 16) & 0xFFu;
                const uint32_t g = (c >> 8) & 0xFFu;
                const uint32_t b = c & 0xFFu;
                const uint32_t v = 0xFF000000u | (b << 16) | (g << 8) | r;
                for (int dy = 0; dy < scale; ++dy) {
                    uint32_t* row =
                        reinterpret_cast<uint32_t*>(dst_base) +
                        static_cast<size_t>(off_y + sy * scale + dy) * width_;
                    uint32_t* out = row + (off_x + sx * scale);
                    for (int dx = 0; dx < scale; ++dx) {
                        out[dx] = v;
                    }
                }
            }
        }

        const int32_t rc = sceVideoOutSubmitFlip(
            handle_, write_, ORBIS_VIDEO_OUT_FLIP_VSYNC,
            static_cast<int64_t>(frame_id_));
        if (rc != 0 && !flip_error_logged_) {
            flip_error_logged_ = true;
            jill_paths::trace("videoout: submit flip failed 0x%x", rc);
        }
        if (!first_frame_logged_) {
            first_frame_logged_ = true;
            jill_paths::trace("videoout: first present %dx%d -> %dx%d @%d,%d",
                              w, h, out_w, out_h, off_x, off_y);
        }

        // Keep two flips in flight: waiting for the oldest one is normally
        // already complete, so the CPU is never stalled on the display.
        if (frame_id_ >= 2) {
            OrbisKernelEvent ev{};
            int out = 0;
            sceKernelWaitEqueue(flip_queue_, &ev, 1, &out, nullptr);
        }
        ++frame_id_;
        write_ = (write_ + 1) % kBufferCount;
    }

    void set_title(const char* title) override { (void)title; }

    void shutdown() override {
        if (shut_down_) {
            return;
        }
        shut_down_ = true;
        if (handle_ <= 0) {
            return;
        }
        // Consume the flip events that are still queued so the display has
        // finished everything submitted by present().
        if (flip_queue_ != 0) {
            for (int i = 0; i < kBufferCount; ++i) {
                OrbisKernelEvent ev{};
                int out = 0;
                OrbisKernelUseconds timeout = 50000;  // 50 ms
                if (sceKernelWaitEqueue(flip_queue_, &ev, 1, &out, &timeout) !=
                    0) {
                    break;
                }
            }
        }
        jill_paths::trace("videoout: flips drained (pending=%d)",
                          sceVideoOutIsFlipPending(handle_));

        int32_t urc = 0;
        if (frame_id_ > 0) {
            // Swap the display away from our buffers in two steps.  First the
            // spare blank buffer, which lets the presentation group be
            // unregistered; then the system's own blank frame (buffer index
            // -1), so no memory of ours is being scanned when the process
            // exits.  Skipping the second step is what leaves group 1 busy.
            const auto flip_and_wait = [&](int index, int64_t arg,
                                           const char* what) {
                const int32_t rc = sceVideoOutSubmitFlip(
                    handle_, index, ORBIS_VIDEO_OUT_FLIP_VSYNC, arg);
                jill_paths::trace("videoout: %s flip rc=0x%x", what, rc);
                if (rc != 0) {
                    return false;
                }
                for (int i = 0; i < 20; ++i) {
                    OrbisVideoOutFlipStatus st{};
                    if (sceVideoOutGetFlipStatus(handle_, &st) == 0 &&
                        st.flipArg == arg && st.numFlipPending == 0) {
                        jill_paths::trace("videoout: %s frame presented", what);
                        return true;
                    }
                    if (flip_queue_ == 0) {
                        break;
                    }
                    OrbisKernelEvent ev{};
                    int out = 0;
                    OrbisKernelUseconds timeout = 50000;  // 50 ms
                    if (sceKernelWaitEqueue(flip_queue_, &ev, 1, &out,
                                            &timeout) != 0) {
                        break;
                    }
                }
                return false;
            };

            int64_t flip_arg = frame_id_ + 1;
            const bool have_spare = blank_group_ >= 0;
            if (have_spare) {
                flip_and_wait(kBlankIndex, flip_arg, "spare");
                ++flip_arg;
            }
            // Release the presentation group; it is no longer on screen.
            if (group_ >= 0) {
                urc = sceVideoOutUnregisterBuffers(handle_, group_);
                jill_paths::trace("videoout: unregister group %d rc=0x%x",
                                  group_, urc);
            }
            if (have_spare) {
                // Hand the display back to the system before dropping the
                // spare group, otherwise group 1 is still being scanned.
                flip_and_wait(-1, flip_arg, "system blank");
                const int32_t brc =
                    sceVideoOutUnregisterBuffers(handle_, blank_group_);
                jill_paths::trace("videoout: unregister group %d rc=0x%x",
                                  blank_group_, brc);
                if (urc == 0) {
                    urc = brc;
                }
            } else {
                flip_and_wait(-1, flip_arg, "system blank");
            }
        } else if (group_ >= 0) {
            // No frame was ever presented, but the groups may still exist.
            urc = sceVideoOutUnregisterBuffers(handle_, group_);
            jill_paths::trace("videoout: unregister group %d rc=0x%x", group_,
                              urc);
            if (blank_group_ >= 0 && blank_group_ != group_) {
                const int32_t brc =
                    sceVideoOutUnregisterBuffers(handle_, blank_group_);
                jill_paths::trace("videoout: unregister group %d rc=0x%x",
                                  blank_group_, brc);
                if (urc == 0) {
                    urc = brc;
                }
            }
        }

        const int32_t crc = sceVideoOutClose(handle_);
        handle_ = 0;
        jill_paths::trace("videoout: close rc=0x%x", crc);

        if (urc != 0 || crc != 0) {
            // The display may still reference the buffers and the port may
            // still be live.  Leaving the memory mapped and letting process
            // exit reclaim it is the only safe option.
            jill_paths::trace("videoout: leaving display memory to process exit");
            return;
        }
        // Give the display pipeline a moment to drop references to the last
        // scanned buffer before the memory goes back to the allocator.
        sceKernelUsleep(100000);

        if (flip_queue_ != 0) {
            sceKernelDeleteEqueue(flip_queue_);
            flip_queue_ = 0;
        }
        if (map_ != nullptr) {
            sceKernelMunmap(map_, total_size_);
            map_ = nullptr;
        }
        if (phys_ != 0) {
            sceKernelReleaseDirectMemory(phys_, total_size_);
            phys_ = 0;
        }
        jill_paths::trace("videoout: shut down");
        for (int i = 0; i < kTotalBuffers; ++i) {
            buffers_[i] = nullptr;
        }
    }

private:
    int handle_ = 0;
    int width_ = 0;
    int height_ = 0;
    size_t buffer_size_ = 0;
    size_t total_size_ = 0;
    OrbisVideoOutBufferAttribute attr_{};
    uint8_t* map_ = nullptr;
    off_t phys_ = 0;
    uint8_t* buffers_[kTotalBuffers] = {};
    int group_ = -1;
    int blank_group_ = -1;
    OrbisKernelEqueue flip_queue_ = 0;
    int write_ = 0;
    int64_t frame_id_ = 0;
    bool flip_error_logged_ = false;
    bool first_frame_logged_ = false;
    bool shut_down_ = false;
};

}  // namespace

std::unique_ptr<Screen> create_screen_videoout() {
    return std::unique_ptr<Screen>(new ScreenVideoOut());
}
