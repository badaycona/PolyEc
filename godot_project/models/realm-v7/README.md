# Realm v7 model bundle

This directory is the destination for the three compatible v7 family bundles:
`governance/`, `economic/`, and `civilian/`.

No v7 policy has been trained or evaluated yet. Do not add placeholder weights
or manifests. The Godot preflight requires a real manifest and ONNX model for
each family and will keep the Realm unloaded until a compatible bundle exists.

Train and evaluate through `ai/training/realm_train_v7.py`, then export and
record the bundle with `ai/training/realm_export_v7.py` after the learned-policy
quality gates are ready.
