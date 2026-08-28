import matplotlib.pyplot as plt

vref_vals = []
bits_vals = []

with open("plot_vref_1.csv") as f:
    for line in f:
        line = line.strip()
        if not line:
            continue
        parts = line.split(",")
        vref_vals.append(float(parts[0].strip()))
        bits_vals.append(float(parts[1].strip()))

time_vals = [i * 1 for i in range(len(vref_vals))]

buffer_frac = 0.05

vref_min, vref_max = min(vref_vals), max(vref_vals)
vref_buf = (vref_max - vref_min) 
plt.figure(figsize=(10, 6))
plt.scatter(time_vals, vref_vals, color="#01d7ff", s=9)
plt.xlabel("Time (s)")
plt.ylabel("Voltage ref(5v)")
plt.ylim(vref_min - vref_buf, vref_max + vref_buf)
plt.title("Column 1 vs Time")
plt.grid(True)
plt.savefig("plot_col1.png", dpi=150)
plt.show()

bits_min, bits_max = min(bits_vals), max(bits_vals)
bits_buf = (bits_max - bits_min) * buffer_frac
plt.figure(figsize=(10, 6))
plt.scatter(time_vals, bits_vals, color="#ff5733", s=9)
plt.xlabel("Time (s)")
plt.ylabel("adc bits")
plt.ylim(bits_min - bits_buf, bits_max + bits_buf)
plt.title("Column 2 vs Time")
plt.grid(True)
plt.savefig("plot_col2.png", dpi=150)
plt.show()
