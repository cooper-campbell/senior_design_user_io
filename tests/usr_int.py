import os
import matplotlib.pyplot as plt
import csv
import sys
import statistics

data =[]

def find_location(x):
    tmp = 2 * (x + 1)
    return tmp + tmp//10 - tmp//100 -1

with open('memdump2.txt') as f:
    reader = csv.reader(f, delimiter=' ')
    for line in reader:
        for x in line:
            data.append(x)

data = [int(y+x, 16) for x,y in zip(data[0::2], data[1::2])]

print(data)
print(len(data))

close = []
for i in range(800):
    close.append(data[find_location(i)])

print(close)

plt.figure()
plt.plot(data, 'ro')

plt.show()

plt.figure()
plt.plot(close, 'bo')
plt.show()
