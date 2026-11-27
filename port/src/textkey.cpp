#include "textkey.h"

#include <cstring>

#include "engine.h"
#include "framebuffer.h"

namespace {

// Original Noisemaker keyboard order (see noisemaker_slot in app.cpp); rows of
// 12/12/11/10 keys with blanks padding the short rows.
const char kLayout[4][TextKeyboard::kCols][3] = {
    {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "="},
    {"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "[", "]"},
    {"A", "S", "D", "F", "G", "H", "J", "K", "L", ";", "'", ""},
    {"Z", "X", "C", "V", "B", "N", "M", ",", ".", "/", "", ""},
};

const char* const kActions[4] = {"SPACE", "DEL", "OK", "CANCEL"};
constexpr int kActionCols[4][2] = {{0, 3}, {4, 6}, {7, 9}, {10, 11}};

// Runtime "use the D-pad keyboard on PC" option (level editor menu / jill.ini).
bool g_pad_keyboard = false;

}  // namespace

namespace textkey {
bool pad_keyboard_enabled() { return g_pad_keyboard; }
void set_pad_keyboard_enabled(bool on) { g_pad_keyboard = on; }
}  // namespace textkey

void TextKeyboard::open(std::string* buffer, size_t max_len) {
    buffer_ = buffer;
    max_len_ = max_len == 0 ? 1 : max_len;
    active_ = true;
    cancelled_ = false;
    row_ = 1;
    col_ = 0;
}

void TextKeyboard::close() {
    active_ = false;
    buffer_ = nullptr;
    cancelled_ = false;
}

int TextKeyboard::span_start(int col) const {
    for (const auto& span : kActionCols) {
        if (col >= span[0] && col <= span[1]) {
            return span[0];
        }
    }
    return 0;
}

int TextKeyboard::span_end(int col) const {
    for (const auto& span : kActionCols) {
        if (col >= span[0] && col <= span[1]) {
            return span[1];
        }
    }
    return kCols - 1;
}

void TextKeyboard::clamp_selection() {
    if (row_ < 0) {
        row_ = 0;
    } else if (row_ >= kRows) {
        row_ = kRows - 1;
    }
    if (col_ < 0) {
        col_ = 0;
    } else if (col_ >= kCols) {
        col_ = kCols - 1;
    }
    if (row_ == kRows - 1) {
        col_ = span_start(col_);
        return;
    }
    if (kLayout[row_][col_][0] != '\0') {
        return;
    }
    // Land on the nearest real key in this row.
    for (int d = 1; d < kCols; ++d) {
        if (col_ - d >= 0 && kLayout[row_][col_ - d][0] != '\0') {
            col_ -= d;
            return;
        }
        if (col_ + d < kCols && kLayout[row_][col_ + d][0] != '\0') {
            col_ += d;
            return;
        }
    }
}

void TextKeyboard::move(int dx, int dy) {
    if (!active_) {
        return;
    }
    if (dy != 0) {
        row_ = (row_ + dy + kRows) % kRows;
        clamp_selection();
        return;
    }
    if (dx == 0) {
        return;
    }
    if (row_ == kRows - 1) {
        // Move between the four action buttons, not between their columns.
        int index = 0;
        for (int i = 0; i < 4; ++i) {
            if (col_ >= kActionCols[i][0] && col_ <= kActionCols[i][1]) {
                index = i;
                break;
            }
        }
        index = (index + (dx > 0 ? 1 : 3)) % 4;
        col_ = kActionCols[index][0];
        return;
    }
    int col = col_;
    for (int step = 0; step < kCols; ++step) {
        col = (col + (dx > 0 ? 1 : kCols - 1)) % kCols;
        if (kLayout[row_][col][0] != '\0') {
            col_ = col;
            return;
        }
    }
}

bool TextKeyboard::press() {
    if (!active_) {
        return false;
    }
    if (row_ < 4) {
        const char c = kLayout[row_][col_][0];
        if (c != '\0' && buffer_ && buffer_->size() < max_len_) {
            buffer_->push_back(c);
        }
        return false;
    }
    switch (selected_action()) {
        case Action::Space:
            if (buffer_ && buffer_->size() < max_len_) {
                buffer_->push_back(' ');
            }
            return false;
        case Action::Del:
            backspace();
            return false;
        case Action::Ok:
            return true;
        case Action::Cancel:
            cancelled_ = true;
            return true;
        default:
            return false;
    }
}

void TextKeyboard::backspace() {
    if (buffer_ && !buffer_->empty() && buffer_->size() > 0) {
        buffer_->pop_back();
    }
}

char TextKeyboard::selected_char() const {
    if (row_ < 0 || row_ >= 4 || col_ < 0 || col_ >= kCols) {
        return '\0';
    }
    return kLayout[row_][col_][0];
}

const char* TextKeyboard::selected_label() const {
    if (row_ < 4) {
        return kLayout[row_][col_];
    }
    return kActions[span_start(col_) == 0 ? 0
                       : (span_start(col_) == 4 ? 1
                          : (span_start(col_) == 7 ? 2 : 3))];
}

TextKeyboard::Action TextKeyboard::selected_action() const {
    if (row_ != 4) {
        return Action::None;
    }
    const int start = span_start(col_);
    if (start == kActionCols[0][0]) {
        return Action::Space;
    }
    if (start == kActionCols[1][0]) {
        return Action::Del;
    }
    if (start == kActionCols[2][0]) {
        return Action::Ok;
    }
    if (start == kActionCols[3][0]) {
        return Action::Cancel;
    }
    return Action::None;
}

void TextKeyboard::draw(Framebuffer& fb, Engine& engine, int x, int y) const {
    fb.fill_rect(x - 2, y - 2, kWidth + 4, kHeight + 4, 8);
    fb.fill_rect(x - 1, y - 1, kWidth + 2, kHeight + 2, 0);
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < kCols; ++col) {
            const char* label = kLayout[row][col];
            if (label[0] == '\0') {
                continue;
            }
            const bool sel = row_ == row && col_ == col;
            const int cx = x + col * kCellW;
            const int cy = y + row * kCellH;
            fb.fill_rect(cx + 1, cy + 1, kCellW - 2, kCellH - 2, 8);
            engine.draw_text(fb, cx + kCellW / 2 - 3, cy + 3, 15, label);
            if (sel) {
                fb.fill_rect(cx, cy, kCellW, 1, 15);
                fb.fill_rect(cx, cy + kCellH - 1, kCellW, 1, 15);
                fb.fill_rect(cx, cy, 1, kCellH, 15);
                fb.fill_rect(cx + kCellW - 1, cy, 1, kCellH, 15);
            }
        }
    }
    for (int i = 0; i < 4; ++i) {
        const int x0 = x + kActionCols[i][0] * kCellW;
        const int x1 = x + (kActionCols[i][1] + 1) * kCellW;
        const int cy = y + 4 * kCellH;
        const bool sel = row_ == 4 &&
                         col_ >= kActionCols[i][0] && col_ <= kActionCols[i][1];
        fb.fill_rect(x0 + 1, cy + 1, x1 - x0 - 2, kCellH - 2, 8);
        engine.draw_text(fb, x0 + 3, cy + 3, 15, kActions[i]);
        if (sel) {
            fb.fill_rect(x0, cy, x1 - x0, 1, 15);
            fb.fill_rect(x0, cy + kCellH - 1, x1 - x0, 1, 15);
            fb.fill_rect(x0, cy, 1, kCellH, 15);
            fb.fill_rect(x1 - 1, cy, 1, kCellH, 15);
        }
    }
}
