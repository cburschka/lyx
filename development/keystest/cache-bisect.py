#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
from subprocess import call, check_call
from random import randrange
import getpass


outfilename = "/tmp/cache-bisect." + getpass.getuser() + ".log"
outfile = open(outfilename, 'w')
print 'BBBISECT_BEGIN'
#print >> outfile, 'BBBISECT_BEGIN'
#print >> outfile, 'BBBISECT_BEGIN'
#print >> outfile, 'BBBISECT_BEGIN'
#print >> outfile, 'BBBISECT_BEGIN'
#print >> outfile, 'BBBISECT_BEGIN'
#print 'BBBISECT_BEGIN'

source_dir = '/mnt/big/xp/src/lyx-1.6.x-bisect'  # must NOT end in a slash
cache_dir = source_dir + '.cache/'  # must end in a slash
source_dir = '/mnt/big/xp/src/lyx-1.6.x-bisect2'  # must NOT end in a slash

#make_cmd = 'autoconf && ./configure && cd src && echo __________ `pwd` && sleep 9 && make'
make_cmd = 'rm -rf autom4te.cache && autoconf && ./configure && cd src && make'

reverse_search = True
reverse_search = False
must_make = True  # If we fail to make the file, we could this a "bad" rather than "canot test"
must_make = False

# ToDo:
# replace .tmp with .partial_copy and .not_yet_made


def set_revision(new_v, tmp_d):
    #check_call(['svn', 'up', '-r' + new_v, '--force'], cwd=tmp_d)
    os.system ('cd "'+tmp_d+'" && yes tf | svn up -r'+new_v+'--force')

def cmp_version(x, y):
    return cmp(int(x), int(y))


def get_cached_versions():
    vers = [f for f in os.listdir(cache_dir) if not f.count('.')]
    vers.sort(cmp_version)
    return vers


def version_in_range(v, lo, hi):
    if cmp_version(v, lo) < 0:
        return False
    elif cmp_version(v, hi) > 0:
        return False
    return True

def killall_p (s):
    # Unlike killall, this searchs within command parameters, as well as the
    # command name

        #os.system("kPID=`ps a | grep '"+s+"' | grep -v grep | sed 's/^ *//g'|  sed 's/ .*$//'`
    os.system("(kPID=`ps a | grep '"+s+
    "' | grep -v grep | sed 's/^ *//g'|  sed 's/ .*$//'`\n\
        echo kPID $kPID "+s+"\n\
        echo kill $kPID\n\
        kill $kPID\n\
        sleep 0.1\n\
        echo kill -9 $kPID\n\
        kill -9 $kPID) 2> /dev/null")

def clean_up ():
    killall_p("autolyx")
    killall_p("lyx")
    killall_p("keytest.py")
    killall_p("xclip")

def filter_versions(vers, lo, hi):
    return [v for v in vers if cmp]


def ver2dir(v):
    return cache_dir + v


def make_ver(new_v, old_v=None, alt_v=None):
    print 'MAKING', new_v, old_v, alt_v
    new_d = ver2dir(new_v)
    if old_v is None:
        old_d = source_dir
    else:
        old_d = ver2dir(old_v)
    fail_d = new_d + '.fail'
    tmp_d = new_d + '.tmp'
    if os.path.exists(cache_dir + fail_d):
        return 1
    if os.path.exists(new_d):
        return 0
    if not os.path.exists(tmp_d):
        if not os.path.exists(old_d):
            old_d = old_d + '.tmp'
        call(['rm', '-rf', tmp_d + '.cp'])
        call(['cp', '-rvu', old_d, tmp_d + '.cp'])
        check_call(['mv', tmp_d + '.cp', tmp_d])
        set_revision(new_v, tmp_d)
    call('pwd && sleep 5 && echo ' + make_cmd, cwd=tmp_d, shell=True)
    result = call(make_cmd, cwd=tmp_d, shell=True)
    if result == 0:
        print 'Make successful'
        check_call(['mv', tmp_d, new_d])
    return result


def change_after(cmd, v):
    result = run_cmd(cmd, v)
    ca = result_after(result)
    print >> outfile, 'BISECT_change_after', v, ca
    print 'BISECT_change_after', v, ca
    return ca


def change_before(cmd, v):
    result = run_cmd(cmd, v)
    cb = result_before(result)
    print >> outfile, 'BISECT_change_before', v, cb
    print 'BISECT_change_before', v, cb
    return cb


def result_after(i):
    if reverse_search:
        return result_bad(i)
    else:
        return result_good(i)


def result_before(i):
    if reverse_search:
        return result_good(i)
    else:
        return result_bad(i)


def result_good(i):
    return i == 0


def result_bad(i):
    return not result_ugly(i) and not result_good(i)


def result_ugly(i):
    return i == 125  # Like git, we treat 125 as "We cannot test this version"


