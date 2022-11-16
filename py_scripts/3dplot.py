import numpy as np
import matplotlib.pyplot as plt
import math
import struct

from mpl_toolkits.mplot3d import Axes3D
from scipy.optimize import curve_fit

def func(x,  a, b):
    return  a*x+b

def MNK1(x, y):
    n = len(x)
    k = (n*np.dot(x, y) - np.sum(x)*np.sum(y))/(n*np.sum(np.square(x)) - np.sum(x)**2)
    b=np.mean(y) - k*np.mean(x)
    return k, b

# arch = "/home/tenderbook/rs_test/vertex.data"
arch = "/home/tenderbook/logs/parking/frames.data"


with open(arch,'rb') as file:
    fileContent = file.read()

ii=0
while(ii<len(fileContent)):
    x = []
    y = []
    z = []
    v = []

    lk = int(fileContent[ii:ii+6])

    res = [(x, y, z) for x, y, z in struct.iter_unpack("fff", fileContent[ii+6:ii+6+lk*12])]
    ii=ii+6+lk*12

    for item in res:
        
        if item[2] < 0.8 and item[2] > 0.55:
            x_ = item[0]
            y_ = item[1]
            z_ = item[2]
            
            x.append(x_)
            y.append(y_)
            z.append(z_)
            v.append((x_, z_))
    
    xx = np.array(x)
    yy = np.array(y)
    zz = np.array(z)

    # v.sort(key= lambda t: t[0])

    # vv = np.array(v)

    # k, b = MNK1(xx, zz)

    # ll = len(v)
    # print(ll)
    # new_v = []
    # for i in range(1, ll-1):
    #     d1 = math.fabs(v[i][1]-k*v[i][0]-b)
    #     # d2 = math.fabs(v[i+1][1]-v[i][1])
    #     if d1 < 0.05:
    #         new_v.append(v[i])

    # print(len(new_v))

    # xx_1 = np.array([v_[0] for v_ in new_v])
    # zz_1 = np.array([v_[1] for v_ in new_v])
    # k1, b1 = MNK1(xx_1, zz_1)



    # phi_r = math.atan(1/k)
    # phi_d = math.degrees(phi_r)
    # print("k = {}, b = {}".format(k, b))
    # print("phi = {}".format(90. + phi_d))

    # p0 = [1.0, 1.0]

    # popt, pcov = curve_fit(f = func, xdata = x, ydata = z, p0=p0, absolute_sigma = True, check_finite=True, method='dogbox')
    # perr = np.sqrt(np.diag(pcov))

    # #data = np.genfromtxt(arch, dtype = None, names = ['x', 'y', 'q'], delimiter=",")
    # fig = plt.figure(1)
    fig2 = plt.figure(2)


    # pl1 = fig.add_subplot(2,2,1)
    # pl2 = fig.add_subplot(2,2,2)
    pl3 = fig2.add_subplot(1,1,1, projection = '3d')
    pl3.set_xlabel('x')
    pl3.set_ylabel('y')
    pl3.set_zlabel('z')

    # pl1.scatter(xx, zz, color='orange')
    # pl2.scatter(xx, zz, color = 'orange')

    # y_1 = []
    # y_2 = []

    # for x_ in x:
    #     y_1.append(k*x_ + b)
    #     y_2.append(popt[0]*x_ +popt[1])


    # pl1.set(title = 'my method')
    # pl2.set(title = 'fit function')

    # pl1.plot(x, y_1, 'r--')
    # pl2.plot(x, y_2, 'g--')


    pl3.scatter(xx, yy, zz)

    #######################################################33

    # fig3 = plt.figure(3)

    # pl3_1 = fig3.add_subplot(2,2,1)

    # pl3_1.scatter(xx_1, zz_1, color='orange')

    # pl3_1.set(title = 'my method')

    # pl3_1.plot(xx_1, func(xx_1, k1, b1), 'r--')

    plt.show()

    pass

