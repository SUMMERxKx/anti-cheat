# Safety boundary

This project builds an aimbot and an anti-cheat that detects it. The aimbot exists only so the
detector has something real to catch. This file sets the boundary the project stays inside and
lists how each part of that boundary is enforced.

## The boundary

The aimbot works only against our own modified fork of Red Eclipse, offline, against bots.

- It reads player positions only through an interface we add to that fork. Nothing else exposes
  that interface.
- It never reads or writes the memory of another process.
- It never injects code into another process and never hooks another program.
- It uses no memory offsets and no byte signatures.
- It never fakes mouse or keyboard input at the operating system level.
- Nothing in this repository works against the stock or Steam release of Red Eclipse, or on any
  public server.
- Nothing in this repository helps anyone evade a real anti-cheat.

The game fork is based on Red Eclipse and is not affiliated with the Red Eclipse project.

## Enforcement layers

Each layer is backed by a test or a CI check, not only by this document. Some layers depend on
code that does not exist yet. Those layers are marked as planned and arrive with the pull request
that adds the code they protect.

| # | Layer | Status |
|---|---|---|
| 1 | Capability only inside the fork | Planned, arrives with the fork and the lab interface |
| 2 | Network code compiled out of the lab build | Planned, arrives with the fork lockout |
| 3 | Renamed protocol and game identity | Planned, arrives with the fork lockout |
| 4 | Local-game runtime gate | Planned, arrives with the lab interface |
| 5 | Forbidden-API guard in CI | Enforced now |
| 6 | No tuning the cheat against the detector, no "undetectable" settings | In effect now as a project rule |
| 7 | Scope stated in the README and in this file | In effect now |

The CMake option `LAB_BUILD` already exists and defaults to ON. Configuring with it OFF stops with
an error, because this repository only supports the offline lab build. The fork lockout in layer 2
will hang off that option.

### 1. Capability only inside the fork

The aimbot is a library linked into the lab build of the fork. It reads a snapshot of the game
through `lab_get_snapshot()` and sets the view through `lab_set_view()`. Both functions exist only
in our fork. A stock binary has nothing for the aimbot to attach to. Because the aimbot never
fakes operating system input, it cannot drive any other game either.

### 2. No networking to remote hosts in the lab build

With `LAB_BUILD` on, the fork compiles out the code that connects to remote servers, the master
server link, LAN discovery and server join links. The in-process server binds to loopback only. A
CI step will inspect the symbols of the lab binary and fail if the remote connect code is present.

### 3. Distinct protocol and identity

The fork changes the game version, game ID and game name values. Even if someone turned
networking back on, stock servers would refuse the fork and the fork would refuse stock clients. A
unit test will assert that these values differ from upstream.

### 4. Local-game runtime gate

`lab_get_snapshot()` returns an empty snapshot and `lab_set_view()` does nothing unless the session
is a local game. This is defence in depth in case layer 2 ever regresses.

### 5. Forbidden-API guard

`scripts/check-forbidden-apis.sh` scans every file we own and fails if any entry from the list
below appears in it. The match is a case-insensitive substring match, so an entry also catches its
variants (for example every function whose name starts with an entry). The scan skips the future
`game/` submodule, which holds upstream game code, and skips the list block in this file. Every
other line of this file is scanned.

The guard reads its list from the block below, so this block is the single source of truth. Keep
one entry per line, each wrapped in backticks.

<!-- forbidden-api-list:begin -->
- `task_for_pid`
- `mach_vm_read` (and variants)
- `mach_vm_write`
- `vm_read`
- `vm_write`
- `ReadProcessMemory`
- `WriteProcessMemory`
- `NtReadVirtualMemory`
- `NtWriteVirtualMemory`
- `OpenProcess`
- `VirtualAllocEx`
- `CreateRemoteThread`
- `SetWindowsHookEx`
- `ptrace`
- `process_vm_readv`
- `process_vm_writev`
- `/mem"` (path literals such as /proc/<pid>/mem)
- `DYLD_INSERT_LIBRARIES`
- `LD_PRELOAD`
- `CGEventPost`
- `CGEventTapPostEvent`
- `CGEventCreateMouseEvent`
- `CGWarpMouseCursorPosition`
- `SendInput`
- `mouse_event`
- `keybd_event`
- `IOHIDPostEvent`
- `uinput`
- `XTestFake` (and variants)
- `XSendEvent`
<!-- forbidden-api-list:end -->

`tests/safety/forbidden_apis_selftest.sh` proves the guard works. It plants each entry in a
throwaway repository and checks that the guard fails every time. It also checks that a clean tree
passes, that the `game/` submodule path is skipped, and that a match elsewhere in this file is
still caught. Both scripts run in CI on every push and pull request.

### 6. Scope rules for the anti-cheat side

Detection runs only on telemetry from our own fork. There is no kernel or driver work and no study
of how real anti-cheats work internally. The smooth aimbot mode and the synthetic human model exist
to measure detection. We report where detection fails, but we do not tune the cheat against the
detector in a loop, and we publish no settings described as "undetectable".

### 7. Documentation

The README and this file state the scope, that the fork is based on Red Eclipse and not
affiliated, and that nothing here works with the Steam or stock release or on any public server.

## Residual risk

In any open-source game, anyone can edit their own client to move the camera. This repository adds
no real capability beyond that. It publishes nothing that works against a stock binary, a live
server or a real anti-cheat, and layers 2 to 4 keep the fork off public servers.
