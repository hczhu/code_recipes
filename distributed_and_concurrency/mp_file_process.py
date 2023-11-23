"""
mp_file_process.py: process a file line-by-line in parallel using multiple processes.
The output is writter to the output file line-by-line in the same order as the input file.
Empty strings are skipped.
"""

import time
import io
import os
import multiprocessing as mp
from multiprocessing import process
from typing import Callable, IO, NamedTuple, Any
import logging
from multiprocessing import queues
from threading import Thread

LineProcesser = Callable[[str], str]
Queue = queues.SimpleQueue[str]
exceptionLinePlaceholder: str = "__#exception#__"

class WorkerInfo(NamedTuple):
    logger: logging.Logger
    queue: Queue
    output_file_name: str
    line_processer: LineProcesser
    # A counter shared between multiple processes.
    processed_lines: Any

# mp.Process() can only take a top-level function as the target.
def mfpWorker(worker_info: WorkerInfo) -> None:
    # An empty string is the poison pill.
    worker_info.logger.info(f"Starting worker {process.current_process().name}")
    with open(worker_info.output_file_name, "w") as output_file:
        while (line := worker_info.queue.get().strip()) != "":
            try:
                output_line = worker_info.line_processer(line)
                output_file.write(output_line + "\n")
            except Exception as e:
                worker_info.logger.error(f"Error processing line [{line}]: {e}")
                output_file.write(exceptionLinePlaceholder + "\n")
            worker_info.processed_lines.value += 1

class MpFileProcess:
    def __init__(self, logger: logging.Logger|None = None, tmp_file_dir: str = "/tmp") -> None:
        if logger is None:
            logger = logging.getLogger(__name__)
            handler = logging.StreamHandler()
            formatter = logging.Formatter(
                f"%(asctime)s p:%(processName)s t:%(threadName)s %(filename)s:%(lineno)s %(levelname)s: %(message)s")
            handler.setFormatter(formatter)
            logger.addHandler(handler)
            logger.setLevel(logging.DEBUG)
        self.logger = logger
        self.read_lines = 0
        self.output_lines = 0
        self.stop = False
        self.progress_report_interval_seconds = 10
        self.temp_file_dir = tmp_file_dir
     
    def reportProgress(self, worker_info: WorkerInfo) -> None:
        while not self.stop:
            processed_lines = 0
            for worker in worker_info:
                processed_lines += worker.processed_lines.value
            self.logger.info(f"Progress: read {self.read_lines}, processed {processed_lines}, "
                             f"and output {self.output_lines} lines.")
            time.sleep(self.progress_report_interval_seconds)
    
    def run(self,
            input_file: IO[str],
            output_file: IO[str],
            line_processer: LineProcesser,
            num_processes: int) -> None:
        num_processes = min(num_processes, mp.cpu_count())
        self.logger.info(f"Using {num_processes} processes")
        worker_infos: list[WorkerInfo] = []
        workers: list[mp.Process] = []
        self.read_lines = 0
        self.output_lines = 0
        self.stop = False
        for i in range(num_processes):
            q = mp.SimpleQueue()
            output_file_name = os.path.join(self.temp_file_dir, f"worker_output_{i}.txt")
            worker_info = WorkerInfo(self.logger, q, output_file_name, line_processer, mp.Value('i', 0))
            worker = mp.Process(target=mfpWorker, args=(worker_info,), name=f"worker_{i}")
            worker.start()
            self.logger.info(f"Started worker {worker.name}")
            worker_infos.append(worker_info)
            workers.append(worker)

        reporter = Thread(target=self.reportProgress, args=(worker_infos,), name="reporter")
        reporter.start()

        q_idx = 0
        for line in input_file:
            worker_infos[q_idx].queue.put(line)
            q_idx = (q_idx + 1) % num_processes
            self.read_lines += 1
        
        self.logger.info("Finished reading input file and putting positions into worker queues.")
        for worker_info in worker_infos:
            # Poison pill.
            worker_info.queue.put("")
        self.logger.info(f"Joining {len(workers)} workers.")
        for worker in workers:
            # This is blocking until the worker finishes.
            worker.join()

        self.logger.info(f"Finished processing {self.read_lines} lines. "
                         "Joining temp output files from workers.")

        temp_output_files = [
            open(worker_info.output_file_name, "r") for worker_info in worker_infos
        ]

        closed_files = 0
        while closed_files < len(worker_infos):
            for temp_output_file in temp_output_files:
                if not temp_output_file.closed:
                    l = temp_output_file.readline().strip()
                    if l == "":
                        temp_output_file.close()
                        closed_files += 1
                    else:
                        if l != exceptionLinePlaceholder:
                            output_file.write(l + "\n")
                        self.output_lines += 1

        self.logger.info(f"Finished outputing {self.output_lines} lines. "
                         "Joining the reporter.")
        self.stop = True
        reporter.join()

    def runWithFiles(self,
                     input_file: str,
                     output_file: str,
                     line_processer: LineProcesser,
                     num_processes: int) -> None:
        with open(input_file, 'r') as input, open(output_file, 'w') as output:
            self.run(input, output, line_processer, num_processes)


# Has to be at the top-level to work with mp.Process().
def incrementMapper(line: str) -> str:
    return str(int(line) + 1)

# Has to be protected in __name__ == "__main__" to work with mp.Process().
if __name__ == "__main__":
    input_f = io.StringIO("0\n1\n5\n2\n3\n4")
    output_f = io.StringIO()
    p = MpFileProcess()
    p.progress_report_interval_seconds = 1
    p.run(input_f, output_f, incrementMapper, 3)
    assert output_f.getvalue() == "1\n2\n6\n3\n4\n5\n", output_f.getvalue()