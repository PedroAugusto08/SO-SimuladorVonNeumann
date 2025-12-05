#!/usr/bin/env python3
"""Comparador rápido para CSVs gerados pelo teste unificado.

Uso:
    scripts/compare_metrics.py --left dados_graficos/csv/unified_complete.csv \
                                --right backup/unified_complete_prev.csv

O script cruza as linhas por (policy,num_cores) e mostra as diferenças das
principais métricas, facilitando identificar regressões após mudanças no
escalonador.
"""
from __future__ import annotations

import argparse
import csv
from collections import OrderedDict
from pathlib import Path
from typing import Dict, List

DEFAULT_METRICS = [
    "execution_time_ms",
    "avg_wait_time",
    "avg_turnaround_time",
    "cpu_utilization_pct",
    "throughput",
    "hit_rate_pct",
]


def load_csv(path: Path, key_fields: List[str]) -> Dict[str, Dict[str, float]]:
    data: Dict[str, Dict[str, float]] = OrderedDict()
    with path.open(newline="", encoding="utf-8") as handle:
        reader = csv.DictReader(handle)
        missing = [field for field in key_fields if field not in reader.fieldnames]
        if missing:
            raise SystemExit(f"CSV {path} não possui colunas obrigatórias: {missing}")
        for row in reader:
            key = "|".join(row[field] for field in key_fields)
            data[key] = row
    return data


def format_diff(new: float, old: float) -> str:
    diff = new - old
    sign = "+" if diff >= 0 else ""
    return f"{new:.3f} ({sign}{diff:.3f})"


def main() -> None:
    parser = argparse.ArgumentParser(description="Compara métricas por política/núcleo")
    parser.add_argument("--left", default="dados_graficos/csv/unified_complete.csv",
                        help="CSV mais recente (padrão: dados_graficos/csv/unified_complete.csv)")
    parser.add_argument("--right", required=True,
                        help="CSV base para comparação (ex: arquivo anterior)")
    parser.add_argument("--metrics", nargs="*", default=DEFAULT_METRICS,
                        help="Colunas numéricas para comparar")
    args = parser.parse_args()

    left_path = Path(args.left)
    right_path = Path(args.right)
    if not left_path.exists():
        raise SystemExit(f"Arquivo {left_path} não encontrado")
    if not right_path.exists():
        raise SystemExit(f"Arquivo {right_path} não encontrado")

    key_fields = ["policy", "num_cores"]
    left = load_csv(left_path, key_fields)
    right = load_csv(right_path, key_fields)

    all_keys = sorted(set(left) | set(right))
    if not all_keys:
        print("Nenhum dado encontrado para comparar.")
        return

    header = ["policy", "cores"] + args.metrics
    print(" | ".join(header))
    print("-" * (len(header) * 15))

    for key in all_keys:
        policy, cores = key.split("|")
        left_row = left.get(key)
        right_row = right.get(key)
        row_display = [policy, cores]
        for metric in args.metrics:
            if not left_row or metric not in left_row:
                row_display.append("-")
                continue
            if not right_row or metric not in right_row:
                row_display.append(f"{float(left_row[metric]):.3f} (+∞)")
                continue
            try:
                new_value = float(left_row[metric])
                old_value = float(right_row[metric])
            except ValueError:
                row_display.append("n/a")
                continue
            row_display.append(format_diff(new_value, old_value))
        print(" | ".join(row_display))

    print("\nLegenda: valor_atual (diferença vs. base). Diferenças positivas indicam regressões para métricas de tempo; negativas indicam melhora.")


if __name__ == "__main__":
    main()
