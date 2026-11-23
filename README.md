# anti-cheat

A C++ aimbot and a server-side anti-cheat that detects it, built only against an offline "lab"
fork of the open-source shooter Red Eclipse.

The lab fork is based on Red Eclipse and is not affiliated with the Red Eclipse project. Nothing in
this repository works with the Steam or stock release of Red Eclipse, or on any public server.

## Scope

The project has three parts.

- **The lab fork.** A renamed fork of Red Eclipse that plays offline against bots only. It adds a
  small interface that hands a snapshot of the game to our code and lets our code set the view.
- **The aimbot.** A library linked into the lab fork. It reads positions only through that
  interface and aims at a target's head. It exists so the detector has something real to catch.
- **The anti-cheat.** A detector that works the way a game server would. It reads aim telemetry
  recorded on the server side (view angles, shots, hits and kills) and flags sessions where the aim
  does not look human. It never reads the aimbot's code or its settings.

The aimbot, the detector and the tools are written in C++20. C99 is used only for the boundary
layer: the `lab_api.h` interface between the game and our code, and the `labtrace` telemetry
library.

## Safety boundary

The aimbot works only against our own fork, offline, against bots. It never reads or writes the
memory of another process, never injects code, never hooks another program, uses no memory offsets
or signatures, and never fakes mouse or keyboard input. Nothing here works against the stock game
or on public servers, and nothing here helps anyone evade a real anti-cheat.

[`SAFETY.md`](SAFETY.md) explains the boundary in full and lists how each part of it is enforced,
including the forbidden-API guard that CI runs on every pull request and every push to `main`.

## Status

The offline game fork, network lockout and C99 lab interface are in place. The superbuild builds
the client and server, and CI runs the existing toolchain and forbidden-API checks on macOS arm64
and Linux. The aimbot supports snap and smooth aim. Telemetry and the detector are still planned.

## Repository layout

```
anti-cheat/
├── README.md            scope, safety boundary, build steps
├── SAFETY.md            the boundary and how it is enforced
├── LICENSE              MIT for our own code
├── CMakeLists.txt       superbuild: the game fork, then our libs and tools
├── scripts/
│   ├── check-forbidden-apis.sh   forbidden-API guard
│   └── fetch-game-data.sh        fetches the upstream game data, not vendored
├── game/                git submodule pointing at SUMMERxKx/anti-cheat-lab
├── include/lab/lab_api.h         C99 interface between the game and our code
├── libs/
│   ├── labtrace/        (planned) C99 telemetry writer and reader
│   ├── aimmath/         C++20 aim maths
│   ├── aimbot/          C++20 target selection and aim controllers
│   ├── humanmodel/      (planned) C++20 scripted human-like aim model, always labelled SYNTHETIC
│   └── detect/          (planned) C++20 feature extraction, heuristics and scoring
├── tools/               (planned, empty for now)
│   ├── acdetect/        recorded runs to a JSON and Markdown detection report
│   ├── labsim/          headless simulator for CI, no GPU needed
│   └── evalrun/         scripted session runner
├── eval/                (planned) configs, plotting scripts and committed results
├── tests/               toolchain check and safety tests, later unit and end-to-end tests
└── docs/                (planned) architecture, telemetry format, heuristics, evaluation method
```

## Building

You need CMake 3.20 or newer, a C99 and C++20 compiler, pkg-config, bash and git.
The game also needs SDL2, SDL2_image, OpenAL Soft, libsndfile, FreeType, zlib and OpenGL.

On macOS arm64, install the game dependencies with Homebrew:

```sh
brew install cmake pkg-config sdl2 sdl2_image openal-soft libsndfile freetype
```

If a newer OpenAL Soft release fails to build against the Mac SDK, use OpenAL Soft 1.24.3 from
source in a local dependency prefix. SDL2_image 2.8.8 also builds from source if needed. Set
`PKG_CONFIG_PATH` to that prefix's `lib/pkgconfig` directory when configuring this project.

On Ubuntu:

```sh
sudo apt-get install cmake build-essential pkg-config zlib1g-dev libsdl2-dev libsdl2-image-dev libopenal-dev libsndfile1-dev libgl1-mesa-dev libfreetype-dev
```

