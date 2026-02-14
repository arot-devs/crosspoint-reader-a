# Testing Setup

This repo uses PlatformIO for unit tests and firmware builds.

## Prerequisites

- PlatformIO CLI (`pio`)
- Host toolchain: `gcc` and `g++`
- `clang-format` (the repo `.clang-format` is compatible with clang-format 20)

On Ubuntu 22.04 (example):

```bash
sudo apt-get update
sudo apt-get install -y gcc g++ wget gnupg lsb-release
```

Install clang-format 20+ from LLVM APT (example):

```bash
wget -q https://apt.llvm.org/llvm.sh -O /tmp/llvm.sh
chmod +x /tmp/llvm.sh
sudo /tmp/llvm.sh 20
sudo apt-get install -y clang-format-20
sudo update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-20 200
```

## Unit Tests (host/native)

```bash
pio test -e native
```

## Firmware Build

```bash
pio run
```

## Formatting

```bash
bin/clang-format-fix -g
```
