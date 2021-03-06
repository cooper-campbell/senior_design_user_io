import os
import matplotlib.pyplot as plt
import csv
import sys
import statistics

show = False
if len(sys.argv) > 1:
    show = sys.argv[1] == 's'
data = []

with open('tests.txt') as csvfile:
    reader = csv.reader(csvfile, delimiter=',')
    for row in reader:
        data.append([int(i) for i in row])

x1 = data[0]
x2 = range(0,800)
og_line = data[1]
exp_line = data[2]
norm_line = data[3]
og_sin = data[4]
exp_sin = data[5]
norm_sin = data[6]
og_dc = data[7]
exp_dc = data[8]
norm_dc = data[9]

print(statistics.mean(norm_line), statistics.mean(norm_line))
print(statistics.mean(norm_sin), statistics.mean(norm_sin))
print(statistics.mean(norm_dc), statistics.mean(exp_dc))
plt.figure()
plt.title("Simple Comparison of Expansion Algorithm")
plt.xlabel("Sample Number")
plt.ylabel("Value")
plt.plot(x2, og_line, 'b')
plt.plot(x1, exp_line, color='orange')
plt.plot(x2, og_sin, 'g')
plt.plot(x1, exp_sin, color='red')
plt.plot(x2, og_dc, color='cyan')
plt.plot(x1, norm_dc, color='yellow')
plt.legend(['Original linear', 'Expanded linear', 'Original Sine', 'Expanded Sine', 'Original Sine offset', 'Normalized Sine offset'])
plt.savefig(os.path.join("docs", "combined_expansion_plot.png"))
if show:
    plt.show()

plt.figure()
plt.title("Original Signals")
plt.xlabel("Sample Number")
plt.ylabel("Value [0,479]")
plt.plot(x2, og_line, 'b')
plt.plot(x2, og_sin, 'g')
plt.plot(x2, og_dc, color='cyan')
plt.legend(['Original linear', 'Original Sine', 'Original Sine DC Offset'])
plt.savefig(os.path.join("docs", "original_plot.png"))
if show:
    plt.show()

plt.figure()
plt.title("Expanded Signals")
plt.xlabel("Sample Number")
plt.ylabel("Value [0,2^16-1]")
plt.plot(x1, exp_line, color='orange')
plt.plot(x1, exp_sin, color='red')
plt.plot(x1, norm_dc, color='yellow')
plt.legend(['Expanded linear', 'Expanded Sine', 'Normalized Sine DC Offset'])
plt.savefig(os.path.join("docs", "expansion_plot.png"))
if show:
    plt.show()