def run_cmd(cmd, v):
    #result = call('pwd ; echo SS ' + cmd, shell=True, cwd=ver2dir(v))
    print "CMD", cmd
    print "V2D", ver2dir(v)
    os
    #result = subprocess.call(cmd, shell=True, cwd=ver2dir(v))
    result = call(cmd, cwd=ver2dir(v))
    clean_up()
    print cmd, result
    return result


def do_bisect(cmd, vers, build):
    lo = 0
    hi = len(vers) - 1
    m = (lo + hi) / 2

    print lo, hi, m
    print vers[lo], vers[hi], vers[m]
    print vers

    print >> outfile, 'VERS', final_vers

    while len(vers) > 2:
        print 'i', lo, hi, m, cmd
        print 'v', vers[lo], vers[hi], vers[m], cmd
        print vers

        print '#ugly = Nonese'

        if build or must_make:
            ugly = False
            result = make_ver(vers[m], vers[lo], vers[hi])
            print 'AMKE RESULT', result
            if not must_make:
                if result > 0 and not must_make:
                    ugly = True  # Not good, or bad, just ugly.
                else:
                    result = run_cmd(cmd, vers[m])
        if not ugly:
            if result > 127:
                os._exit(1)
            ugly = result_ugly(result)
        if ugly:
            print vers[m] + ' is UGLY'
            del vers[m]
            hi = len(vers) - 1
            m = randrange(0, len(vers))
        else:
            if result_after(result):
                print vers[m] + ' is AFTER'
                del vers[lo:m]
            else:
                print vers[m] + ' is BEFORE'
                del vers[m + 1:hi + 1]
            hi = len(vers) - 1
            m = (lo + hi) / 2

        print 'VERS REMAINING:', vers

    return vers


def check_bisect(cmd, vers):
    lo = 0
    hi = len(vers) - 1
    l = vers[lo]
    h = vers[hi]
    if make_ver(l):
        return False
    if make_ver(h):
        return False
    if change_before(cmd, l):
        print 'Cannot bisect, change before ' + l\
             + ' or regression test invalid'
        return False
    if change_after(cmd, h):
        print 'Cannot bisect, change after ' + h\
             + ' or regression test invalid'
        return False
    return True


def do_check_bisect(cmd, vers, build):
    print vers
    if check_bisect(cmd, vers):
        return do_bisect(cmd, vers, build)
    else:
        return


def open_and_readlines(fname):
    f = open(fname, 'r')
    lines = f.readlines()
    for i in range(0, len(lines)):
        lines[i] = lines[i].rstrip('\n')
    return lines


def get_versions_between(l, h):
    vers = [f for f in open_and_readlines('all_versions')
            if version_in_range(f, l, h)]
    vers.sort(cmp_version)
    return vers


def get_cached_versions_between(l, h):
    vers = [f for f in get_cached_versions() if version_in_range(f, l, h)]
    vers.sort(cmp_version)
    print 'BTWN', l, h, vers
    return vers


def two_level_bisect(cmd, LO, HI):
    if make_ver(LO):
        return False
    if make_ver(HI):
        return False
    vers = get_cached_versions_between(LO, HI)
    print 'CACHED_VERSIONS', vers
    vers = do_check_bisect(cmd, vers, False)
    print 'Closest Cached Versions', vers
    if vers is None:
        return
    if len(vers) != 2:
        return
    vers = get_versions_between(vers[0], vers[1])
    print 'BETWEEN VERSIONS', vers
    vers = do_check_bisect(cmd, vers, True)


def multisect(cmd, vers):
    i = 1
    while i < len(vers):
        print >> outfile, 'MULTISECT', vers[i]
        print 'MULTISECT', vers[i]
        if not ( make_ver(vers[i], vers[i-1])==0 and change_after(cmd, vers[i]) ) :
            i = i + 1
        else:
            return two_level_bisect(cmd, vers[i], vers[0])


print 'BISECT_BEGIN'
print >> outfile, 'BISECT_BEGIN'
outfile.flush()
#final_vers = multisect('$TEST_COMMAND', ['30614', '27418', '23000'])
cmd = os.sys.argv
del cmd[0]

VERS = os.environ.get('MULTISECT_VERS')
if VERS is None:
    VERS = ['30614', '27418', '23000']
else:
    VERS = VERS.split()

final_vers = multisect(cmd, VERS)
#final_vers = two_level_bisect('true', "21107","23000")
#final_vers = do_bisect('true', get_versions_between("21107","23000"),True)
outfile.flush()
print
print >> outfile, 'BISECT_FINAL', final_vers
print 'BISECT_FINAL', final_vers
os.system('echo BISECT_BEGIN >> /tmp/adsfadsf.log')
os.system('echo BISECT_FINAL >> /tmp/adsfadsf.log')

clean_up()
os._exit(0)
