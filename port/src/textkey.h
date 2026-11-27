#pragma once
#include <cstddef>
#include <string>

class Engine;
struct Framebuffer;

// Controller-operated keyboard for console targets whose SDL has no usable
// native text service (PS4 / Xbox).  The same widget doubles as the grid for
// the Noisemaker screen, which needs the original QWERTY layout.
//
// Rows 0..3 are the layout keys "1234567890-= / QWERTYUIOP[] / ASDFGHJKL;' /
// ZXCVBNM,./" (the original Noisemaker order, so character -> sound slot is a
// direct index).  Row 4 holds SPACE / DEL / OK / CANCEL.
class TextKeyboard {
public:
    static constexpr int kCols = 12;
    static constexpr int kRows = 5;
    static constexpr int kCellW = 24;
    static constexpr int kCellH = 14;
    static constexpr int kWidth = kCols * kCellW;
    static constexpr int kHeight = kRows * kCellH;

    enum class Action { None, Space, Del, Ok, Cancel };

    // `buffer` may be null for the Noisemaker, where only the highlighted
    // character is used.
    void open(std::string* buffer, size_t max_len);
    void close();
    bool active() const { return active_; }

    void move(int dx, int dy);
    // Press the highlighted key.  Character keys append to the buffer; OK and
    // CANCEL end the entry and return true.  `cancelled()` tells them apart.
    bool press();
    bool cancelled() const { return cancelled_; }
    void backspace();

    char selected_char() const;
    const char* selected_label() const;
    Action selected_action() const;

    void draw(Framebuffer& fb, Engine& engine, int x, int y) const;

private:
    int span_start(int col) const;
    int span_end(int col) const;
    void clamp_selection();

    bool active_ = false;
    bool cancelled_ = false;
    std::string* buffer_ = nullptr;
    size_t max_len_ = 255;
    int row_ = 1;
    int col_ = 0;
};

namespace textkey {
// Runtime override for the in-game D-pad keyboard.  Consoles always use it;
// on PC it can be switched on so a player with a pad can type letters for
// level-editor strings (and hi-score names) without a keyboard.
bool pad_keyboard_enabled();
void set_pad_keyboard_enabled(bool on);

// True on targets where the in-game keyboard should be used instead of
// SDL_StartTextInput / a platform IME.
inline bool controller_only() {
#if defined(JILL_PS4) || defined(JILL_XBOX) || defined(JILL_VITA)
    return true;
#else
    return pad_keyboard_enabled();
#endif
}

// True on console/handheld targets where the Noisemaker page should get a pad
// grid instead of relying on keyboard scancodes.
inline bool pad_noisemaker() {
#if defined(JILL_PS4) || defined(JILL_XBOX) || defined(JILL_VITA)
    return true;
#else
    return false;
#endif
}
}  // namespace textkey
