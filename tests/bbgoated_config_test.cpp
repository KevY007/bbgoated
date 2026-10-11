// core/config.cpp built for bbgoated (BBHOST_BBGOATED): it shares bbhost's
// per-user folder but never writes its bbhost.toml - its own choices go to
// bbgoated.toml beside it - and keeps its saves in a data folder of its own,
// seeded once with a copy of bbhost's.
#include "core/config.h"

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace {

int g_fail = 0;
#define CHECK(c)                                                          \
    do {                                                                  \
        if (!(c)) {                                                       \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #c);     \
            ++g_fail;                                                     \
        }                                                                 \
    } while (0)

namespace fs = std::filesystem;

std::string slurp(const fs::path& p) {
    std::ifstream f(p, std::ios::binary);
    std::stringstream s;
    s << f.rdbuf();
    return s.str();
}

void spit(const fs::path& p, const std::string& s) {
    fs::create_directories(p.parent_path());
    std::ofstream(p, std::ios::binary | std::ios::trunc) << s;
}

// A home of its own per case: XDG folders under a fresh temporary directory,
// a bbhost.toml naming a bbhost data folder that holds one save.
struct Home {
    fs::path root;
    explicit Home(const char* name) : root(fs::temp_directory_path() / name) {
        fs::remove_all(root);
        fs::create_directories(root / "config" / "bbhost");
        setenv("XDG_CONFIG_HOME", (root / "config").c_str(), 1);
        setenv("XDG_DATA_HOME", (root / "share").c_str(), 1);
        setenv("HOME", root.c_str(), 1);
        unsetenv("BBHOST_CONFIG_DIR");
        spit(user_toml(), "[paths]\napp0 = \"" + (root / "game").generic_string() + "\"\ndata = \"" +
                              bbhost_data().generic_string() + "\"\n");
        spit(bbhost_data() / "saves" / "SPRJ0005" / "userdata0000", "save-bytes");
        fs::create_directories(root / "game" / "dvdroot_ps4");
    }
    fs::path user_toml() const { return root / "config" / "bbhost" / "bbhost.toml"; }
    fs::path own_toml() const { return root / "config" / "bbhost" / "bbgoated.toml"; }
    fs::path bbhost_data() const { return root / "bbhost-data"; }
    fs::path own_data() const { return root / "share" / "bbgoated" / "data"; }
};

HostConfig load() {
    char arg0[] = "bbgoated_config_test";
    char* argv[] = {arg0, nullptr};
    HostConfig c;
    std::string err;
    CHECK(config_load(1, argv, &c, &err));
    return c;
}

void bbhost_toml_never_written() {
    Home h("bbg_never_written");
    const std::string before = slurp(h.user_toml());
    load();
    CHECK(fs::path(config_user_file()) == h.own_toml());
    CHECK(config_set_values(config_user_file(), {{"startup", "skip_intro", "true"}}));
    CHECK(slurp(h.user_toml()) == before);  // byte-identical after a load and a save
    CHECK(fs::exists(h.own_toml()));        // the save went to bbgoated.toml
    const HostConfig c = load();
    CHECK(c.skip_intro);                    // bbgoated.toml is a layer over bbhost.toml
    CHECK(slurp(h.user_toml()) == before);
}

void seeds_once() {
    Home h("bbg_seeds_once");
    const HostConfig c = load();
    CHECK(fs::path(c.data) == h.own_data());
    CHECK(slurp(h.own_data() / "saves" / "SPRJ0005" / "userdata0000") == "save-bytes");
    CHECK(slurp(h.bbhost_data() / "saves" / "SPRJ0005" / "userdata0000") == "save-bytes");  // source unchanged
}

void no_reseed_after_play() {
    Home h("bbg_no_reseed");
    load();
    spit(h.own_data() / "saves" / "SPRJ0005" / "userdata0000", "played-in-bbgoated");
    load();
    CHECK(slurp(h.own_data() / "saves" / "SPRJ0005" / "userdata0000") == "played-in-bbgoated");
}

void config_dir_override_keeps_data() {
    Home h("bbg_config_dir");
    setenv("BBHOST_CONFIG_DIR", (h.root / "config" / "bbhost").c_str(), 1);
    const HostConfig c = load();
    CHECK(fs::path(c.data) == h.bbhost_data());  // test and sandbox runs keep the data their bbhost.toml names
    CHECK(!fs::exists(h.own_data() / "saves"));
    unsetenv("BBHOST_CONFIG_DIR");
}

}  // namespace

int main() {
    bbhost_toml_never_written();
    seeds_once();
    no_reseed_after_play();
    config_dir_override_keeps_data();
    if (g_fail) {
        std::printf("bbgoated_config_test: %d failure(s)\n", g_fail);
        return 1;
    }
    std::printf("bbgoated_config_test: ok\n");
    return 0;
}
