#!/usr/bin/python
# -*- coding: utf-8 -*-
# This script generates hundreds of random keypresses per second,
#  and sends them to the lyx window
# It requires xvkbd and wmctrl
# It generates a log of the KEYCODES it sends as development/keystest/out/KEYCODES
#
# Adapted by Tommaso Cucinotta from the original MonKey Test by
# John McCabe-Dansted.

from __future__ import print_function
import random
import os
import re
import sys
import time
import tempfile
import shutil

#from subprocess import call
import subprocess

print('Beginning keytest.py')

FNULL = open('/dev/null', 'w')

key_delay = ''

# Ignore status == "dead" if this is set. Used at the last commands after "\Cq"
dead_expected = False

def die(excode, text):
    if text != "":
        print(text)
    sys.stdout.flush()
    os._exit(excode)

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
            die(0, "")
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
        print(p, self.p, 'self.p')
        self.i = 0
        self.count = 0
        self.loops = 0

        # Now we start randomly dropping lines, which we hope are redundant
        # p is the probability that any given line will be removed

        if p > 0.001:
            if random.uniform(0, 1) < 0.5:
                print('randomdrop_independant\n')
                self.randomdrop_independant()
            else:
                print('randomdrop_slice\n')
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
                print('Randomly dropping line ' + l + '\n')
            else:
                self.lines.append(l)
        print('LINES\n')
        print(self.lines)
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
        print(drop_start, drop_mid, drop_end)
        print(lines)
        del lines[drop_start:drop_end]
        print(lines)
        self.lines = lines

    def getCommand(self):
        if self.count >= self.max_count:
            die(0, "")
        if self.i >= len(self.lines):
            self.loops = self.loops + 1
            if self.loops >= int(max_loops):
                return None
            self.i = 0
            return 'Loop'
        line = self.lines[self.i].rstrip('\n')
        self.count = self.count + 1
        self.i = self.i + 1
        return line

class ControlFile:

    def __init__(self):
        self.control = re.compile(r'^(C[ONPpRrC]):\s*(.*)$')
        self.fileformat = re.compile(r'^((\>\>?)[,\s]\s*)?([^\s]+)\s*$')
        self.cntrname = None
        self.cntrfile = None

    def open(self, filename):
        if not self.cntrfile is None:
            self.cntrfile.close()
            self.cntrfile = None
            self.cntrname = None
        m = self.fileformat.match(filename)
        if m:
            type = m.group(2)
            filename = m.group(3)
            if type == '>>':
                append = True
            else:
                append = False
        else:
            append = False
        self.cntrname = filename
        if append:
            self.cntrfile = open(filename, 'a')
        else:
            self.cntrfile = open(filename, 'w')

    def close(self):
        if not self.cntrfile is None:
            self.cntrfile.close()
            self.cntrfile = None
            self.cntrname = None
    # make the method below 'private'
    def __addline(self, pat):
        self.cntrfile.writelines(pat + "\n")

    def getfname(self):
        return self.cntrname

    def dispatch(self, c):
        m = self.control.match(c)
        if not m:
            return False
        command = m.group(1)
        text = m.group(2)
        if command == "CO":
            self.open(text);
        elif command == "CC":
            self.close()
        else:
            if not self.cntrfile is None:
                if command == "CN":
                    self.__addline("Comment: " + text)
                elif command == "CP":
                    self.__addline("Simple: " + text)
                elif command == "Cp":
                    self.__addline("ErrSimple: " + text)
                elif command == "CR":
                    self.__addline("Regex: " + text)
                elif command == "Cr":
                    self.__addline("ErrRegex: " + text)
                else:
                    die(1,"Error, Unrecognised Command '" + command + "'")
        return True


def get_proc_pid(proc_name):
    pid=os.popen("pidof " + proc_name).read().rstrip()
    return pid

wlistreg = re.compile(r'^(0x[0-9a-f]{5,9})\s+[^\s]+\s+([0-9]+)\s.*$')
def get_proc_win_id(pid, ignoreid):
    nlist = os.popen("wmctrl -l -p").read()
    wlist = nlist.split("\n")
    for item in wlist:
        m = wlistreg.match(item)
        if m:
            win_id = m.group(1)
            win_pid = m.group(2)
            if win_pid == pid:
                if win_id != ignoreid:
                    return win_id
    return None

