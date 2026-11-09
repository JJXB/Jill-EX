#include "textpages.h"

#include <cstdio>
#include <cstring>

namespace {

uint16_t rd16(const uint8_t* p) {
    return static_cast<uint16_t>(p[0] | (p[1] << 8));
}

uint32_t rd32(const uint8_t* p) {
    return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) |
           (static_cast<uint32_t>(p[2]) << 16) |
           (static_cast<uint32_t>(p[3]) << 24);
}

// The original page renderer takes a leading '0'..'7' as the line colour and
// defaults to 7 (`FUN_22ab_0dc6`).
void split_page(const uint8_t* data, size_t size, TextPage& page) {
    std::string line;
    auto flush = [&]() {
        // Drop trailing NULs / high-bit padding.
        while (!line.empty() &&
               (static_cast<uint8_t>(line.back()) < 0x20 ||
                static_cast<uint8_t>(line.back()) >= 0x7F)) {
            line.pop_back();
        }
        if (line.empty()) {
            page.lines.push_back(TextPage::Line{7, std::string()});
            return;
        }
        TextPage::Line out;
        out.colour = 7;
        size_t pos = 0;
        // Only the ASCII digit range the original accepts.
        if (line.size() > 1 && line[0] > '/' && line[0] < '8') {
            out.colour = static_cast<uint8_t>(line[0] - '0');
            pos = 1;
        }
        out.text = line.substr(pos);
        page.lines.push_back(std::move(out));
    };
    for (size_t i = 0; i < size; ++i) {
        const char c = static_cast<char>(data[i]);
        if (c == '\r') {
            flush();
            line.clear();
        } else if (c == '\n') {
            // CRLF; 'flush' already ran on the CR.
        } else if (static_cast<uint8_t>(c) >= 0x20 && static_cast<uint8_t>(c) < 0x7F) {
            line.push_back(c);
        }
    }
    if (!line.empty()) {
        flush();
    }
    // Trim the blank tail left by the final CR.
    while (!page.lines.empty() && page.lines.back().text.empty()) {
        page.lines.pop_back();
    }
}

}  // namespace

bool TextPages::load(const std::string& path, std::string& err) {
    pages_.clear();
    loaded_ = false;
    FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) {
        err = "cannot open " + path;
        return false;
    }
    std::fseek(f, 0, SEEK_END);
    long size = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);
    if (size < 0x280) {
        std::fclose(f);
        err = path + ": too small for a .vcl page table";
        return false;
    }
    std::vector<uint8_t> data(static_cast<size_t>(size));
    if (std::fread(data.data(), 1, data.size(), f) != data.size()) {
        std::fclose(f);
        err = "short read on " + path;
        return false;
    }
    std::fclose(f);

    // Header: 50 x u32 sound offsets (0x000), 50 x u16 sizes (0x0c8),
    // 50 x u16 rates (0x12c), 40 x u32 page offsets (0x190),
    // 40 x u16 page sizes (0x230).  The page number is the 0-based index into
    // that table (menu INSTRUCTIONS passes 1 = "How to Play").
    pages_.assign(40, TextPage{});
    found_ = 0;
    for (int n = 0; n < 40; ++n) {
        const uint32_t off = rd32(&data[0x190 + 4 * n]);
        const uint16_t len = rd16(&data[0x230 + 2 * n]);
        if (!len) {
            continue;
        }
        if (off > data.size() || len > data.size() - off) {
            err = path + ": page " + std::to_string(n) + " out of range";
            pages_.clear();
            return false;
        }
        split_page(&data[off], len, pages_[static_cast<size_t>(n)]);
        ++found_;
    }
    loaded_ = true;
    if (!found_) {
        pages_.clear();
    }
    return true;
}

const TextPage* TextPages::page(int number) const {
    if (number < 0 || number >= static_cast<int>(pages_.size())) {
        return nullptr;
    }
    const TextPage& p = pages_[static_cast<size_t>(number)];
    return p.valid() ? &p : nullptr;
}
