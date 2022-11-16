import struct
import numpy as np
import matplotlib.pyplot as plt
import math
from not_ros.py_scripts.include.settings import *

def conversion(x, y, x0, y0, phi):
    new_x = x*math.cos(phi) - y*math.sin(phi) + x0
    new_y = x*math.sin(phi) + y*math.cos(phi) + y0
    return new_x, new_y

def shift(x1, y1, tg, W, Sx = 1, Sy = -1):
    # По умолчанию значения для центра
    #
    Wx = x1 + Sx*W*tg/math.sqrt(1+tg**2)
    Wy = y1 + Sy*W/math.sqrt(1+tg**2)
    return Wx, Wy

def length(x1, y1, x2, y2):
    return math.sqrt((x2-x1)**2 + (y2-y1)**2)

def tg(x1, y1, x2, y2):
    return (y2-y1)/(x2-x1)

def phi(x1, y1, x2, y2):
    return math.atan((y2-y1)/(x2-x1))

def points_for_frame(d_fr, fr):
    x = []
    y = []
    xy=d_fr.get(fr)
    if xy is not None:
        x=xy[::2]
        y=xy[1::2]
    return x, y