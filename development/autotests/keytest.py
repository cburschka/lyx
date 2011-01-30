#!/usr/bin/python
# -*- coding: utf-8 -*-
# This script generates hundreds of random keypresses per second,
#  and sends them to the lyx window
# It requires xvkbd and wmctrl
# It generates a log of the KEYCODES it sends as development/keystest/out/KEYCODES
#
# Adapted by Tommaso Cucinotta from the original MonKey Test by
# John McCabe-Dansted.

import random
import os
import re
import sys
import time
#from subprocess import call
import subprocess

print 'Beginning keytest.py'

FNULL = open('/dev/null', 'w')

DELAY = '59'

class CommandSource:

    def __init__(self):
        keycode = [
            "\[Left]",
            '\[Right]',
            '\[Down]',
            '\[Up]',
            '\[BackSpace]',
            '\[Delete]',
            '\[Escape]',
            ]
        keycode[:0] = keycode
        keycode[:0] = keycode

        keycode[:0] = ['\\']

        for k in range(97, 123):
            keycode[:0] = chr(k)

        for k in range(97, 123):
            keycode[:0] = ["\A" + chr(k)]

        for k in range(97, 123):
            keycode[:0] = ["\A" + chr(k)]

        for k in range(97, 123):
            keycode[:0] = ["\C" + chr(k)]

        self.keycode = keycode
        self.count = 0
        self.count_max = 1999

    def getCommand(self):
        self.count = self.count + 1
        if self.count % 200 == 0:
            return 'RaiseLyx'
        elif self.count > self.count_max:
            os._exit(0)
        else:
            keystr = ''
            for k in range(1, 2):
                keystr = keystr + self.keycode[random.randint(1,
                        len(self.keycode)) - 1]
            return 'KK: ' + keystr


class CommandSourceFromFile(CommandSource):

    def __init__(self, filename, p):

        self.infile = open(filename, 'r')
        self.lines = self.infile.readlines()
        self.infile.close()
        linesbak = self.lines
        self.p = p
        print p, self.p, 'self.p'
        self.i = 0
        self.count = 0
        self.loops = 0

        # Now we start randomly dropping lines, which we hope are redundant
        # p is the probability that any given line will be removed

        if p > 0.001:
            if random.uniform(0, 1) < 0.5:
                print 'randomdrop_independant\n'
                self.randomdrop_independant()
            else:
                print 'randomdrop_slice\n'
                self.randomdrop_slice()
        if screenshot_out is None:
            count_atleast = 100
        else:
            count_atleast = 1
        self.max_count = max(len(self.lines) + 20, count_atleast)
        if len(self.lines) < 1:
            self.lines = linesbak

    def randomdrop_independant(self):
        p = self.p

        # The next couple of lines are to ensure that at least one line is dropped

        drop = random.randint(0, len(self.lines) - 1)
        del self.lines[drop]
        #p = p - 1 / len(self.lines)
        origlines = self.lines
        self.lines = []
        for l in origlines:
            if random.uniform(0, 1) < self.p:
                print 'Randomly dropping line ' + l + '\n'
            else:
                self.lines.append(l)
        print 'LINES\n'
        print self.lines
        sys.stdout.flush()

    def randomdrop_slice(self):
        lines = self.lines
        if random.uniform(0, 1) < 0.4:
            lines.append(lines[0])
            del lines[0]
        num_lines = len(lines)
        max_drop = max(5, num_lines / 5)
        num_drop = random.randint(1, 5)
        drop_mid = random.randint(0, num_lines)
        drop_start = max(drop_mid - num_drop / 2, 0)
        drop_end = min(drop_start + num_drop, num_lines)
        print drop_start, drop_mid, drop_end
        print lines
        del lines[drop_start:drop_end]
        print lines
        self.lines = lines

    def getCommand(self):
        if self.count >= self.max_count:
            os._exit(0)
        if self.i >= len(self.lines):
            self.loops = self.loops + 1
            if self.loops >= int(max_loops):
                return None
            self.i = 0
            return 'Loop'
        line = self.lines[self.i].rstrip('\n')
        self.count = self.count + 1
        self.i = self.i + 1
        #print '\nLine read: <<' + line + '>>\n'
        sys.stdout.write('r')
        return line

