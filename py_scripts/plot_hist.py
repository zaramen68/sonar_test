from cProfile import label
import numpy as np
import matplotlib.pyplot as plt

arch = "/home/tenderbook/data/slalom/eye_d5.data"
k=0

with open(arch, 'rt') as file:
    fcont = file.read()
fcont = fcont.strip('\n')
fcont = fcont.strip()
lcont = fcont.split('\n')
x = []
t = []

center = []
right = []
left = []

c_cor = []
l_cor = []
r_cor = []

while(k < len(lcont)-1):
    k=k+1
    item = lcont[k]
    data = item.split(',')
    if len(data) == 3:
        x.append(float(data[1]))
        t.append(float(data[2]))

        if data[0] == 'center':
            center.append(float(data[1]))
            c_cor.append(float(data[1]) - float(data[2])/1000)

        elif data[0] == 'left':
            left.append(float(data[1]))
            l_cor.append(float(data[1]) - float(data[2])/1000)

        elif data[0] == 'right':
            right.append(float(data[1]))
            r_cor.append(float(data[1]) - float(data[2])/1000)


xx = np.array(x)
cc = np.array(center)
ll = np.array(left)
rr = np.array(right)

tn = np.array(t)
cn = np.array(c_cor)
ln = np.array(l_cor)
rn = np.array(r_cor)

titels = ["work time", "center", "left", "right"]

fig, axs = plt.subplots(2, 2)

n1, bins1, patches1 = axs[0, 0].hist(tn, bins = 50, facecolor='g', label = f"max work time {tn.max()} microseconds")
n2, bins2, patches2 = axs[0, 1].hist(cn, bins = 50, facecolor='b', label = "center")
n3, bins3, patches3 = axs[1, 0].hist(ln, bins = 50, facecolor='r', label = "left")
n4, bins4, patches4 = axs[1, 1].hist(rn, bins = 50, facecolor='y', label = "right")

a = axs.ravel()
for idx, ax in enumerate(a):
    ax.set_title(titels[idx])
    ax.legend()
    ax.set_xlabel("time")



# fig2 = plt.figure(2)

# gr1 = fig2.add_subplot(2, 2, 1)
# gr1.set(title = 'work time')
# gr1.plot(bins1[1:], n1, 'g--')
# gr1.scatter(bins1[1:], n1, color='orange')

# gr2 = fig2.add_subplot(2, 2, 2)
# gr2.set(title = 'center')
# gr2.plot(bins2[1:], n2, 'b--')
# gr2.scatter( bins2[1:], n2, color='orange')

# gr3 = fig2.add_subplot(2, 2, 3)
# gr3.set(title = 'left')
# gr3.plot(bins3[1:], n3, 'r--')
# gr3.scatter(bins3[1:], n3, color='orange')

# gr3 = fig2.add_subplot(2, 2, 4)
# gr3.set(title = 'right')
# gr3.plot(bins4[1:], n4, 'y--')
# gr3.scatter(bins4[1:], n4, color='orange')



fig3 = plt.figure(3)
fig3.suptitle("Распределения частот обновлений облаков точек для камер глубины", fontsize = 18)

counts1, bins1 = np.histogram(xx, bins=50, range=(0., 150.), normed=False)

gr4 = fig3.add_subplot(2, 2, 1)
gr4.set(title = 'common')
gr4.set_xlabel("time ms")
gr4.hist(bins1[:-1], bins1, weights=counts1, color='green', label = f'max time = {xx.max()} ms')
gr4.legend()

counts2, bins2 = np.histogram(cc, bins=50, range=(0., 150.), normed=False)
gr5 = fig3.add_subplot(2, 2, 2)
gr5.set(title = 'center')
gr5.set_xlabel("time ms")
gr5.hist(bins2[:-1], bins2, weights=counts2, color = 'blue', label = f'max time = {cc.max()} ms')
gr5.legend()

counts3, bins3 = np.histogram(ll, bins=50, range=(0., 150.), normed=False)
gr6 = fig3.add_subplot(2, 2, 3)
gr6.set(title = 'left')
gr6.set_xlabel("time ms")
gr6.hist(bins3[:-1], bins3, weights=counts3, color = 'red', label = f'max time = {ll.max()} ms')
gr6.legend()

counts4, bins4 = np.histogram(rr, bins=50, range=(0., 150.), normed=False)
gr7 = fig3.add_subplot(2, 2, 4)
gr7.set(title = 'right')
gr7.set_xlabel("time ms")
gr7.hist(bins4[:-1], bins4, weights=counts4, color='orange', label = f'max time = {rr.max()} ms')
gr7.legend()

plt.show()
pass


