# Core/M5.5 — secret knowledge observation index: evidence (2026-10-21)

Plan: [secret-knowledge index](../design/M5-core-observation-secret-knowledge-plan.md). SHA-256 checkpoint: [manifest](../../artifacts/m5-core-observation-secret-knowledge-2026-10-21-manifest.json). This is a continuation of the 2026-10-21 decision-snapshot optimization. The shared checkout contains other dirty M5/core work; no commit or push was made. Self-review only.

## Change and oracle

- Replaced the per-observation scan of up to 8,192 facts and secret-knowledge lookups with a derived per-person active secret-fact count.
- Rebuild counts on load and at day begin, increment them for secret actor/target visibility and newly completed investigation knowledge, resize for births, and rebuild after pruning. The derived data is not serialized and does not change schema or interfaces.
- Added a deliberately naive oracle in the decision snapshot test using `facts_` plus the existing `knows()` predicate. It checks direct visibility, investigation completion, save/load, and pruning beyond the 8,192-fact cap. Feature 87 matches exactly.
- Decision snapshot suite: **463 checks** in both Release and Debug.

## Build and regression gates

| Gate | Result |
|---|---|
| Windows full Release build, including Godot extension | pass |
| Windows Release full CTest | **19/19**, 66.67 s |
| Windows full Debug build, including Godot extension | pass |
| Windows Debug full CTest | **19/19**, 266.68 s |
| Python binding replay | **269 checks**, 64-day C++ replay |
| Learned training preflight | **476 checks** |
| PyTorch/ONNX/C++ v7 parity | **507 checks**, all families and batch sizes |
| Godot 4.5.1 headless Realm/UI/save/land-use smoke | **477/477**; generated schema22 fixture in `artifacts/m5-t5.1-secret-index-realm-expected-2026-10-21.json` |
| Repository local links/resources | **902** passed |

## M5.5 profile B diagnosis

Same seed 37, pilot v7 bundle (all three manifests `full_R1_pass=false`), 100,000 people, 31 days, and 128/512 complex actors. These are population-scale-only timing samples; the benchmark does not validate policy quality. All six runs reported 100,000 population, full inference counts, route shock, `money_conserved=true`, and `invariant=true`.

| Complex actors | Three p95 samples (ms) | Median p95 |
|---:|---:|---:|
| 128 | 322.495, 328.074, 312.465 | 322.495 |
| 512 | 340.889, 338.469, 317.472 | 338.469 |

One additional 128-actor run measured 192.288 ms; it is kept separate and not substituted into the three-run set. A later instrumented follow-up measured p95 347.423 ms with observation packing 1,217,825,700 ns over 31 days. Host process inspection showed substantial unrelated CPU activity during these runs. The results do not isolate implementation impact, and the six-run median does not meet the ≤80 ms gate.

Two instrumented diagnostic runs for 128 actors reported p95 180.312 and 347.423 ms, with `phase_times_consistent=true`. Observation packing summed to 845,448,600 and 1,217,825,700 ns over 31 days, versus 1,255,235,000 ns in the prior instrumented snapshot profile. The variation and timer instrumentation do not establish a stable speedup; they support further clean profiling only.

Raw outputs: `artifacts/m5-t5.1-secret-index-clean-100k-{128,512}-run{1,2,3}-2026-10-21.json`, `artifacts/m5-t5.1-secret-index-100k-128-followup-2026-10-21.json`, `artifacts/m5-t5.1-secret-index-phase-100k-128-2026-10-21.json`, and `artifacts/m5-t5.1-secret-index-phase-followup-100k-128-2026-10-21.json`. Earlier `secret-index-100k-*-run*` samples are retained for audit; the clean set above is the declared three-run set.

## Remaining gates

Feature semantics and cross-binding integration pass. M5.5 profile B remains open at ≤80 ms; current latency samples are confounded by host load. Learned-policy R1–R5/R2–R4, offline package, full-cycle profile, and remaining M5 gameplay/core refactor work remain open. No commit or push.
