
import matplotlib.colors as colors

from include.center import *
from include.left import *
from include.right import *

clrs_hex = []
for name, hex in colors.cnames.items():
    clrs_hex.append(hex)

file1 = "/home/tenderbook/data/slalom/front_r.data"
file2 = "/home/tenderbook/data/slalom/right.data"

d_fr = dict()

for item in open(file2,'r'):
    item = item.strip('\n')
    item_ = item.split(',')

    str_frame = item_[0]
    frame = int(item_[0])
    if item_[1:] != ['']:
        d_fr[frame] = [float(x) for x in item_[1:]]


right(d_fr)







