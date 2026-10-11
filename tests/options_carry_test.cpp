// host/options_carry.cpp (bbgoated): bbhost-options.toml is shared with bbhost, so a
// bbgoated save keeps what it does not know - keys a newer bbhost added - and never
// brings back an account the player signed out of.
#include "host/options_carry.h"

#include <cstdio>
#include <string>

namespace {
int g_fail = 0;
void check(bool ok, const char* what) {
    if (!ok) {
        std::printf("FAIL: %s\n", what);
        ++g_fail;
    }
}
bool has(const std::string& s, const char* needle) { return s.find(needle) != std::string::npos; }
int count(const std::string& s, const char* needle) {
    int n = 0;
    for (std::size_t at = s.find(needle); at != std::string::npos; at = s.find(needle, at + 1)) ++n;
    return n;
}
}  // namespace

int main() {
    const std::string old_text =
        "# bbhost PC port options. Written by the options screen (F10).\n"
        "[options]\nversion = \"3\"\nvsync = \"On\"\nhdr_output = \"On\"\n"
        "[keys]\nroll = \"Space\"\nnew_action = \"P\"\n"
        "[future]\nthing = \"1\"\n"
        "[account]\nname = \"hunter\"\ntoken = \"secret\"\nregion = \"eu\"\n";
    const std::string new_signed_in =
        "# bbhost PC port options. Written by the options screen (F10).\n"
        "[options]\nversion = \"3\"\nvsync = \"Off\"\n"
        "[keys]\nroll = \"Space\"\n"
        "[account]\nname = \"hunter\"\ntoken = \"secret\"\n";
    const std::string m = options_carry_unknown(old_text, new_signed_in);
    check(has(m, "hdr_output = \"On\""), "an unknown [options] key is kept");
    check(has(m, "new_action = \"P\""), "an unknown [keys] binding is kept");
    check(has(m, "[future]\nthing = \"1\""), "an unknown section is kept");
    check(has(m, "region = \"eu\""), "an unknown [account] field is kept");
    check(has(m, "vsync = \"Off\"") && !has(m, "vsync = \"On\""), "a known key keeps the new value only");
    check(count(m, "token = ") == 1, "the token appears once");
    check(m.find("hdr_output") < m.find("[keys]"), "carried keys go back into their own section");

    const std::string new_signed_out =
        "# bbhost PC port options. Written by the options screen (F10).\n"
        "[options]\nversion = \"3\"\nvsync = \"Off\"\n"
        "[keys]\nroll = \"Space\"\n";
    const std::string o = options_carry_unknown(old_text, new_signed_out);
    check(!has(o, "token = ") && !has(o, "name = \"hunter\""), "a signed-out account is not brought back");
    check(has(o, "hdr_output = \"On\""), "unknown keys are still kept when signed out");

    check(options_carry_unknown("", new_signed_in) == new_signed_in, "no old file: the new text as it is");
    if (!g_fail) std::printf("options_carry_test: ok\n");
    return g_fail ? 1 : 0;
}