def lyx_exists():
    if lyx_pid is None:
        return False
    fname = '/proc/' + lyx_pid + '/status'
    return os.path.exists(fname)

def lyx_sleeping():
    fname = '/proc/' + lyx_pid + '/status'
    f = open(fname, 'r')
    lines = f.readlines()
    sleeping = lines[1].find('(sleeping)') > 0

    # print 'LYX_STATE', lines[1] , 'SLEEPING=', sleeping

    return sleeping


def sendKeystring(keystr, LYX_PID):

    # print "sending keystring "+keystr+"\n"

    if not re.match(".*\w.*", keystr):
        print 'print .' + keystr + '.\n'
        keystr = 'a'
    before_secs = time.time()
    while lyx_exists() and not lyx_sleeping():
        time.sleep(0.02)
        sys.stdout.write('.')
        sys.stdout.flush()
        if time.time() - before_secs > 180:
            print 'Killing due to freeze (KILL_FREEZE)'

            # Do profiling, but sysprof has no command line interface?
            # os.system("killall -KILL lyx")

            os._exit(1)
    if not screenshot_out is None:
        while lyx_exists() and not lyx_sleeping():
            time.sleep(0.02)
            sys.stdout.write('.')
            sys.stdout.flush()
        print 'Making Screenshot: ' + screenshot_out + ' OF ' + infilename
        time.sleep(0.2)
        os.system('import -window root '+screenshot_out+str(x.count)+".png")
        time.sleep(0.1)
    sys.stdout.flush()
    if (subprocess.call(
            ["xvkbd", "-xsendevent", "-window", lyx_window_name, "-delay", DELAY, "-text", keystr],
            stdout=FNULL,stderr=FNULL
            ) == 0):
        sys.stdout.write('*')
    else:
        sys.stdout.write('X')

def system_retry(num_retry, cmd):
    i = 0
    rtn = os.system(cmd)
    while ( ( i < num_retry ) and ( rtn != 0) ):
        i = i + 1
	rtn=os.system(cmd)
        time.sleep(1)
    if ( rtn != 0 ):
        print "Command Failed: "+cmd
        print " EXITING!\n"
        os._exit(1)

def RaiseWindow():
    #os.system("echo x-session-manager PID: $X_PID.")
    #os.system("echo x-session-manager open files: `lsof -p $X_PID | grep ICE-unix | wc -l`")
    ####os.system("wmctrl -l | ( grep '"+lyx_window_name+"' || ( killall lyx ; sleep 1 ; killall -9 lyx ))")
    #os.system("wmctrl -R '"+lyx_window_name+"' ;sleep 0.1")
    system_retry(30, "wmctrl -R '"+lyx_window_name+"'")


lyx_pid = os.environ.get('LYX_PID')
print 'lyx_pid: ' + str(lyx_pid) + '\n'
infilename = os.environ.get('KEYTEST_INFILE')
outfilename = os.environ.get('KEYTEST_OUTFILE')
max_drop = os.environ.get('MAX_DROP')
lyx_window_name = os.environ.get('LYX_WINDOW_NAME')
screenshot_out = os.environ.get('SCREENSHOT_OUT')

max_loops = os.environ.get('MAX_LOOPS')
if max_loops is None:
    max_loops = 3

lyx_exe = os.environ.get('LYX_EXE')
if lyx_exe is None:
    lyx_exe = "lyx"

file_new_command = os.environ.get('FILE_NEW_COMMAND')
if file_new_command is None:
    file_new_command = "\Afn"

ResetCommand = os.environ.get('RESET_COMMAND')
if ResetCommand is None:
    ResetCommand = "\[Escape]\[Escape]\[Escape]\[Escape]" + file_new_command
    #ResetCommand="\[Escape]\[Escape]\[Escape]\[Escape]\Cw\Cw\Cw\Cw\Cw\Afn"

if lyx_window_name is None:
    lyx_window_name = 'LyX'

print 'outfilename: ' + outfilename + '\n'
print 'max_drop: ' + max_drop + '\n'

if infilename is None:
    print 'infilename is None\n'
    x = CommandSource()
    print 'Using x=CommandSource\n'
