

import sqlite3
import os
import re
import string
import stat
import popen2
import glob
import sys
import pprint




def main():
    conn = sqlite3.connect('roms.sqlite3')
    c = conn.cursor()
    for i in [(4,),(8,),(16,),(32,)]:
        dirname = "%02i" % i
        if not os.path.isdir(dirname):
            os.mkdir(dirname)
        print "#" * 60
        print "%i MBit" % i
        print "#" * 60
        c.execute('''SELECT
                          rom_name,
                          rom_mb,
                          file_name
                      FROM
                          roms
                      WHERE
                          rom_mb = ?
                      AND
                          rom_hirom = 0
                      AND
                          rom_sram = 0
                      AND
                          rom_region like "Europe%"
                      ORDER BY file_name
                      ''',i)
        for row in c:
            name,size,filename =  row
            if '[' not in filename:
                cmd = 'scp burst:"%s" %s/' % ( filename,dirname)
                print cmd

if __name__ == '__main__':
      main()







