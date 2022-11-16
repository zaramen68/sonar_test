file1 = "/home/tenderbook/data/slalom/front_r.data"
file2 = "/home/tenderbook/data/slalom/right.data"
file_res = "/home/tenderbook/data/slalom/res_right.data"
d_fr = dict()
d_right = dict()
d_res = dict()


for item in open(file1,'r'):
    item = item.strip('\n')
    item_ = item.split(',')

    str_frame = item_[0]
    frame = int(item_[0])
    d_fr[frame] = [x for x in item_[1:]]


for item in open(file2,'r'):
    item = item.strip('\n')
    item_ = item.split(',')

    str_frame = item_[0]
    frame = int(item_[0])
    d_right[frame] = [x for x in item_[1:]]

s_fr = set(d_fr)
s_right = set(d_right)

rs = s_fr.intersection(s_right)

for key in rs:
    d_res[key] = [*d_fr[key], *d_right[key]]

# for key in s_fr.difference(s_right):
#     d_res[key] = [*d_fr[key], *["inf"]*8]

# for key in s_right.difference(s_fr):
#     d_res[key] = [*["inf"]*8, *d_right[key]]

with open(file_res, 'w') as outfile:
    for key, item in d_res.items():
        line = str(key)+','+','.join(item)+'\n'
        outfile.write(line)





