"""Check local Markdown links and static Godot resource paths after moves."""
from pathlib import Path
import re
from urllib.parse import unquote

ROOT = Path(__file__).resolve().parent.parent
errors = []
checked = 0
documents = [ROOT / n for n in ("README.md", "WORK_STATUS.md", "AGENTS.md")]
documents += sorted((ROOT / "docs").rglob("*.md"))
for document in documents:
    text = document.read_text(encoding="utf-8")
    for match in re.finditer(r"!?\[[^\]\n]*\]\((<[^>\n]+>|[^\s)]+)\)", text):
        value = match[1].strip("<>")
        if value.startswith("#") or re.match(r"^[a-zA-Z][a-zA-Z0-9+.-]*:", value):
            continue
        target = (document.parent / unquote(value.split("#")[0])).resolve()
        checked += 1
        if not target.exists():
            errors.append(f"{document.relative_to(ROOT)}: missing {value}")

project = ROOT / "godot_project"
for source in sorted(project.rglob("*")):
    if source.suffix not in (".gd", ".tscn") or ".godot" in source.parts:
        continue
    for match in re.finditer(r'["\']res://([^"\']+)["\']', source.read_text(encoding="utf-8")):
        value = match[1]
        # Directory prefixes used to assemble model paths are checked as dirs.
        checked += 1
        if not (project / value).exists():
            errors.append(f"{source.relative_to(ROOT)}: missing res://{value}")

if errors:
    print("\n".join(errors))
    raise SystemExit(1)
print(f"Repository paths: {checked} local document/resource references passed")
