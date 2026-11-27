#pragma once

#include <string>

// Native Vita on-screen keyboard for text prompts.  SDL video is not
// initialised on this target (vita2d owns the display), so SDL_StartTextInput
// cannot open the IME dialog.  These calls use sceImeDialog directly.
namespace vita_ime {

bool start(const char* initial);
void close();

// Returns true once when the dialog closes.  `result` is 1 for ENTER/commit and
// 2 for cancel.  `text` is the UTF-8 buffer entered by the user.
bool poll(std::string& text, int& result);

}  // namespace vita_ime
