#!/usr/bin/env python3
"""Tests for tools/bbg_sites.py: the census lines read, and the offsets given to
addr2line and named the way the census means them (bbgoated)."""
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent / "tools"))
import bbg_sites  # noqa: E402

LOG = """\
[bbhost] bbg pass ends: all=3.0/flip 1a2b=2.00/flip 40=1.00/flip
[bbhost] gpu: something else 1a2b=9.00/flip
[bbhost] bbg transfer batches: all=1.0/flip 77=1.00/flip
[bbhost] bbg pass ends: all=1.0/flip 1a2b=1.00/flip
"""


class ReadCensus(unittest.TestCase):
    def test_sums_per_tag(self):
        totals, reports = bbg_sites.read_census(LOG.splitlines(), bbg_sites.TAGS)
        self.assertEqual(reports["bbg pass ends:"], 2)
        self.assertEqual(reports["bbg transfer batches:"], 1)
        self.assertEqual(totals["bbg pass ends:"], {"1a2b": 3.0, "40": 1.0})  # "all" is not an offset; other lines do not count
        self.assertEqual(totals["bbg transfer batches:"], {"77": 1.0})


class Naming(unittest.TestCase):
    def test_call_instruction_not_return_address(self):
        # A census offset is a return address: the line after the call, or the
        # inlined code that follows it. addr2line is asked about the call.
        self.assertEqual(bbg_sites.call_address("1a2b", 0), 0x1A2A)
        self.assertEqual(bbg_sites.call_address("1a2b", 0x140000000), 0x140001A2A)

    def test_function_names(self):
        self.assertEqual(bbg_sites.function_name("gpu::submit_locked()"), "gpu::submit_locked")
        self.assertEqual(
            bbg_sites.function_name("gpu::(anonymous namespace)::begin_pass(gpu::(anonymous namespace)::DrawState const&, unsigned long, bool)"),
            "gpu::begin_pass")
        self.assertEqual(bbg_sites.function_name("(anonymous namespace)::transfer_begin_locked()"), "transfer_begin_locked")
        self.assertEqual(bbg_sites.function_name("??"), "??")


if __name__ == "__main__":
    unittest.main()
