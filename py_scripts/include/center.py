from not_ros.py_scripts.include.common_fun import *


def center():
    fig = plt.figure(1)
    # fig2 = plt.figure(2)

    # pl1 = fig.add_subplot(1,2,1)
    # pl1.set_xlim(-0.5, 0.5)
    pl2 = fig.add_subplot(1,1,1)
    pl2.vlines((-WINDTH, WINDTH), 0, 2, colors = 'red')

    # pl3 = fig2.add_subplot(1,1,1, projection = '3d')
#  reading center
#
    with open(arch_c,'rb') as file_c:
        fileContent_c = file_c.read()

    ii=0
    st = 0
    frame = 0
    while(ii<len(fileContent_c)):
        frame = frame+1
        st = st+1
        x = []
        y = []
        z = []
        v = []

        lk = int(fileContent_c[ii:ii+6])

        res = [(x, y, z) for x, y, z in struct.iter_unpack("fff", fileContent_c[ii+6:ii+6+lk*12])]
        ii=ii+6+lk*12
        if st == STP:
            st = 0
            rake_step = 2*WINDTH/RAKE_LENGTH
            rake = [[(-WINDTH + x_rake*rake_step), float("inf")] for x_rake in range(RAKE_LENGTH) ]

            for item in res:
                # if item[2] < 0.7 and item[2] > 0.65:
                # Заполнение массива точек для горизонтального среза
                #
                if item[2] < GAP_B and item[2] > GAP_A:
                    y_ = item[0]
                    x_ = -item[1]
                    z_ = item[2]

                    x.append(x_)
                    y.append(y_)
                    z.append(z_)
                    v.append((y_, x_))
                    if x_ <= WINDTH and x_ >= -WINDTH and y_ <= HEAD+0.5:
                        r_index = int(math.fabs(x_ + WINDTH)/rake_step)
                        rake[r_index][1] = y_ if rake[r_index][1] > y_ else rake[r_index][1]

                # Заполнение Массива точек для вертикального среза
                #

            xx = np.array(x)
            yy = np.array(y)
            zz = np.array(z)

            rake_x = np.array([rake_item[0] for rake_item in rake])
            rake_y = np.array([rake_item[1] for rake_item in rake])
            rake_noinf = [rake_ for rake_ in rake if rake_[1] < float("inf")]


            fig.clf()
            # fig2.clf()

            # pl1 = fig.add_subplot(1,2,1)
            pl2 = fig.add_subplot(1,1,1)
            # pl1.set_xlim(-0.5, 0.5)

            pl2.vlines((-WINDTH, WINDTH), 0, 2, colors = 'red')

            # pl1.set(title = 'номер фрейма {}'.format(frame))
            pl2.set(title = 'номер фрейма {}'.format(frame))
            # pl3 = fig2.add_subplot(1,1,1, projection = '3d')

            try:
                x1=rake_noinf[0][0]
                y1=rake_noinf[0][1]
                x2=rake_noinf[1][0]
                y2=rake_noinf[1][1]
                x3=rake_noinf[-2][0]
                y3=rake_noinf[-2][1]
                x4=rake_noinf[-1][0]
                y4=rake_noinf[-1][1]

                # pl1.scatter(rake_x, rake_y)
                # pl1.plot([x1, x2], [y1, y2],'r--')
                # pl1.plot([x3, x4], [y3, y4],'r--')

                pl2.scatter(xx, yy, color = 'orange', s=1, label = 'проекция облака точек на XY')
                pl2.scatter(rake_x, rake_y, label = 'точки на граблях')
                pl2.plot([x1, x2], [y1, y2],'r--')
                pl2.plot([x3, x4], [y3, y4],'r--')


                tg1 = tg(x1, y1, x2, y2)
                t11_x, t11_y = shift(x1=x1, y1=y1, tg=tg1, W=WINDTH)
                t12_x, t12_y = shift(x1=x2, y1=y2, tg=tg1, W=WINDTH)

                tg2 = tg(x3, y3, x4, y4)
                t21_x, t21_y = shift(x1=x3, y1=y3, tg=tg2, W=WINDTH)
                t22_x, t22_y = shift(x1=x4, y1=y4, tg=tg2, W=WINDTH)

                pl2.plot([t11_x,t12_x], [t11_y, t12_y],
                    color = 'magenta',
                    marker = 'o',
                    linestyle = 'dashed',
                    markersize = 5,
                    label = 'целевая линия')
                pl2.plot([t21_x, t22_x], [t21_y, t22_y],
                    color = 'magenta',
                    marker = 'o',
                    linestyle = 'dashed',
                    markersize = 5)
                plt.legend()

                # pl3.scatter(xx, yy, zz)
            except Exception as exc:
                print(exc)
            plt.show()