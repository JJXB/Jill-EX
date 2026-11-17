#include "macro.h"

#include <algorithm>
#include <cstdio>

namespace {

constexpr size_t kMaxBytes = 16 * 1024 * 1024;
constexpr size_t kMaxRecords = 1000000;

void append_diag(std::string& diag, const std::string& text) {
    if (!diag.empty()) {
        diag += "; ";
    }
    diag += text;
}

void write_varint(std::vector<uint8_t>& out, int value) {
    uint32_t v = static_cast<uint32_t>(value);
    while (v >= 0x80) {
        out.push_back(static_cast<uint8_t>((v & 0x7F) | 0x80));
        v >>= 7;
    }
    out.push_back(static_cast<uint8_t>(v));
}

}  // namespace

bool macro_parse(const uint8_t* data, size_t size,
                 std::vector<MacroRecord>& out, std::string& diag) {
    out.clear();
    diag.clear();
    if (size == 0) {
        diag = "empty file";
        return false;
    }
    if (size > kMaxBytes) {
        diag = "file too large";
        return false;
    }
    size_t p = 0;
    bool ok = true;
    int time = 0;
    bool first = true;
    // Record layout: rec[0] is mask + values; every later record starts with an
    // absolute tick (base-128 varint) and then mask + values.
    while (p < size && ok) {
        if (!first) {
            int value = 0;
            int shift = 0;
            bool more = true;
            while (more) {
                if (p >= size) {
                    ok = false;
                    break;
                }
                const uint8_t b = data[p++];
                value |= static_cast<int>((b & 0x7F) << shift);
                shift += 7;
                more = (b & 0x80) != 0;
                if (shift > 28) {
                    ok = false;
                }
            }
            if (!ok) {
                break;
            }
            if (value < time) {
                append_diag(diag, "tick " + std::to_string(value) +
                                      " goes backwards (previous " +
                                      std::to_string(time) + ") at byte " +
                                      std::to_string(p));
            }
            time = value;
        } else {
            first = false;
        }
        if (p >= size) {
            ok = false;
            break;
        }
        const int mask = data[p++];
        int vals[8] = {0, 0, 0, 0, 0, 0, 0, 0};
        // Consume every mask bit so reserved bits stay aligned.
        for (int bit = 0; bit < 8; ++bit) {
            if ((mask & (1 << bit)) == 0) {
                continue;
            }
            if (p >= size) {
                ok = false;
                break;
            }
            vals[bit] = static_cast<int8_t>(data[p++]);
        }
        if (!ok) {
            break;
        }
        MacroRecord r;
        r.time = time;
        r.mask = mask;
        r.h = vals[0] < 0 ? -1 : (vals[0] > 0 ? 1 : 0);
        r.v = vals[1] < 0 ? -1 : (vals[1] > 0 ? 1 : 0);
        r.btn1 = vals[2] != 0;
        r.btn2 = vals[3] != 0;
        r.key = vals[4] & 0xFF;
        out.push_back(r);
        if (out.size() > kMaxRecords) {
            append_diag(diag, "too many records");
            ok = false;
            break;
        }
    }
    if (!ok) {
        append_diag(diag, "malformed or truncated at byte " + std::to_string(p) +
                              "; loaded " + std::to_string(out.size()) +
                              " records");
    }
    return !out.empty();
}

bool macro_load(const std::string& path, std::vector<MacroRecord>& out,
                std::string& diag) {
    out.clear();
    diag.clear();
    FILE* f = std::fopen(path.c_str(), "rb");
    if (!f) {
        diag = "cannot open";
        return false;
    }
    std::fseek(f, 0, SEEK_END);
    const long size = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);
    if (size <= 0) {
        std::fclose(f);
        diag = size == 0 ? "empty file" : "cannot read file size";
        return false;
    }
    if (static_cast<size_t>(size) > kMaxBytes) {
        std::fclose(f);
        diag = "file too large (" + std::to_string(size) + " bytes)";
        return false;
    }
    std::vector<uint8_t> data(static_cast<size_t>(size));
    const size_t got = std::fread(data.data(), 1, data.size(), f);
    std::fclose(f);
    if (got != data.size()) {
        data.resize(got);
        append_diag(diag, "short read (" + std::to_string(got) + " of " +
                              std::to_string(size) + " bytes)");
    }
    if (data.empty()) {
        append_diag(diag, "empty file");
        return false;
    }
    std::string parse_diag;
    const bool ok = macro_parse(data.data(), data.size(), out, parse_diag);
    if (!parse_diag.empty()) {
        append_diag(diag, parse_diag);
    }
    return ok;
}

