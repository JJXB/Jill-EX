#pragma once
#include <cstdint>
#include <string>
#include <vector>

// One page from the sampled-sound bank's text section (`.vcl`).
//
// The bank has a 0x280-byte header: 50 sound offsets/sizes/rates, then a
// 40-entry page table (u32 file offset, u16 size) at 0x190 / 0x230.  A page is
// a `\r`-separated block whose first line is the title; a leading digit '0'-'7'
// on a line is its colour, exactly like the original line renderer
// (22AB:0dc6).
struct TextPage {
    struct Line {
        uint8_t colour = 7;
        std::string text;
    };
    std::vector<Line> lines;
    bool valid() const { return !lines.empty(); }
};

class TextPages {
public:
    // Returns false and sets `err` for a missing or malformed bank.  A bank
    // with an all-zero page table loads successfully but has no pages.
    bool load(const std::string& path, std::string& err);

    bool loaded() const { return loaded_; }
    // Page numbers are 0..39, straight indexes into the bank's page table
    // (`FUN_1581_03ef` indexes it with the caller's page number).
    int count() const { return static_cast<int>(pages_.size()); }
    bool has_pages() const { return found_ > 0; }
    const TextPage* page(int number) const;

private:
    std::vector<TextPage> pages_;
    int found_ = 0;
    bool loaded_ = false;
};
