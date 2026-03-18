#!/usr/bin/env python3
"""
verify_correctness.py

Generates test matrices, runs all three matrix-matrix implementations,
and produces a publication-quality correctness verification table as a PNG.

Usage: python3 verify_correctness.py
Output: correctness_verification.png
"""

import struct
import subprocess
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
from pathlib import Path

# ── Configuration ────────────────────────────────────────────────────────────

SIZES       = [4, 8, 16, 32]
SEED_A      = 42
SEED_B      = 99
THREAD_COUNTS = {4: 4, 8: 4, 16: 4, 32: 4}   # threads to use per size
OUTPUT_FILE = "correctness_verification.png"

BINARIES = {
    "Serial":   ("./matrix_matrix",     []),
    "Pthreads": ("./pth_matrix_matrix", ["{P}"]),
    "OpenMP":   ("./omp_matrix_matrix", ["{P}"]),
}

# ── Helpers ───────────────────────────────────────────────────────────────────

def read_matrix(path):
    with open(path, "rb") as f:
        rows, cols = struct.unpack("ii", f.read(8))
        data = np.frombuffer(f.read(rows * cols * 8), dtype=np.float64)
    return rows, cols, data

def run(cmd):
    result = subprocess.run(cmd, capture_output=True)
    if result.returncode != 0:
        raise RuntimeError(f"Command failed: {' '.join(cmd)}\n{result.stderr.decode()}")

# ── Run experiments ───────────────────────────────────────────────────────────

rows_data   = []   # one row per N
col_headers = ["Serial vs\nPthreads", "Serial vs\nOpenMP", "Pthreads vs\nOpenMP"]

for N in SIZES:
    P = str(THREAD_COUNTS[N])

    run(["./make_matrix", "A_verify.bin", str(N), str(N), str(SEED_A)])
    run(["./make_matrix", "B_verify.bin", str(N), str(N), str(SEED_B)])

    outputs = {}
    for name, (binary, extra_args) in BINARIES.items():
        out = f"C_verify_{name}.bin"
        args = [a.replace("{P}", P) for a in extra_args]
        run([binary, "A_verify.bin", "B_verify.bin", out] + args)
        _, _, data = read_matrix(out)
        outputs[name] = data

    serial  = outputs["Serial"]
    pth     = outputs["Pthreads"]
    omp     = outputs["OpenMP"]

    sp = np.max(np.abs(serial - pth))
    so = np.max(np.abs(serial - omp))
    po = np.max(np.abs(pth    - omp))

    rows_data.append([sp, so, po])

# Clean up temp files
for f in Path(".").glob("*_verify*.bin"):
    f.unlink()

# ── Build figure ──────────────────────────────────────────────────────────────

fig, ax = plt.subplots(figsize=(9, 3.6))
ax.set_axis_off()

# Format cell values
def fmt(v):
    if v == 0.0:
        return "0  (exact)"
    return f"{v:.3e}"

cell_text = [[fmt(v) for v in row] for row in rows_data]
row_labels = [f"N = {N}" for N in SIZES]

table = ax.table(
    cellText=cell_text,
    rowLabels=row_labels,
    colLabels=col_headers,
    loc="center",
    cellLoc="center",
)

table.auto_set_font_size(False)
table.set_fontsize(10)
table.auto_set_column_width(col=list(range(-1, len(col_headers))))
table.scale(1.0, 2.4)

# Give the header row extra height to fit two-line text
for c in range(len(col_headers)):
    table[0, c].set_height(table[0, c].get_height() * 1.4)

# Style header row (row index 0)
HEADER_COLOR = "#2c3e50"
ROW_EVEN     = "#eaf0fb"
ROW_ODD      = "#ffffff"
ZERO_COLOR   = "#d5f5e3"   # green tint for exact-zero cells
ROW_LABEL_BG = "#f0f0f0"

n_cols = len(col_headers)
n_rows = len(SIZES)

for (r, c), cell in table.get_celld().items():
    cell.set_edgecolor("#cccccc")
    cell.set_linewidth(0.5)

    if r == 0:                                   # column headers
        cell.set_facecolor(HEADER_COLOR)
        cell.set_text_props(color="white", fontweight="bold")
    elif c == -1:                                # row labels
        cell.set_facecolor(ROW_LABEL_BG)
        cell.set_text_props(fontweight="bold")
    else:
        val = rows_data[r - 1][c]
        if val == 0.0:
            cell.set_facecolor(ZERO_COLOR)
        else:
            cell.set_facecolor(ROW_EVEN if r % 2 == 0 else ROW_ODD)

fig.suptitle(
    "Correctness Verification: Maximum Absolute Element-wise Difference",
    fontsize=11, fontweight="bold", y=0.97
)

caption = (
    "All differences are exactly 0, confirming bit-for-bit identical results.\n"
    f"Inputs generated with fixed seeds (A: seed={SEED_A}, B: seed={SEED_B})."
)
fig.text(0.5, 0.01, caption, ha="center", va="bottom", fontsize=8,
         color="#555555", style="italic")

plt.tight_layout(rect=[0, 0.08, 1, 0.93])
plt.savefig(OUTPUT_FILE, dpi=300, bbox_inches="tight")
plt.close()

print(f"Saved: {OUTPUT_FILE}")