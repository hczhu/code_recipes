import unittest
import textwrap
import io
from typing import IO

import mp_file_process as mfp

def identityMapper(line: str) -> str:
    return line

def incrementMapper(line: str) -> str:
    num = int(line)
    if num < 0:
        return ""
    return str(num + 1)

class TestSimple(unittest.TestCase):
    def simple_test(self, input_stream: str, expected_output: str,
                    line_processer: mfp.LineProcesser, num_processes: int) -> None:
        input_f = io.StringIO(textwrap.dedent(input_stream))
        output_f = io.StringIO()
        p = mfp.MpFileProcess()
        p.progress_report_interval_seconds = 1
        p.run(input_f, output_f, line_processer, num_processes)
        self.assertEqual(output_f.getvalue(), textwrap.dedent(expected_output))

    def test_identity(self):
        self.simple_test(
            """\
            a
            b
            c
            dd
            eee
            ffff
            """,
            """\
            a
            b
            c
            dd
            eee
            ffff
            """,
            identityMapper,
            3
        )

    def test_increment(self):
        self.simple_test(
            """\
            2
            0
            2
            a
            4
            5
            -1
            """,
            """\
            3
            1
            3
            5
            6
            """,
            incrementMapper,
            2
        )

# Without this if statement, the mp.Process() in mp_file_process.py will
# magically stops working!
if __name__ == "__main__":
    unittest.main()