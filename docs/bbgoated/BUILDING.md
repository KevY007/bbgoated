# Building bbgoated

bbgoated builds exactly as bbhost does. The Windows executable is cross-compiled with Clang and the mingw-w64 runtime by upstream's own `tools/ci_windows.sh`, plus one CMake option, `-DBBGOATED=ON`. MSVC cannot build it: the host needs `__attribute__((sysv_abi))` to call the game's code.

On a Windows PC, the reliable route is the one upstream's CI uses: Arch Linux, here in WSL2.

## 1. One-time setup (Windows 11/10 with WSL2)

```powershell
wsl --install archlinux --no-launch
wsl -d archlinux -u root -- bash -lc "pacman-key --init; pacman-key --populate archlinux; pacman -Syu --noconfirm --needed base-devel git clang lld llvm cmake ninja nasm curl zip unzip python mingw-w64-gcc mingw-w64-winpthreads vulkan-headers wine rsync"
```

`wine` is needed even though nothing is run under it: `tools/win_deps.sh` takes the Vulkan loader's import library (`libvulkan-1.a`) from it.

## 2. Get the source

A **full** clone. The tags drive the version string, which the online server sees as `bbhost/<version>`.

```bash
git clone https://github.com/<you>/bbgoated.git
cd bbgoated
git switch bbgoated/main
```

## 3. Build

Inside WSL, from the source folder. Build from the Linux file system: rsync the Windows checkout to `/root/...` first, which is much faster than building on `/mnt/c`.

```bash
WIN_DEPS_ROOT=/root/bbg/win-deps WIN_BUILD=/root/bbg/build-win CI_CMAKE_ARGS="-DBBGOATED=ON" tools/ci_windows.sh
```

- The first run builds the dependencies (zlib, SDL3, curl with Windows' own TLS, a minimal ffmpeg) into `WIN_DEPS_ROOT`, about 15-30 minutes. Later runs only rebuild bbhost.
- The result: `build/win/bbhost-win-<version>/` (the package folder), its `.zip`, and `bbhost-win-<version>.debug.exe` (unstripped, for symbolising crash reports).
- `-DBBGOATED=ON` turns on bbgoated's config sharing, its own data folder and its diagnostics (`cmake/bbgoated.cmake`). Without it the build is plain upstream bbhost plus the patch stack's general fixes.
- A custom version string: `BBHOST_VERSION=v0.2.18-bbgoated.1 tools/ci_windows.sh`. A tag on the commit does the same.

## 4. Tests

The unit tests run in a Linux build, in the same WSL distro:

```bash
cmake -S . -B /root/bbg/build-linux -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBBGOATED=ON
cmake --build /root/bbg/build-linux --target config_test bbgoated_config_test patch_manifest_cpu_test redzone_audit_test
cd /root/bbg/build-linux && ctest -R 'config_test|bbgoated_config_test|patch_manifest_cpu_test|redzone_audit_test' --output-on-failure
```

- `python tools/test_area_check_win.py` checks the area check's Windows pieces.
- On Windows, a smoke boot (headless, to the title screen) and the area check run from the workspace scripts. Each run gets its own config folder (`BBHOST_CONFIG_DIR`), a copy of the saves, and goes offline.

## 5. The DLSS bridge (coming with the upscaler work)

The NVIDIA DLSS path loads a small bridge DLL at run time. NVIDIA's static library cannot link into the mingw build, so the bridge is built with Visual Studio 2022/2026 (Release only) against the DLSS SDK. The DLSS runtime (`nvngx_dlss.dll`) comes from NVIDIA's SDK under NVIDIA's license and is never committed to this repository.

## The workspace scripts

The companion repository **bbgoated-workspace** wraps all of this in PowerShell:
- `scripts/build.ps1`: WSL export, `ci_windows.sh`, package copy-out, and the area-check probe DLL.
- `scripts/smoke.ps1` and `scripts/sandbox.ps1`.
- `scripts/area-check.ps1`: performance A/B.
- `scripts/check-online-untouched.ps1`.
- `scripts/sync-upstream.ps1`.
