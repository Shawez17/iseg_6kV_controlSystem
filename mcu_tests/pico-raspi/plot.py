import statistics
from collections import Counter
import csv

odd_vals = []
even_vals = []

with open("plot_data_dac2_1.csv") as f:
    for i, line in enumerate(f, start=1):
        line = line.strip()
        if not line:
            continue
        val = float(line)
        if i % 2 == 1:
            odd_vals.append(round(val, 3))
        else:
            even_vals.append(round(val, 3))

odd_counts = Counter(odd_vals)
even_counts = Counter(even_vals)

odd_mean = round(statistics.mean(odd_vals), 3)
odd_median = round(statistics.median(odd_vals), 3)
odd_mode_val, odd_mode_freq = odd_counts.most_common(1)[0]

even_mean = round(statistics.mean(even_vals), 3)
even_median = round(statistics.median(even_vals), 3)
even_mode_val, even_mode_freq = even_counts.most_common(1)[0]

print("Odd  -> mean:", odd_mean, "median:", odd_median, "mode:", odd_mode_val, "freq:", odd_mode_freq)
print("Even -> mean:", even_mean, "median:", even_median, "mode:", even_mode_val, "freq:", even_mode_freq)
print()
print("Odd frequencies:")
for val, freq in sorted(odd_counts.items()):
    print(val, freq)
print()
print("Even frequencies:")
for val, freq in sorted(even_counts.items()):
    print(val, freq)

with open("stats_frequency.csv", "w", newline="") as f:
    writer = csv.writer(f)
    writer.writerow(["Type", "Statistic", "Value", "Frequency"])
    writer.writerow(["Odd", "Mean", odd_mean, ""])
    writer.writerow(["Odd", "Median", odd_median, ""])
    writer.writerow(["Odd", "Mode", odd_mode_val, odd_mode_freq])
    writer.writerow(["Even", "Mean", even_mean, ""])
    writer.writerow(["Even", "Median", even_median, ""])
    writer.writerow(["Even", "Mode", even_mode_val, even_mode_freq])
    writer.writerow([])
    writer.writerow(["Type", "Value", "Frequency"])
    for val, freq in sorted(odd_counts.items()):
        writer.writerow(["Odd", val, freq])
    for val, freq in sorted(even_counts.items()):
        writer.writerow(["Even", val, freq])
