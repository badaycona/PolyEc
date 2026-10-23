# M5 learned policy — versioned model bundle contract loader

Date: 2026-10-21. Current baseline is the dirty Windows worktree; preserve all M5 work. User delegated the full M5/model path. This slice makes future trained models with current C++ semantics loadable while keeping legacy pilot files rejected by default.

## Contract

- Each of the three model family directories includes `manifest.json` with `family`, `schema`, `model_contract`, `sha256`, `observation`, `actions`, `hidden`, `trained_with_rl`, and `imitation` fields. Godot verifies the ONNX bytes against the manifest SHA-256 before calling runtime; the native manifest reader validates the digest format and metadata, while native benchmark runs stay explicitly labeled development.
- Product runtime accepts only family order governance/economic/civilian, schema `realm-v6` tensor layout, exact C++ `Realm::model_contract()`, matching model tensor dimensions, `trained_with_rl=true`, and `imitation=false`.
- Development override can admit only historical manifests that have schema `realm-v6` and no `model_contract`; it cannot override an explicit mismatching contract. Existing pilot manifests and weights are not changed.
- Python PPO trainer obtains `model_contract` from the shared C++ `Realm`; exporter carries the frozen manifest contract. Training remains gated on feature freeze and quality protocol; this slice does not train or claim quality.
- Save/restore remains bound to the loaded model bundle and rejects a Realm whose contract differs from the model contract.

## Files and gates

- `native/realm_runtime.cpp`, runtime header/contracts, Python RealmBatch/training/export, Godot current bundle preflight/smoke, and current task/report/history.
- Native Release runtime contracts prove default legacy rejection, explicit legacy dev admission, current manifest metadata validation, contract mismatch rejection, and restore behavior.
- Build Python Release/Debug bindings and check they expose the C++ model contract; run relevant model-contract/training manifest checks without training.
- Build GDExtension Release and run Godot headless model guard against the untouched historical bundle.
- Run relevant CTest and scoped diff/link/hash checks. No long training, no model manifest rewrite, no publish.

## Review

Codex plan review before implementation. No independent reviewer is available in this execution turn; acceptance must say self-review only and separate loader-format checks from learned-policy quality.
