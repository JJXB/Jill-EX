#pragma once
#include <cstdint>
#include <string>
#include <vector>

struct SDL_Renderer;
struct SDL_Texture;
class Engine;

namespace demo_ui {

// UI colours (0xRRGGBB).
constexpr uint32_t kBg = 0x14171E;
constexpr uint32_t kPanel = 0x1C2029;
constexpr uint32_t kPanelAlt = 0x232833;
constexpr uint32_t kBorder = 0x3A4354;
constexpr uint32_t kText = 0xD8DEE9;
constexpr uint32_t kTextDim = 0x8B95A7;
constexpr uint32_t kAccent = 0x5AA9FF;
constexpr uint32_t kAccentDim = 0x2B5E93;
constexpr uint32_t kGood = 0x74D68C;
constexpr uint32_t kWarn = 0xE8B45A;
constexpr uint32_t kBad = 0xE0645C;
constexpr uint32_t kSelection = 0x2C4B6E;

struct Rect {
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
    bool contains(int px, int py) const {
        return px >= x && py >= y && px < x + w && py < y + h;
    }
};

// Thin drawing layer over SDL_Renderer plus a glyph atlas built from the
// game's own font shapes, so the tool looks like the engine's UI.
class Ui {
public:
    ~Ui();
    // `font_engine` must be initialised (any episode's shape bank works).
    bool init(SDL_Renderer* renderer, Engine& font_engine, std::string& err);
    void shutdown();

    // Integer-scaled text; scale 1 is the native 6x8 cell.
    void text(int x, int y, const std::string& s, uint32_t rgb,
              int scale = 1);
    void text_shadow(int x, int y, const std::string& s, uint32_t rgb,
                     int scale = 1, uint32_t shadow = 0x000000);
    int text_w(const std::string& s, int scale = 1) const {
        return static_cast<int>(s.size()) * 6 * scale;
    }
    // Cuts a string to fit `max_w` pixels (with ".." when shortened).
    std::string ellipsize(const std::string& s, int max_w,
                          int scale = 1) const;
    // Greedy word wrap to `max_w` pixels; words longer than a line are split.
    std::vector<std::string> wrap(const std::string& s, int max_w,
                                  int scale = 1) const;

    void fill(const Rect& r, uint32_t rgb);
    void fill(int x, int y, int w, int h, uint32_t rgb);
    void frame(const Rect& r, uint32_t rgb, int thickness = 1);
    void hline(int x, int y, int w, uint32_t rgb);
    void vline(int x, int y, int h, uint32_t rgb);
    int line_h(int scale = 1) const { return 8 * scale; }

    SDL_Renderer* renderer() const { return renderer_; }

private:
    SDL_Renderer* renderer_ = nullptr;
    SDL_Texture* glyphs_ = nullptr;
};

}  // namespace demo_ui
