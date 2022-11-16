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
arch = "/home/tenderbook/data/slalom/r_history.data"

save_dir_name = "./not_ros/data/img/"

WINDTH = 0.405
LIMIT = 1.2
LIMIT_F = 1.2
HIST = 50
FR = int(LIMIT/0.1)-1
FR = 9

GAP = LIMIT_F/FR
h_lines = [x*GAP for x in range(FR+1)]

k=0
st = 9

frame = 0

fig1 = plt.figure(1)

gr1 = fig1.add_subplot(1, 1, 1)

gr1.hlines(h_lines, 0., LIMIT)
gr1.set_xlim([0., LIMIT + 0.05])
gr1.set_ylim([0., LIMIT_F + 0.05])

# plt.rcParams['figure.figsize'] = (plt.rcParamsDefault['figure.figsize'][0]*5, plt.rcParamsDefault['figure.figsize'][0]*5)

with open(arch, 'rt') as file:
    fcont = file.read()
fcont = fcont.strip('\n')
fcont = fcont.strip()
lcont = fcont.split('\n')

while(k < len(lcont)):
    st = st+1
    frame = frame + 1
    odo_move = lcont[k]
    odo = odo_move.split(',')
    odo_x = float(odo[1])
    odo_y = float(odo[2])
    odo_t = float(odo[3])
    k = k+1

    for i in range(HIST):
        x=[]
        y=[]
        l = lcont[k+i*FR:k+i*FR+FR]
        for item in l:
            n = item.split(',')
            if float(n[1]) != 0.0 and float(n[2]) !=0.0:
                x.append(float(n[1]))
                y.append(float(n[2]))

        xx = np.array(x)
        yy = np.array(y)
        gr1.plot(xx, yy, color=clrs_hex[i*2 +10], marker = 'o', markersize = 10, alpha = 0.5, label = "линия истории {}".format(i) )
        if st  >= 1:
            plt.show()

    k = k+HIST*FR +1
    res = lcont[k: k+FR]

    x_res = []
    y_res = []
    for item in res:
        r = item.split(',')
        if float(r[1]) != 0.0 and float(r[2]) !=0.0:
            x_res.append(float(r[1]))
            y_res.append(float(r[2]))

    xx=np.array(x_res)
    yy=np.array(y_res)
    gr1.plot(xx, yy, marker = 'o', markersize = 12, label = "результирующая линия")
    gr1.legend()

    k = k+FR+1
    gr1.set(title = f'левая камера. номер фрейма {frame}. odo: x = {odo_x:.4f}, y = {odo_y:.4f}, theta = {odo_t:.4f} ')
    if st >= 1:
        plt.show()
        st = 0

    fig1.clf()
    gr1 = fig1.add_subplot(1, 1, 1)

    gr1.hlines(h_lines, 0., LIMIT)
    gr1.set_xlim([0., LIMIT + 0.05])
    gr1.set_ylim([0., LIMIT_F + 0.05])


        # save_file_name = "profile_frame{}.png".format(str_frame_prev)
        # save_file_name = save_dir_name+save_file_name




        # fig1.savefig(save_file_name)







