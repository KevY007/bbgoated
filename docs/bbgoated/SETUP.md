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
  - your official plugins;
  - your saves (`saves\` and `save-backups\`), shader caches and downloaded assets (`bbhost\`) and trophies, from the data folder bbhost's config names (or bbhost's default; or a `data` folder beside the eboot, bbhost's old default).

  bbhost's files are only read. Saves are never copied into a bbgoated data folder that already has some, and no file there is ever replaced. The log says `bbgoated: first start - ... imported` and `bbgoated: copied saves ...`, and a marker file in the data folder stops the save copy from ever happening again. From then on the two programs have separate settings and separate progress, and changing one never affects the other.
- **If you don't:** the setup window asks for the two paths (the game folder that holds `dvdroot_ps4`, and the decrypted eboot) and offers to link a Hunter's Dream account.
- **If bbhost is running during bbgoated's first start:** the save copy waits until a start when it isn't, so a save being written is never copied half-done.

Thanks to the copied shader caches the first start goes straight in. Without them (no bbhost before, or after deleting them) it builds the caches first ("Building shaders: N left"), which takes a few minutes once.

### Playing the same saves in both

If you'd rather share one save folder, so progress continues in both, point bbgoated at bbhost's data folder in `%APPDATA%\bbgoated\bbgoated.toml`:

```toml
[paths]
data = "E:/Bloodborne/GameSaveData"   # the folder bbhost's bbhost.toml names
```

Then never run both at once. bbhost keeps backups of the newest saves in `save-backups\`.

## 4. Online

bbgoated is a bbhost client to The Hunter's Dream. You meet ordinary bbhost players, with the same summons, invasions, messages and bloodstains.

- **One at a time:** don't run bbhost and bbgoated at the same time with the same account. bbgoated logs a warning when it sees `bbhost.exe` running.
- **Windows Firewall:** it asks once for the new exe. Allow it: the P2P port is UDP 9307, and a blocked port is the most common reason summons fail.
- **"Recover account":** never use it to "move" your account to bbgoated. It cancels every other device's sign-in, bbhost's included. bbgoated doesn't need it: your sign-in came over on the first start.
- **No cheats:** bbgoated ships none. The debug menu plugin used against other players gets accounts banned.

## 5. Switches

Patch switches go under `[patches]` in `%APPDATA%\bbgoated\bbgoated.toml`. Environment switches are set before starting, for example in a copy of `run-bbgoated.bat`: `set BBHOST_X=1`.

| What | Default | Switch |
|---|---|---|
| Intel tone-map fix | on, on Intel CPUs only | `intel-tonemap = false` |
| The Old Hunters | on | `old-hunters = false` |
| Light-grid tiles from the render size instead of the window | off | `BBHOST_LIGHT_GRID_BASIS=render` |
| Red-zone audit (observe-only diagnostics) | off | `BBHOST_REDZONE_AUDIT=1` |
| Every upstream bbhost switch | as upstream | `docs/running.md` |

## 6. Troubleshooting

- **Logs:** `logs\bbgoated-<date>.log` in the bbgoated folder, or `logs\bbhost-<date>.log` when a source build is started with `run-bbhost.bat`. Its first lines name the version, for example `bbhost v0.2.18-bbgoated.1`, plus the config layers it read and the data folder it used.
- **"eboot sha256 ... is not the 1.09 build":** decrypt the eboot (UnfSelf, ps4_unfself) or redump.
- **Black screen or broken frame after an update:** delete the shader caches in `%LOCALAPPDATA%\bbgoated\data\bbhost\`. They are rebuilt.
- **Summons don't connect:** check the firewall, and make sure bbhost isn't running at the same time.
- **Going back to bbhost:** just start bbhost. Its files were never changed.

Report problems on the bbgoated repository's issue tracker with the log attached. Don't report bbgoated problems to bbhost's tracker unless you reproduced them on bbhost itself.
