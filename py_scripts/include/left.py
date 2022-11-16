
from not_ros.py_scripts.include.common_fun import *

def left():
    fig1_l = plt.figure(1)
    pl2_l = fig1_l.add_subplot(1, 1, 1)

    pl2_l.hlines((0, HEAD), 0, 2, colors = 'red')

    with open(arch_l,'rb') as file:
        fileContent = file.read()

    ii=0
    st = 0
    frame = 0

    while(ii<len(fileContent)):
        frame = frame + 1
        st = st+1
        x = []
        y = []
        z = []
        v = []

        lk = int(fileContent[ii:ii+6])
        res = [(x, y, z) for x, y, z in struct.iter_unpack("fff", fileContent[ii+6:ii+6+lk*12])]
        ii=ii+6+lk*12
        if st == STP:
            st = 0
            rake_step = HEAD/RAKE_LENGTH
            rake = [[x_rake*rake_step, float("-inf")] for x_rake in range(RAKE_LENGTH) ]
            rake_max = [[(x_rake*rake_step), float("-inf")] for x_rake in range(RAKE_LENGTH) ]

            for item in res:
                # if item[2] < 0.7 and item[2] > 0.65:
                # Заполнение массива точек для горизонтального среза
                #
                if item[2] < GAP_B and item[2] > GAP_A:
                    y_ = item[0]
                    x_ = -item[1]
                    # z_ = item[2]

                    x.append(x_)
                    y.append(y_)
                    # z.append(z_)
                    # v.append((y_, x_))
                    if y_ <= HEAD and y_ >=0 and x_ <= -WINDTH and x_ >= -LIMIT:
                        r_index = int(math.fabs(y_)/rake_step)
                        rake[r_index][1] = x_ if rake[r_index][1] < x_ else rake[r_index][1]
                        rake_max[r_index][1] = x_ if rake_max[r_index][1] >= x_ or rake_max[r_index][1] == float("-inf") else rake_max[r_index][1]

                # Заполнение Массива точек для вертикального среза
                #

            xx = np.array(x)
            yy = np.array(y)
            # zz = np.array(z)

            l_fl = False
            rake_noinf = []
            for rake_ in rake:
                if rake_[1] > float("-inf"):
                    rake_noinf.append(rake_)
                    if rake_[1] >= -WINDTH-PREC:
                        l_fl=True
                        break

            rake_x = np.array([rake_item[1] for rake_item in rake_noinf])
            rake_y = np.array([rake_item[0] for rake_item in rake_noinf])

            rake_max_noinf = []
            for rake_ in rake_max:
                if rake_[1] > float("-inf"):
                    rake_max_noinf.append(rake_)
                    if rake_[1] <= -LIMIT+PREC:
                        break

            rake_x_max = np.array([rake_item[1] for rake_item in rake_max_noinf])
            rake_y_max = np.array([rake_item[0] for rake_item in rake_max_noinf])


            fig1_l.clf()
            fig1_l = plt.figure(1)
            # pl1_l = fig1_l.add_subplot(1, 2, 1)
            pl2_l = fig1_l.add_subplot(1, 1, 1)

            # pl1_l.set_ylim(-0.5, HEAD+0.5)
            # pl1_l.set_xlim(-1., 0.)

            pl2_l.hlines((0, HEAD), 0, -2, colors = 'red', linestyle = 'dashed')
            pl2_l.vlines((-LIMIT, -WINDTH), 0, HEAD, colors= 'green', linestyle = 'dashed')

            # pl1_l.set(title = 'номер фрейма {}'.format(frame))
            pl2_l.set(title = 'номер фрейма {}'.format(frame))

            try:
                if len(rake_noinf) >= 2:
                    x1=rake_noinf[0][1]
                    y1=rake_noinf[0][0]
                    x2=rake_noinf[1][1]
                    y2=rake_noinf[1][0]
                    x3=rake_noinf[-2][1]
                    y3=rake_noinf[-2][0]
                    x4=rake_noinf[-1][1]
                    y4=rake_noinf[-1][0]
                # elif len(rake_noinf) > 2:
                #     x1=rake_noinf[1][1]
                #     y1=rake_noinf[1][0]
                #     x2=rake_noinf[2][1]
                #     y2=rake_noinf[2][0]
                #     x3=rake_noinf[-3][1]
                #     y3=rake_noinf[-3][0]
                #     x4=rake_noinf[-2][1]
                #     y4=rake_noinf[-2][0]
                else:
                    continue

                if len(rake_max_noinf) >=2:
                    x1_m=rake_max_noinf[0][1]
                    y1_m=rake_max_noinf[0][0]
                    x2_m=rake_max_noinf[1][1]
                    y2_m=rake_max_noinf[1][0]
                    x3_m=rake_max_noinf[-2][1]
                    y3_m=rake_max_noinf[-2][0]
                    x4_m=rake_max_noinf[-1][1]
                    y4_m=rake_max_noinf[-1][0]
                # elif len(rake_max_noinf) > 2:
                #     x1_m=rake_max_noinf[1][1]
                #     y1_m=rake_max_noinf[1][0]
                #     x2_m=rake_max_noinf[2][1]
                #     y2_m=rake_max_noinf[2][0]
                #     x3_m=rake_max_noinf[-3][1]
                #     y3_m=rake_max_noinf[-3][0]
                #     x4_m=rake_max_noinf[-2][1]
                #     y4_m=rake_max_noinf[-2][0]
                else:
                    continue


                pl2_l.scatter(xx, yy, color= 'orange', s=1, label = 'проекция облака точек на XY')
                pl2_l.scatter(rake_x, rake_y, label = 'точки на граблях')
                pl2_l.plot([x1, x2], [y1, y2],'r--')
                pl2_l.plot([x3, x4], [y3, y4], 'r--')

                tg1 = tg(x1, y1, x2, y2)
                sx = -1 if tg1 < 0. else 1
                sy = 1 if tg1 < 0. else -1
                t11_x, t11_y = shift(x1=x1, y1=y1, tg=tg1, W=WINDTH, Sx=sx, Sy=sy)
                t12_x, t12_y = shift(x1=x2, y1=y2, tg=tg1, W=WINDTH, Sx=sx, Sy=sy)

                tg2 = tg(x3, y3, x4, y4)
                sx = -1 if tg2 < 0. else 1
                sy = 1 if tg2 < 0. else -1
                t21_x, t21_y = shift(x1=x3, y1=y3, tg=tg2, W=WINDTH, Sx=sx, Sy=sy)
                t22_x, t22_y = shift(x1=x4, y1=y4, tg=tg2, W=WINDTH, Sx=sx, Sy=sy)

                pl2_l.scatter(rake_x_max, rake_y_max, color = 'black')
                pl2_l.plot([x1_m, x2_m], [y1_m, y2_m],'b--')
                pl2_l.plot([x3_m, x4_m], [y3_m, y4_m], 'b--')

                tg1_m = tg(x1_m, y1_m, x2_m, y2_m)
                sx = 1 if tg1_m < 0. else -1
                sy = -1 if tg1_m < 0. else 1
                t11_x_m, t11_y_m = shift(x1=x1_m, y1=y1_m, tg=tg1_m, W=WINDTH, Sx=sx, Sy=sy)
                t12_x_m, t12_y_m = shift(x1=x2_m, y1=y2_m, tg=tg1_m, W=WINDTH, Sx=sx, Sy=sy)

                tg2_m = tg(x3_m, y3_m, x4_m, y4_m)
                sx = 1 if tg2_m < 0. else -1
                sy = -1 if tg2_m < 0. else 1
                t21_x_m, t21_y_m = shift(x1=x3_m, y1=y3_m, tg=tg2_m, W=WINDTH, Sx=sx, Sy=sy)
                t22_x_m, t22_y_m = shift(x1=x4_m, y1=y4_m, tg=tg2_m, W=WINDTH, Sx=sx, Sy=sy)

                # critical_x1 = ((y2+y1)*(x2-x1) - (x1+x2)*(y2-y1))/(2*(y2-y1))
                # critical_x2 = ((y4+y3)*(x4-x3) - (x3+x4)*(y4-y3))/(2*(y4-y3))
                critical_x1 = x1 - y1/tg1
                critical_x2 = x3 - y3/tg2

                l_fl = False
                if not l_fl:
                    if tg1 >= 0.:
                        f_1x = t11_x
                        f_1y = t11_y
                        f_2x = t12_x
                        f_2y = t12_y
                    elif critical_x1 <= 0:
                        f_1x = t11_x
                        f_1y = t11_y
                        f_2x = t12_x
                        f_2y = t12_y
                    else:
                        f_1x = t11_x_m
                        f_1y = t11_y_m
                        f_2x = t12_x_m
                        f_2y = t12_y_m

                    if tg2 >=0.:
                        s_1x = t21_x
                        s_1y = t21_y
                        s_2x = t22_x
                        s_2y = t22_y
                    elif critical_x2 <= 0:
                        s_1x = t21_x
                        s_1y = t21_y
                        s_2x = t22_x
                        s_2y = t22_y
                    else:
                        s_1x = t21_x_m
                        s_1y = t21_y_m
                        s_2x = t22_x_m
                        s_2y = t22_y_m
                else:
                    f_1x = t11_x_m
                    f_1y = t11_y_m
                    f_2x = t12_x_m
                    f_2y = t12_y_m
                    s_1x = t21_x_m
                    s_1y = t21_y_m
                    s_2x = t22_x_m
                    s_2y = t22_y_m

                pl2_l.plot([f_1x,f_2x], [f_1y, f_2y],
                    color = 'aqua',
                    marker = 'o',
                    linestyle = 'dashed',
                    markersize = 5,
                    label = 'целевая линия')
                pl2_l.plot([s_1x, s_2x], [s_1y, s_2y],
                    color = 'aqua',
                    marker = 'o',
                    linestyle = 'dashed',
                    markersize = 5)

                # pl2_l.plot([t11_x,t12_x], [t11_y, t12_y],
                #     color = 'magenta',
                #     marker = 'o',
                #     linestyle = 'dashed',
                #     markersize = 5,
                #     label = 'целевая линия')
                # pl2_l.plot([t21_x, t22_x], [t21_y, t22_y],
                #     color = 'magenta',
                #     marker = 'o',
                #     linestyle = 'dashed',
                #     markersize = 5)

                # pl2_l.plot([t11_x_m,t12_x_m], [t11_y_m, t12_y_m],
                #     color = 'aqua',
                #     marker = 'o',
                #     linestyle = 'dashed',
                #     markersize = 5,
                #     label = 'целевая линия')
                # pl2_l.plot([t21_x_m, t22_x_m], [t21_y_m, t22_y_m],
                #     color = 'aqua',
                #     marker = 'o',
                #     linestyle = 'dashed',
                #     markersize = 5)

                plt.legend()
            except Exception as exc:
                print(exc)
            plt.show()