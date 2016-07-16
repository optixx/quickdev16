import sqlite3
import os
import sys
import getpass
import socket
import traceback
import paramiko


def distance(a, b):
    c = {}
    n = len(a)
    m = len(b)

    for i in range(0, n + 1):
        c[i, 0] = i
    for j in range(0, m + 1):
        c[0, j] = j

    for i in range(1, n + 1):
        for j in range(1, m + 1):
            x = c[i - 1, j] + 1
            y = c[i, j - 1] + 1
            if a[i - 1] == b[j - 1]:
                z = c[i - 1, j - 1]
            else:
                z = c[i - 1, j - 1] + 1
            c[i, j] = min(x, y, z)
    return c[n, m]

paramiko.util.log_to_file('rom_sftp.log')

if socket.gethostname() == 'slap':
    path = "/home/david/Devel/arch/avr/code/snesram/roms/"
    username = "david"
    hostname = "slap"
elif socket.gethostname() == 'box':
    path = "/Users/david/Devel/arch/avr/code/snesram/roms/"
    username = "david"
    hostname = "burst"
else:
    sys.exit()


def shellquote(s):
    return "'" + s.replace("'", "'\\''") + "'"


def main():

    port = 22
    password = getpass.getpass('Password for %s@%s: ' % (username, hostname))
    hostkeytype = None
    hostkey = None
    host_keys = paramiko.util.load_host_keys(
        os.path.expanduser('~/.ssh/known_hosts'))
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

        conn = sqlite3.connect('roms_cleanup.sqlite3')
        c = conn.cursor()
        for i in [(1,), (2,), (4,), (8,), (16,)]:
            dirname = os.path.join(path, "%02i" % i)
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
                          ''', i)

            last_name = str()
            for row in c:
                name, size, filename = row
                d = distance(
                    os.path.basename(filename), os.path.basename(last_name))
                if d < 7:
                    print "Skip ", filename
                    continue
                filename_dst = os.path.join(
                    dirname, os.path.basename(filename))
                print "Remote: %s -> %s" % (filename, filename_dst)
                last_name = filename
                try:
                    sftp.get(filename, filename_dst)
                except Exception, e:
                    print '*** Caught exception: %s: %s' % (e.__class__, e)
                    traceback.print_exc()

            print "Done"
    except Exception, e:
        print '*** Caught exception: %s: %s' % (e.__class__, e)
        traceback.print_exc()
        try:
            t.close()
        except:
            pass
        sys.exit(1)
    t.close()


if __name__ == '__main__':
    main()
