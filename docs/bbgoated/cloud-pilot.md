# Cloud pilot environment report (2026-10-12)

## Environment
- OS: Ubuntu 24.04.5 LTS (noble); kernel Linux 6.18.44-fc-v114 x86_64
- CPUs: 4; RAM: 15 GiB, no swap; disk: ~30 GiB free on /
- sudo without password: yes; apt-get: works (update ~5 s)
- Repos: /home/user/bbgoated (bbgoated/main @ a00921d), /home/user/bbgoated-workspace (main @ 98b99e8)

## Network
- `git ls-remote https://github.com/droogie/bbhost.git refs/pull/73/head`: OK -> 539d45f41e16
- `git fetch` of the same ref into the code repo: OK (~1.3 s)
- Git over HTTPS to github.com works; plain HTTPS downloads of github.com
  archive tarballs and codeload.github.com return 403 from the egress proxy;
  ffmpeg.org is blocked (CONNECT 403).

## Toolchain (apt, ~4 s)
- clang 18.1.3, lld 18.1.3, cmake 3.28.3, ninja 1.11.1, pkg-config 1.8.1,
  python3 3.11.17, libvulkan-dev 1.3.275
- Missing: libsdl3-dev (not in noble; libsdl2-dev 2.30 available)
- libstdc++-14-dev and nasm are available from apt.

## tools/ci_build.sh
Not runnable as-is: its apt step would work (Ubuntu 24.04, passwordless sudo),
but it fetches Vulkan-Headers/SDL3 as GitHub archive tarballs and ffmpeg from
ffmpeg.org via curl, which the proxy refuses. It would need git clones of
those tags (git to github.com works) or a pre-seeded CI_WORK/src, or a
network policy allowing those hosts.
