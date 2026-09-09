import matplotlib.pyplot as plt

vals = []

with open("plot_vref_2.csv") as f:
    for line in f:
        line = line.strip()
        if not line:
            continue
        vals.append(float(line.split(",")[0].strip()))

time_vals = [i * 30 for i in range(len(vals))]

fig, ax = plt.subplots(figsize=(10, 6))
ax.plot(time_vals, vals, color="#01d7ff", marker="o", markersize=3, linewidth=1)
ax.set_xlabel("Time (s)[15min]")
ax.set_ylabel("Vref")
ax.set_title("Vref vs Time")
ax.grid(True)
ax.ticklabel_format(axis="y", useOffset=False, style="plain")
plt.savefig("plot.png", dpi=150)
plt.show()
