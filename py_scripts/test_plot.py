import matplotlib.pyplot as plt
import matplotlib.lines as mlines
import math
import numpy as np

from scipy.optimize import curve_fit

def func(x,  a, b):
    return  a*x+b

# read data from a text file. One number per line
arch_path = "/home/tenderbook/logs/sonar/"
arch_dict = {"node":{"file":"time_log.data", "color":"blue", "y": 0},
              "sonar 12":{"file":"tty_sonar12_id_1.data", "color":"orange", "y": 1},
              "sonar 22":{"file":"tty_sonar22_id_1.data", "color":"pink", "y": 2},
              "sonar 13":{"file":"tty_sonar13_u_a_r_t.data", "color":"green", "y": 6},
              "sonar 14":{"file":"tty_sonar14_u_a_r_t.data", "color":"red", "y": 4},
              "sonar 23":{"file":"tty_sonar23_u_a_r_t.data", "color":"purple", "y": 5},
              "sonar 24":{"file":"tty_sonar24_u_a_r_t.data", "color":"brown", "y": 3}}

n = 20
msize = 5
fig, ax = plt.subplots()
ax.grid(True)
ax.set_xlabel('epoch time (ms)')
ax.set_ylabel('start request order')

label_lines = []

starts = []

for key in arch_dict:
    line = arch_dict[key]
    arch = arch_path + line["file"]
    col = line["color"]
    y = line["y"]

    k=0
    tt=[]
    yy=[]
    vv = []
    label_lines.append(mlines.Line2D([], [], color = col, marker = 'o', markersize = msize, label=key))

    for item in open(arch,'r'):

        try:
            item = item.strip('\n')
            item_ = item.split(',')

            start_ = int(item_[0])
            stop_= int(item_[1])
            v = int(item_[2])

            if key == "node":
                starts.append(start_)

        except Exception as ex:
            print(ex)
        else:
            k=k+1
            step = int((stop_-start_)/n)
            if step !=0 :
                t = list(range(start_, stop_, step))
            else:
                t = [start_, stop_]
            yy = yy+[y]*len(t)
            tt = tt+t
            if v ==1 :
                vv = vv+[50]*len(t)
                ax.plot([start_, stop_], [y, y], color=col, marker = 's', linewidth = int(msize/2), markersize = msize)
            else:
                vv = vv+[10]*len(t)
                ax.plot([start_, stop_], [y, y], color='black', marker = 'x', linewidth = int(msize/2), markersize = msize)

    # ax.scatter(tt, yy, s=vv, color=col, label='{}'.format(key))

ax.legend(handles = label_lines)

    # plt.show()
ax.vlines(starts, 0, 6, colors=["blue"]*len(starts), linestyles="dashed")

plt.show()
pass






