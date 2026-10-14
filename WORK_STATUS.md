# PolyEc — working status
Updated: 2026-10-14 (Asia/Bangkok)

## Result
Presentable Windows native M0a prototype built and packaged.
Core, adapter, map/inspector, time controls, F1 help and portable launcher exist.
This is not the full game or all of M0. M0b is not started.

## Scope and architecture
- C++20 GameWorld owns territories, date, selection and authoritative pause state.
- PlayerView is a detached copy. GDExtension exposes snapshots and command results.
- Pause revision clears fractional time across commands sent directly to world.
- Adapter uses std::chrono::steady_clock; UI render delta never advances the date.
- Sleep gap >0.25s pauses; focus loss pauses, focus return does not resume.
- Supported time scales 0..4; UI exposes1x/2x/4x.
- Two native territories IDs7/8, Vietnamese names; geography is prototype data.
- Godot CanvasItem cartography, polygon picking, cursor zoom, pan, route visibility.
- F1 and keyboard handling tested with focused buttons to avoid double commands.
- Launcher sets the runtime working directory, including when invoked elsewhere.

## Verified dependencies
- Godot4.5.1 official Windows binary: f62fdbde1; archive SHA512 checked.
- godot-cpp4.5 tag godot-4.5-stable, commit e83fd0904c13356ed1d4c3d09f8bb9132bdc6b77.
  There is no godot-4.5.1-stable tag.4.5 bindings are used with4.5.1 runtime.
- CMake4.4.4, Ninja1.13.2, Zig0.16.0 (Clang21.1.0).
- Fonts: Be Vietnam Pro + Newsreader, bundled with OFL notices.
- Runtime/player dependencies: bundled native executable/PCK/DLL only, no Python/Node/API.
- Python3.14.7 used only to generate bindings during build.
- UI rendered with OpenGL3.3 Compatibility on Radeon RX580; no performance benchmark claim.

## Actual checks
- Release core CTest:1/1 test,70/70 explicit checks.
- Native integration:19/19 (class load, IDs, UTF8 names, detached snapshot,
  action rejection, monotonic advance, pause, inactive resume rejection, sleep gap).
- UI integration:18/18 (selection, cursor zoom, pan, speed, F1, Escape,
  Space with button focus, notification-driven focus behavior).
- Native and UI tests also executed through the packaged executable, outside
  its directory; both passed.
- Real rendered viewport captures reviewed for Thanh Điền, Hắc Sơn and F1 layout.
- Native desktop app launched; OS screenshot helper returned an unrelated
  foreground surface, so viewport renders were used for visual QA instead.
  No physical click/focus/minimize manual-pass claim is made.
- Build/runtime errors encountered and fixed: missing OS class in trimmed SDK;
  DLL locked by preview process; library location when using resource pack from
  a different working directory (fixed by native launcher).
- Linux/MSVC not run. No RL training, performance/profile or packaging claims
  for those platforms.

## Reproduction
Project: D:/Code/PolyEc
Portable tools:
C:/Users/Admin/Documents/Codex/2026-10-14/d-code-polyec-ch-y-opencode/work/toolchain
Godot:
C:/Users/Admin/Documents/Codex/2026-10-14/d-code-polyec-ch-y-opencode/work/godot
Toolchain file:
C:/Users/Admin/Documents/Codex/2026-10-14/d-code-polyec-ch-y-opencode/work/zig-toolchain.cmake
Native build:
C:/Users/Admin/Documents/Codex/2026-10-14/d-code-polyec-ch-y-opencode/work/polyec-native
Commands: scripts/build-windows.ps1, scripts/test-native.ps1,
scripts/package-windows.ps1. Pass portable -CMake/-CTest/-ToolchainFile/-Godot
paths as needed. Set PYTHONPATH to work/toolchain/python when using pip's
portable Ninja executable.

Deliverable:
C:/Users/Admin/Documents/Codex/2026-10-14/d-code-polyec-ch-y-opencode/outputs/PolyEc-M0a-Windows

## OpenCode
- Model opencode/step-5-preview-free, no authenticated paid provider.
- start-opencode.ps1 skips global plugins, uses project build agent.
- Visible PowerShell CLI/log windows were opened at user request.
- Model repeatedly over-investigated binding headers; coordinator implemented
  adapter/UI and validated generated core fixes. No completed model review
  result is counted as a check.
- No commit, push, publication or paid resource purchase.

## Next gate
Physical input/focus QA when convenient and Linux toolchain/package validation.
Shared-core binding, ONNX CPU parity and sparse storage primitives are now
implemented below. These do not close the full cross-platform M0b gate.


