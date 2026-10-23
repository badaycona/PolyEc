# M5 v22 — sampled policy action audit in controlled worksite contexts

## Objective and baseline

Use the v21 action-intent diagnostics to inspect why full-world v20 policies rarely complete workshop production. This is an inference audit of already-trained v20 Economic seeds that failed model selection and are explicitly not peer-eligible. Do not feed them into Realm co-play or change selection status.

## Before → after

Before: aggregate full-world counts show few input transfers and little output. The controlled shared-core test establishes that Fiber inventory gates the transfer candidate.

After: a reproducible diagnostic runs each v20 seed through three matched states built in the same C++ Realm setup: ordinary starting state, private Cloth workshop with no Fiber, and the same workshop after Fiber purchase. It records legal-candidate counts, exact masked categorical probability mass by Intent, and a fixed-seed 256-draw sample matching the runtime categorical rule. The script verifies invariants and asserts transfer availability changes only after the input purchase.

## Files and safeguards

- New `ai/evaluation/realm_worksite_context_audit_v11.py`: read-only policy inference over the shared C++ Realm. It creates contexts only through approved typed commands and does not alter reward, trainer, or model files.
- Add evidence JSON under `artifacts/` and report results in docs.
- Lock schema, model contract, reward version, role, source checkpoint manifest and policy SHA-256. Refuse incomplete or non-Economic v20 artifacts.
- Output explicitly marks every seed ineligible; this diagnostic cannot select/promote a model.

## Verification and limits

Run against all three v20 seeds, check contexts and probability sums, and run Python syntax plus related shared-core workshop checks. This tests action preference in three snapshots only. It does not estimate return, causal effect, generalization, partner diversity, production rate, or full-world transfer. No long training run is included.
