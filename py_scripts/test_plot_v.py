import matplotlib.pyplot as plt
import matplotlib.colors as colors
import math
import numpy as np

from scipy.optimize import curve_fit

def func(x,  a, b):
    return  a*x+b

clrs_hex = []
for name, hex in colors.cnames.items():
    clrs_hex.append(hex)
use_colors = ["red", "green", "blue", "yelow"]
# read data from a text file. One number per line
baza_file = "/home/tenderbook/data/slalom/front_r.data"

save_dir_name = "/home/tenderbook/data/compare/"


color = []
rdist = []
k=0
frame = 1
str_frame_prev = "     1"

fig1 = plt.figure(1)

gr1 = fig1.add_subplot(1, 1, 1)

# gr2 = fig2.add_subplot(1, 1, 1)
gr1.set_xlim([-1.5, 1.5])
gr1.set_ylim([0., 2.5])

# plt.rcParams['figure.figsize'] = (plt.rcParamsDefault['figure.figsize'][0]*5, plt.rcParamsDefault['figure.figsize'][0]*5)

for item in open(baza_file,'r'):
    try:

        item = item.strip('\n')
        item_ = item.split(',')

        str_frame = item_[0]
        frame = int(item_[0])
        a1x_ = float(item_[1])
        a1y_ = float(item_[2])
        a2x_ = float(item_[3])
        a2y_ = float(item_[4])

        a3x_ = float(item_[5])
        a3y_ = float(item_[6])
        a4x_ = float(item_[7])
        a4y_ = float(item_[8])

        gr1.set(title = 'номер фрейма {}'.format(frame))

        gr1.plot([a1x_, a2x_], [a1y_, a2y_], "r--")
        gr1.plot([a3x_, a4x_], [a3y_, a4y_], "g--")

        gr1.scatter(a1x_, a1y_, c='red')
        gr1.scatter(a2x_, a2y_, c='red')

        gr1.scatter(a3x_, a3y_, c='green')
        gr1.scatter(a4x_, a4y_, c='green')

        plt.show()

        # save_file_name = "baza_frame{}.png".format(str_frame)
        # plt.show()

        # fig1.savefig(save_dir_name+save_file_name)
        fig1.clf()
        gr1 = fig1.add_subplot(1, 1, 1)

        gr1.set_xlim([-1.5, 1.5])
        gr1.set_ylim([0., 2.5])



    except Exception as ex:
        print(ex)

