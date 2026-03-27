# FreeRTOS Kernel - Copilot Instructions

## Build System

### i486-flat port (this repo's active development target)

The `i486-flat/` directory is a bare-metal i486 FreeRTOS port. It has two independent build components:

**1. Bootloader** (build once, or when bootloader sources change):
```bash
cd i486-flat/bootloader
./build.sh          # produces bootloader/build/bios.rom
```

**2. Kernel** (run from `i486-flat/cmake/`):
```bash
cd i486-flat/cmake
cmake -S . -B ../build -DCMAKE_TOOLCHAIN_FILE=toolchain-i486-flat.cmake
cmake --build ../build/

# Combined one-liner (typical workflow):
cmake -S . -B ../build -DCMAKE_TOOLCHAIN_FILE=toolchain-i486-flat.cmake && cmake --build ../build/

# Verbose build:
cmake --build ../build/ -v

# Clean rebuild:
rm -rf ../build && cmake -S . -B ../build -DCMAKE_TOOLCHAIN_FILE=toolchain-i486-flat.cmake && cmake --build ../build/
```

Output binary: `i486-flat/build/i486-flat`

**Run with QEMU** (from `i486-flat/`):
```bash
qemu72 -cpu 486 -m 128M -bios bootloader/build/bios.rom -kernel build/i486-flat -nographic
```

**Run with GDB remote debugging** (two terminals, from `i486-flat/`):
```bash
# Terminal 1 — start QEMU waiting for debugger
qemu72 -cpu 486 -m 128M -bios bootloader/build/bios.rom -kernel build/i486-flat -nographic -s -S

# Terminal 2 — attach GDB
gdb -x gdbcmds.txt build/i486-flat
```

`gdbcmds.txt` contains pre-configured breakpoints at key kernel entry points (context switch, task creation, MMU init, etc.).

### Generic CMake consumer setup

FreeRTOS Kernel is a library consumed via CMake `FetchContent`. A consumer project must provide a `freertos_config` interface library pointing to a `FreeRTOSConfig.h`.

```cmake
add_library(freertos_config INTERFACE)
target_include_directories(freertos_config SYSTEM INTERFACE ${CMAKE_CURRENT_LIST_DIR})

set(FREERTOS_PORT "GCC_ARM_CM4F" CACHE STRING "" FORCE)  # required
set(FREERTOS_HEAP "4" CACHE STRING "" FORCE)              # optional (1–5)

FetchContent_MakeAvailable(freertos_kernel)
```

The `examples/cmake_example/` directory is the canonical minimal project template.

## Tests

Unit tests live in the sibling `FreeRTOS/Test/CMock/` repository (not inside this repo). From that directory:

```bash
# Run all tests with sanitizers
make clean && make ENABLE_SANITIZER=1 run_col_formatted

# Generate coverage report
make clean && make lcovhtml
lcov --config-file FreeRTOS/Test/CMock/lcovrc --summary FreeRTOS/Test/CMock/build/cmock_test.info
```

## Lint / Static Analysis / CI

CI runs automatically on push and PRs via `.github/workflows/`:

| Check | Tool |
|---|---|
| Code formatting | Uncrustify (config: `.github/uncrustify.cfg`); excludes `portable/` |
| Spell check | cSpell (config: `cspell.config.yaml`, words: `.github/.cSpellWords.txt`) |
| MISRA compliance | Coverity 2023.6.1 (`examples/coverity/coverity_misra.config`) |
| File headers | `kernel-checks.yml` validates copyright on all source files |
| Link verification | Allowlist: `.github/allowed_urls.txt` |

To find all inline MISRA suppression comments:
```bash
grep 'MISRA Ref' . -rI
```

## Architecture

