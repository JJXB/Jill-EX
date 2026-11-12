#include "platform_paths.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <filesystem>

namespace jill_paths {
namespace {
#ifdef JILL_PS4
const char kDataRoot[] = "/data/JillEX";
#else
const char kDataRoot[] = "";
#endif

std::FILE* g_log = nullptr;

bool is_absolute_path(const std::string& p) {
    if (p.empty()) {
        return false;
    }
    // POSIX root, Windows drive letter ("C:...") and UNC ("\\server").
    if (p[0] == '/' || p[0] == '\\') {
        return true;
    }
    return p.size() >= 2 && p[1] == ':';
}

}  // namespace

bool is_ps4() {
#ifdef JILL_PS4
    return true;
#else
    return false;
#endif
}

const std::string& data_root() {
    static const std::string root = kDataRoot;
    return root;
}

std::string resolve(const std::string& path) {
    if (path.empty() || is_absolute_path(path) || data_root().empty()) {
        return path;
    }
    return data_root() + "/" + path;
}

bool exists(const std::string& path) {
    if (path.empty()) {
        return false;
    }
    std::error_code ec;
    return std::filesystem::exists(path, ec);
}

void log_open(const std::string& path) {
    if (g_log != nullptr) {
        std::fclose(g_log);
        g_log = nullptr;
    }
    if (path.empty()) {
        return;
    }
    g_log = std::fopen(path.c_str(), "w");
}

void trace(const char* fmt, ...) {
#ifndef JILL_PS4
    // Desktop stays quiet unless explicitly asked.
    if (std::getenv("JILL_TRACE") == nullptr) {
        return;
    }
#endif
    va_list args;
    va_start(args, fmt);
    va_list copy;
    va_copy(copy, args);
    std::vfprintf(stderr, fmt, args);
    va_end(args);
    std::fputc('\n', stderr);
    std::fflush(stderr);
    if (g_log != nullptr) {
        std::vfprintf(g_log, fmt, copy);
        std::fputc('\n', g_log);
        std::fflush(g_log);
    }
    va_end(copy);
}

bool ensure_data_root(std::string& err) {
    err.clear();
    if (data_root().empty()) {
        return true;  // desktop: nothing to create
    }
    std::error_code ec;
    if (std::filesystem::is_directory(data_root(), ec)) {
        return true;
    }
    std::filesystem::create_directories(data_root(), ec);
    if (ec || !std::filesystem::is_directory(data_root(), ec)) {
        err = "cannot create " + data_root() + ": " + ec.message();
        return false;
    }
    return true;
}

}  // namespace jill_paths
