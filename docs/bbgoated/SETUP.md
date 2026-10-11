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

Start `run-bbhost.bat`, or `run-bbgoated.bat` in a packaged build. It keeps a log in `logs\`.

- **If you already use bbhost:** bbgoated reads bbhost's per-user settings in `%APPDATA%\bbhost\`:
  - your game paths (`bbhost.toml`);
  - your F10 options, key bindings and online account (`bbhost-options.toml`);
  - your plugins.

  You don't set anything up again, and you stay signed in to The Hunter's Dream with the same account.
- **If you don't:** the setup window asks for the two paths (the game folder that holds `dvdroot_ps4`, and the decrypted eboot) and offers to link a Hunter's Dream account.

### What bbgoated never touches

- **bbhost's `bbhost.toml`:** bbgoated reads it, but its own choices go to `%APPDATA%\bbhost\bbgoated.toml`, a layer over it. Setup-window saves, plugin settings and the data path all go there.
- **bbhost's saves:** bbgoated keeps its own data folder, `%LOCALAPPDATA%\bbgoated\data` (saves, shader caches, generated files). On its first start it **copies** your bbhost saves, `saves\` and `save-backups\`, from the data folder your `bbhost.toml` names, and logs `bbgoated: copied saves from ... (the source is only read)`. After that the two programs have separate progress.

The first start in a new data folder builds its shader caches ("Building shaders: N left"), which takes a few minutes once.

### Playing the same saves in both

If you'd rather share one save folder, so progress continues in both, point bbgoated at bbhost's data folder in `%APPDATA%\bbhost\bbgoated.toml`:

```toml
[paths]
data = "E:/Bloodborne/GameSaveData"   # the folder bbhost's bbhost.toml names
```

Then never run both at once. bbhost keeps backups of the newest saves in `save-backups\`.

## 4. Online

bbgoated is a bbhost client to The Hunter's Dream. You meet ordinary bbhost players, with the same summons, invasions, messages and bloodstains.

- **One at a time:** don't run bbhost and bbgoated at the same time with the same account. bbgoated logs a warning when it sees `bbhost.exe` running.
- **Windows Firewall:** it asks once for the new exe. Allow it: the P2P port is UDP 9307, and a blocked port is the most common reason summons fail.
- **"Recover account":** never use it to "move" your account to bbgoated. It cancels every other device's sign-in, bbhost's included. bbgoated doesn't need it, because it shares bbhost's sign-in.
- **No cheats:** bbgoated ships none. The debug menu plugin used against other players gets accounts banned.

## 5. Switches

Patch switches go under `[patches]` in `%APPDATA%\bbhost\bbgoated.toml`. Environment switches are set before starting, for example in a copy of `run-bbhost.bat`: `set BBHOST_X=1`.

| What | Default | Switch |
|---|---|---|
| Sprint fix above 90 fps | on | `sprint-fix = false` |
| Intel tone-map fix | on, on Intel CPUs only | `intel-tonemap = false` |
| The Old Hunters | on | `old-hunters = false` |
| Light-grid tiles from the render size instead of the window | off | `BBHOST_LIGHT_GRID_BASIS=render` |
| Red-zone audit (observe-only diagnostics) | off | `BBHOST_REDZONE_AUDIT=1` |
| Every upstream bbhost switch | as upstream | `docs/running.md` |

## 6. Troubleshooting

- **Logs:** `logs\bbhost-<date>.log` in the bbgoated folder. Its first lines name the version, for example `bbhost v0.2.18-bbgoated.1`, plus the config layers it read and the data folder it used.
- **"eboot sha256 ... is not the 1.09 build":** decrypt the eboot (UnfSelf, ps4_unfself) or redump.
- **Black screen or broken frame after an update:** delete the shader caches in `%LOCALAPPDATA%\bbgoated\data\bbhost\`. They are rebuilt.
- **Summons don't connect:** check the firewall, and make sure bbhost isn't running at the same time.
- **Going back to bbhost:** just start bbhost. Its files were never changed.

Report problems on the bbgoated repository's issue tracker with the log attached. Don't report bbgoated problems to bbhost's tracker unless you reproduced them on bbhost itself.
