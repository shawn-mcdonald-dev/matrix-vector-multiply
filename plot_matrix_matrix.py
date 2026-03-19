import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import glob

# =====================================================
# Global Plot Style
# =====================================================

plt.style.use("seaborn-v0_8-whitegrid")

plt.rcParams.update({
    "figure.figsize": (8, 6),
    "axes.titlesize": 14,
    "axes.labelsize": 12,
    "legend.fontsize": 10,
    "xtick.labelsize": 10,
    "ytick.labelsize": 10,
    "lines.linewidth": 2,
    "lines.markersize": 6,
})

# =====================================================
# Load Data
# =====================================================

files = glob.glob("omp_results_p*.csv")

df = pd.concat([pd.read_csv(f) for f in files])
df = df.sort_values(["N", "P"])

N_values = sorted(df["N"].unique())
P_values = sorted(df["P"].unique())

# =====================================================
# Compute Derived Metrics
# =====================================================

# IO time
df["Tp_io"] = df["Tp_overall"] - df["Tp_work"]

# Get T1 baseline per N
t1_overall = df[df["P"] == 1].set_index("N")["Tp_overall"]
t1_work = df[df["P"] == 1].set_index("N")["Tp_work"]
t1_io = df[df["P"] == 1].set_index("N")["Tp_io"]

# Compute speedup
df["Speedup_overall"] = df.apply(
    lambda r: t1_overall[r["N"]] / r["Tp_overall"] if r["N"] in t1_overall else np.nan,
    axis=1
)

df["Speedup_work"] = df.apply(
    lambda r: t1_work[r["N"]] / r["Tp_work"] if r["N"] in t1_work else np.nan,
    axis=1
)

df["Speedup_io"] = df.apply(
    lambda r: t1_io[r["N"]] / r["Tp_io"] if r["N"] in t1_io else np.nan,
    axis=1
)

# Compute efficiency
df["Efficiency_overall"] = df["Speedup_overall"] / df["P"]
df["Efficiency_work"] = df["Speedup_work"] / df["P"]
df["Efficiency_io"] = df["Speedup_io"] / df["P"]

# =====================================================
# Ideal Curves
# =====================================================

P_array = np.array(P_values)

def ideal_speedup():
    return P_array

def ideal_efficiency():
    return np.ones_like(P_array)

# =====================================================
# Plot Function
# =====================================================

def make_plot(column, ylabel, title, filename,
              ideal_type=None, ylim=None):

    plt.figure()

    for N in N_values:
        subset = df[df["N"] == N]
        plt.plot(subset["P"], subset[column], marker='o', label=f"N={N}")

    # Ideal overlays (ONLY for speedup/efficiency)
    if ideal_type == "speedup":
        plt.plot(P_array, ideal_speedup(),
                 linestyle="--", label="Ideal Linear")

    if ideal_type == "efficiency":
        plt.plot(P_array, ideal_efficiency(),
                 linestyle="--", label="Ideal")

    plt.xscale("log", base=2)
    plt.xlabel("Threads (P)")
    plt.ylabel(ylabel)
    plt.title(title)

    if ylim is not None:
        plt.ylim(ylim)

    plt.legend()
    plt.tight_layout()
    plt.savefig(filename, dpi=300)
    plt.close()

# =====================================================
# Generate 9 Plots
# =====================================================

# ---------- TIME ----------

make_plot(
    "Tp_overall",
    "Overall Time (seconds)",
    "Overall Time vs Threads",
    "overall_time.png"
)

make_plot(
    "Tp_work",
    "Compute Time (seconds)",
    "Compute Time vs Threads",
    "compute_time.png"
)

make_plot(
    "Tp_io",
    "IO Time (seconds)",
    "IO Time vs Threads",
    "io_time.png"
)

# ---------- SPEEDUP ----------

make_plot(
    "Speedup_overall",
    "Overall Speedup",
    "Overall Speedup vs Threads",
    "overall_speedup.png",
    ideal_type="speedup",
    ylim=(0, df["Speedup_overall"].max() * 1.2)
)

make_plot(
    "Speedup_work",
    "Compute Speedup",
    "Compute Speedup vs Threads",
    "compute_speedup.png",
    ideal_type="speedup",
    ylim=(0, df["Speedup_work"].max() * 1.2)
)

make_plot(
    "Speedup_io",
    "IO Speedup",
    "IO Speedup vs Threads",
    "io_speedup.png",
    ideal_type="speedup",
    ylim=(0, df["Speedup_io"].max() * 1.2)
)

# ---------- EFFICIENCY ----------

make_plot(
    "Efficiency_overall",
    "Overall Efficiency",
    "Overall Efficiency vs Threads",
    "overall_efficiency.png",
    ideal_type="efficiency",
    ylim=(0, 1.1)
)

make_plot(
    "Efficiency_work",
    "Compute Efficiency",
    "Compute Efficiency vs Threads",
    "compute_efficiency.png",
    ideal_type="efficiency",
    ylim=(0, 1.1)
)

make_plot(
    "Efficiency_io",
    "IO Efficiency",
    "IO Efficiency vs Threads",
    "io_efficiency.png",
    ideal_type="efficiency",
    ylim=(0, 1.1)
)

print("All 9 plots generated successfully.")