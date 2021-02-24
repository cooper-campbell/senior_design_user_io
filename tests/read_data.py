import os
import matplotlib.pyplot as plt
import csv

data = []

with open('tests.txt') as csvfile:
    reader = csv.reader(csvfile, delimiter=',')
    for row in reader:
        data.append([int(i) for i in row])

plt.figure()
plt.title("Simple Comparison of Expansion Algorithm")
plt.xlabel("Sample Number")
plt.ylabel("Value [0,799]")
plt.plot(range(0,800), data[1], 'b')
plt.plot(data[0], data[2], color='orange')
plt.plot(range(0,800), data[3], 'g')
plt.plot(data[0], data[4], color='red')
plt.legend(['Original linear', 'Expanded linear', 'Original Sine', 'Expanded Sine'])
plt.savefig(os.path.join("docs", "combined_expansion_plot.png"))
#plt.show()

plt.figure()
plt.title("Original Signals")
plt.xlabel("Sample Number")
plt.ylabel("Value [0,799]")
plt.plot(range(0,800), data[1], 'b')
plt.plot(range(0,800), data[3], 'g')
plt.legend(['Original linear', 'Original Sine'])
plt.savefig(os.path.join("docs", "original_plot.png"))
#plt.show()

plt.figure()
plt.title("Expanded Signals")
plt.xlabel("Sample Number")
plt.ylabel("Value [0,799]")
plt.plot(data[0], data[2], color='orange')
plt.plot(data[0], data[4], color='red')
plt.legend(['Expanded linear', 'Expanded Sine'])
plt.savefig(os.path.join("docs", "expansion_plot.png"))
#plt.show()
