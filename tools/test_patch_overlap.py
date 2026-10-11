#!/usr/bin/env python3
"""bbgoated: no patch manifest (patches/*.toml) may write where the engine
itself patches or reads the game's code - the frame-rate redirects, the 60/90
FPS++ sites, hooks. A manifest applied first changes the bytes the engine then
expects, and the engine refuses its own change (v0.2.18-bbgoated.1's sprint-fix
on 0x1914c24 dropped all 23 per-frame redirects at 60/90/120 fps).

Every guest address written as 0x0XXXXXXX in src/engine and src/decomp counts as
engine-owned; a manifest site within 16 bytes of one fails.

    python tools/test_patch_overlap.py
"""
import re
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SLACK = 16
ADDR = re.compile(r"\b0x(0[0-9a-fA-F]{7})\b")


def manifest_sites(root=ROOT):
    sites = []
    for p in sorted((root / "patches").glob("*.toml")):
        text = p.read_text(encoding="utf-8")
        for m in re.finditer(r'\[site\.([^\]]+)\]\s*\naddress\s*=\s*"0x([0-9a-fA-F]+)"\s*\nexpect\s*=\s*"([0-9a-fA-F ]+)"', text):
            n = len(bytes.fromhex(m.group(3).replace(" ", "")))
            sites.append((p.name, m.group(1), int(m.group(2), 16), n))
    return sites


def engine_addresses(root=ROOT):
    found = {}
    for sub in ("src/engine", "src/decomp"):
        for p in sorted((root / sub).rglob("*")):
            if p.suffix not in (".cpp", ".h", ".hpp", ".inc"):
                continue
            if p.stem == "patch_manifest":
                continue  # the manifest format's own documentation quotes skip-intro's site as its example
            for i, line in enumerate(p.read_text(encoding="utf-8", errors="replace").splitlines(), 1):
                for m in ADDR.finditer(line):
                    a = int(m.group(1), 16)
                    if 0x400000 <= a < 0x6000000:
                        found.setdefault(a, "%s:%d" % (p.relative_to(root).as_posix(), i))
    return found


def overlaps(root=ROOT):
    eng = engine_addresses(root)
    out = []
    for name, site, addr, n in manifest_sites(root):
        for a, where in eng.items():
            if addr - SLACK <= a < addr + n + SLACK:
                out.append("%s [site.%s] 0x%08x+%d is next to 0x%08x (%s)" % (name, site, addr, n, a, where))
    return out


class PatchOverlapTest(unittest.TestCase):
    def test_manifests_parse(self):
        self.assertTrue(manifest_sites(), "no manifest sites found")

    def test_no_manifest_site_touches_engine_owned_code(self):
        found = overlaps()
        self.assertEqual(found, [], "\n" + "\n".join(found))


if __name__ == "__main__":
    unittest.main()
