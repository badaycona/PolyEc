# PolyEc — native prototype + economic RL

Windows prototype of Đại Bàn Cờ Vương Quyền. The C++20 core owns world state;
Godot receives detached PlayerView snapshots and sends commands.
Windows/Linux remain product targets; only Windows was validated this iteration.

## Run

Unzip the complete portable folder and open PolyEc.exe. Keep the runtime, PCK
and bin folder together. Players need no installed Godot, Python, Node,
credentials or network connection. See docs/PLAYER-README.txt for the one-minute
presentation and controls.

## Implemented

- Two territories: Thanh Điền and Hắc Sơn, ID-based selection and native inspector.
- Geographic prototype map: river, mountain ridges, forest, roads and settlements.
- Cursor-anchored zoom, pan, route visibility and Home reset.
- Real steady_clock adapter, 1x/2x/4x speed, pause/resume, no sleep catch-up.
- Focus loss pauses; returning focus and closing F1 help keep the world paused.
- Vietnamese F1 guide and presentation layout at 1440x900, min window 1100x720.

This presentation prototype now includes a separate economic RL lab: one learned
merchant, six markets and seven goods. It is not a complete campaign, M1/M2/M3,
or learned governance/civilian simulation. See docs/PLAYER-README-RL.txt.

## Pinned and tested toolchain

- Godot 4.5.1 Windows official runtime.
- godot-cpp godot-4.5-stable, commit e83fd0904c13356ed1d4c3d09f8bb9132bdc6b77.
- CMake 4.4.4, Ninja 1.13.2, Zig 0.16.0 / Clang 21.1.0.
- Core Release: 70/70 explicit checks, CTest 1/1.
- Native Godot smoke: 19/19; UI integration smoke: 18/18.
- Packaged executable also passed native and UI integration.
- MSVC and Linux builds were not tested.

Python is required only when building godot-cpp bindings. Build with CMake, Ninja
and a configured C++20 compiler:

~~~powershell
git clone --depth 1 --branch godot-4.5-stable https://github.com/godotengine/godot-cpp third_party/godot-cpp
.\scripts\build-windows.ps1
.\scripts\test-native.ps1 -Godot C:\Tools\Godot\Godot_v4.5.1-stable_win64_console.exe
.\scripts\package-windows.ps1 -Godot C:\Tools\Godot\Godot_v4.5.1-stable_win64_console.exe
~~~

build-windows.ps1 accepts -CMake, -ToolchainFile, -BuildDir and -Jobs.
test-native.ps1 accepts -CTest. Paths to the portable tools actually used are
in WORK_STATUS.md. Third-party notices are under licenses/.

## OpenCode

~~~powershell
.\start-opencode.ps1
~~~

This selects the project's opencode/step-5-preview-free model and uses
--pure --agent build to skip global plugins. Project instructions use PowerShell
and sequential work. Generated code still requires review and executed checks.
Global settings and credentials have not been changed.


## Economic RL lab (2026-10-14)

Open **RL · Phòng thử nghiệm**, then run a 64-decision episode or step once.
The GRU64 actor was trained with recurrent PPO through the same C++ TradeWorld
used by the native executable. ONNX Runtime CPU performs inference, with strict
model hash/shape checks and no heuristic fallback.

Three independent training seeds; validation selects checkpoints before test.
On 100 new held-out worlds (30000–30099), mean profit is 2564.04 coins versus
1839.35 for the greedy reference. Paired difference +724.69, 95% bootstrap CI
[533.94, 928.94]. The economic-only gate passes; full R1 remains false.
V1 failed to beat the baseline and is preserved in ai/models/economic-v1.
V2 artifacts, histories, source hashes and checkpoints: ai/models/economic-v2.

Checks: core70 + trade36546; Godot native19, UI18, RL336; ONNX parity98.
Batch1/3/32/64 PyTorch/Python ORT/native C++ ORT agree; complete 64-step native
replays agree. Windows portable build passes the RL smoke without Python.

Build AI with scripts/build-windows.ps1 -OrtRoot <ORT-SDK>; test with
scripts/test-native.ps1 -IncludeRL; package with scripts/package-windows.ps1
-IncludeRL. Training: scripts/build-training.ps1 and scripts/train-cpu.ps1,
ai/requirements.txt. See docs/rl-experiment-v1.md and docs/rl-schema-v1.md for
reward, split, action mask, scoring and baseline contracts.

Fixed firm/household contracts are environment laws, not learned citizens.
The one-million-record spike tests storage/updates only, not population agency.
Windows validated; Linux/MSVC and full milestone gates remain outstanding.

## Integrated realm / M3 experiments

The **Quốc gia · Sandbox** panel runs an integrated six-province/crown world
with 10,000 people and 128 complex actors, funded jobs, education, travel, loans,
projects, charters and filtered facts. Three ONNX policy families share weights
and keep private recurrent memory and deterministic sampling counters. Save
persists world, RNG and policy memory; mismatched/corrupt loads are atomic.

This is experimental progress toward M1–M3, not a completed release gate. The
first governance run did not improve over initial weights. A regression also
identified expired wage contracts being renewed by starting school; this was
fixed and new training/evaluation is required. History stays in ai/models.
Remaining gaps include complete political negotiation, household resource
sharing, full augment semantics and multistep INT reasoning evidence.

Core CI covers Linux/GCC and Windows/MSVC. Local native app validation remains
Windows only. Packaging and test scripts accept -IncludeRealm. See WORK_STATUS.md
and docs/realm-rl-experiment-v1.md, v2 for precise evidence and limits.