def lyx_exists():
    if lyx_pid is None:
        return False
    fname = '/proc/' + lyx_pid + '/status'
    return os.path.exists(fname)


# Interruptible os.system()
def intr_system(cmd, ignore_err = False):
    print("Executing " + cmd)
    # Assure the output of cmd does not overhaul
    sys.stdout.flush()
    ret = os.system(cmd)
    if os.WIFSIGNALED(ret):
        raise KeyboardInterrupt
    if ret != 0 and not ignore_err:
        raise BaseException("command failed:" + cmd)
    return ret

statreg = re.compile(r'^State:.*\(([a-z]+)\)')

resstatus = []
def printresstatus():
    for line in resstatus:
        line = line.rstrip()
        print("    " + line.rstrip())
    print('End of /proc-lines')

def lyx_status_retry(pid):
    resstatus = []
    if pid is None:
        print('Pid is None')
        return "dead"
    fname = '/proc/' + pid + '/status'
    status = "dead"
    try:
        f = open(fname)
        found = False
        for line in f:
            resstatus.extend([line])
            m = statreg.match(line)
            if m:
                status = m.group(1)
                found = True
        f.close()
        if not found:
            return "retry"
        return status
    except IOError as e:
        print("I/O error({0}): {1}".format(e.errno, e.strerror))
        return "dead"
    except:
        print("Unexpected error:", sys.exc_info()[0])
        return "dead"
    print('This should not happen')
    return status

def lyx_status(pid):
    count = 0
    while 1:
        status = lyx_status_retry(pid)
        if status != "retry":
            break
        if count == 0:
            print('Retrying check for status')
        count += 1
        time.sleep(0.01)
    if count > 1:
        print('Retried to read status ' + str(count) + ' times')
    #print('lys_status() returning ' + status)
    return status

# Return true if LyX (identified via lyx_pid) is sleeping
def lyx_sleeping(LYX_PID):
    return lyx_status(LYX_PID) == "sleeping"

# Return true if LyX (identified via lyx_pid) is zombie
def lyx_zombie(LYX_PID):
    return lyx_status(LYX_PID) == "zombie"

def lyx_dead(LYX_PID):
    status = lyx_status(LYX_PID)
    return (status == "dead") or (status == "zombie")

def wait_until_lyx_sleeping(LYX_PID):
    before_secs = time.time()
    while True:
        status = lyx_status(LYX_PID)
        if status == "sleeping":
            return True
        if (status == "dead") or (status == "zombie"):
            printresstatus()
            if dead_expected:
                print('Lyx died while waiting for status == sleeping')
                return False
            else:
                die(1,"Lyx is dead, exiting")
        if time.time() - before_secs > 180:
            # Do profiling, but sysprof has no command line interface?
            # intr_system("killall -KILL lyx")
            printresstatus()
            die(1,"Killing due to freeze (KILL_FREEZE)")
        time.sleep(0.02)
    # Should be never reached
    print('Wait for sleeping ends unexpectedly')
    return False

def sendKeystringLocal(keystr, LYX_PID):
    is_sleeping = wait_until_lyx_sleeping(LYX_PID)
    if not is_sleeping:
        print("Not sending \"" + keystr + "\"")
        return
    if not screenshot_out is None:
        print('Making Screenshot: ' + screenshot_out + ' OF ' + infilename)
        time.sleep(0.2)
        intr_system('import -window root '+screenshot_out+str(x.count)+".png")
        time.sleep(0.1)
    actual_delay = key_delay
    if actual_delay == '':
        actual_delay = def_delay
    xvpar = [xvkbd_exe]
    if qt_frontend == 'QT5':
        xvpar.extend(["-jump-pointer", "-no-back-pointer"])
    else:
        xvpar.extend(["-xsendevent"])
    if lyx_other_window_name is None:
        xvpar.extend(["-window", lyx_window_name])
    else:
        xvpar.extend(["-window", lyx_other_window_name])
    xvpar.extend(["-delay", actual_delay, "-text", keystr])
    print("Sending \"" + keystr + "\"")
    subprocess.call(xvpar, stdout = FNULL, stderr = FNULL)
    sys.stdout.flush()

def extractmultiple(line, regex):
    #print("extractmultiple " + line)
    res = ["", ""]
    m = regex.match(line)
    if m:
        chr = m.group(1)
        if m.group(2) == "":
            res[0] = chr
            res[1] = ""
        else:
            norm = extractmultiple(m.group(2), regex)
            res[0] = chr + norm[0]
            res[1] = norm[1]
    else:
        res[0] = ""
        res[1] = line
    return res

