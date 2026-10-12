// core/config.cpp built for bbgoated (BBHOST_BBGOATED): a per-user folder of its
// own (bbgoated/bbgoated.toml beside bbhost's bbhost/), filled once from bbhost's
// config on the first start - which is only read - and a data folder of its own,
// seeded once with a copy of bbhost's saves, shader caches and trophies. After that
// the two never share a file.
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
// bbhost's per-user config naming a bbhost data folder that holds one save.
struct Home {
    fs::path root;
    explicit Home(const char* name) : root(fs::temp_directory_path() / name) {
        fs::remove_all(root);
        setenv("XDG_CONFIG_HOME", (root / "config").c_str(), 1);
        setenv("XDG_DATA_HOME", (root / "share").c_str(), 1);
        setenv("HOME", root.c_str(), 1);
        unsetenv("BBHOST_CONFIG_DIR");
        spit(bbhost_toml(), "[paths]\napp0 = \"" + (root / "game").generic_string() + "\"\ndata = \"" +
                                bbhost_data().generic_string() + "\"\n\n[online]\nhost = \"thehuntersdream.com\"\n\n[bbhost]\nconfig_version = 4\n");
        spit(root / "config" / "bbhost" / "plugins" / "randomizer.dll", "dll");
        spit(root / "config" / "bbhost" / "plugins" / "randomizer.dll.sig", "sig");
        spit(bbhost_data() / "saves" / "SPRJ0005" / "userdata0000", "save-bytes");
        fs::create_directories(root / "game" / "dvdroot_ps4");
    }
    fs::path bbhost_toml() const { return root / "config" / "bbhost" / "bbhost.toml"; }
    fs::path own_dir() const { return root / "config" / "bbgoated"; }
    fs::path own_toml() const { return own_dir() / "bbgoated.toml"; }
    fs::path bbhost_data() const { return root / "bbhost-data"; }
    fs::path own_data() const { return root / "share" / "bbgoated" / "data"; }
};

HostConfig load(int argc = 1, char** argv = nullptr) {
    char arg0[] = "bbgoated_config_test";
    char* def[] = {arg0, nullptr};
    HostConfig c;
    std::string err;
    CHECK(config_load(argc, argv ? argv : def, &c, &err));
    return c;
}

void first_start_imports_and_never_writes_bbhost() {
    Home h("bbg_import");
    const std::string before = slurp(h.bbhost_toml());
    const HostConfig c = load();
    CHECK(fs::path(config_user_dir()) == h.own_dir());
    CHECK(fs::path(config_user_file()) == h.own_toml());
    CHECK(fs::exists(h.own_toml()));                           // imported
    const std::string own = slurp(h.own_toml());
    CHECK(own.find("app0 = ") != std::string::npos);           // the game's paths came over
    CHECK(own.find("thehuntersdream.com") != std::string::npos);
    CHECK(own.find("bbhost-data") == std::string::npos);       // bbhost's data folder did not
    CHECK(fs::path(c.app0) == h.root / "game");
    CHECK(fs::path(c.data) == h.own_data());
    CHECK(slurp(h.own_dir() / "plugins" / "randomizer.dll.sig") == "sig");  // official plugins, signatures kept
    CHECK(slurp(h.bbhost_toml()) == before);                   // bbhost's config is only read
}

