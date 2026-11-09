#include "config.h"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <fstream>

std::string ini_lower(const std::string& s) {
    std::string out = s;
    for (char& c : out) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return out;
}

namespace {

std::string trim(const std::string& s) {
    size_t a = 0;
    size_t b = s.size();
    while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) {
        ++a;
    }
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) {
        --b;
    }
    return s.substr(a, b - a);
}

}  // namespace

bool IniFile::load(const std::string& path, std::string& err) {
    data_.clear();
    std::ifstream in(path);
    if (!in) {
        err = "cannot open " + path;
        return false;
    }
    std::string section = "root";
    std::string line;
    int lineno = 0;
    while (std::getline(in, line)) {
        ++lineno;
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        const std::string t = trim(line);
        if (t.empty() || t[0] == '#' || t[0] == ';') {
            continue;
        }
        if (t.front() == '[' && t.back() == ']') {
            section = ini_lower(trim(t.substr(1, t.size() - 2)));
            continue;
        }
        const size_t eq = t.find('=');
        if (eq == std::string::npos) {
            std::fprintf(stderr, "%s:%d: expected key = value\n", path.c_str(),
                         lineno);
            continue;
        }
        const std::string key = ini_lower(trim(t.substr(0, eq)));
        const std::string value = trim(t.substr(eq + 1));
        if (!key.empty()) {
            data_[section][key] = value;
        }
    }
    return true;
}

bool IniFile::save(const std::string& path) const {
    std::ofstream out(path);
    if (!out) {
        return false;
    }
    out << "# Jill port settings.  Edit by hand or from the in-game menus.\n";
    out << "# [paths] holds the games/profiles directories plus one asset\n";
    out << "# directory per installed game id.\n";
    for (const auto& sec : data_) {
        out << "\n[" << sec.first << "]\n";
        for (const auto& kv : sec.second) {
            out << kv.first << " = " << kv.second << "\n";
        }
    }
    return static_cast<bool>(out);
}

std::string IniFile::get(const std::string& section, const std::string& key,
                         const std::string& fallback) const {
    auto s = data_.find(ini_lower(section));
    if (s == data_.end()) {
        return fallback;
    }
    auto k = s->second.find(ini_lower(key));
    return k == s->second.end() ? fallback : k->second;
}

int IniFile::get_int(const std::string& section, const std::string& key,
                     int fallback) const {
    const std::string v = get(section, key);
    if (v.empty()) {
        return fallback;
    }
    return std::atoi(v.c_str());
}

bool IniFile::get_bool(const std::string& section, const std::string& key,
                       bool fallback) const {
    const std::string v = ini_lower(get(section, key));
    if (v.empty()) {
        return fallback;
    }
    return v == "1" || v == "true" || v == "on" || v == "yes";
}

std::map<std::string, std::string> IniFile::section(
    const std::string& name) const {
    auto s = data_.find(ini_lower(name));
    return s == data_.end() ? std::map<std::string, std::string>{} : s->second;
}

void IniFile::set(const std::string& section, const std::string& key,
                  const std::string& value) {
    data_[ini_lower(section)][ini_lower(key)] = value;
}

void IniFile::set_int(const std::string& section, const std::string& key,
                      int value) {
    set(section, key, std::to_string(value));
}

void IniFile::set_bool(const std::string& section, const std::string& key,
                       bool value) {
    set(section, key, value ? "1" : "0");
}