### Core source files (root)
| File | Responsibility |
|---|---|
| `tasks.c` | Scheduler, TCBs, task creation/deletion/notification, context switching |
| `queue.c` | Queues, semaphores, mutexes |
| `list.c` | Doubly-linked list — the fundamental data structure used throughout |
| `event_groups.c` | Event flag synchronization |
| `stream_buffer.c` | Stream buffers and message buffers |
| `timers.c` | Software timers and deferred function calls |
| `croutine.c` | Optional co-routine support (rarely used) |

### Portable layer (`portable/`)
Two-level abstraction: **compiler toolchain** → **CPU architecture**.

```
portable/
  GCC/ARM_CM4F/      ← port.c + portmacro.h + portASM.s per port
  IAR/ARM_CM4F_MPU/
  MemMang/           ← heap_1.c … heap_5.c (choose one per project)
  Common/            ← mpu_wrappers.c, mpu_wrappers_v2.c
  ThirdParty/        ← community-contributed ports
```

Each port directory provides:
- `port.c` — context switch, tick ISR, critical section enter/exit
- `portmacro.h` — `portSTACK_TYPE`, `portBASE_TYPE`, `portYIELD`, etc.

When adding a new port, use an existing port of the same toolchain/architecture family as a reference.

### Configuration (`FreeRTOSConfig.h`)
Every consumer project supplies this file. The template is at `examples/template_configuration/FreeRTOSConfig.h`. Required macros include `configCPU_CLOCK_HZ`, `configTICK_RATE_HZ`, `configMAX_PRIORITIES`, `configUSE_PREEMPTION`. All `configXXX` macros that are not defined fall back to defaults in `include/FreeRTOS.h`.

## Naming Conventions

These are strict and enforced by code review.

### Variable prefixes
| Prefix | Meaning |
|---|---|
| `p` / `pp` | pointer / pointer to pointer |
| `x` | `BaseType_t` or structure instance |
| `u` | unsigned |
| `c` | `char` |
| `uc` | `uint8_t` |
| `s` | `int16_t` |
| `l` | `int32_t` |
| Combine: `px`, `ux`, `pc` | pointer to the base type |

### Function prefixes
| Prefix | Return type |
|---|---|
| `v` | `void` |
| `x` | `BaseType_t` or handle |
| `ux` | `UBaseType_t` |
| `pv` | `void *` |
| `e` | enum |

### Macros
- `config` prefix — compile-time kernel configuration (`configMAX_PRIORITIES`)
- `port` prefix — port-specific definitions (`portYIELD`, `portMAX_DELAY`)
- `pd` / `err` — return codes (`pdTRUE`, `pdFAIL`, `errQUEUE_EMPTY`)
- All other macros: `SCREAMING_SNAKE_CASE`

### Types
All types end in `_t`: `BaseType_t`, `UBaseType_t`, `TickType_t`, `TaskHandle_t`, `QueueHandle_t`.

## MISRA C:2012 Compliance

The kernel targets MISRA C:2012. Deviations are documented in `MISRA.md` with justifications. When suppressing a Coverity finding, add an inline comment of the form:
```c
/* MISRA Ref X.X.X [Deviation reason] */
```

Common documented deviations cover Rules 8.4, 11.1, 11.3, 11.5, 14.3, 18.1, and Dir 4.7.

## Coding Style

- Full style guide: https://www.FreeRTOS.org/FreeRTOS-Coding-Standard-and-Style-Guide.html
- Every source file must carry the MIT SPDX license header.
- Use `configASSERT()` for internal invariant checks (not standard `assert()`).
- Conditional compilation uses `#if ( configXXX == 1 )` with spaces inside parentheses.
- Formatting is enforced by Uncrustify; run it before submitting (excludes `portable/`).

## Pull Request Process

PRs require 2 approvals (including 1 CODEOWNER). The expected lifecycle is:
1. Triage → 2. Concept ACK → 3. Code Review → 4. Testing → 5. Merge

Respond to review comments within 4 weeks or the PR may be closed. See `.github/pull_request_process.md` for full details.
