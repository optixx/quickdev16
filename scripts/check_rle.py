# -*- coding: utf-8 -*-
import binascii
import os
import glob
import zlib


TARGET = "/Users/david/Dropbox/Tech/Quickdev16/roms/08"

count = 0
total_zip_len = 0
total_comp_len = 0

g = glob.glob(os.path.join(TARGET, "*"))

for name in g:
    count += 1
    data = open(name, 'r').read()
    data_len = len(data)
    comp = binascii.rlecode_hqx(data)
    comp_len = len(comp)
    comp_pre = comp_len / (data_len / 100)
    total_comp_len += comp_pre

    zip_data = zlib.compress(data)
    zip_len = len(zip_data)
    zip_pre = zip_len / (data_len / 100)
    total_zip_len += zip_pre
    print "%30s %04i %04i %2.2f %04i %2.2f" % (
        os.path.basename(name)[:30],
        data_len / 1024,
        comp_len / 1024,
        comp_pre,
        zip_len / 1024,
        zip_pre
    )

print "%2.2f %2.2f" % (total_zip_len / count, total_comp_len / count)
