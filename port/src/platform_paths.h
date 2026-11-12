#pragma once

#include <string>

// Platform data root.
//
// On the PS4 every read and write lives under /data/JillEX: jill.ini, games/,
// profiles/, external/, the extracted game folders, save slots, high scores
// and the log file.  On desktop the root is empty, which means "use the
// process working directory" and keeps the existing PC behaviour.
namespace jill_paths {

// True when this build targets the PS4 (defined by build_ps4.cmd / CMake).
bool is_ps4();

// "/data/JillEX" on PS4, "" on desktop.
const std::string& data_root();

// Join a relative path onto data_root(); absolute paths and empty strings are
// returned unchanged.  On desktop this is an identity function.
std::string resolve(const std::string& path);

// True when the path exists on disk (directory or file).
bool exists(const std::string& path);

// Create the data root on PS4.  No-op on desktop.  Returns false and fills
// `err` when the folder cannot be created.
bool ensure_data_root(std::string& err);

// Diagnostic trace.  Writes one line to stderr and flushes immediately, so it
// survives a hang.  On PS4 it also goes to the file opened by log_open()
// (freopen alone is not reliable on every PS4 libc/loader).
void trace(const char* fmt, ...);

// Open/truncate the PS4 diagnostic log.  Independent of stdio redirection.
void log_open(const std::string& path);

}  // namespace jill_paths
