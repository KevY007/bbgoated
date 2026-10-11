#!/usr/bin/env python3
"""Unit checks for tools/area_check.py's cross-platform pieces (bbgoated):
the pipe reader launch() uses on Windows and the --windowed parser.

    python tools/test_area_check_win.py
"""
import subprocess
import sys
import time
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import area_check  # noqa: E402


class PipeReaderTest(unittest.TestCase):
    def test_reads_all_lines_then_reports_eof(self):
        code = "import sys,time\nfor i in range(5):\n print('line', i, flush=True)\n time.sleep(0.05)\n"
        proc = subprocess.Popen([sys.executable, "-c", code], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        sel = area_check.PipeReader(proc.stdout)
        data = b""
        deadline = time.monotonic() + 10
        while sel.get_map() and time.monotonic() < deadline:
            for key, _ in sel.select(timeout=0.25):
                chunk = sel.read(key)
                if not chunk:
                    sel.unregister(key.fileobj)
                    continue
                data += chunk
        proc.wait(timeout=10)
        self.assertFalse(sel.get_map(), "EOF ends the loop")
        self.assertEqual(data.decode().split(), ["line", "0", "line", "1", "line", "2", "line", "3", "line", "4"])

    def test_select_times_out_with_nothing_to_read(self):
        proc = subprocess.Popen([sys.executable, "-c", "import time; time.sleep(1)"], stdout=subprocess.PIPE)
        sel = area_check.PipeReader(proc.stdout)
        t0 = time.monotonic()
        self.assertEqual(sel.select(timeout=0.2), [])
        self.assertLess(time.monotonic() - t0, 0.9)
        proc.wait(timeout=10)


class TomlStrTest(unittest.TestCase):
    @unittest.skipUnless(area_check.WINDOWS, "Windows paths")
    def test_windows_path_written_with_forward_slashes(self):
        self.assertEqual(area_check.toml_str(Path("E:/Bloodborne/Bloodborne-v1.09-wDLCs")),
                         '"E:/Bloodborne/Bloodborne-v1.09-wDLCs"')

    def test_quote_still_refused(self):
        with self.assertRaises(ValueError):
            area_check.toml_str('a"b')


class WindowedTest(unittest.TestCase):
    def test_parse_window(self):
        self.assertIsNone(area_check.parse_window(None))
        self.assertEqual(area_check.parse_window("2560x1440"), (2560, 1440))
        with self.assertRaises(ValueError):
            area_check.parse_window("2560*1440")

    def test_run_toml_window_keeps_render_size(self):
        toml = area_check.run_toml("A", "E", "D", 9446, 1920, 1080, window=(2560, 1440))
        self.assertIn("width = 2560", toml)
        self.assertIn("height = 1440", toml)
        self.assertIn('resolution = "1920x1080"', toml)


if __name__ == "__main__":
    unittest.main()
