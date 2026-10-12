# Setting up bbgoated

bbgoated runs like bbhost: same game files, same setup window, same in-game menus. This page covers what is different, and everything you need from a clean start.

> bbgoated is vibe-coded (see the [README](../../.github/README.md)). Keep backups of your saves; bbgoated starts from a copy of them on purpose.

## 1. What you need

- Windows 10 or 11 (64-bit), and a GPU with Vulkan 1.3 and a current driver (NVIDIA, AMD or Intel).
- Your own Bloodborne dump with the **1.09** update applied (or kept beside it as `CUSA00900-UPDATE`, the way shadPS4 keeps it).
- The 1.09 `eboot.bin` decrypted to an ELF, SHA-256 `941f887a562aae054fac35af8cc8f27cf075f3d4cc2e029fb5ae2a663aaa5ae7`. A dump made for shadPS4 usually has one. The 1.09 executable is the same for every region; a different hash nearly always means the file is still encrypted or came from a different decryption tool.
- The Old Hunters works without the add-on files: bbhost's `old-hunters` patch, on by default, makes the game count it as owned.

No game code or data comes with bbgoated.

## 2. Get bbgoated

There are no prebuilt releases yet. Build it from source ([BUILDING.md](BUILDING.md)); the result is a folder with `bbhost.exe`, renamed `bbgoated.exe` in packaged builds, plus `patches\`, `plugins\` and `run-bbhost.bat`.

Put it in a folder of its own, not inside your bbhost folder and not inside the game dump.

## 3. First start

Start `run-bbgoated.bat` (packaged build) or `run-bbhost.bat` (source build). It keeps a log in `logs\`.

bbgoated is **fully separate from bbhost** and any other launcher:

| | bbgoated | bbhost |
|---|---|---|
| Config | `%APPDATA%\bbgoated\bbgoated.toml` | `%APPDATA%\bbhost\bbhost.toml` |
| F10 settings, key bindings, account | `%APPDATA%\bbgoated\bbgoated-options.toml` | `%APPDATA%\bbhost\bbhost-options.toml` |
| Plugins | `%APPDATA%\bbgoated\plugins` | `%APPDATA%\bbhost\plugins` |
| Saves and shader caches | `%LOCALAPPDATA%\bbgoated\data` | wherever bbhost's config says |

- **If you already use bbhost:** on bbgoated's first start it copies over:
  - your game paths and online server (into `bbgoated.toml`, without bbhost's data folder);
  - your F10 settings, key bindings and signed-in account (into `bbgoated-options.toml`);
  - your plugins (the whole plugins folder, signatures included);
  - your saves (`saves\` and `save-backups\`), shader caches and downloaded assets (`bbhost\`) and trophies, from the data folder bbhost's config names (or bbhost's default; or a `data` folder beside the eboot, bbhost's old default).

  bbhost's files are only read. Saves are never copied into a bbgoated data folder that already has some, and no file there is ever replaced. The log says `bbgoated: first start - ... imported` and `bbgoated: copied saves ...`, and a marker file in the data folder stops the save copy from ever happening again. From then on the two programs have separate settings and separate progress: changing one's never affects the other's. (The account is the one exception: see Online.)
- **If you don't:** the setup window asks for the two paths (the game folder that holds `dvdroot_ps4`, and the decrypted eboot) and offers to link a Hunter's Dream account.
- **If bbhost is running during bbgoated's first start,** or its data folder isn't there yet (a drive not mounted): the save copy waits for a later start, so a save being written is never copied half-done. If anything can't come over (a full disk, say), a later start copies what is missing, but never copies bbhost's saves into progress made in bbgoated.

Thanks to the copied shader caches the first start goes straight in. Without them (no bbhost before, or after deleting them) it builds the caches first ("Building shaders: N left"), which takes a few minutes once.

### bbhost's data folder is off limits

bbgoated never runs on bbhost's data folder, or on a folder inside it. Its saves, its backup rotation (which deletes old backups) and its temporary files would be bbhost's. If `bbgoated.toml`, `--data` or the setup window's Data folder names it, bbgoated logs `bbgoated: the data folder ... is bbhost's` and uses its own. To bring bbhost progress over again later, copy the save folders yourself while neither program runs.

## 4. Online

bbgoated is a bbhost client to The Hunter's Dream. You meet ordinary bbhost players, with the same summons, invasions, messages and bloodstains.

