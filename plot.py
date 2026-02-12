import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("results.csv")
N_values = sorted(df["N"].unique())
P_values = sorted(df["P"].unique())

# =====================================================
# Helper: Ideal Time Curve (based on smallest N)
# =====================================================

def ideal_time(column):
    smallest_N = min(N_values)
    subset = df[(df["N"] == smallest_N) & (df["P"] == 1)]
    T1 = subset[column].values[0]
    return [T1 / p for p in P_values]

# =====================================================
# (a) Overall Time
# =====================================================

plt.figure()
for N in N_values:
    subset = df[df["N"] == N]
    plt.plot(subset["P"], subset["Tp_overall"], marker='o', label=f"N={N}")

plt.plot(P_values, ideal_time("Tp_overall"), linestyle='--', label="Ideal")
plt.xlabel("Threads (P)")
plt.ylabel("Overall Time (seconds)")
plt.legend()
plt.grid(True)
plt.savefig("overall_time.png")
plt.close()

# =====================================================
# (b) Work Time
# =====================================================

plt.figure()
for N in N_values:
    subset = df[df["N"] == N]
    plt.plot(subset["P"], subset["Tp_work"], marker='o', label=f"N={N}")

plt.plot(P_values, ideal_time("Tp_work"), linestyle='--', label="Ideal")
plt.xlabel("Threads (P)")
plt.ylabel("Work Time (seconds)")
plt.legend()
plt.grid(True)
plt.savefig("work_time.png")
plt.close()

# =====================================================
# (c) Overall Speedup
# =====================================================

plt.figure()
for N in N_values:
    subset = df[df["N"] == N]
    plt.plot(subset["P"], subset["Speedup_overall"], marker='o', label=f"N={N}")

plt.plot(P_values, P_values, linestyle='--', label="Ideal")
plt.xlabel("Threads (P)")
plt.ylabel("Overall Speedup")
plt.legend()
plt.grid(True)
plt.savefig("overall_speedup.png")
plt.close()

# =====================================================
# (d) Work Speedup
# =====================================================

plt.figure()
for N in N_values:
    subset = df[df["N"] == N]
    plt.plot(subset["P"], subset["Speedup_work"], marker='o', label=f"N={N}")

plt.plot(P_values, P_values, linestyle='--', label="Ideal")
plt.xlabel("Threads (P)")
plt.ylabel("Work Speedup")
plt.legend()
plt.grid(True)
plt.savefig("work_speedup.png")
plt.close()

# =====================================================
# (e) Overall Efficiency
# =====================================================

plt.figure()
for N in N_values:
    subset = df[df["N"] == N]
    plt.plot(subset["P"], subset["Efficiency_overall"], marker='o', label=f"N={N}")

plt.axhline(1.0, linestyle='--', label="Ideal")
plt.xlabel("Threads (P)")
plt.ylabel("Overall Efficiency")
plt.legend()
plt.grid(True)
plt.savefig("overall_efficiency.png")
plt.close()

# =====================================================
# (f) Work Efficiency
# =====================================================

plt.figure()
for N in N_values:
    subset = df[df["N"] == N]
    plt.plot(subset["P"], subset["Efficiency_work"], marker='o', label=f"N={N}")

plt.axhline(1.0, linestyle='--', label="Ideal")
plt.xlabel("Threads (P)")
plt.ylabel("Work Efficiency")
plt.legend()
plt.grid(True)
plt.savefig("work_efficiency.png")
plt.close()

print("All 6 plots generated successfully.")
