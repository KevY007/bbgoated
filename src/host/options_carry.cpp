// See options_carry.h.
#include "host/options_carry.h"

#include <fstream>
#include <iterator>
#include <map>
#include <set>
#include <sstream>
#include <vector>

namespace {

struct Line {
    std::string raw;      // without its line ending
    std::string section;  // the section it is in ("" before the first)
    std::string key;      // a key line's key, else ""
    bool header = false;
};

std::string trim(const std::string& s) {
    const std::size_t a = s.find_first_not_of(" \t\r");
    if (a == std::string::npos) return {};
    return s.substr(a, s.find_last_not_of(" \t\r") - a + 1);
}

std::vector<Line> parse(const std::string& text) {
    std::vector<Line> out;
    std::string section;
    std::size_t at = 0;
    while (at < text.size()) {
        std::size_t nl = text.find('\n', at);
        if (nl == std::string::npos) nl = text.size();
        std::string raw = text.substr(at, nl - at);
        if (!raw.empty() && raw.back() == '\r') raw.pop_back();
        at = nl + 1;
        Line l;
        l.raw = raw;
        const std::string t = trim(raw);
        if (t.size() >= 2 && t.front() == '[' && t.back() == ']') {
            section = trim(t.substr(1, t.size() - 2));
            l.header = true;
        } else if (!t.empty() && t[0] != '#') {
            const std::size_t eq = t.find('=');
            if (eq != std::string::npos) l.key = trim(t.substr(0, eq));
        }
        l.section = section;
        out.push_back(l);
    }
    return out;
}

}  // namespace

std::string options_carry_unknown(const std::string& old_text, const std::string& new_text) {
    const std::vector<Line> olds = parse(old_text), news = parse(new_text);
    std::set<std::pair<std::string, std::string>> known;
    std::set<std::string> new_sections;
    for (const Line& l : news) {
        if (l.header) new_sections.insert(l.section);
        if (!l.key.empty()) known.insert({l.section, l.key});
    }
    std::map<std::string, std::vector<std::string>> carry;  // section -> lines to add
    std::vector<std::string> carry_order;                   // sections, as the old file had them
    for (const Line& l : olds) {
        if (l.key.empty() || known.count({l.section, l.key})) continue;
        if (l.section == "account" && (!new_sections.count("account") || l.key == "name" || l.key == "token")) continue;
        if (!carry.count(l.section)) carry_order.push_back(l.section);
        carry[l.section].push_back(l.raw);
    }
    if (carry.empty()) return new_text;
    const std::string eol = new_text.find("\r\n") != std::string::npos ? "\r\n" : "\n";
    std::string out;
    std::string current;
    auto flush = [&](const std::string& section) {
        auto it = carry.find(section);
        if (it == carry.end() || !new_sections.count(section)) return;
        for (const std::string& raw : it->second) out += raw + eol;
        carry.erase(it);
    };
    for (std::size_t i = 0; i < news.size(); ++i) {
        const Line& l = news[i];
        if (l.header) {
            flush(current);
            current = l.section;
        }
        out += l.raw + eol;
    }
    flush(current);
    for (const std::string& section : carry_order) {
        auto it = carry.find(section);
        if (it == carry.end()) continue;
        if (!section.empty()) out += "[" + section + "]" + eol;
        for (const std::string& raw : it->second) out += raw + eol;
    }
    return out;
}

void options_carry_apply(const std::string& path, const std::string& old_text) {
    if (old_text.empty()) return;
    std::ifstream in(path, std::ios::binary);
    if (!in) return;
    const std::string new_text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();
    const std::string merged = options_carry_unknown(old_text, new_text);
    if (merged == new_text) return;
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out << merged;
}