normal_re = re.compile(r'^([^\\]|\\\\)(.*)$')
def extractnormal(line):
    # collect non-special chars from start of line
    return extractmultiple(line, normal_re)

modifier_re = re.compile(r'^(\\[CAS])(.+)$')
def extractmodifiers(line):
    # collect modifiers like '\\A' at start of line
    return extractmultiple(line, modifier_re)

special_re = re.compile(r'^(\\\[[A-Z][a-z0-9]+\])(.*)$')
def extractsingle(line):
    # check for single key following a modifier
    # either ascii like 'a'
    # or special like '\[Return]'
    res = [False, "", ""]
    m = normal_re.match(line)
    if m:
        res[0] = False
        res[1] = m.group(1)
        res[2] = m.group(2)
    else:
        m = special_re.match(line)
        if m:
            res[0] = True
            res[1] = m.group(1)
            res[2] = m.group(2)
        else:
            die(1, "Undecodable key for line \'" + line + "\"")
    return res

def sendKeystring(line, LYX_PID):
    if line == "":
        return
    normalchars = extractnormal(line)
    line = normalchars[1]
    if normalchars[0] != "":
        sendKeystringLocal(normalchars[0], LYX_PID)
    if line == "":
        return
    modchars = extractmodifiers(line)
    line = modchars[1]
    if line == "":
        die(1, "Missing modified key")
    modifiedchar = extractsingle(line)
    line = modifiedchar[2]
    special = modchars[0] != "" or modifiedchar[0]
    sendKeystringLocal(modchars[0] + modifiedchar[1], LYX_PID)
    if special:
        # give the os time to update the status info (in /proc)
        time.sleep(controlkey_delay)
    sendKeystring(line, LYX_PID)

def system_retry(num_retry, cmd):
    i = 0
    rtn = intr_system(cmd, True)
    while ( ( i < num_retry ) and ( rtn != 0) ):
        i = i + 1
        rtn = intr_system(cmd, True)
        time.sleep(1)
    if ( rtn != 0 ):
        print("Command Failed: "+cmd)
        die(1," EXITING!")

def RaiseWindow():
    #intr_system("echo x-session-manager PID: $X_PID.")
    #intr_system("echo x-session-manager open files: `lsof -p $X_PID | grep ICE-unix | wc -l`")
    ####intr_system("wmctrl -l | ( grep '"+lyx_window_name+"' || ( killall lyx ; sleep 1 ; killall -9 lyx ))")
    print("lyx_window_name = " + lyx_window_name + "\n")
    intr_system("wmctrl -R '"+lyx_window_name+"' ;sleep 0.1")
    system_retry(30, "wmctrl -i -a '"+lyx_window_name+"'")

class Shortcuts:

    def __init__(self):
        self.shortcut_entry = re.compile(r'^\s*"([^"]+)"\s*\"([^"]+)\"')
        self.bindings = {}
        self.bind = re.compile(r'^\s*\\bind\s+"([^"]+)"')
        if lyx_userdir_ver is None:
            self.dir = lyx_userdir
        else:
            self.dir = lyx_userdir_ver

    def __UseShortcut(self, c):
        m = self.shortcut_entry.match(c)
        if m:
            sh = m.group(1)
            fkt = m.group(2)
            self.bindings[sh] = fkt
        else:
            die(1, "cad shortcut spec(" + c + ")")

    def __PrepareShortcuts(self):
        if not self.dir is None:
            tmp = tempfile.NamedTemporaryFile(suffix='.bind', delete=False)
            try:
                old = open(self.dir + '/bind/user.bind', 'r')
            except IOError as e:
                old = None
            if not old is None:
                lines = old.read().split("\n")
                old.close()
                bindfound = False
                for line in lines:
                    m = self.bind.match(line)
                    if m:
                        bindfound = True
                        val = m.group(1)
                        if val in self.bindings:
                            if self.bindings[val] != "":
                                tmp.write("\\bind \"" + val + "\" \"" + self.bindings[val] + "\"\n")
                                self.bindings[val] = ""
                        else:
                            tmp.write(line + '\n')
                    elif not bindfound:
                        tmp.write(line + '\n')
            else:
                tmp.writelines(
                    '## This file is used for keytests only\n\n' +
                    'Format 4\n\n'
                )
            for val in self.bindings:
                if not self.bindings[val] is None:
                    if  self.bindings[val] != "":
                        tmp.write("\\bind \"" + val + "\" \"" + self.bindings[val] + "\"\n")
                        self.bindings[val] = ""
            tmp.close()
            shutil.move(tmp.name, self.dir + '/bind/user.bind')
        else:
            print("User dir not specified")

    def dispatch(self, c):
        if c[0:12] == 'UseShortcut ':
            self.__UseShortcut(c[12:])
        elif c == 'PrepareShortcuts':
            print('Preparing usefull sortcuts for tests')
            self.__PrepareShortcuts()
        else:
            return False
        return True

