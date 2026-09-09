import pandas as pd
import matplotlib.pyplot as plt

# Read the CSV (assumes a single column, no header)
df = pd.read_csv("plot_data_dac2_1.csv", header=None)
values = df[0].values

# Split into even-indexed and odd-indexed values
# Note: index 0 is the 1st line (odd line), index 1 is the 2nd line (even line), etc.
odd_line_data = values[0::2]   # 1st, 3rd, 5th, ... lines
even_line_data = values[1::2]  # 2nd, 4th, 6th, ... lines

# Make sure both arrays are the same length for plotting
min_len = min(len(odd_line_data), len(even_line_data))
odd_line_data = odd_line_data[:min_len]
even_line_data = even_line_data[:min_len]

# Plot odd-line data vs even-line data
plt.figure(figsize=(8, 6))
plt.scatter(even_line_data, odd_line_data, color='orange', alpha=0.7)

# Label each point with its y-axis (odd line) value
#for x, y in zip(even_line_data, odd_line_data):
#    plt.annotate(str(y), (x, y), textcoords="offset points", xytext=(5, 5), fontsize=8)

plt.xlabel("Dac bits(4095)")
plt.ylabel("Vset-Vdac")
plt.title("Vset-Vdac Vs Dac bits")
plt.grid(True)
plt.show()
