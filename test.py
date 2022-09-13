import matplotlib.pyplot as plt

import math
import numpy as np

from scipy.optimize import curve_fit

def func(x,  a, b):
    return  a*x+b

# read data from a text file. One number per line
arch = "/home/tenderbook/data/sonar/solid/static/60/sonar_2.data"
dist = []
delta_time = []
n = []
rdist = []
w_time = []

for item in open(arch,'r'):
    try:
        item = item.strip('\n')
        item_ = item.split(',')
        wt = item_[3]
        dt = item_[2]
        x_= item_[1]
        n_ = item_[0]

    except Exception as ex:
        print(ex)
    else:
        if item != '':
            try:
                dist.append(float(x_))
                delta_time.append(float(dt))
                n.append(float(n_))
                w_time.append(float(wt))

            except ValueError:
                pass

data = np.array(dist)
# counts, bins = np.histogram(data)
# plt.hist(bins[:-1], bins, weights=counts)


fig, ax = plt.subplots()
ax.grid(True)
i=[i_ for i_ in range(len(dist))]
ax.scatter(w_time, dist)

plt.show()