lyx_pid = os.environ.get('LYX_PID')
print('lyx_pid: ' + str(lyx_pid) + '\n')
infilename = os.environ.get('KEYTEST_INFILE')
outfilename = os.environ.get('KEYTEST_OUTFILE')
max_drop = os.environ.get('MAX_DROP')
lyx_window_name = os.environ.get('LYX_WINDOW_NAME')
lyx_other_window_name = None
screenshot_out = os.environ.get('SCREENSHOT_OUT')
lyx_userdir = os.environ.get('LYX_USERDIR')
lyx_userdir_ver = os.environ.get('LYX_USERDIR_23x')

max_loops = os.environ.get('MAX_LOOPS')
if max_loops is None:
    max_loops = 3

extra_path = os.environ.get('EXTRA_PATH')
if not extra_path is None:
  os.environ['PATH'] = extra_path + os.pathsep + os.environ['PATH']
  print("Added " + extra_path + " to path")
  print(os.environ['PATH'])

PACKAGE = os.environ.get('PACKAGE')
if not PACKAGE is None:
  print("PACKAGE = " + PACKAGE + "\n")

PO_BUILD_DIR = os.environ.get('PO_BUILD_DIR')
if not PO_BUILD_DIR is None:
  print("PO_BUILD_DIR = " + PO_BUILD_DIR + "\n")

lyx = os.environ.get('LYX')
if lyx is None:
    lyx = "lyx"

lyx_exe = os.environ.get('LYX_EXE')
if lyx_exe is None:
    lyx_exe = lyx

xvkbd_exe = os.environ.get('XVKBD_EXE')
if xvkbd_exe is None:
    xvkbd_exe = "xvkbd"

qt_frontend = os.environ.get('QT_FRONTEND')
if qt_frontend is None:
    qt_frontend = 'QT4'
if qt_frontend == 'QT5':
    controlkey_delay = 0.01
else:
    controlkey_delay = 0.4

locale_dir = os.environ.get('LOCALE_DIR')
if locale_dir is None:
    locale_dir = '.'

def_delay = os.environ.get('XVKBD_DELAY')
if def_delay is None:
    if qt_frontend == 'QT5':
        def_delay = '1'
    else:
        def_delay = '1'

file_new_command = os.environ.get('FILE_NEW_COMMAND')
if file_new_command is None:
    file_new_command = "\Afn"

ResetCommand = os.environ.get('RESET_COMMAND')
if ResetCommand is None:
    ResetCommand = "\[Escape]\[Escape]\[Escape]\[Escape]" + file_new_command
    #ResetCommand="\[Escape]\[Escape]\[Escape]\[Escape]\Cw\Cw\Cw\Cw\Cw\Afn"

if lyx_window_name is None:
    lyx_window_name = 'LyX'

print('outfilename: ' + outfilename + '\n')
print('max_drop: ' + max_drop + '\n')

if infilename is None:
    print('infilename is None\n')
    x = CommandSource()
    print('Using x=CommandSource\n')
else:
    print('infilename: ' + infilename + '\n')
    probability_we_drop_a_command = random.uniform(0, float(max_drop))
    print('probability_we_drop_a_command: ')
    print('%s' % probability_we_drop_a_command)
    print('\n')
    x = CommandSourceFromFile(infilename, probability_we_drop_a_command)
    print('Using x=CommandSourceFromFile\n')

outfile = open(outfilename, 'w')

if not lyx_pid is None:
    RaiseWindow()
    # Next command is language dependent
    #sendKeystring("\Afn", lyx_pid)

