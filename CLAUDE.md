# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Repository Overview

Personal coding recipe repository — a collection of learning examples, experiments, and practical implementations. Organized by language/domain: `ai-ml/`, `cpp/`, `python/`, `golang/`, `kubernetes/`, `dotfiles/`, `scripts/`.

## Dev Container

`containers/` holds a Docker image (`dev-container.Dockerfile`) that supports all three languages.

```bash
./containers/build.sh          # build image (tag: code-recipes-dev)
./containers/run.sh            # interactive bash shell with repo mounted at /workspace
./containers/run.sh bash -c "cd golang && go test ./..."
./containers/run.sh bash -c "cd python && pytest test_async_crawler.py -n auto"
./containers/run.sh bash -c "cd ai-ml/mlight && make"
```

The repo is bind-mounted into `/workspace` at run time; the image itself only bakes in the Go module cache and pre-built gtest libraries.

## Build & Test Commands

### C++
```bash
cd cpp
mkdir build && cd build
cmake ..
make
```
- CMake 3.5+, outputs to `bin/`
- Test files follow `*Test.cpp` naming pattern
- Formatting: `.clang-format` in `cpp/`

### Go
```bash
cd golang
go test ./...          # run all tests
go test ./etcd_leader_election/...
go test ./http_client_pool/...
```

### Python
Most scripts embed `unittest` and run directly: `python script.py`. Two files use `pytest`:
```bash
cd python
pytest test_async_crawler.py -n auto   # requires pytest-asyncio, pytest-xdist
python mp_file_process_test.py         # unittest-based, run directly
```

### ai-ml/mlight (C++ ML sub-project)
```bash
cd ai-ml/mlight
make        # build and run tests
make clean
```
Requires the Armadillo library.

## Architecture Notes

- **cpp/**: ~79 standalone `.cc`/`.cpp` files covering algorithms, data structures, concurrency, and C++ language features. `cpp/4fun/` has self-contained mini-projects (Sudoku solver, scheduler, etc.).
- **python/**: Standalone scripts — segment trees, heaps, external sort, async crawler, multiprocessing, a minimal SQL interpreter, and LeetCode solutions.
- **golang/**: Three sub-projects with their own packages: `etcd_leader_election/`, `http_client_pool/`, `leetcode/`. Shared `go.mod` at `golang/` root (Go 1.20, depends on etcd client v3). The etcd leader election tests spin up an embedded etcd cluster via `integration.NewCluster` — no external etcd process needed.
- **dotfiles/**: Personal shell/editor config (`bashrc`, `vimrc`, `tmux.conf`, `gitconfig`, etc.) plus `install-scripts/` for Ubuntu Docker setup. `.clang-format` at repo root mirrors `cpp/.clang-format`.
- **scripts/**: Standalone bash utilities (`stockfish_chess_session.sh`, `learning_repos.sh`).
- **from-ai/**: AI-generated helpers — currently `minecraft/` with Bedrock server management scripts (list non-natural blocks, fetch latest download URL, auto-start).
- **cpp/system/**: Low-level OS examples (`mmap_example.cc`). `cpp/scm-challenge-2017/` has two standalone challenge solutions.
- **IBM-Ponder-This-solvers/** (git submodule): solvers for [IBM Ponder This](https://research.ibm.com/haifa/ponderthis/index.shtml) monthly puzzles. C++ files use gtest, gflags, glog, fmt, libevent, libbenchmark, and OpenMP; Python files use stdlib only. See its own `CLAUDE.md` and `Makefile` for build details.
