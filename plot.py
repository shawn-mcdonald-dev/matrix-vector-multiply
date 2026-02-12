import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv("results.csv")

N_values = sorted(df["N"].unique())

# ==========================
# Time Plot
# ==========================
plt.figure()
for N in N_values:
    subset = df[df["N"] == N]
    plt.plot(subset["P"], subset["Tp"], marker='o', label=f"N={N}")

plt.xlabel("Threads (P)")
plt.ylabel("Overall Time (seconds)")
plt.legend()
plt.grid(True)
plt.savefig("time_plot.png")
plt.close()

# ==========================
# Speedup Plot
# ==========================
plt.figure()
for N in N_values:
    subset = df[df["N"] == N]
    plt.plot(subset["P"], subset["Speedup"], marker='o', label=f"N={N}")

# Ideal speedup
P_values = sorted(df["P"].unique())
plt.plot(P_values, P_values, linestyle='--', label="Ideal")

plt.xlabel("Threads (P)")
plt.ylabel("Speedup")
plt.legend()
plt.grid(True)
plt.savefig("speedup_plot.png")
plt.close()

# ==========================
# Efficiency Plot
# ==========================
plt.figure()
for N in N_values:
    subset = df[df["N"] == N]
    plt.plot(subset["P"], subset["Efficiency"], marker='o', label=f"N={N}")

plt.axhline(1.0, linestyle='--', label="Ideal")

plt.xlabel("Threads (P)")
plt.ylabel("Efficiency")
plt.legend()
plt.grid(True)
plt.savefig("efficiency_plot.png")
plt.close()

print("Plots generated.")
