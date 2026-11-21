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

This is the scaffold. It holds the docs, the licence, a CMake superbuild skeleton, CI on macOS arm64
and Linux, and the forbidden-API guard. Everything else in the layout below is planned.

## Repository layout

```
anti-cheat/
├── README.md            scope, safety boundary, build steps
├── SAFETY.md            the boundary and how it is enforced
├── LICENSE              MIT for our own code
├── CMakeLists.txt       superbuild: dependencies, then the game fork, then our libs and tools
├── scripts/
│   ├── check-forbidden-apis.sh   forbidden-API guard
│   └── fetch-game-data.sh        (planned) fetches the upstream game data, not vendored
├── game/                (planned) git submodule pointing at the Red Eclipse lab fork
├── include/lab/lab_api.h         (planned) C99 interface between the game and our code
├── libs/                (planned, empty for now)
│   ├── labtrace/        C99 telemetry writer and reader
│   ├── aimmath/         C++20 aim maths
│   ├── aimbot/          C++20 target selection and aim controllers
│   ├── humanmodel/      C++20 scripted human-like aim model, always labelled SYNTHETIC
│   └── detect/          C++20 feature extraction, heuristics and scoring
├── tools/               (planned, empty for now)
│   ├── acdetect/        recorded runs to a JSON and Markdown detection report
│   ├── labsim/          headless simulator for CI, no GPU needed
│   └── evalrun/         scripted session runner
├── eval/                (planned) configs, plotting scripts and committed results
├── tests/               toolchain check and safety tests, later unit and end-to-end tests
└── docs/                (planned) architecture, telemetry format, heuristics, evaluation method
```

## Building

You need CMake 3.20 or newer, a C99 and C++20 compiler (a recent Clang or GCC), bash and git.

```sh
cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

The `LAB_BUILD` option defaults to ON and has to stay ON. Configuring with `-DLAB_BUILD=OFF` stops
with an error.

To run the forbidden-API guard on its own:

```sh
bash scripts/check-forbidden-apis.sh
```

The game fork, its data download and the build steps for it are added by a later pull request.

## Licence

Our own code is under the MIT licence. See [`LICENSE`](LICENSE). Red Eclipse code and content keep
their own licences and live in the fork, not in this repository.
