#!/usr/bin/env python3
"""Name the call sites in bbgoated's census lines (BBHOST_BBG_PASS_BREAKS=1).

    tools/bbg_sites.py LOG --exe EXE [--windows] [--tag "bbg pass ends:"]... [--top 20]

Each "bbg pass ends:" / "bbg transfer batches:" line lists <image offset>=<n>/flip,
offsets from the image base as in the "cmd census:" lines. This averages the
lines of LOG per tag and names the offsets with addr2line. A Windows log takes
--windows and the package's bbhost-win-<rev>.debug.exe; run it in WSL Arch,
which has x86_64-w64-mingw32-addr2line and -objdump.
"""
import argparse
import collections
import re
import shutil
import subprocess

TAGS = ['bbg pass ends:', 'bbg transfer batches:']


def read_census(lines, tags):
    """Per tag: how many lines carry it, and each offset's per-flip counts summed."""
    totals = {t: collections.Counter() for t in tags}
    reports = collections.Counter()
    for line in lines:
        for t in tags:
            i = line.find(t)
            if i < 0:
                continue
            reports[t] += 1
            for m in re.finditer(r' ([0-9a-f]+)=([0-9.]+)/flip', line[i + len(t):]):
                totals[t][m.group(1)] += float(m.group(2))
    return totals, reports


def call_address(offset, base):
    """The call instruction's address, for addr2line. A census offset is a return
    address, which addr2line can place on the line after the call, or in the
    inlined code that follows it."""
    return int(offset, 16) + base - 1


def function_name(demangled):
    """addr2line -C's function name without its parameters, and without the
    anonymous namespaces whose parentheses would cut it short."""
    return re.sub(r'\(.*', '', demangled.replace('(anonymous namespace)::', ''))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('log')
    ap.add_argument('--exe', required=True)
    ap.add_argument('--windows', action='store_true', help='a Windows exe: add the PE link base, use the mingw binutils')
    ap.add_argument('--tag', action='append', help='a line tag (default: both census tags)')
    ap.add_argument('--top', type=int, default=20)
    a = ap.parse_args()
    tags = a.tag or TAGS
    tool = 'x86_64-w64-mingw32-' if a.windows and shutil.which('x86_64-w64-mingw32-addr2line') else ''
    with open(a.log, errors='replace') as f:
        totals, reports = read_census(f, tags)
    if not sum(reports.values()):
        raise SystemExit('no census lines in %s (run with BBHOST_BBG_PASS_BREAKS=1)' % a.log)
    base = 0
    if a.windows:
        out = subprocess.run([tool + 'objdump', '-p', a.exe], capture_output=True, text=True).stdout
        m = re.search(r'ImageBase\s+([0-9a-fA-F]+)', out)
        base = int(m.group(1), 16) if m else 0x140000000
    sites = sorted({s for t in tags for s in totals[t]})
    names = {}
    if sites:
        out = subprocess.run([tool + 'addr2line', '-f', '-C', '-s', '-e', a.exe] + ['%x' % call_address(s, base) for s in sites],
                             capture_output=True, text=True).stdout.split('\n')
        for i, s in enumerate(sites):
            fn = function_name(out[2 * i]) if 2 * i < len(out) else '?'
            loc = out[2 * i + 1] if 2 * i + 1 < len(out) else '?'
            names[s] = '%s (%s)' % (fn, loc)
    for t in tags:
        if not reports[t]:
            continue
        print('%s %d reports, mean per flip' % (t, reports[t]))
        for site, n in totals[t].most_common(a.top):
            print('%8.2f /flip  %s' % (n / reports[t], names.get(site, site)))


if __name__ == '__main__':
    main()
