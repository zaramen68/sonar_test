import numpy as np
import matplotlib.pyplot as plt
import matplotlib.transforms as mtransforms
import matplotlib.colors as colors
import math
import struct
import copy

from mpl_toolkits.mplot3d import Axes3D
from scipy.optimize import curve_fit

st_step=1

def func(x,  a, b):
    return  a*x+b

def MNK1(x, y):
    n = len(x)
    k = (n*np.dot(x, y) - np.sum(x)*np.sum(y))/(n*np.sum(np.square(x)) - np.sum(x)**2)
    b=np.mean(y) - k*np.mean(x)
    return k, b

def MAXF(n):
    nmax=[]
    for i in range(3, len(n)-4):
        vml = (n[i-3]+n[i-2]+n[i-1])/3
        vmr = (n[i+1]+n[i+2]+n[i+3])/3
        if n[i]> vml and n[i]> vmr and n[i] > n[i+1] and n[i] > n[i-1]:
            nmax.append((i, n[i]))
    return nmax

def MAX_1(n):
    nmax=[]
    for i in range(1, len(n)-1):
        vml = n[i]-n[i-1]
        vmr = n[i+1]-n[i]
    
        if (vml > 0 and vmr <=0 ) or (vml>=0 and vmr <0):
            nmax.append((i, n[i]))
    return nmax

def MINF(n):
    nmin=[]
    for i in range(1, len(n)-1):
        vml = n[i]-n[i-1]
        vmr = n[i+1]-n[i]
    
        if (vml < 0 and vmr >=0 ) or (vml<=0 and vmr >0):
            nmin.append((i, n[i]))
    return nmin



def correct(n, k, b, c=0.005):
    ll = len(n)
    new_v = []
    for i in range(1, ll-1):
        d1 = math.fabs(n[i][1]-k*n[i][0]-b)
        # d2 = math.fabs(v[i+1][1]-v[i][1])
        if d1 < c:
            new_v.append(n[i])

    v_x = np.array([v_[0] for v_ in new_v])
    v_y = np.array([v_[1] for v_ in new_v])
    return v_x, v_y

def vert_to_arr(v):
    v_x = np.array([v_[0] for v_ in v])
    v_y = np.array([v_[1] for v_ in v])
    return v_x, v_y

def arr_to_vert(x, y):
    v = []
    for i in range(len(x)):
        v.append((x[i], y[i]))
    return v

def conversion(x, y, cx, cy, phi):
    new_x = (x-cx)*math.cos(phi)+(y-cy)*math.sin(phi)
    new_y = -(x-cx)*math.sin(phi)+(y-cy)*math.cos(phi)
    return new_x, new_y

def last_in_list(n, l):
    if len(l)>0:
        return n>l[-1][0]
    else:
        return True

def SMA(n, v):
    new_v=[]
    sma = 0
    
    for i in range(n):
        sma = sma+v[i]

    sma = sma/n
    new_v.append(sma)

    for j in range(n+1, len(v)-n-1):
        sma = sma +(v[j+n]-v[j-1-n])/(1+2*n)
        new_v.append(sma)
    

    return new_v


clrs_hex = []
for name, hex in colors.cnames.items():
    clrs_hex.append(hex)

arch = "./not_ros/data/clouds.data"
# arch = "./not_ros/data/old_base.data"
save_dir_name = "./not_ros/data/img/"
# arch = "/home/tenderbook/logs/parking/frames.data"


figH = plt.figure(1)
fig = plt.figure(2)
pl6 = fig.add_subplot(1,1,1)
pl6.set_xlabel('X')
pl6.set_ylabel('расстояние от БС до колесной базы робота')
pl6.set(title = 'положение робота в системе координат БС')

with open(arch,'rb') as file:
    fileContent = file.read()

ii=0
st = 0
while(ii<len(fileContent)):
    st = st+1

    num_of_frame = int(fileContent[ii:ii+6])
    str_num_of_frame =  fileContent[ii:ii+6].decode("utf-8")

    local_time = int(fileContent[ii+6:ii+12])
    str_time = fileContent[ii+6:ii+12].decode("utf-8")

    save_file_name = "clouds_frame{}.png".format(str_num_of_frame)
    save_file_name = save_dir_name+save_file_name

    hist_name = "hist_frame{}.png".format(str_num_of_frame)
    hist_name = save_dir_name + hist_name

    num_of_clouds = int(fileContent[ii+12:ii+14])

    figH.clf()
    plH = figH.add_subplot(1,1,1)

    fig.clf()
    pl6 = fig.add_subplot(1,1,1)
    pl6.set_xlim([-1.5, 1.5])
    pl6.set_ylim([0., 3.0])

    ii=ii+14

    for j in range(num_of_clouds):

        is_base_fl = int(fileContent[ii:ii+1])
        phi = float(fileContent[ii+1:ii+13])
        bi = float(fileContent[ii+13:ii+25])


        lk = int(fileContent[ii+25:ii+31])

        res = [(x, y) for x, y in struct.iter_unpack("ff", fileContent[ii+7:ii+7+lk*8])]
        ii=ii+7+24+lk*8 
        x = []
        y = []
        v = []

        if st == st_step:

            for item in res:
                x_ = -item[0]
                y_ = item[1]
        
                
                x.append(x_)
                y.append(y_)

                v.append((y_, x_))
            
            xx = np.array(x)
            yy = np.array(y)


            n_bins = 300
            max_y = 3.
            delta_bin = max_y/n_bins
            c=max_y/n_bins
            w = 3



            n, bins, patches = plH.hist(yy, n_bins, range = (0., max_y), density=True, facecolor='g')

            # nmax = MAXF(n)

            n_sma = SMA(w, n)
            
            # nmax = MAXF(n_sma)
            nmax = MAX_1(n_sma)
            nmin = MINF(n_sma)

            bins_sma = bins[w+1:-w-1]

            min_bins = []
            min_value = []
            for m in nmin:
                min_bins.append(bins_sma[m[0]])
                min_value.append(m[1])

            max_bins = []
            max_value = []
            for m in nmax:
                max_bins.append(bins_sma[m[0]])
                max_value.append(m[1])
            plH.plot(bins_sma, n_sma, 'r*')
            plH.plot(min_bins, min_value, 'bo')
            plH.plot(max_bins, max_value, 'mo')

            
            pl6.scatter(xx, yy, c=clrs_hex[2*j+10], s =10, label = "is base={} phi = {} b = {}".format(is_base_fl, phi, bi))
            pl6.set(title = 'номер фрейма {}'.format(num_of_frame))
            pl6.legend()



            plH.set_xlabel('distance')
            plH.set_ylabel('Probability')
            plH.set(title = 'Histogram of distance num of frame {}'.format(num_of_frame))
            # plt.text(60, .025, r'$\mu=100,\ \sigma=15$')
            plH.set_xlim(0., 3.)
            # plt.ylim(0, 0.03)
            plH.grid(True)
            # plt.show()


    #######################################################33
    if st == st_step:
        st = 0
        fig.savefig(save_file_name)
        figH.savefig(hist_name)
        # plt.show()
        


        