write_commands = True
failed = False
lineempty = re.compile(r'^\s*$')
marked = ControlFile()
shortcuts = Shortcuts()
while not failed:
    #intr_system('echo -n LOADAVG:; cat /proc/loadavg')
    c = x.getCommand()
    if c is None:
        break

    # Do not strip trailing spaces, only check for 'empty' lines
    if lineempty.match(c):
        continue
    outfile.writelines(c + '\n')
    outfile.flush()
    if marked.dispatch(c):
        continue
    elif shortcuts.dispatch(c):
        continue
    if c[0] == '#':
        print("Ignoring comment line: " + c)
    elif c[0:9] == 'TestBegin':
        print("\n")
        lyx_pid=get_proc_pid(lyx)
        if lyx_pid != "":
            print("Found running instance(s) of LyX: " + lyx_pid + ": killing them all\n")
            intr_system("killall " + lyx, True)
            time.sleep(0.5)
            intr_system("killall -KILL " + lyx, True)
            time.sleep(0.2)
        print("Starting LyX . . .")
        if lyx_userdir is None:
            intr_system(lyx_exe + c[9:] + "&")
        else:
            intr_system(lyx_exe + " -userdir " + lyx_userdir + " " + c[9:] + "&")
        count = 10
        old_lyx_pid = "-7"
        old_lyx_window_name = None
        print("Waiting for LyX to show up . . .")
        while count > 0:
            lyx_pid=get_proc_pid(lyx)
            if lyx_pid != old_lyx_pid:
                print('lyx_pid=' + lyx_pid)
                old_lyx_pid = lyx_pid
            if lyx_pid != "":
                lyx_window_name=get_proc_win_id(lyx_pid, "")
                if not lyx_window_name is None:
                    if old_lyx_window_name != lyx_window_name:
                        print('lyx_win=' + lyx_window_name, '\n')
                        old_lyx_window_name = lyx_window_name
                    break
            else:
                count = count - 1
            time.sleep(0.5)
        if count <= 0:
            print('Timeout: could not start ' + lyx_exe, '\n')
            sys.stdout.flush()
            failed = True
        else:
            print('lyx_pid: ' + lyx_pid)
            print('lyx_win: ' + lyx_window_name)
            dead_expected = False
            sendKeystring("\C\[Home]", lyx_pid)
    elif c[0:5] == 'Sleep':
        print("Sleeping for " + c[6:] + " seconds")
        time.sleep(float(c[6:]))
    elif c[0:4] == 'Exec':
        cmd = c[5:].rstrip()
        intr_system(cmd)
    elif c == 'Loop':
        outfile.close()
        outfile = open(outfilename + '+', 'w')
        print('Now Looping')
    elif c == 'RaiseLyx':
        print('Raising Lyx')
        RaiseWindow()
    elif c[0:4] == 'KK: ':
        if lyx_exists():
            sendKeystring(c[4:], lyx_pid)
        else:
            ##intr_system('killall lyx; sleep 2 ; killall -9 lyx')
            if lyx_pid is None:
              die(1, 'No path /proc/xxxx/status, exiting')
            else:
              die(1, 'No path /proc/' + lyx_pid + '/status, exiting')
    elif c[0:4] == 'KD: ':
        key_delay = c[4:].rstrip('\n')
        print('Setting DELAY to ' + key_delay)
    elif c == 'Loop':
        RaiseWindow()
        sendKeystring(ResetCommand, lyx_pid)
    elif c[0:6] == 'Assert':
        cmd = c[7:].rstrip()
        result = intr_system(cmd, True)
        failed = failed or (result != 0)
        print("result=" + str(result) + ", failed=" + str(failed))
    elif c[0:15] == 'TestEndWithKill':
        marked.close()
        cmd = c[16:].rstrip()
        if lyx_dead(lyx_pid):
            print("LyX instance not found because of crash or assert !\n")
            failed = True
        else:
            print("    ------------    Forcing kill of lyx instance: " + str(lyx_pid) + "    ------------")
            # This line below is there only to allow lyx to update its log-file
            sendKeystring("\[Escape]", lyx_pid)
            dead_expected = True
            while not lyx_dead(lyx_pid):
                intr_system("kill -9 " + str(lyx_pid), True);
                time.sleep(0.5)
            if cmd != "":
                print("Executing " + cmd)
                result = intr_system(cmd, True)
                failed = failed or (result != 0)
                print("result=" + str(result) + ", failed=" + str(failed))
            else:
                print("failed=" + str(failed))
    elif c[0:7] == 'TestEnd':
         #lyx_other_window_name = None
        if lyx_dead(lyx_pid):
            print("LyX instance not found because of crash or assert !\n")
            marked.close()
            failed = True
        else:
            print("    ------------    Forcing quit of lyx instance: " + str(lyx_pid) + "    ------------")
            # \[Escape]+ should work as RESET focus to main window
            sendKeystring("\[Escape]\[Escape]\[Escape]\[Escape]", lyx_pid)
            # now we should be outside any dialog
            # and so the function lyx-quit should work
            sendKeystring("\Cq", lyx_pid)
            marked.dispatch('CP: action=lyx-quit')
            marked.close()
            time.sleep(0.5)
            dead_expected = True
            is_sleeping = wait_until_lyx_sleeping(lyx_pid)
            if is_sleeping:
                print('wait_until_lyx_sleeping() indicated "sleeping"')
                # For a short time lyx-status is 'sleeping', even if it is nearly dead.
                # Without the wait below, the \[Tab]-char is sent to nirvana
                # causing a 'beep'
                time.sleep(0.5)
                # probably waiting for Save/Discard/Abort, we select 'Discard'
                sendKeystring("\[Tab]\[Return]", lyx_pid)
                lcount = 0
            else:
                lcount = 1
            while not lyx_dead(lyx_pid):
                lcount = lcount + 1
                if lcount > 20:
                    print("LyX still up, killing process and waiting for it to die...\n")
                    intr_system("kill -9 " + str(lyx_pid), True);
                time.sleep(0.5)
        cmd = c[8:].rstrip()
        if cmd != "":
            print("Executing " + cmd)
            result = intr_system(cmd, True)
            failed = failed or (result != 0)
            print("result=" + str(result) + ", failed=" + str(failed))
        else:
            print("failed=" + str(failed))
    elif c[0:4] == 'Lang':
        lang = c[5:].rstrip()
        print("Setting LANG=" + lang)
        os.environ['LANG'] = lang
        os.environ['LC_ALL'] = lang
