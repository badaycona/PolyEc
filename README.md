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

## Integrated realm / M3 evidence

Open **Quốc gia · Sandbox** for the six provinces and crown, 10,000 independent
people, and three recurrent PPO policy families. Jobs, funded payroll/relief,
households, sparse organization funds, education, transport, credit, charter
rights and delayed public/secret evidence use the shared C++ core. UI and AI
commands use the same validator. Saves retain world, sampling and GRU memory.
Twelve draft augments now have typed court actions and ledger/capacity tests.
Appointments preserve the existing person's ID/assets and reset family memory.

**M3 remains an evidence/diagnosis prototype, not a certified campaign release.**
The v3 learned population became extinct in all eight20year co-play runs.
Accounting passed, so it was insufficient as a quality gate. V6 corrects funded
income, food/health incentives, bounded construction, fiscal incentives,
authority and recurrent training/evaluation boundaries. Its final measured
results and outstanding strategic gates belong in [M3 evidence](docs/M3-evidence.md).
The frozen training protocol is [realm-v6](docs/realm-rl-experiment-v6.md).

Standalone core CI covers GCC/Linux and MSVC/Windows; native Linux CI additionally
builds the extension, runs source and portable smokes and captures a software
render. Consult the linked workflow results for the exact validated revision.
These supersede the historical platform limitations above.
