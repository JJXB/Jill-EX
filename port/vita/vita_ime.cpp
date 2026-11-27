#include "vita_ime.h"

#include <psp2/common_dialog.h>
#include <psp2/ime_dialog.h>

#include <cstdint>

namespace vita_ime {
namespace {

bool active_ = false;
SceWChar16 buffer_[SCE_IME_DIALOG_MAX_TEXT_LENGTH + 1] = {};
SceWChar16 title_[] = {'E', 'N', 'T', 'E', 'R', ' ', 'N', 'A', 'M', 'E', 0};
SceWChar16 initial_[SCE_IME_DIALOG_MAX_TEXT_LENGTH + 1] = {};

void append_utf8(std::string& out, uint32_t cp) {
    if (cp < 0x80) {
        out.push_back(static_cast<char>(cp));
    } else if (cp < 0x800) {
        out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp < 0x10000) {
        out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else {
        out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
}

std::string to_utf8(const SceWChar16* text) {
    std::string out;
    if (!text) {
        return out;
    }
    for (size_t i = 0; text[i] != 0; ++i) {
        uint32_t cp = text[i];
        if (cp >= 0xD800 && cp <= 0xDBFF && text[i + 1] >= 0xDC00 &&
            text[i + 1] <= 0xDFFF) {
            cp = 0x10000 + ((cp - 0xD800) << 10) + (text[i + 1] - 0xDC00);
            ++i;
        }
        append_utf8(out, cp);
    }
    return out;
}

void from_utf8(const char* text, SceWChar16* out, size_t max_chars) {
    if (!out || max_chars == 0) {
        return;
    }
    size_t n = 0;
    if (text) {
        for (const unsigned char* p =
                 reinterpret_cast<const unsigned char*>(text);
             *p != 0 && n + 1 < max_chars;) {
            uint32_t cp = *p++;
            if ((cp & 0x80) == 0) {
                // 1 byte
            } else if ((cp & 0xE0) == 0xC0 && *p) {
                cp = ((cp & 0x1F) << 6) | (*p++ & 0x3F);
            } else if ((cp & 0xF0) == 0xE0 && p[0] && p[1]) {
                cp = ((cp & 0x0F) << 12) | ((p[0] & 0x3F) << 6) |
                     (p[1] & 0x3F);
                p += 2;
            } else {
                cp = '?';
            }
            out[n++] = static_cast<SceWChar16>(cp > 0xFFFF ? '?' : cp);
        }
    }
    out[n] = 0;
}

}  // namespace

bool start(const char* initial) {
    close();
    from_utf8(initial, initial_, SCE_IME_DIALOG_MAX_TEXT_LENGTH + 1);
    for (SceWChar16& c : buffer_) {
        c = 0;
    }
    SceImeDialogParam param;
    sceImeDialogParamInit(&param);
    param.supportedLanguages = 0;
    param.languagesForced = SCE_FALSE;
    param.type = SCE_IME_TYPE_DEFAULT;
    param.option = 0;
    param.textBoxMode = SCE_IME_DIALOG_TEXTBOX_MODE_WITH_CLEAR;
    param.maxTextLength = SCE_IME_DIALOG_MAX_TEXT_LENGTH;
    param.title = title_;
    param.initialText = initial_;
    param.inputTextBuffer = buffer_;
    if (sceImeDialogInit(&param) < 0) {
        return false;
    }
    active_ = true;
    return true;
}

void close() {
    if (!active_) {
        return;
    }
    if (sceImeDialogGetStatus() == SCE_COMMON_DIALOG_STATUS_RUNNING) {
        sceImeDialogAbort();
    }
    sceImeDialogTerm();
    active_ = false;
}

bool poll(std::string& text, int& result) {
    if (!active_) {
        return false;
    }
    const SceCommonDialogStatus status = sceImeDialogGetStatus();
    if (status == SCE_COMMON_DIALOG_STATUS_FINISHED) {
        SceImeDialogResult res{};
        sceImeDialogGetResult(&res);
        text = to_utf8(buffer_);
        result = res.button == SCE_IME_DIALOG_BUTTON_ENTER ? 1 : 2;
        sceImeDialogTerm();
        active_ = false;
        return true;
    }
    if (status == SCE_COMMON_DIALOG_STATUS_NONE) {
        text.clear();
        result = 2;
        active_ = false;
        return true;
    }
    return false;
}

}  // namespace vita_ime
