#!/usr/bin/env python3
"""Create plots from the saved M5 v30 validation and v31 partial-training runs."""

from __future__ import annotations

import hashlib
import json
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np


ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "artifacts"
V30 = ROOT / "build/m5-v30-governance-civilian-training"
FAMILIES = {
    "Governance": {
        "folder": "governance",
        "history_folder": "governance",
        "validation": "governance/validation/selection.json",
    },
    "Civilian": {
        "folder": "civilian-fixed",
        "history_folder": "civilian",
        "validation": "civilian-fixed/validation/selection.json",
    },
}
V31_HISTORY = ROOT / "build/m5-v31-extended-training/civilian/seed-1/history.jsonl"


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def load_history(path: Path) -> list[dict]:
    return [json.loads(line) for line in path.read_text(encoding="utf-8").splitlines() if line.strip()]


def save(fig: plt.Figure, name: str) -> None:
    fig.savefig(OUT / name, dpi=180, bbox_inches="tight")
    plt.close(fig)


def training_plot(data: dict) -> None:
    fig, axes = plt.subplots(1, 2, figsize=(13, 5.2), sharey=True)
    palette = ["#2878b5", "#e07a24", "#37966f"]
    for ax, family in zip(axes, FAMILIES):
        curves = []
        for seed in (1, 2, 3):
            rows = data["training"][family][str(seed)]
            x = [r["update"] for r in rows]
            y = [r["return_distribution"]["mean"] for r in rows]
            curves.append(y)
            ax.plot(x, y, color=palette[seed - 1], alpha=0.42, linewidth=1.1, label=f"Seed {seed}")
        values = np.asarray(curves, dtype=float)
        mean = values.mean(axis=0)
        sd = values.std(axis=0, ddof=1)
        x = np.arange(1, len(mean) + 1)
        ax.plot(x, mean, color="#202020", linewidth=2.5, label="Trung bình 3 seed")
        ax.fill_between(x, mean - sd, mean + sd, color="#777777", alpha=0.18, label="±1 độ lệch chuẩn giữa seed")
        ax.set_title(f"{family} — 3 lượt huấn luyện")
        ax.set_xlabel("Lần cập nhật policy")
        ax.grid(True, alpha=0.25)
        ax.legend(fontsize=8, loc="best")
    axes[0].set_ylabel("Return trung bình trong rollout huấn luyện")
    fig.suptitle("M5 v30: diễn biến return trong dữ liệu huấn luyện\nReturn trong rollout không phải kết quả đánh giá độc lập", fontsize=13)
    fig.tight_layout()
    save(fig, "m5-v30-training-return-by-update-2026-10-22.png")


def validation_plots(data: dict) -> None:
    # Each point is one independently trained seed. Its interval comes from 20 paired worlds.
    fig, axes = plt.subplots(1, 2, figsize=(12.5, 5.8), sharey=True)
    comparisons = [
        ("learned_vs_baseline", "Policy học được − baseline"),
        ("learned_vs_initial", "Policy học được − policy ban đầu"),
    ]
    for ax, family in zip(axes, FAMILIES):
        selection = data["validation"][family]
        for yi, (key, label) in enumerate(comparisons):
            for seed_i, result in enumerate(selection["results"]):
                item = result["utility"][key]
                y = yi + (seed_i - 1) * 0.12
                mean = item["paired_mean"]
                lo, hi = item["ci95"]
                ax.errorbar(mean, y, xerr=[[mean - lo], [hi - mean]], fmt="o", capsize=3,
                            color=["#2878b5", "#e07a24", "#37966f"][seed_i],
                            label=f"Seed {result['training_seed']}" if yi == 0 else None)
        ax.axvline(0, color="#333333", linewidth=1, linestyle="--")
        ax.set_yticks([0, 1], ["So với baseline", "So với policy ban đầu"])
        ax.set_title(f"{family} — mỗi seed trên 20 world validation")
        ax.set_xlabel("Chênh lệch utility trung bình (điểm; kèm CI 95%)")
        ax.grid(axis="x", alpha=0.25)
        ax.legend(fontsize=8, loc="best")
    fig.suptitle("M5 v30: đánh giá ghép cặp trên world seed độc lập\nMỗi khoảng tin cậy so sánh cùng 20 seed thế giới", fontsize=13)
    fig.tight_layout()
    save(fig, "m5-v30-validation-utility-paired-ci-2026-10-22.png")

    fig, axes = plt.subplots(1, 2, figsize=(14, 6.4), sharey=False)
    metrics = [
        ("debt_delta_vs_baseline", "Nợ", "Mức thay đổi nợ trung bình (đơn vị trong Realm)"),
        ("insolvent_delta_vs_baseline", "Mất khả năng thanh toán", "Số người thay đổi trung bình mỗi thế giới"),
        ("alive_delta_vs_baseline", "Số người còn sống", "Số người thay đổi trung bình mỗi thế giới"),
    ]
    colors = ["#2878b5", "#e07a24", "#37966f"]
    for ax, family in zip(axes, FAMILIES):
        selection = data["validation"][family]
        for metric_i, (key, title, _) in enumerate(metrics):
            for seed_i, result in enumerate(selection["results"]):
                item = result["outcomes"][key]
                y = metric_i + (seed_i - 1) * 0.16
                mean = item["mean"]
                lo, hi = item["ci95"]
                ax.errorbar(mean, y, xerr=[[mean - lo], [hi - mean]], fmt="o", capsize=3,
                            color=colors[seed_i], label=f"Seed {result['training_seed']}" if metric_i == 0 else None)
        ax.axvline(0, color="#333333", linewidth=1, linestyle="--")
        ax.set_yticks(range(3), [m[1] for m in metrics])
        ax.set_title(f"{family} — chênh lệch so với baseline")
        ax.set_xlabel("Trung bình trên 20 world (kèm CI 95%)")
        ax.grid(axis="x", alpha=0.25)
        ax.legend(fontsize=8, loc="best")
    fig.suptitle("M5 v30: các kết quả người chơi liên quan trong validation\nGiá trị âm nghĩa là ít hơn baseline; số sống bằng 0 nghĩa là bằng baseline", fontsize=13)
    fig.tight_layout()
    save(fig, "m5-v30-validation-outcomes-paired-ci-2026-10-22.png")


