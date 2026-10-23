# Bundle runtime `realm-v11`

This is the expected location for the three policy files used by the Godot runtime. The project checkout currently has **no compatible `realm-v11` model bundle** here, so the UI and long-running Realm smoke cannot start inference. The directory only documents the expected location; it does not contain a model and does not mean training or model validation has completed.

A complete bundle has one subdirectory per policy family:

- `governance/governance.onnx` and `governance/manifest.json`
- `economic/economic.onnx` and `economic/manifest.json`
- `civilian/civilian.onnx` and `civilian/manifest.json`

Each manifest must identify schema `realm-v11`, declare the exact model contract expected by the running core, and give the SHA-256 digest of its ONNX file. The runtime checks these values before loading the world. A prior `realm-v6` bundle does not satisfy this requirement.
