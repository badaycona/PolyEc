# M5.0/1 — guardian administration of dependent land-use permission: evidence

Plan: [guardian administration plan](../design/M5-core-guardian-land-use-administration-plan.md). SHA-256 checkpoint: [manifest](../../artifacts/m5-core-guardian-land-use-administration-2026-10-21-manifest.json). The shared checkout includes prior M5/core work; no commit or push. Self-review only.

## Contract result

- A living adult who is the current derived guardian of a living minor title-holder may grant, replace, and revoke the existing unpriced whole-holding use permission.
- The dependent retains title. `LandUseRight::grantor` stays the title-holder to preserve the existing ownership invariant; the command fact records the guardian as actor. The guardian cannot transfer the title or grant permission to themselves.
- A non-guardian adult's grant/revoke rejects before mutation. No save schema, command signature, money, goods, output, policy observation or model contract changed.

## Verification

| Gate | Result |
|---|---|
| Windows full Release build, including GDExtension | pass |
| Windows full Release CTest | **19/19**, 71.88 s |
| Windows full Debug build, including GDExtension | pass |
| Windows full Debug CTest | **19/19**, 294.41 s |
| Direct land-title tests | **88 checks** each in Release and Debug |
| Python C++ binding replay | **273 checks** each in Release and Debug, 64-day replay |
| Learned training preflight | **476 checks** each configuration |
| PyTorch/ONNX/C++ v7 parity | **507 checks**; pilot quality remains R1=false |
| Godot 4.5.1 Realm/UI/save/land smoke | **477/477** with a fresh 64-day native fixture |
| Repository local links/resources | **910 passed** |

The Python binding test transfers a household title to a minor, grants/revokes use as the current guardian, verifies the dependent remains the recorded grantor/title-holder, rejects an unrelated adult atomically, and resumes revocation after save/load. The Godot smoke confirms the rebuilt extension and existing land-use/title command path; guardian-specific authority is directly tested through C++ and Python.

No new settlement, farming, tenancy, rent, yield, title deed, or inheritance behavior is claimed. Guardian authority is derived from current household composition; it is not serialized as a second guardianship record.
