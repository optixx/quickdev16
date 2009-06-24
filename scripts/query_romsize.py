

import sqlite3
import os
import re
import string
import stat
import popen2
import glob
import sys
import pprint
from subprocess import Popen


path = "/Users/david/Devel/arch/avr/code/snesram/roms/"


def shellquote(s):
    return "'" + s.replace("'", "'\\''") + "'"

def main():
    conn = sqlite3.connect('roms.sqlite3')
    c = conn.cursor()
    for i in [(4,),(8,),(16,),(32,)]:
        dirname = os.path.join(path,"%02i" % i)
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
                src = "david@burst:%s" % filename
                command = ["scp",shellquote(src), dirname]
                proc = Popen(command)
                print dir(proc)
                proc.communicate()
                
                raise

if __name__ == '__main__':
      main()







