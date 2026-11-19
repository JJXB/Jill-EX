#pragma once

// Portable filesystem facade.
//
// The engine needs only a small, read-mostly subset of std::filesystem:
// existence/type tests, directory enumeration, create_directories and a few
// path operations.  Every target except Xbox aliases the real std::filesystem
// so behaviour there is unchanged.
//
// The Xbox (NXDK) target cannot use std::filesystem: libc++ 10 ships the
// C++17 <filesystem> headers, but the implementing translation units are
// POSIX-only (unistd.h, sys/stat.h, opendir/readdir) and the Xbox kernel has
// none of those.  NXDK's libc++ build also does not compile them, so the
// symbols would be missing at link time.  platform_fs.cpp therefore provides a
// small Win32-backed implementation (FindFirstFileA / GetFileAttributesA /
// CreateDirectoryA), which is what this header exposes as jill_fs.

#ifdef JILL_XBOX

#include <cstddef>
#include <memory>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

namespace jill_fs {

class path {
public:
    path() = default;
    path(std::string value) : s_(std::move(value)) {}
    path(const char* value) : s_(value != nullptr ? value : "") {}

    const std::string& string() const { return s_; }

    // Native path with '/' separators (the DOS-era data files use '/', and
    // compare/print code in external.cpp relies on it).
    std::string generic_string() const;

    path parent_path() const;
    path filename() const;
    path stem() const;
    path extension() const;

    bool empty() const { return s_.empty(); }

    path& operator/=(const path& other);
    path operator/(const path& other) const;

    bool operator==(const path& other) const { return s_ == other.s_; }
    bool operator<(const path& other) const { return s_ < other.s_; }

private:
    std::string s_;
};

struct directory_entry {
    path p;

    directory_entry() = default;
    explicit directory_entry(path value) : p(std::move(value)) {}

    const path& path() const { return p; }
    bool is_regular_file(std::error_code& ec) const;
    bool is_directory(std::error_code& ec) const;
};

// Both iterators enumerate eagerly into a shared vector, so they are cheap to
// copy and safe to use in range-for expressions (the list survives the
// temporary).  Pack and game directories hold a few dozen entries at most.
class directory_iterator {
public:
    directory_iterator() = default;
    directory_iterator(const path& dir, std::error_code& ec);

    directory_iterator begin() const;
    directory_iterator end() const;
    bool operator!=(const directory_iterator& other) const {
        return index_ != other.index_;
    }
    const directory_entry& operator*() const { return (*items_)[index_]; }
    const directory_entry* operator->() const { return &(*items_)[index_]; }
    directory_iterator& operator++() {
        ++index_;
        return *this;
    }

private:
    std::shared_ptr<std::vector<directory_entry>> items_;
    std::size_t index_ = 0;
};

class recursive_directory_iterator {
public:
    recursive_directory_iterator() = default;
    recursive_directory_iterator(const path& dir, std::error_code& ec);

    recursive_directory_iterator begin() const;
    recursive_directory_iterator end() const;
    bool operator!=(const recursive_directory_iterator& other) const {
        return index_ != other.index_;
    }
    const directory_entry& operator*() const { return (*items_)[index_]; }
    const directory_entry* operator->() const { return &(*items_)[index_]; }
    recursive_directory_iterator& operator++() {
        ++index_;
        return *this;
    }

private:
    std::shared_ptr<std::vector<directory_entry>> items_;
    std::size_t index_ = 0;
};

bool exists(const path& p, std::error_code& ec);
bool is_directory(const path& p, std::error_code& ec);
bool is_regular_file(const path& p, std::error_code& ec);

// Creates every missing parent.  Returns false and sets `ec` when the final
// directory still does not exist afterwards.
bool create_directories(const path& p, std::error_code& ec);

}  // namespace jill_fs

#else  // !JILL_XBOX

#include <filesystem>

namespace jill_fs = std::filesystem;

#endif  // JILL_XBOX