bool macro_encode(const std::vector<MacroRecord>& records,
                  std::vector<uint8_t>& out, std::string& diag) {
    out.clear();
    diag.clear();
    if (records.empty()) {
        diag = "no records";
        return false;
    }
    std::vector<MacroRecord> sorted = records;
    std::stable_sort(sorted.begin(), sorted.end(),
                     [](const MacroRecord& a, const MacroRecord& b) {
                         return a.time < b.time;
                     });
    for (const MacroRecord& r : sorted) {
        if (r.time < 0) {
            diag = "negative tick " + std::to_string(r.time);
            return false;
        }
        if (r.time > 0x0FFFFFFF) {
            diag = "tick " + std::to_string(r.time) + " is too large";
            return false;
        }
        if (r.h < -1 || r.h > 1 || r.v < -1 || r.v > 1) {
            diag = "direction must be -1, 0 or 1 (tick " +
                   std::to_string(r.time) + ")";
            return false;
        }
        if (r.key < 0 || r.key > 0xFF) {
            diag = "key must be 0..255 (tick " + std::to_string(r.time) + ")";
            return false;
        }
    }
    // Record 0 carries no timestamp, so anything that should start later needs
    // a leading mask-0 marker at tick 0 (a legal time marker in the format).
    if (sorted.front().time != 0) {
        MacroRecord marker;
        marker.time = 0;
        marker.mask = 0;
        sorted.insert(sorted.begin(), marker);
    }
    out.reserve(sorted.size() * 4);
    for (size_t i = 0; i < sorted.size(); ++i) {
        const MacroRecord& r = sorted[i];
        if (i > 0) {
            write_varint(out, r.time);
        }
        const int mask = r.mask & kMacroMaskKnown;
        out.push_back(static_cast<uint8_t>(mask));
        if (mask & kMacroMaskH) {
            out.push_back(static_cast<uint8_t>(static_cast<int8_t>(r.h)));
        }
        if (mask & kMacroMaskV) {
            out.push_back(static_cast<uint8_t>(static_cast<int8_t>(r.v)));
        }
        if (mask & kMacroMaskBtn1) {
            out.push_back(static_cast<uint8_t>(r.btn1 ? 1 : 0));
        }
        if (mask & kMacroMaskBtn2) {
            out.push_back(static_cast<uint8_t>(r.btn2 ? 1 : 0));
        }
        if (mask & kMacroMaskKey) {
            out.push_back(static_cast<uint8_t>(r.key & 0xFF));
        }
    }
    return true;
}

bool macro_save(const std::string& path, const std::vector<MacroRecord>& records,
                std::string& diag) {
    std::vector<uint8_t> data;
    if (!macro_encode(records, data, diag)) {
        return false;
    }
    FILE* f = std::fopen(path.c_str(), "wb");
    if (!f) {
        diag = "cannot write " + path;
        return false;
    }
    const size_t wrote = std::fwrite(data.data(), 1, data.size(), f);
    const bool closed = std::fclose(f) == 0;
    if (wrote != data.size() || !closed) {
        diag = "short write to " + path;
        return false;
    }
    return true;
}

int macro_last_tick(const std::vector<MacroRecord>& records) {
    int last = 0;
    for (const MacroRecord& r : records) {
        last = std::max(last, r.time);
    }
    return last;
}

MacroInputState macro_state_at(const std::vector<MacroRecord>& records,
                               int tick) {
    MacroInputState state;
    for (const MacroRecord& r : records) {
        if (r.time > tick) {
            break;
        }
        if (r.mask & kMacroMaskH) {
            state.h = r.h;
        }
        if (r.mask & kMacroMaskV) {
            state.v = r.v;
        }
        if (r.mask & kMacroMaskBtn1) {
            state.btn1 = r.btn1;
        }
        if (r.mask & kMacroMaskBtn2) {
            state.btn2 = r.btn2;
        }
        if (r.mask & kMacroMaskKey) {
            state.key = r.key;
        }
    }
    return state;
}
