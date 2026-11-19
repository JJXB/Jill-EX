#include "platform_paths.h"

#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

#include "platform_fs.h"

#ifdef JILL_ANDROID
#include <SDL_system.h>
#include <android/log.h>
#endif

#ifdef JILL_XBOX
#include <hal/debug.h>
#include <xboxkrnl/xboxkrnl.h>
#endif

namespace jill_paths {
namespace {
#if defined(JILL_PS4)
const char kDataRoot[] = "/data/JillEX";
#elif defined(JILL_VITA)
// Vita has no writable system partition: the app mounts at ux0:app/<TITLEID>
// and every mutable file belongs under ux0:data.
const char kDataRoot[] = "ux0:data/JillEX";
#elif defined(JILL_XBOX)
// The Xbox build reads jill.ini and the game folders from the folder it is
// launched from: the disc root (D:\) for an XISO, or a folder on the hard
// disk.  xbox_data_root() below picks the first candidate that has a jill.ini
// so an E:\Apps\Jill-EX install works without editing anything.
#elif !defined(JILL_ANDROID)
const char kDataRoot[] = "";
#endif

std::FILE* g_log = nullptr;

#ifdef JILL_XBOX
// Folder the XBE was loaded from.  XeImageFileName holds the full device path
// of the running executable, which makes the game work from a disc, an HDD
// folder or an FTP deployment without editing anything.
std::string xbox_install_root() {
    const ANSI_STRING& image = XeImageFileName[0];
    if (image.Buffer == nullptr || image.Length <= 0) {
        return std::string();
    }
    std::string full(image.Buffer, static_cast<size_t>(image.Length));
    const std::string::size_type slash = full.find_last_of("\\/");
    if (slash == std::string::npos) {
        return std::string();
    }
    return full.substr(0, slash + 1);
}

std::string xbox_data_root() {
    std::error_code ec;
    const std::string install = xbox_install_root();
    trace("startup: xbe folder %s", install.c_str());
    if (!install.empty() && jill_fs::exists(install + "jill.ini", ec)) {
        return install;
    }
    // An XBE copied to the dashboard/HDD while the data stays on the disc, or
    // a launcher that ran it from elsewhere.
    static const char* const kCandidates[] = {
        "D:\\",
        "E:\\JillEX",
        "E:\\Apps\\Jill-EX",
        "E:\\Apps\\JillEX",
        "E:\\Games\\Jill-EX",
        "E:\\Games\\JillEX",
    };
    for (const char* candidate : kCandidates) {
        std::string root = candidate;
        if (root.empty() || root.back() != '\\') {
            root += '\\';
        }
        if (jill_fs::exists(root + "jill.ini", ec)) {
            return root;
        }
    }
    if (!install.empty()) {
        return install;
    }
    return "D:\\";
}
#endif

#ifdef JILL_ANDROID
// Shared, user-visible game folder: adb push / file manager / cloud sync all
// work without knowing the package name.  The app requests All files access
// (MANAGE_EXTERNAL_STORAGE) because targetSdk 30+ blocks direct I/O elsewhere.
std::string android_data_root() {
    return "/sdcard/JillEX";
}
#endif

bool is_absolute_path(const std::string& p) {
    if (p.empty()) {
        return false;
    }
    // POSIX root, Windows drive letter ("C:...") and UNC ("\\server").
    if (p[0] == '/' || p[0] == '\\') {
        return true;
    }
    if (p.size() >= 2 && p[1] == ':') {
        return true;
    }
#ifdef JILL_VITA
    // Vita device prefixes put the colon later: "ux0:", "ur0:", "uma0:",
    // "imc0:", "app0:", "savedata0:".  Without this, resolve() prepends the
    // data root to a path that already has it (ux0:data/JillEX/ux0:data/...).
    const std::string::size_type colon = p.find(':');
    if (colon >= 2 && colon <= 8) {
        return true;
    }
#endif
    return false;
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
#ifdef JILL_ANDROID
    static const std::string root = android_data_root();
#elif defined(JILL_XBOX)
    static const std::string root = xbox_data_root();
#else
    static const std::string root = kDataRoot;
#endif
    return root;
}

std::string resolve(const std::string& path) {
    if (path.empty() || is_absolute_path(path) || data_root().empty()) {
        return path;
    }
    const std::string& root = data_root();
    if (root.back() == '/' || root.back() == '\\') {
        return root + path;  // "D:\" already ends with a separator
    }
    return root + "/" + path;
}

bool exists(const std::string& path) {
    if (path.empty()) {
        return false;
    }
    std::error_code ec;
    return jill_fs::exists(path, ec);
}

std::string resolve_file(const std::string& path) {
    std::error_code ec;
    if (path.empty() || jill_fs::exists(path, ec)) {
        return path;
    }
    const jill_fs::path p(path);
    const jill_fs::path dir = p.parent_path();
    if (dir.empty()) {
        return path;
    }
    auto lower = [](std::string s) {
        for (char& c : s) {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
        return s;
    };
    const std::string want = lower(p.filename().string());
    jill_fs::directory_iterator it(dir, ec);
    if (ec) {
        return path;
    }
    for (const auto& e : it) {
        if (lower(e.path().filename().string()) == want) {
            trace("path: %s -> %s", path.c_str(), e.path().string().c_str());
            return e.path().string();
        }
    }
    return path;
}

bool log_open(const std::string& path) {
    if (g_log != nullptr) {
        std::fclose(g_log);
        g_log = nullptr;
    }
    if (path.empty()) {
        return false;
    }
    g_log = std::fopen(path.c_str(), "w");
    return g_log != nullptr;
}

void trace(const char* fmt, ...) {
#if !defined(JILL_PS4) && !defined(JILL_VITA) && !defined(JILL_ANDROID) && \
    !defined(JILL_XBOX)
    // Desktop stays quiet unless explicitly asked.
    if (std::getenv("JILL_TRACE") == nullptr) {
        return;
    }
#endif
    char line[1024];
    va_list args;
    va_start(args, fmt);
    std::vsnprintf(line, sizeof(line), fmt, args);
    va_end(args);
#if defined(JILL_XBOX)
    // pdclib's stdout/stderr are pseudo streams with an INVALID_HANDLE_VALUE;
    // the Xbox build logs to the debug channel and to jill.log instead.
    debugPrint("Jill-EX: %s\n", line);
#else
    std::fprintf(stderr, "%s\n", line);
    std::fflush(stderr);
#endif
#ifdef JILL_ANDROID
    __android_log_write(ANDROID_LOG_INFO, "JillEX", line);
#endif
    if (g_log != nullptr) {
        std::fprintf(g_log, "%s\n", line);
        std::fflush(g_log);
    }
}

bool ensure_data_root(std::string& err) {
    err.clear();
    if (data_root().empty()) {
        return true;  // desktop: nothing to create
    }
    std::error_code ec;
    if (jill_fs::is_directory(data_root(), ec)) {
        return true;
    }
    jill_fs::create_directories(data_root(), ec);
    if (ec || !jill_fs::is_directory(data_root(), ec)) {
        err = "cannot create " + data_root() + ": " + ec.message();
        return false;
    }
    return true;
}

}  // namespace jill_paths