- **One at a time:** don't run bbhost and bbgoated at the same time with the same account. bbgoated logs a warning when it sees `bbhost.exe` running.
- **Windows Firewall:** it asks once for the new exe. Allow it: the P2P port is UDP 9307, and a blocked port is the most common reason summons fail.
- **Signing out** in either program signs out both: they use the one sign-in that came over on the first start, and signing out ends it on the server. Sign in again in each afterwards.
- **"Recover account":** never use it to "move" your account to bbgoated. It cancels every other device's sign-in, bbhost's included. bbgoated doesn't need it: your sign-in came over on the first start.
- **No cheats:** bbgoated ships none. The debug menu plugin used against other players gets accounts banned.

## 5. Switches

Patch switches go under `[patches]` in `%APPDATA%\bbgoated\bbgoated.toml`. Environment switches are set before starting, for example in a copy of `run-bbgoated.bat`: `set BBHOST_X=1`.

| What | Default | Switch |
|---|---|---|
| Intel tone-map fix | on, on Intel CPUs only | `intel-tonemap = false` |
| The Old Hunters | on | `old-hunters = false` |
| NVIDIA DLAA (RTX cards; other GPUs skip it) | on | `mode = "off"` under `[dlss]` (it wins over `[dlaa]`), or `enabled = false` under `[dlaa]`; `BBHOST_DLSS_MODE=off` or `BBHOST_DLAA=0`; `Ctrl+F1` or `Ctrl+F5` in game |
| DLAA's DLSS preset (a letter, e.g. `K`) | the driver's | `preset = "K"` under `[dlaa]` |
| next-steps' per-draw vertex copies (a ring kept in host memory, copied for each draw) | off | `BBHOST_VERTEX_STAGE=1`; with it on, `BBHOST_VERTEX_STAGE_SHARE=0` copies for every draw |
| Light-grid tiles from the render size instead of the window | off | `BBHOST_LIGHT_GRID_BASIS=render` |
| Red-zone audit (observe-only diagnostics) | off | `BBHOST_REDZONE_AUDIT=1` |
| Every upstream bbhost switch | as upstream | `docs/running.md` |

### DLAA

On an NVIDIA RTX card bbgoated runs NVIDIA's DLAA on the game's HDR scene, before its post-processing. It needs two files beside `bbgoated.exe`: `ngx_bridge.dll` (version 2, built from `tools/ngx_bridge`) and NVIDIA's `nvngx_dlss.dll`. Packaged builds include them, with NVIDIA's license (`NVIDIA-DLSS-LICENSE.txt`). For a source build, build the bridge with Visual Studio against NVIDIA's DLSS SDK (`docs/dlaa.md`, "ngx_bridge") and copy `nvngx_dlss.dll` from the SDK. The DLAA fork's older bridge is refused: the log says `dlaa: ngx_bridge.dll is not version 2; off`. Without them, or on another GPU, the game renders exactly as in bbhost.

- While DLAA runs, the game's own anti-aliasing (System > Graphics) is held off: on top of DLAA it would only blur. Your setting is kept in `bbgoated-options.toml`, and it comes back as soon as DLAA is turned off.
- `Ctrl+F1` turns DLAA off and on while you play (`Ctrl+F5` does the same, for keyboards where another program takes `Ctrl+F1`). The log says `dlaa: on` or `dlaa: off`.
- The log's `dlaa: NGX init ok` line, after the first loaded area, means it is running.

## 6. Troubleshooting

- **Logs:** `logs\bbgoated-<date>.log` in the bbgoated folder, or `logs\bbhost-<date>.log` when a source build is started with `run-bbhost.bat`. Its first lines name the version, for example `bbhost v0.2.18-bbgoated.1`, plus the config layers it read and the data folder it used.
- **"eboot sha256 ... is not the 1.09 build":** decrypt the eboot (UnfSelf, ps4_unfself) or redump.
- **Black screen or broken frame after an update:** delete the shader caches in `%LOCALAPPDATA%\bbgoated\data\bbhost\`. They are rebuilt.
- **Summons don't connect:** check the firewall, and make sure bbhost isn't running at the same time.
- **Going back to bbhost:** just start bbhost. Its files were never changed.

Report problems on the bbgoated repository's issue tracker with the log attached. Don't report bbgoated problems to bbhost's tracker unless you reproduced them on bbhost itself.