# If it doesn't exist, create a link <locale_dir>/<country-code>/LC_MESSAGES/lyx<version-suffix>.mo
# pointing to the corresponding .gmo file. Needed to let lyx find the right translation files.
# See http://www.mail-archive.com/lyx-devel@lists.lyx.org/msg165613.html
        idx = lang.rfind(".")
        if idx != -1:
            ccode = lang[0:idx]
        else:
            ccode = lang

        print("Setting LANGUAGE=" + ccode)
        os.environ['LANGUAGE'] = ccode

        idx = lang.find("_")
        if idx != -1:
            short_code = lang[0:idx]
        else:
            short_code = ccode
        lyx_dir = os.popen("dirname \"" + lyx_exe + "\"").read().rstrip()
        if PACKAGE is None:
          # on cmake-build there is no Makefile in this directory
          # so PACKAGE has to be provided
          if os.path.exists(lyx_dir + "/Makefile"):
            print("Executing: grep 'PACKAGE =' " + lyx_dir + "/Makefile | sed -e 's/PACKAGE = \(.*\)/\\1/'")
            lyx_name = os.popen("grep 'PACKAGE =' " + lyx_dir + "/Makefile | sed -e 's/PACKAGE = \(.*\)/\\1/'").read().rstrip()
          else:
            print('Could not determine PACKAGE name needed for translations\n')
            failed = True
        else:
          lyx_name = PACKAGE
        intr_system("mkdir -p " + locale_dir + "/" + ccode + "/LC_MESSAGES")
        intr_system("rm -f " + locale_dir + "/" + ccode + "/LC_MESSAGES/" + lyx_name + ".mo")
        if PO_BUILD_DIR is None:
            if lyx_dir[0:3] == "../":
                rel_dir = "../../" + lyx_dir
            else:
                rel_dir = lyx_dir
            intr_system("ln -s " + rel_dir + "/../po/" + short_code + ".gmo " + locale_dir + "/" + ccode + "/LC_MESSAGES/" + lyx_name + ".mo")
        else:
            intr_system("ln -s " + PO_BUILD_DIR + "/" + short_code + ".gmo " + locale_dir + "/" + ccode + "/LC_MESSAGES/" + lyx_name + ".mo")
    else:
        print("Unrecognised Command '" + c + "'\n")
        failed = True

print("Test case terminated: ", end = '')
if failed:
    die(1,"FAIL")
else:
    die(0, "Ok")
