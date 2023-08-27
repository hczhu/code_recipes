#!/usr/bin/env python3

import sys
import string
import typing
import logging
import heapq
from collections import defaultdict
import io
import unittest
import textwrap

logging.basicConfig(level=logging.INFO)

class PeekableStringIO(io.StringIO):
    def __init__(self, value=''):
        value = value.encode('utf8', 'backslashreplace').decode('utf8')
        io.StringIO.__init__(self, value)
    def peek(self, n=1):
        pos = self.tell()
        res = self.read(n)
        self.seek(pos)
        return res

def peek(f: typing.TextIO, n=1):
    pos = f.tell()
    res = f.read(n)
    f.seek(pos)
    return res

def read_next_token(f):
    is_dilimiter = lambda c: c == "\n" or c == " "
    is_token_char = lambda c: (not is_dilimiter(c)) and c != ""
    while is_dilimiter(peek(f)):
        f.read(1)
    if peek(f) == "":
        return ""
    res = ""
    while is_token_char(peek(f)):
        res += f.read(1)
    return res

def split_into_files(input: typing.TextIO, memory_budget: int) -> (typing.List[str], int):
    """Splits the input stream into multiple files, each of which is no larger
    than the memory budget.

    Args:
      input (typing.TextIO): the input stream.
      memory_budget (int): the maximum amount of memory this process should

    Returns:
      typing.List[str]: a list of file names that contain the split input.
      int: the maximum key/value pair size in bytes.
    """
    file_names = []
    file_idx = 0
    max_kv_bytes = 0
    def read_kv(input):
        nonlocal max_kv_bytes
        k = read_next_token(input)
        if k == "":
            return None, None
        v = read_next_token(input)
        max_kv_bytes = max(max_kv_bytes, len(k) + len(v))
        return k, v

    while True:
        current_memory_budget = memory_budget
        eof = False
        k2vs = defaultdict(list)
        while current_memory_budget > 0:
            k, v = read_kv(input)
            if k is None:
                eof = True
                break
            k2vs[k].append(v)
            current_memory_budget -= len(k) + len(v)
        if len(k2vs) == 0: break
        file_name = f"file_{file_idx}.txt"
        logging.info(f"Writing {len(k2vs)} keys to file {file_name}")
        file_idx += 1
        file_names.append(file_name)
        keys = sorted(k2vs.keys())
        with open(file_name, 'w') as f:
            for k in keys:
                f.write(f"{k} {' '.join(sorted(k2vs[k]))}\n")
    return file_names, max_kv_bytes

def merge_files(files: typing.List[str], output: typing.TextIO):
    """Merges the given files and writes the merged key/value pairs to
    the given output stream.

    Args:
      files (typing.List[str]): the list of files to merge.
      output (typing.TextIO): the output stream.
    """
    logging.info(f"Merging {len(files)} files")
    kv_and_file = []
    for file_name in files:
        f = open(file_name)
        kv_and_file.append((read_next_token(f), read_next_token(f), file_name, f))
    heapq.heapify(kv_and_file)

    prev_k = None
    while len(kv_and_file) > 0:
        k, v, file_name, f = heapq.heappop(kv_and_file)
        if prev_k == k:
            output.write(f" {v}")
        else:
            newline = "\n" if prev_k is not None else ""
            output.write(f"{newline}{k} {v}")
            prev_k = k
        if peek(f) == "\n":
            k = read_next_token(f)
        v = read_next_token(f)
        if k == "" or v == "":
            # EOF
            f.close()
            continue
        heapq.heappush(kv_and_file, (k, v, file_name, f))

def sort_kvs(input: typing.TextIO, output: typing.TextIO, memory_budget: int):
    """
    Sorts the given input stream and writes the sorted key/values pairs to the given output stream.
    The function should use at most memory_budget bytes of memory.
    """
    files, max_kv_bytes = split_into_files(input, memory_budget)
    logging.info(f"Split into {len(files)} files")
    file_batch_size = memory_budget // max_kv_bytes
    assert file_batch_size >= 2, f"memory_budget is too small: {memory_budget} < 2 * {max_kv_bytes} "
    merge_round = 0
    while len(files) > file_batch_size:
        logging.info(f"Merge round {merge_round}")
        merge_round += 1
        new_files = []
        new_file_idx = 0
        while len(files) > 1:
            new_file = f"merge_{merge_round}_{new_file_idx}.txt"
            new_file_idx += 1
            with open(new_file, 'w') as f:
                merge_files(files[:file_batch_size], f)
                new_files.append(new_file)
                files = files[file_batch_size:]
        if len(files) > 0:
            new_files.append(files[0])
        files = new_files
    assert len(files) <= file_batch_size
    merge_files(files, output)

class TestExternalSort(unittest.TestCase):
    def simple_test(self, memory_budget):
        input_stream = textwrap.dedent(
        """\
        x 1
        y 2
        x 3""")
        stdin = PeekableStringIO(input_stream)
        stdout = PeekableStringIO()
        expected_output = textwrap.dedent(
        """\
        x 1 3
        y 2""")
        sort_kvs(stdin, stdout, memory_budget)
        self.assertEqual(stdout.getvalue(), expected_output)

    def test_simple(self):
        self.simple_test(1000000000)
        self.simple_test(4)
        self.simple_test(7)
        self.simple_test(15)

    def test_long_values_minimum_memory(self):
        input_stream = "\n".join([
            f"a {i}" for i in range(100)
        ])
        stdin = PeekableStringIO(input_stream)
        stdout = PeekableStringIO()
        expected_output = "a " + " ".join(sorted([
            f"{i}" for i in range(100)
        ]))
        sort_kvs(stdin, stdout, 6)
        self.assertEqual(stdout.getvalue(), expected_output)

if __name__ == '__main__':
    unittest.main()
    try:
        memory_budget = int(sys.stdin.readline())
        sort_kvs(sys.stdin, sys.stdout, memory_budget)
    except Exception as e:
        logging.error(e)
