#pragma once
#include <cstdint>
#include <string>
#include <vector>

// One parsed CMF/MIDI track event, with an absolute tick time.
struct CmfEvent {
    uint32_t tick = 0;
    uint8_t type = 0;  // 0x80 note off, 0x90 note on, 0xB0 controller, 0xC0 program
    uint8_t chan = 0;
    uint8_t a = 0;
    uint8_t b = 0;
};

// A Creative Music File (`*.ddt`, and the Onesimus `SONG*.ARK` tracks).  The
// instrument block is stored as the eleven OPL2 register bytes the original
// driver writes, so the sequencer can feed them straight to the chip.
struct CmfSong {
    std::string filename;
    std::string title;
    int ticks_per_second = 96;
    int instrument_count = 0;
    uint8_t instruments[128][11] = {};
    std::vector<CmfEvent> events;
    uint32_t end_tick = 0;
};

bool cmf_load(const std::string& path, CmfSong& song, std::string& err);
bool cmf_parse(const std::vector<uint8_t>& data, CmfSong& song, std::string& err);