void later_starts_ignore_bbhost() {
    Home h("bbg_separate");
    load();
    spit(h.bbhost_toml(), "[paths]\napp0 = \"/somewhere/else\"\n");
    const HostConfig c = load();
    CHECK(fs::path(c.app0) == h.root / "game");                // bbgoated.toml is the only config now
    CHECK(config_set_values(config_user_file(), {{"startup", "skip_intro", "true"}}));
    CHECK(slurp(h.bbhost_toml()) == "[paths]\napp0 = \"/somewhere/else\"\n");
    CHECK(load().skip_intro);
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

void seeds_caches_and_trophies() {
    Home h("bbg_seeds_caches");
    spit(h.bbhost_data() / "bbhost" / "translation-cache.bin", "tc");
    spit(h.bbhost_data() / "bbhost" / "invasion-assets" / "event.dcx", "asset");
    spit(h.bbhost_data() / "trophies.txt", "1,2");
    load();
    CHECK(slurp(h.own_data() / "bbhost" / "translation-cache.bin") == "tc");  // no shader hitches at the first start
    CHECK(slurp(h.own_data() / "bbhost" / "invasion-assets" / "event.dcx") == "asset");
    CHECK(slurp(h.own_data() / "trophies.txt") == "1,2");
    CHECK(slurp(h.bbhost_data() / "bbhost" / "translation-cache.bin") == "tc");  // source unchanged
}

void finishes_a_copy_cut_short() {
    Home h("bbg_cut_short");
    spit(h.bbhost_data() / "saves" / "SPRJ0005" / "userdata0001", "second-save");
    // A first start stopped after one file: its guard file is still there.
    spit(h.own_data() / "saves" / "SPRJ0005" / "userdata0000", "save-bytes");
    spit(h.own_data() / "saves.copying", "from\n");
    load();
    CHECK(slurp(h.own_data() / "saves" / "SPRJ0005" / "userdata0001") == "second-save");
    CHECK(!fs::exists(h.own_data() / "saves.copying"));
    CHECK(fs::exists(h.own_data() / "bbgoated-saves-from-bbhost.txt"));
}

void saves_kept_caches_still_copied() {
    Home h("bbg_kept_saves");
    // An earlier bbgoated copied the saves only, and was played since.
    spit(h.own_data() / "saves" / "SPRJ0005" / "userdata0000", "played-in-bbgoated");
    spit(h.bbhost_data() / "bbhost" / "translation-cache.bin", "tc");
    load();
    CHECK(slurp(h.own_data() / "saves" / "SPRJ0005" / "userdata0000") == "played-in-bbgoated");
    CHECK(slurp(h.own_data() / "bbhost" / "translation-cache.bin") == "tc");
    CHECK(fs::exists(h.own_data() / "bbgoated-saves-from-bbhost.txt"));
}

void bbhost_saves_beside_its_eboot() {
    Home h("bbg_beside_eboot");
    // bbhost's old default: no data line, saves in a data folder beside the eboot.
    spit(h.bbhost_toml(), "[paths]\napp0 = \"" + (h.root / "game").generic_string() + "\"\neboot = \"" +
                              (h.root / "game" / "eboot.bin").generic_string() + "\"\n");
    spit(h.root / "game" / "data" / "saves" / "SPRJ0005" / "userdata0000", "old-default-save");
    load();
    CHECK(slurp(h.own_data() / "saves" / "SPRJ0005" / "userdata0000") == "old-default-save");
}

void copied_once_only() {
    Home h("bbg_once_only");
    load();
    fs::remove_all(h.own_data() / "saves");  // the player cleared bbgoated's saves on purpose
    load();
    CHECK(!fs::exists(h.own_data() / "saves"));
}

void config_dir_override_imports_nothing() {
    Home h("bbg_config_dir");
    const fs::path dir = h.root / "harness";
    spit(dir / "run.toml", "[paths]\napp0 = \"" + (h.root / "game").generic_string() + "\"\ndata = \"" +
                               (h.root / "run-data").generic_string() + "\"\n");
    setenv("BBHOST_CONFIG_DIR", dir.c_str(), 1);
    const std::string cfg = (dir / "run.toml").string();
    char arg0[] = "bbgoated_config_test";
    char arg1[] = "--config";
    std::string a2 = cfg;
    char* argv[] = {arg0, arg1, a2.data(), nullptr};
    const HostConfig c = load(3, argv);
    CHECK(fs::path(c.data) == h.root / "run-data");            // test and sandbox runs keep their own data
    CHECK(!fs::exists(dir / "plugins"));                       // and import nothing from bbhost
    CHECK(!fs::exists(h.own_dir()));
    unsetenv("BBHOST_CONFIG_DIR");
}

}  // namespace

int main() {
    first_start_imports_and_never_writes_bbhost();
    later_starts_ignore_bbhost();
    seeds_once();
    no_reseed_after_play();
    seeds_caches_and_trophies();
    finishes_a_copy_cut_short();
    saves_kept_caches_still_copied();
    bbhost_saves_beside_its_eboot();
    copied_once_only();
    config_dir_override_imports_nothing();
    if (g_fail) {
        std::printf("bbgoated_config_test: %d failure(s)\n", g_fail);
        return 1;
    }
    std::printf("bbgoated_config_test: ok\n");
    return 0;
}
