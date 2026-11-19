#pragma once

// The original runs its world update from the PC timer interrupt.  Sound
// initialisation (`1530:0440`, called from `1581:0236`) installs a custom
// int 1Ch handler and programmes PIT channel 0.  Without the SB FM driver it
// uses divisor 65536/64 = 1024 and only bumps its counter every 64th
// interrupt; with the driver the frequency argument is 0, which the routine
// turns into divisor 65536 and divider 1.  Either way the game tick is
// exactly one 8253 reload:
//
//     1,193,182 Hz / 65,536 = 18.206510... Hz
//
// The old literal 18.2065 truncated that ratio by ~0.7 ppm.  Keep the ratio
// itself so the port, the demo studio and any timing diagnostic all agree.
namespace jill_time {

constexpr double kPitInputHz = 1193182.0;  // 8253 input clock (nominal)
constexpr double kPitReload = 65536.0;     // channel-0 reload

constexpr double kLogicHz = kPitInputHz / kPitReload;   // 18.206510...
constexpr double kLogicStep = kPitReload / kPitInputHz;  // seconds per tick

}  // namespace jill_time
