#!/usr/bin/env bash
set -euo pipefail
root="$(cd -- "$(dirname -- "$0")/.." && pwd)"
godot="$(realpath "$1")"
output="$(realpath -m "${2:-$root/dist/PolyEc-Linux}")"
mkdir -p "$output/bin" "$output/models" "$output/licenses"
"$godot" --headless --path "$root/godot_project" --export-pack Linux "$output/PolyEc-runtime.pck"
cp "$godot" "$output/PolyEc-runtime"
cp "$root"/godot_project/bin/*.so* "$output/bin/"
cp "$root"/godot_project/models/{economic.onnx,manifest.json,evaluation.json,native-parity.json} "$output/models/"
if [[ -d "$root/godot_project/models/realm-v6" ]]; then
  cp -R "$root/godot_project/models/realm-v6" "$output/models/"
fi
cp "$root"/licenses/* "$output/licenses/"
if [[ -f "$root/docs/reports/M3-evidence.md" ]]; then
  cp "$root/docs/reports/M3-evidence.md" "$output/BUILD-REPORT.md"
fi
cat > "$output/PolyEc" <<'LAUNCHER'
#!/usr/bin/env sh
cd -- "$(dirname -- "$0")" || exit 1
exec ./PolyEc-runtime --main-pack "$PWD/PolyEc-runtime.pck" "$@"
LAUNCHER
chmod +x "$output/PolyEc" "$output/PolyEc-runtime"
printf '%s\n' 'PolyEc Linux x86_64 / Ubuntu24.04 reference' 'Run ./PolyEc (keep bin/ and models/ beside it).' 'Offline native Godot/C++/ONNX CPU; Python/Node not required.' 'System graphics libraries and a desktop session are required for the GUI.' 'Quốc gia · Sandbox: choose a draft card, then run/step; F1 help, Esc back.' 'Experimental milestone/quality status is in BUILD-REPORT.md.' > "$output/READ-ME.txt"
