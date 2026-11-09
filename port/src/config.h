#pragma once
#include <map>
#include <string>

// Minimal INI reader/writer for `jill.ini`.  Keys are case-insensitive and
// values are kept verbatim; `#` and `;` start a comment on their own line.
class IniFile {
public:
    bool load(const std::string& path, std::string& err);
    bool save(const std::string& path) const;

    std::string get(const std::string& section, const std::string& key,
                    const std::string& fallback = std::string()) const;
    int get_int(const std::string& section, const std::string& key,
                int fallback) const;
    bool get_bool(const std::string& section, const std::string& key,
                  bool fallback) const;
    // Returns the keys in a section (used for per-game data paths).
    std::map<std::string, std::string> section(
        const std::string& name) const;

    void set(const std::string& section, const std::string& key,
             const std::string& value);
    void set_int(const std::string& section, const std::string& key, int value);
    void set_bool(const std::string& section, const std::string& key,
                  bool value);

private:
    std::map<std::string, std::map<std::string, std::string>> data_;
};

std::string ini_lower(const std::string& s);
