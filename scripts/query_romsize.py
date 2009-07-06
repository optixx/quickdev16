

import sqlite3
import os
import re
import string
import stat
import popen2
import glob
import sys
import os
import pprint
import base64
import getpass
import os
import socket
import sys
import traceback
import paramiko


from subprocess import Popen

paramiko.util.log_to_file('rom_sftp.log')

if os.name == 'posix':
    path = "/home/david/Devel/arch/avr/code/snesram/roms/"
    username = "david"
    hostname = "slap"
else:
    path = "/Users/david/Devel/arch/avr/code/snesram/roms/"
    username = "david"
    hostname = "burst"

def shellquote(s):
    return "'" + s.replace("'", "'\\''") + "'"

def main():

    port = 22
    password = getpass.getpass('Password for %s@%s: ' % (username, hostname))
    hostkeytype = None
    hostkey = None
    host_keys = paramiko.util.load_host_keys(os.path.expanduser('~/.ssh/known_hosts'))
    if host_keys.has_key(hostname):
        hostkeytype = host_keys[hostname].keys()[0]
        hostkey = host_keys[hostname][hostkeytype]
        print 'Using host key of type %s' % hostkeytype
    try:
        print "Connect %s:%s" % (hostname, port)
        t = paramiko.Transport((hostname, port))
        t.connect(username=username, password=password, hostkey=hostkey)
        sftp = paramiko.SFTPClient.from_transport(t)

        # dirlist on remote host
        dirlist = sftp.listdir('.')
        print "Dirlist:", dirlist


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
                              rom_type = 0
                          AND
                              rom_region like "Europe%"
                          ORDER BY file_name
                          ''',i)
            for row in c:
                name,size,filename =  row
                filename_dst = os.path.join(dirname,os.path.basename(filename))
                print "Remote: %s -> %s" % ( filename,filename_dst)
                data = open(filename, 'r').read()
                open(filename_dst,"w").write(data)

    except Exception, e:
        print '*** Caught exception: %s: %s' % (e.__class__, e)
        traceback.print_exc()
        try:
            t.close()
        except:
            pass
        sys.exit(1)


if __name__ == '__main__':
      main()







