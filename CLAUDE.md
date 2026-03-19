# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Repository Overview

Personal coding recipe repository — a collection of learning examples, experiments, and practical implementations. Organized by language/domain: `ai-ml/`, `cpp/`, `python/`, `golang/`, `kubernetes/`, `dotfiles/`, `scripts/`.

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
Individual scripts run directly: `python script.py`. No centralized test runner configured.

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
- **golang/**: Three sub-projects with their own packages: `etcd_leader_election/`, `http_client_pool/`, `leetcode/`. Shared `go.mod` at `golang/` root (Go 1.20, depends on etcd client v3).
- **dotfiles/**: Personal shell/editor config (`bashrc`, `vimrc`, `tmux.conf`, `gitconfig`, etc.) plus `install-scripts/` for Ubuntu Docker setup.
- **scripts/**: Standalone bash utilities (`stockfish_chess_session.sh`, `learning_repos.sh`).
