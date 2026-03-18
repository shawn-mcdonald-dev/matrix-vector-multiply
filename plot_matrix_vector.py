import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# =====================================================
# Global Plot Style (Clean + Publication Quality)
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
# Load and Prepare Data
# =====================================================

df = pd.read_csv("results.csv")
df = df.sort_values(["N", "P"])

N_values = sorted(df["N"].unique())
P_values = sorted(df["P"].unique())

# Ensure P is numeric and sorted
P_values = np.array(P_values)


# =====================================================
# Helper Functions
# =====================================================

def ideal_time_for_N(N, column):
    """
    Ideal time assuming perfect scaling:
        T_p = T_1 / p
    """
    T1 = df[(df["N"] == N) & (df["P"] == 1)][column].values[0]
    return T1 / P_values


def ideal_speedup():
    """Ideal linear speedup"""
    return P_values


def ideal_efficiency():
    """Ideal efficiency = 1"""
    return np.ones_like(P_values)


def make_plot(column, ylabel, title, filename,
              ideal_type=None, ylim=None):
    """
    Generic plot generator.
    ideal_type: None | "time" | "speedup" | "efficiency"
    """

    plt.figure()

    for N in N_values:
        subset = df[df["N"] == N]
        plt.plot(subset["P"], subset[column], marker='o', label=f"N={N}")

        # Overlay ideal curve (per N for time)
        if ideal_type == "time":
            plt.plot(P_values,
                     ideal_time_for_N(N, column),
                     linestyle="--",
                     alpha=0.4)

    # Ideal overlays for speedup/efficiency
    if ideal_type == "speedup":
        plt.plot(P_values,
                 ideal_speedup(),
                 linestyle="--",
                 label="Ideal Linear")

    if ideal_type == "efficiency":
        plt.plot(P_values,
                 ideal_efficiency(),
                 linestyle="--",
                 label="Ideal")

    # Log scale for thread count (best practice)
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
# Generate 6 Required Plots
# =====================================================

# 1. Overall Time
make_plot(
    column="Tp_overall",
    ylabel="Overall Time (seconds)",
    title="Total Program Time vs Threads",
    filename="overall_time.png",
    ideal_type="time"
)

# 2. Overall Speedup
make_plot(
    column="Speedup_overall",
    ylabel="Overall Speedup",
    title="Overall Speedup vs Threads",
    filename="overall_speedup.png",
    ideal_type="speedup",
    ylim=(0, df["Speedup_overall"].max() * 1.2)
)

# 3. Overall Efficiency
make_plot(
    column="Efficiency_overall",
    ylabel="Overall Efficiency",
    title="Overall Parallel Efficiency vs Threads",
    filename="overall_efficiency.png",
    ideal_type="efficiency",
    ylim=(0, 1.1)
)

# 4. Compute Time
make_plot(
    column="Tp_work",
    ylabel="Compute Time (seconds)",
    title="Compute Time Scaling vs Threads",
    filename="compute_time.png",
    ideal_type="time"
)

# 5. Compute Speedup
make_plot(
    column="Speedup_work",
    ylabel="Compute Speedup",
    title="Compute Speedup vs Threads",
    filename="compute_speedup.png",
    ideal_type="speedup",
    ylim=(0, df["Speedup_work"].max() * 1.2)
)

# 6. Compute Efficiency
make_plot(
    column="Efficiency_work",
    ylabel="Compute Efficiency",
    title="Compute Parallel Efficiency vs Threads",
    filename="compute_efficiency.png",
    ideal_type="efficiency",
    ylim=(0, 1.1)
)

print("All 6 high-quality scaling plots generated successfully.")
