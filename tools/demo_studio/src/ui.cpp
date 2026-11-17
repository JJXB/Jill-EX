#include "ui.h"

#include <SDL.h>

#include <algorithm>
#include <cstring>

#include "engine.h"
#include "framebuffer.h"

namespace demo_ui {

namespace {
constexpr int kCellW = 6;
constexpr int kCellH = 8;
constexpr int kFirst = 32;
constexpr int kLast = 126;
constexpr int kGlyphCount = kLast - kFirst + 1;

void set_colour(SDL_Renderer* r, uint32_t rgb) {
    SDL_SetRenderDrawColor(r, static_cast<Uint8>((rgb >> 16) & 0xFF),
                           static_cast<Uint8>((rgb >> 8) & 0xFF),
                           static_cast<Uint8>(rgb & 0xFF), 0xFF);
}
}  // namespace

Ui::~Ui() { shutdown(); }

bool Ui::init(SDL_Renderer* renderer, Engine& font_engine, std::string& err) {
    shutdown();
    renderer_ = renderer;
    // Build one mask atlas from the engine's 6-px font: clear cell, draw the
    // character, keep every non-zero pixel as opaque white.
    std::vector<uint32_t> pixels(static_cast<size_t>(kCellW * kGlyphCount) *
                                 kCellH);
    Framebuffer fb;
    for (int ch = kFirst; ch <= kLast; ++ch) {
        fb.clear(0);
        const char s[2] = {static_cast<char>(ch), 0};
        font_engine.draw_text(fb, 0, 0, 15, s);
        for (int y = 0; y < kCellH; ++y) {
            for (int x = 0; x < kCellW; ++x) {
                const size_t dx = static_cast<size_t>((ch - kFirst) * kCellW + x);
                const size_t dy = static_cast<size_t>(y);
                const uint8_t v = fb.get(x, y);
                pixels[dy * static_cast<size_t>(kCellW * kGlyphCount) + dx] =
                    v != 0 ? 0xFFFFFFFFu : 0x00000000u;
            }
        }
    }
    SDL_Texture* tex = SDL_CreateTexture(
        renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC,
        kCellW * kGlyphCount, kCellH);
    if (!tex) {
        err = std::string("SDL_CreateTexture(font): ") + SDL_GetError();
        return false;
    }
    SDL_UpdateTexture(tex,
                      nullptr, pixels.data(),
                      kCellW * kGlyphCount * static_cast<int>(sizeof(uint32_t)));
    SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
    SDL_SetTextureScaleMode(tex, SDL_ScaleModeNearest);
    glyphs_ = tex;
    return true;
}

void Ui::shutdown() {
    if (glyphs_) {
        SDL_DestroyTexture(glyphs_);
        glyphs_ = nullptr;
    }
    renderer_ = nullptr;
}

void Ui::text(int x, int y, const std::string& s, uint32_t rgb, int scale) {
    if (!renderer_ || !glyphs_ || scale <= 0) {
        return;
    }
    SDL_SetTextureColorMod(glyphs_, static_cast<Uint8>((rgb >> 16) & 0xFF),
                           static_cast<Uint8>((rgb >> 8) & 0xFF),
                           static_cast<Uint8>(rgb & 0xFF));
    int cx = x;
    for (char raw : s) {
        int ch = static_cast<unsigned char>(raw);
        if (ch < kFirst || ch > kLast) {
            ch = '?';
        }
        if (ch != ' ') {
            SDL_Rect src{kCellW * (ch - kFirst), 0, kCellW, kCellH};
            SDL_Rect dst{cx, y, kCellW * scale, kCellH * scale};
            SDL_RenderCopy(renderer_, glyphs_, &src, &dst);
        }
        cx += kCellW * scale;
    }
}

void Ui::text_shadow(int x, int y, const std::string& s, uint32_t rgb,
                     int scale, uint32_t shadow) {
    text(x + scale, y + scale, s, shadow, scale);
    text(x, y, s, rgb, scale);
}

std::string Ui::ellipsize(const std::string& s, int max_w, int scale) const {
    const int cell = kCellW * (scale > 0 ? scale : 1);
    const int max_chars = max_w / cell;
    if (static_cast<int>(s.size()) <= max_chars) {
        return s;
    }
    if (max_chars <= 2) {
        return std::string(static_cast<size_t>(max_chars > 0 ? max_chars : 0),
                           ' ');
    }
    return s.substr(0, static_cast<size_t>(max_chars - 2)) + "..";
}

std::vector<std::string> Ui::wrap(const std::string& s, int max_w,
                                  int scale) const {
    const int cell = kCellW * (scale > 0 ? scale : 1);
    const size_t max_chars =
        static_cast<size_t>(std::max(1, max_w / cell));
    std::vector<std::string> out;
    std::string line;
    size_t pos = 0;
    while (pos <= s.size()) {
        size_t space = s.find(' ', pos);
        std::string word =
            s.substr(pos, space == std::string::npos ? std::string::npos
                                                     : space - pos);
        if (!word.empty()) {
            const std::string candidate =
                line.empty() ? word : line + " " + word;
            if (candidate.size() <= max_chars) {
                line = candidate;
            } else {
                if (!line.empty()) {
                    out.push_back(line);
                }
                line = word;
                // A single word longer than the line is hard-split.
                while (line.size() > max_chars) {
                    out.push_back(line.substr(0, max_chars));
                    line.erase(0, max_chars);
                }
            }
        }
        if (space == std::string::npos) {
            break;
        }
        pos = space + 1;
    }
    if (!line.empty()) {
        out.push_back(line);
    }
    return out;
}

void Ui::fill(const Rect& r, uint32_t rgb) { fill(r.x, r.y, r.w, r.h, rgb); }

void Ui::fill(int x, int y, int w, int h, uint32_t rgb) {
    if (!renderer_ || w <= 0 || h <= 0) {
        return;
    }
    set_colour(renderer_, rgb);
    SDL_Rect r{x, y, w, h};
    SDL_RenderFillRect(renderer_, &r);
}

void Ui::frame(const Rect& r, uint32_t rgb, int thickness) {
    hline(r.x, r.y, r.w, rgb);
    hline(r.x, r.y + r.h - thickness, r.w, rgb);
    vline(r.x, r.y, r.h, rgb);
    vline(r.x + r.w - thickness, r.y, r.h, rgb);
}

void Ui::hline(int x, int y, int w, uint32_t rgb) {
    fill(x, y, w, 1, rgb);
}

void Ui::vline(int x, int y, int h, uint32_t rgb) {
    fill(x, y, 1, h, rgb);
}

}  // namespace demo_ui
