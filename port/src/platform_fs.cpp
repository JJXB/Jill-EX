// Xbox (NXDK) implementation of the jill_fs facade declared in platform_fs.h.
//
// Everything goes through the Win32 file API that NXDK's winapi layer
// implements: GetFileAttributesA for type/existence tests, FindFirstFileA /
// FindNextFileA for enumeration and CreateDirectoryA for mkdir -p.  The Xbox
// file system is FATX; the data files use '/' in the DOS-era configuration,
// which the kernel accepts, but directory enumeration always builds native
// '\\' paths.

#include "platform_fs.h"

#ifdef JILL_XBOX

#include <windows.h>

namespace jill_fs {
namespace {

// The engine and the DOS-era configuration use '/' in places (jill.ini,
// pack.ini, join()); the NT object manager only knows '\\'.
std::string native_path(const std::string& in) {
    std::string out = in;
    for (char& c : out) {
        if (c == '/') {
            c = '\\';
        }
    }
    return out;
}

bool valid_attributes(const std::string& native, DWORD& attrs) {
    if (native.empty()) {
        return false;
    }
    attrs = GetFileAttributesA(native.c_str());
    return attrs != INVALID_FILE_ATTRIBUTES;
}

bool is_separator(char c) { return c == '\\' || c == '/'; }

std::string native_join(const std::string& base, const std::string& name) {
    if (base.empty()) {
        return name;
    }
    if (name.empty()) {
        return base;
    }
    if (is_separator(base.back())) {
        return base + name;
    }
    return base + "\\" + name;
}

// Enumerate one directory (native path, no recursion).
void list_one(const std::string& raw_dir, std::vector<directory_entry>& out) {
    if (raw_dir.empty()) {
        return;
    }
    const std::string dir = native_path(raw_dir);
    std::string pattern = dir;
    if (!is_separator(pattern.back())) {
        pattern += '\\';
    }
    pattern += '*';

    WIN32_FIND_DATAA data{};
    HANDLE find = FindFirstFileA(pattern.c_str(), &data);
    if (find == INVALID_HANDLE_VALUE) {
        return;
    }
    do {
        const char* name = data.cFileName;
        if (name[0] == '\0' || (name[0] == '.' && name[1] == '\0') ||
            (name[0] == '.' && name[1] == '.' && name[2] == '\0')) {
            continue;
        }
        out.emplace_back(path(native_join(dir, name)));
    } while (FindNextFileA(find, &data));
    FindClose(find);
}

void list_recursive(const std::string& dir,
                    std::vector<directory_entry>& out) {
    std::vector<std::string> pending;
    pending.push_back(dir);
    while (!pending.empty()) {
        const std::string current = pending.back();
        pending.pop_back();
        std::vector<directory_entry> entries;
        list_one(current, entries);
        for (const directory_entry& entry : entries) {
            out.push_back(entry);
            DWORD attrs = 0;
            if (valid_attributes(entry.path().string(), attrs) &&
                (attrs & FILE_ATTRIBUTE_DIRECTORY) != 0) {
                pending.push_back(entry.path().string());
            }
        }
    }
}

}  // namespace

std::string path::generic_string() const {
    std::string out = s_;
    for (char& c : out) {
        if (c == '\\') {
            c = '/';
        }
    }
    return out;
}

path path::parent_path() const {
    std::string::size_type end = s_.find_last_of("\\/");
    if (end == std::string::npos) {
        return path();
    }
    // Keep the root separator: "D:\games\pack" -> "D:\games", "D:\games" -> "D:".
    if (end == 0) {
        return path(s_.substr(0, 1));
    }
    return path(s_.substr(0, end));
}

path path::filename() const {
    const std::string::size_type end = s_.find_last_of("\\/");
    if (end == std::string::npos) {
        return path(s_);
    }
    return path(s_.substr(end + 1));
}

path path::extension() const {
    const path name = filename();
    const std::string::size_type dot = name.s_.find_last_of('.');
    if (dot == std::string::npos || dot == 0) {
        return path();
    }
    return path(name.s_.substr(dot));
}

path path::stem() const {
    const path name = filename();
    const std::string::size_type dot = name.s_.find_last_of('.');
    if (dot == std::string::npos || dot == 0) {
        return name;
    }
    return path(name.s_.substr(0, dot));
}

path& path::operator/=(const path& other) {
    if (!other.s_.empty()) {
        s_ = native_join(s_, other.s_);
    }
    return *this;
}

path path::operator/(const path& other) const {
    path result(*this);
    result /= other;
    return result;
}

bool directory_entry::is_regular_file(std::error_code& ec) const {
    ec.clear();
    DWORD attrs = 0;
    if (!valid_attributes(p.string(), attrs)) {
        return false;
    }
    return (attrs & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

bool directory_entry::is_directory(std::error_code& ec) const {
    ec.clear();
    DWORD attrs = 0;
    if (!valid_attributes(p.string(), attrs)) {
        return false;
    }
    return (attrs & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

directory_iterator::directory_iterator(const path& dir, std::error_code& ec) {
    ec.clear();
    items_ = std::make_shared<std::vector<directory_entry>>();
    list_one(dir.string(), *items_);
}

directory_iterator directory_iterator::begin() const {
    directory_iterator copy = *this;
    copy.index_ = 0;
    return copy;
}

directory_iterator directory_iterator::end() const {
    directory_iterator copy = *this;
    copy.index_ = copy.items_ ? copy.items_->size() : 0;
    return copy;
}

recursive_directory_iterator::recursive_directory_iterator(const path& dir,
                                                           std::error_code& ec) {
    ec.clear();
    items_ = std::make_shared<std::vector<directory_entry>>();
    list_recursive(dir.string(), *items_);
}

recursive_directory_iterator recursive_directory_iterator::begin() const {
    recursive_directory_iterator copy = *this;
    copy.index_ = 0;
    return copy;
}

recursive_directory_iterator recursive_directory_iterator::end() const {
    recursive_directory_iterator copy = *this;
    copy.index_ = copy.items_ ? copy.items_->size() : 0;
    return copy;
}

bool exists(const path& p, std::error_code& ec) {
    ec.clear();
    DWORD attrs = 0;
    return valid_attributes(p.string(), attrs);
}

bool is_directory(const path& p, std::error_code& ec) {
    ec.clear();
    DWORD attrs = 0;
    return valid_attributes(p.string(), attrs) &&
           (attrs & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

bool is_regular_file(const path& p, std::error_code& ec) {
    ec.clear();
    DWORD attrs = 0;
    return valid_attributes(p.string(), attrs) &&
           (attrs & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

bool create_directories(const path& p, std::error_code& ec) {
    ec.clear();
    const std::string native = native_path(p.string());
    if (native.empty()) {
        return true;
    }
    DWORD attrs = 0;
    if (valid_attributes(native, attrs)) {
        return (attrs & FILE_ATTRIBUTE_DIRECTORY) != 0;
    }
    // Build the parent chain first.  Skip a drive prefix ("D:" / "D:\").
    const std::string::size_type sep = native.find_last_of("\\/");
    if (sep != std::string::npos && sep > 1) {
        create_directories(path(native.substr(0, sep)), ec);
        if (ec) {
            return false;
        }
    }
    if (!CreateDirectoryA(native.c_str(), nullptr)) {
        // Either another entry created it in the meantime or the volume is
        // read-only; report the real state.
        if (valid_attributes(native, attrs) &&
            (attrs & FILE_ATTRIBUTE_DIRECTORY) != 0) {
            return true;
        }
        ec = std::error_code(static_cast<int>(GetLastError()),
                             std::system_category());
        return false;
    }
    return true;
}

}  // namespace jill_fs

#endif  // JILL_XBOX