Initialize the pinned code submodule, then build:

```sh
git submodule update --init -- game
cmake -S . -B build
cmake --build build --parallel 4
ctest --test-dir build --output-on-failure
```

The `LAB_BUILD` option defaults to ON and has to stay ON. Configuring with `-DLAB_BUILD=OFF` stops
with an error. Steam and Discord are disabled even if requested. For a server-only build, add
`-DBUILD_CLIENT=OFF` to the configure command.

The data is a separate multi-GB download. It is needed to play, but not to compile or run the
existing tests. The script fetches the pinned upstream data submodules with shallow history:

```sh
./scripts/fetch-game-data.sh
```

Launch the binary directly from the game directory. On macOS:

```sh
cd game
../build/game-build/anti_cheat_lab_macos -df0 -dw1280 -dh720
```

On Linux the executable is `anti_cheat_lab_linux`. The dedicated server is
`anti_cheat_lab_server_macos` or `anti_cheat_lab_server_linux` in the same build directory.
The lab uses its own home directory, separate from a real Red Eclipse install. On macOS it is
`~/Library/Application Support/Anti-Cheat Lab/`. The Unix product name is `anti-cheat-lab`.

Start a local match from the Offline Match menu and add bots with Insert. Open the console with
the backquote key and run `lab_dump` to print the current snapshot. It reports `active=0` outside a local match.
The API only includes enemy bots. In a team match, teammates are omitted.

`include/lab/lab_api.h` documents the small C99 boundary. The game refreshes the snapshot each
frame before the player's shot, with positions in world units and angles in degrees. `lab_set_view()` uses the same angle
assignment and range correction as mouse look. Both calls do nothing outside a local offline match.
`fire_held` follows the engine's fire action, not the physical trigger: a semi-auto weapon such as
the shotgun clears it after each shot, while a full-auto weapon such as the SMG keeps it set.
Layer 4 in [`SAFETY.md`](SAFETY.md) describes the gate.

### Aiming in the lab

In a local match with bots, enter these console commands on separate lines:

```text
bind F [lab_aim_key]
lab_aim 1
```

Hold F or fire to aim at a visible enemy bot's head. `lab_aim 0` turns aiming off (the default),
`lab_aim 1` snaps in one frame, and `lab_aim 2` turns smoothly. The closest angle to the crosshair
wins, with distance breaking ties. The selected bot stays selected until it dies, loses visibility,
or leaves the cone. Releasing both aim and fire clears the selection and reaction timer.

`lab_aim_fov` sets the maximum angle from the crosshair in degrees (default 25).
`lab_aim_speed` caps smooth turning in degrees per second (default 360).
`lab_aim_ease` adds exponential ease-out in inverse seconds (default 0 disables easing).
`lab_aim_reaction` delays smooth aim after acquiring a target in milliseconds (default 0).
`lab_aim_origin 0` uses the eye (default), and `lab_aim_origin 1` uses the muzzle to compare parallax.
The aim key is a game action bound through the console, with no operating system input.

At this upstream revision, ranged shots are projectiles, including the rifle. Zapper primary uses
`COLLIDE_SCAN`, but that scans between the muzzle and the moving projectile, not an instant ray to
the target. Melee attacks use immediate short weapon segments. There is no default ranged weapon
with true instant hits. Rifle secondary is fast at 100000 world units per second, but still has
travel time. The aimbot aims at the current head position and does not lead projectiles or control recoil.

The macOS arm64 build was checked on an Apple M2 with OpenAL Soft 1.24.3 and SDL2_image 2.8.8.
A command script launched the graphical client with a separate home directory, started an offline
Bloodlust match with three bots and ran `lab_dump`. The snapshot was active during the match and
all zero before play and after disconnect. The game log and a captured frame were inspected.
The Mac UI tool could not reliably address the unbundled executable, so the check used the game's
own console commands. No manual movement or aiming was checked.

To run the forbidden-API guard on its own:

```sh
bash scripts/check-forbidden-apis.sh
```

## Licence

Our own code is under the MIT licence. See [`LICENSE`](LICENSE). Red Eclipse code and content keep
their own licences and live in the fork, not in this repository.