def partial_plot(data: dict) -> None:
    rows = data["v31_partial"]
    x = [r["update"] for r in rows]
    fig, axes = plt.subplots(2, 1, figsize=(11.5, 7.4), sharex=True)
    axes[0].plot(x, [r["reward_mean"] for r in rows], marker="o", color="#2878b5")
    axes[0].set_ylabel("Reward trung bình mỗi update")
    axes[0].set_title("Reward nhận được trong rollout của update")
    axes[0].grid(True, alpha=0.25)
    axes[1].plot(x, [r["return_distribution"]["mean"] for r in rows], marker="o", color="#e07a24")
    axes[1].set_ylabel("Return trung bình của rollout")
    axes[1].set_xlabel("Lần cập nhật policy (chỉ các update 26–43 còn trong history)")
    axes[1].set_title("Return rollout không phải validation hoặc điểm đánh giá độc lập")
    axes[1].grid(True, alpha=0.25)
    fig.suptitle("M5 v31 Civilian seed 1: phần huấn luyện đã lưu trước khi dừng\nTiếp tục từ checkpoint v30 sau update 25; dừng tại 43/75 update, 44.032 quyết định", fontsize=13)
    fig.tight_layout()
    save(fig, "m5-v31-partial-training-update-26-43-2026-10-22.png")


def main() -> None:
    OUT.mkdir(parents=True, exist_ok=True)
    data: dict = {"training": {}, "validation": {}, "v31_partial": []}
    inputs: list[Path] = []
    for family, config in FAMILIES.items():
        data["training"][family] = {}
        for seed in (1, 2, 3):
            path = V30 / config["folder"] / config["history_folder"] / f"seed-{seed}/history.jsonl"
            rows = load_history(path)
            if len(rows) != 25 or [r["update"] for r in rows] != list(range(1, 26)):
                raise ValueError(f"Unexpected v30 update coverage: {path}")
            if any(r["decisions"] != r["update"] * 1024 for r in rows):
                raise ValueError(f"Unexpected v30 decision counts: {path}")
            data["training"][family][str(seed)] = rows
            inputs.append(path)
        validation_path = V30 / config["validation"]
        selection = json.loads(validation_path.read_text(encoding="utf-8"))
        if len(selection["validation_seeds"]) != 20 or selection.get("selected_seed") is not None:
            raise ValueError(f"Unexpected validation set or selector result: {validation_path}")
        if len(selection["results"]) != 3 or any(r["utility"]["learned_vs_baseline"]["samples"] != 20 for r in selection["results"]):
            raise ValueError(f"Unexpected validation result shape: {validation_path}")
        data["validation"][family] = selection
        inputs.append(validation_path)
    data["v31_partial"] = load_history(V31_HISTORY)
    if [r["update"] for r in data["v31_partial"]] != list(range(26, 44)):
        raise ValueError("Unexpected v31 partial update range")
    if data["v31_partial"][-1]["decisions"] != 44032:
        raise ValueError("Unexpected v31 cumulative decision count")
    inputs.append(V31_HISTORY)
    data_path = OUT / "m5-v30-v31-plot-data-2026-10-22.json"
    data_path.write_text(json.dumps(data, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    training_plot(data)
    validation_plots(data)
    partial_plot(data)
    outputs = [data_path] + [OUT / name for name in (
        "m5-v30-training-return-by-update-2026-10-22.png",
        "m5-v30-validation-utility-paired-ci-2026-10-22.png",
        "m5-v30-validation-outcomes-paired-ci-2026-10-22.png",
        "m5-v31-partial-training-update-26-43-2026-10-22.png",
    )]
    manifest = {
        "record": "M5 v30/v31 RL visual evidence",
        "date": "2026-10-22",
        "script": str(Path(__file__).relative_to(ROOT)).replace("\\", "/"),
        "script_sha256": sha256(Path(__file__)),
        "inputs": [{"path": p.relative_to(ROOT).as_posix(), "sha256": sha256(p)} for p in inputs],
        "outputs": [{"path": p.relative_to(ROOT).as_posix(), "sha256": sha256(p), "bytes": p.stat().st_size} for p in outputs],
        "scope": "plots only; no training, validation rerun, selector, model export, or holdout",
    }
    (OUT / "m5-v30-v31-visual-evidence-manifest-2026-10-22.json").write_text(
        json.dumps(manifest, ensure_ascii=False, indent=2) + "\n", encoding="utf-8"
    )
    print(json.dumps({"images": [p.relative_to(ROOT).as_posix() for p in outputs[1:]], "input_count": len(inputs), "data": data_path.relative_to(ROOT).as_posix()}, ensure_ascii=False))


if __name__ == "__main__":
    main()
