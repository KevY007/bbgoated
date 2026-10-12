# bbgoated

> **Vibe-coded.** bbgoated is being designed and written by an AI coding agent (Anthropic's
> Claude, through Claude Code) at the direction of its maintainer. It is built on top of the
> human-written projects credited below. Expect rough edges, read the code before you trust
> it, and report what breaks.

bbgoated is [**bbhost**](https://github.com/droogie/bbhost), the native PC host for
Bloodborne 1.09, with a patch stack on top. The goal is to keep bbhost's online play,
controls and gameplay exactly as they are, and add graphics and performance work from the
rest of the community: the
[DLAA fork](https://github.com/YoucefNabil/bbhost),
[FIocker's DLSS/FSR work](https://github.com/droogie/bbhost/pull/73),
[bbport](https://github.com/deadinside28/bloodborne_pc) and
[Supermedo's Windows port](https://github.com/Supermedo/bloodborne_pc).

The game's own 1.09 executable runs natively on your x86-64 CPU, as it does in bbhost.
bbgoated changes how it is drawn and patched, never how it talks to other players.

## What stays exactly as in bbhost

- **Online** on [The Hunter's Dream](https://thehuntersdream.com), as a bbhost client.
  - It plays co-op, invasions, messages and bloodstains with ordinary bbhost players.
  - It uses the same account: it comes over from bbhost once, on bbgoated's first start.
  - The online code is upstream's. The one exception is upstream's own pending TLS fix (PR #74),
    taken ahead of its merge. Before every release, a check script in the workspace repo
    confirms that bbgoated added nothing else there (`docs/bbgoated/UPSTREAM.md`).
- **Controls**: keyboard and mouse (Dark Souls III-style mouse camera, rebindable keys,
  key prompts), controller, the in-game PC menus.
- **Gameplay**: the game's own code, bbhost's decompiled functions at upstream's revision,
  the PC enhancements, plugins and mods.

## What bbgoated adds (so far)

| Change | Why | Switch |
|---|---|---|
| Based on bbhost's next release (`release/next-steps`) | The flip-after-GPU fix (light flicker when the GPU is at 99%), correct physics and timing at 60/90/120 fps - sprint keeps its speed above 90 fps (bbhost #66) among them - and more shaders lifted | n/a |
| Upstream fixes ahead of their merge: TLS trust (#74), chromatic aberration toggle on Windows (#70), a ~14 MiB/min trace leak (#117) | Windows bugs | n/a |
| Intel tone-map fix (Kyo's), Intel CPUs only | Dark/black tone mapping in DLC areas on Intel CPUs | `intel-tonemap = false` under `[patches]` |
| Fully separate from bbhost and other launchers: its own config (`%APPDATA%\bbgoated`) and data (`%LOCALAPPDATA%\bbgoated\data`) | On the first start it copies your bbhost settings, key bindings, account, official plugins, saves and shader caches. After that the two never share a file, and bbhost's files are only ever read | `bbgoated.toml`, `bbgoated-options.toml` |
| Light-grid tile basis switch | Measures window-sized vs render-sized light tiles | `BBHOST_LIGHT_GRID_BASIS=render` |
| Red-zone audit | Counts Windows exceptions inside game functions that use their red zone (a crash class on Intel 12th gen+) | `BBHOST_REDZONE_AUDIT=1` (off by default, observe-only) |
| Area check on Windows | Automated, repeatable performance A/B at 12 places in the game | `tools/area_check.py --probe` |

**Coming next** (see the plan in the workspace repo):
- DLSS Super Resolution, DLAA and FSR 3.1 in the in-game graphics menu, ported from PR #73;
- upscaling on the HDR scene before post-processing;
- renderer work on the GPU overhead measured on Windows;
- every retail region;
- a few effect toggles from Supermedo's port.

## Requirements

- Windows 10 or 11, 64-bit, and a GPU with Vulkan 1.3 and a current driver.
- **Your own** Bloodborne dump with the 1.09 update, and its decrypted `eboot.bin`. This is
  the same requirement as bbhost's (SHA-256
  `941f887a562aae054fac35af8cc8f27cf075f3d4cc2e029fb5ae2a663aaa5ae7`). No game code or data
  is included here.

## Getting started

- **Players:** [docs/bbgoated/SETUP.md](../docs/bbgoated/SETUP.md) covers installing,
  first start, sharing bbhost's account, saves, switches and troubleshooting.
- **Building from source:** [docs/bbgoated/BUILDING.md](../docs/bbgoated/BUILDING.md).
- **Development workspace:** the spec, implementation plans, research reports (what every
  upstream project does and why bbgoated is built this way) and the build/test scripts are
  in the companion repo
  [bbgoated-workspace](https://github.com/KevY007/bbgoated-workspace).

## Online etiquette

The Hunter's Dream is a community server. Using cheats or the debug menu against other
players (in their worlds, in invasions, on leaderboards) gets an account banned. bbgoated
ships no cheats. Don't run bbhost and bbgoated online at the same time with the same
account.

## How it stays current

bbgoated is upstream bbhost plus a linear stack of `bbg/<topic>:` commits, re-applied on
each new bbhost release. Upstream commits taken early are cherry-picked with `-x` and
recorded in [docs/bbgoated/UPSTREAM.md](../docs/bbgoated/UPSTREAM.md), along with every
other project it tracks.

## Credits

- **bbhost:** droogie and contributors. Everything here stands on it. bbhost's own README is
  [README.md](../README.md).
- **The Hunter's Dream:** droogie and team.
- **DLAA fork:** YoucefNabil. **DLSS/FSR/frame-generation work (PR #73), PRs #70, #74 and #117:** FIocker.
- **bbport:** deadinside28. **Windows port:** Supermedo. **Red-zone site list (Supermedo PR #2):** DarkSpartan.
- **Community patches:** Kyo (60 FPS++, tone-map fix, The Old Hunters patch), Lance McDonald,
  illusion, emoose.
- **References:** [shadPS4](https://github.com/shadps4-emu/shadPS4).

## License

GPL-3.0-or-later, as bbhost ([LICENSE](../LICENSE)). Third-party code keeps its own license.
NVIDIA DLSS runtime files are never committed here.

Bloodborne is a trademark of Sony Interactive Entertainment. bbgoated is not affiliated with
or endorsed by Sony, FromSoftware, NVIDIA, AMD or the bbhost project, and contains none of the
game's code or data.
