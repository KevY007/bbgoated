// host/redzone_audit.cpp: which red-zone store site an image offset falls in.
#include "host/redzone_audit.h"

#include <cstdio>

int main() {
    int fail = 0;
    auto check = [&](bool ok, const char* what) {
        if (!ok) {
            std::printf("FAIL: %s\n", what);
            ++fail;
        }
    };
    check(redzone_site_count() == 607, "607 sites (Supermedo PR #2's list for the 1.09 eboot)");
    check(redzone_site_index(0x140c0) == 0, "first site's first byte");
    check(redzone_site_index(0x140c6) == 0, "first site's last byte (7 bytes)");
    check(redzone_site_index(0x140c7) == -1, "the gap after it");
    check(redzone_site_index(0x140d7) == 1, "the second site");
    check(redzone_site_index(0x140bf) == -1, "before the first");
    check(redzone_site_index(0xffffffffull) == -1, "past the last");
    if (!fail) std::printf("redzone_audit_test: ok\n");
    return fail ? 1 : 0;
}