## Final artifact check
- Portable ZIP created, SHA256 sidecar saved.
- ZIP extracted to a separate folder and native smoke re-run:19/19 pass.
- Final packaged viewport reviewed at1440x900 and1100x720.
- Deliverable: outputs/PolyEc-M0a-Windows.zip; open PolyEc.exe after extraction.
- No OpenCode inference process is left running. Visible log windows remain for review.

## Economic RL delivered (2026-10-14)
- Added TradeWorld shared by pybind11 training and native app, seven goods/six markets.
- This is an early economic curriculum, not full M1/M2/M3 or complete R1.
- Core70/70 + trade36546/36546 checks; C++/Python64-step replay matches.
- M0b primitive spike10k/100k/1m completed;1m data48MB, process peak82.9MB.
  Synthetic storage/graph only, no claim of1m learned citizens.
- V1 PPO: three seeds983040 decisions each; selected via validation. Held-out
  mean673.11 coins, greedy1758.19: baseline quality failed and was preserved.
- V1 native ONNX parity98 checks, batch1/3/32/64, max logit error3.34e-6.
- EconomicLab/Godot336 checks:64 actions and state fingerprints match native ORT.
- Standalone preliminary RL package also passes336 checks without Python.
- V2 continues each own PPO checkpoint,1200 updates, max600s/seed, two CPU threads.
  New untouched test30000..30099, validation10000..10031; selection frozen before test.
  Economic-only margin25 coins locked before test. Training completed:14,745,600
  v2 decisions,1297s total. Test mean2564.04 vs greedy1839.35; paired delta724.69
  with95% bootstrap CI[533.94,928.94]. Economic-only pass; full R1 false.
- Source records/reward contract: docs/rl-experiment-v1.md and docs/rl-schema-v1.md.
- Native app memory sample423.3MiB for one learned merchant/six markets;
  RX580 OpenGL3.3, i7-12700K,15.78GiB RAM. Not profileA/B/C performance evidence.

## Final RL validation and artifact
- V2 final ONNX hash3e41ce286bbfd0ea9aa6a147f51e66873317ccb8fb5c60ab77a05eeb9c88c7ae.
- Final parity98/98; max logits5.72e-6, memory4.47e-7; invalid probability0.
- Native CTest2/2; Godot19/19 native,18/18 UI,336/336 RL with final v2 model.
- Final portable package336/336 RL checks, independent C++ trajectory fixture.
- Actual viewport captured and reviewed, six-market prices/merchant trace/test evidence.
- Deliverable outputs/PolyEc-RL-Windows.zip; unzip and open PolyEc.exe.
- Source and checkpoints retained; no commit/push or unrelated project changes.
- Remaining: complete world systems and learned governance/civilian/child families,
  actual large-population agency/performance and Linux/MSVC validation.
- Final ZIP extracted under a Unicode/spaces path;336/336 RL checks pass again. ZIP SHA256 f11882611c5fbaf5fc57e440a7b0278e0aa1a0c9200929476bfdc4711646aa4f. No training/OpenCode process remains running.

## M1–M3 implementation in progress (2026-10-14)
- Repository isolated at D:/Code/PolyEc/.git, origin badaycona/PolyEc.
- New shared Realm: six provinces plus crown, seven goods, labor/education,
  finite markets, debt/interest/default, material-funded projects and routes.
- 10k distinct people, households, sparse relationships, health/age/birth/death,
  charters, delayed public reporting, private facts and investigation.
- Player and AI use the same validator; sampled actions from three learned
  ONNX families; per-person memory and sampling persisted in save.
- Realm core tests14617 checks; native/Godot realm smoke95/95 executed.
- V1 training: all three families, three seeds each, config/holdout preserved.
  Governance initial-improvement gate failed; civilian baseline lowerCI misses
  noninferiority margin. These are diagnosed experiments, NOT full R1 passes.
- V2 fresh training underway after funded tax/aid fixes; new untouched test set.
- Several augment effects, deep political conflict and stat strategy validation
  remain incomplete. Do not mark complete M1/M2/M3 release gates from this log.

## Long-run quality failure found before delivery
V3 eight20-year co-play runs preserved under ai/models/realm-v3/long-sandboxes.
Accounting passed, but all runs ended with zero living people. This is NOT a
successful self-sustaining economy or completed M3. Investigating missing funded
role incomes, unfair cash-constrained payroll and fiscal reward scale. A new
environment/version and untouched test split are required before a new claim.

## V6 implementation and checks
Twelve typed court primitives and funded family/role mechanics implemented.
All9PPO fits completed:2,625,536 decisions. Local CTest5/5; core70,trade36546,
realm24980,court168,runtime37. Binding258/training476, ONNX489, Godot19/18/336/411.
Native viewport reviewed1440x900/min1100x720; portable same four smokes pass.
Categorical audit retains failed full R1/R2/R3. Eight20year native holdouts are
running; first two alive11516/11525, versus v3 extinction. Full evidence pending.
