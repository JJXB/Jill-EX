#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

// `*.mac` recorded input scripts (`docs/jill1-engine.md` 13.1).  There is no
// header: byte 0 starts record 0.  Record 0 has no timestamp and takes effect
// at tick 0; every later record is preceded by its absolute tick (base-128
// varint) and only updates the channels named by its mask.
//
// The file is a stream of input *changes*, so a record's mask bit is also the
// "this channel is present in this record" flag.
enum MacroMask : int {
    kMacroMaskH = 0x01,     // horizontal direction (-1 / 0 / +1)
    kMacroMaskV = 0x02,     // vertical direction (-1 / 0 / +1)
    kMacroMaskBtn1 = 0x04,  // action button 1 (jump)
    kMacroMaskBtn2 = 0x08,  // action button 2 (throw)
    kMacroMaskKey = 0x10,   // last ASCII key code
    kMacroMaskKnown = 0x1F,
};

struct MacroRecord {
    int time = 0;   // absolute tick when the record takes effect
    int mask = 0;   // kMacroMask* bits that this record carries
    int h = 0;      // -1 / 0 / +1 (valid when mask & kMacroMaskH)
    int v = 0;      // -1 / 0 / +1 (valid when mask & kMacroMaskV)
    bool btn1 = false;
    bool btn2 = false;
    int key = 0;    // 0..255 (valid when mask & kMacroMaskKey)
};

// Parse a `*.mac` byte stream.  `out` receives every record that decoded, even
// when a later record is truncated; `diag` is empty on a clean parse and
// otherwise carries a human-readable warning.  Returns false only when no
// record could be read at all.
bool macro_parse(const uint8_t* data, size_t size, std::vector<MacroRecord>& out,
                 std::string& diag);

// Read a `*.mac` from disk (16 MiB cap, same as the engine replayer).
bool macro_load(const std::string& path, std::vector<MacroRecord>& out,
                std::string& diag);

// Serialise records.  The input is not modified; records are written in
// ascending tick order (stable, so equal ticks keep their order) and a leading
// mask-0 marker is added when the first record has a non-zero tick (record 0
// has no timestamp in the format).  Returns false and fills `diag` for
// out-of-range values.
bool macro_encode(const std::vector<MacroRecord>& records,
                  std::vector<uint8_t>& out, std::string& diag);

// macro_encode() to disk (truncates an existing file only after a successful
// encode).
bool macro_save(const std::string& path, const std::vector<MacroRecord>& records,
                std::string& diag);

// Highest tick in the stream (0 for an empty list).
int macro_last_tick(const std::vector<MacroRecord>& records);

// Input state produced by applying all records with time <= `tick`.  Used by
// the editor to show what the engine is holding at the playhead.
struct MacroInputState {
    int h = 0;
    int v = 0;
    bool btn1 = false;
    bool btn2 = false;
    int key = 0;
};
MacroInputState macro_state_at(const std::vector<MacroRecord>& records, int tick);