else:
    print 'infilename: ' + infilename + '\n'
    probability_we_drop_a_command = random.uniform(0, float(max_drop))
    print 'probability_we_drop_a_command: '
    print '%s' % probability_we_drop_a_command
    print '\n'
    x = CommandSourceFromFile(infilename, probability_we_drop_a_command)
    print 'Using x=CommandSourceFromFile\n'

outfile = open(outfilename, 'w')

if lyx_pid != "":
    RaiseWindow()
    sendKeystring("\Afn", lyx_pid)

write_commands = True
failed = False

while not failed:
    #os.system('echo -n LOADAVG:; cat /proc/loadavg')
    c = x.getCommand()
    if c is None:
        break
    if c.strip() == "":
        continue
    outfile.writelines(c + '\n')
    outfile.flush()
    if c[0] == '#':
        print "\nIgnoring comment line: " + c
    elif c[0:9] == 'TestBegin':
        print "\n"
        lyx_pid=os.popen("pidof lyx").read()
        if lyx_pid != "":
            print "Found running instance(s) of LyX: " + lyx_pid + ": killing them all\n"
            os.system("killall lyx")
        time.sleep(0.2)
        print "Starting LyX . . ."
        os.system(lyx_exe + c[9:] + "&")
        while True:
            lyx_pid=os.popen("pidof lyx").read().rstrip()
            lyx_window_name=os.popen("wmctrl -l | grep 'lyx$\\|LyX:' | sed -e 's/.*\\([Ll]y[Xx].*\\)$/\\1/'").read().rstrip()
            if lyx_window_name != "":
                break
            print 'lyx_win: ' + lyx_window_name + '\n'
            print "Waiting for LyX to show up . . ."
            time.sleep(1)
        print 'lyx_pid: ' + lyx_pid + '\n'
        print 'lyx_win: ' + lyx_window_name + '\n'
        time.sleep(1)
        #RaiseWindow()
        #sendKeystring("\Afn", lyx_pid)
    elif c[0:5] == 'Sleep':
        print "\nSleeping for " + c[6:] + "\n"
        time.sleep(float(c[6:]))
    elif c[0:4] == 'Exec':
        cmd = c[5:].rstrip()
        print "\nExecuting " + cmd + "\n"
        os.system(cmd)
    elif c == 'Loop':
        outfile.close()
        outfile = open(outfilename + '+', 'w')
        print 'Now Looping'
    elif c == 'RaiseLyx':
        print 'Raising Lyx'
        RaiseWindow()
    elif c[0:4] == 'KK: ':
        if lyx_exists():
            sendKeystring(c[4:], lyx_pid)
        else:
            ##os.system('killall lyx; sleep 2 ; killall -9 lyx')
            print 'No path /proc/' + lyx_pid + '/status, exiting'
            os._exit(1)
    elif c[0:4] == 'KD: ':
        DELAY = c[4:].rstrip('\n')
        print 'Setting DELAY to ' + DELAY + '.'
    elif c == 'Loop':
        RaiseWindow()
        sendKeystring(ResetCommand, lyx_pid)
    elif c[0:6] == 'Assert':
        cmd = c[7:].rstrip()
        print "\nExecuting " + cmd
        result = os.system(cmd)
        failed = failed or (result != 0)
        print "result=" + str(result) + ", failed=" + str(failed)
    elif c[0:7] == 'TestEnd':
        time.sleep(0.5)
        print "\nTerminating lyx instance: " + str(lyx_pid) + "\n"
        os.system("kill -9 " + str(lyx_pid) + "\n");
        while lyx_exists():
            print "Waiting for lyx to die...\n"
            time.sleep(0.5)
        cmd = c[8:].rstrip()
        print "\nExecuting " + cmd
        result = os.system(cmd)
        failed = failed or (result != 0)
        print "result=" + str(result) + ", failed=" + str(failed)
    elif c[0:4] == 'Lang':
        lang = c[5:].rstrip()
        print "\nSetting LANG=" + lang
        os.environ['LANG'] = lang
    else:
        print "Unrecognised Command '" + c + "'\n"
        failed = True

print "Test case terminated: "
if failed:
    print "FAIL\n"
    os._exit(1)
else:
    print "Ok\n"
    os._exit(0)
