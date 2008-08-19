#! /usr/bin/env python

# file lyxpreview_tools.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# author Angus Leeming
# Full author contact details are available in file CREDITS

# and with much help testing the code under Windows from
#   Paul A. Rubin, rubin@msu.edu.

# A repository of the following functions, used by the lyxpreview2xyz scripts.
# copyfileobj, error, find_exe, find_exe_or_terminate, make_texcolor, mkstemp,
# run_command, warning

import os, re, string, sys, tempfile

use_win32_modules = 0
if os.name == "nt":
    use_win32_modules = 1
    try:
        import pywintypes
        import win32con
        import win32event
        import win32file
        import win32pipe
        import win32process
        import win32security
        import winerror
    except:
        sys.stderr.write("Consider installing the PyWin extension modules "\
                         "if you're irritated by windows appearing briefly.\n")
        use_win32_modules = 0


def warning(message):
    sys.stderr.write(message + '\n')


def error(message):
    sys.stderr.write(message + '\n')
    sys.exit(1)


def make_texcolor(hexcolor, graphics):
    # Test that the input string contains 6 hexadecimal chars.
    hexcolor_re = re.compile("^[0-9a-fA-F]{6}$")
    if not hexcolor_re.match(hexcolor):
        error("Cannot convert color '%s'" % hexcolor)

    red   = float(string.atoi(hexcolor[0:2], 16)) / 255.0
    green = float(string.atoi(hexcolor[2:4], 16)) / 255.0
    blue  = float(string.atoi(hexcolor[4:6], 16)) / 255.0

    if graphics:
        return "%f,%f,%f" % (red, green, blue)
    else:
        return "rgb %f %f %f" % (red, green, blue)


def find_exe(candidates, path):
    for prog in candidates:
        for directory in path:
            if os.name == "nt":
                full_path = os.path.join(directory, prog + ".exe")
            else:
                full_path = os.path.join(directory, prog)

            if os.access(full_path, os.X_OK):
                # The thing is in the PATH already (or we wouldn't
                # have found it). Return just the basename to avoid
                # problems when the path to the executable contains
                # spaces.
                return os.path.basename(full_path)

    return None


def find_exe_or_terminate(candidates, path):
    exe = find_exe(candidates, path)
    if exe == None:
        error("Unable to find executable from '%s'" % string.join(candidates))

    return exe


def run_command_popen(cmd):
    handle = os.popen(cmd, 'r')
    cmd_stdout = handle.read()
    cmd_status = handle.close()

    return cmd_status, cmd_stdout


def run_command_win32(cmd):
    sa = win32security.SECURITY_ATTRIBUTES()
    sa.bInheritHandle = True
    stdout_r, stdout_w = win32pipe.CreatePipe(sa, 0)

    si = win32process.STARTUPINFO()
    si.dwFlags = (win32process.STARTF_USESTDHANDLES
                  | win32process.STARTF_USESHOWWINDOW)
    si.wShowWindow = win32con.SW_HIDE
    si.hStdOutput = stdout_w

    process, thread, pid, tid = \
             win32process.CreateProcess(None, cmd, None, None, True,
                                        0, None, None, si)
    if process == None:
        return -1, ""

    # Must close the write handle in this process, or ReadFile will hang.
    stdout_w.Close()

    # Read the pipe until we get an error (including ERROR_BROKEN_PIPE,
    # which is okay because it happens when child process ends).
    data = ""
    error = 0
    while 1:
        try:
            hr, buffer = win32file.ReadFile(stdout_r, 4096)
            if hr != winerror.ERROR_IO_PENDING:
                data = data + buffer

        except pywintypes.error, e:
            if e.args[0] != winerror.ERROR_BROKEN_PIPE:
                error = 1
            break

    if error:
        return -2, ""

    # Everything is okay --- the called process has closed the pipe.
    # For safety, check that the process ended, then pick up its exit code.
    win32event.WaitForSingleObject(process, win32event.INFINITE)
    if win32process.GetExitCodeProcess(process):
        return -3, ""

    return None, data


def run_command(cmd):
    if use_win32_modules:
        return run_command_win32(cmd)
    else:
        return run_command_popen(cmd)


def get_version_info():
    version_re = re.compile("([0-9])\.([0-9])")

    match = version_re.match(sys.version)
    if match == None:
        error("Unable to extract version info from 'sys.version'")

    return string.atoi(match.group(1)), string.atoi(match.group(2))


def copyfileobj(fsrc, fdst, rewind=0, length=16*1024):
    """copy data from file-like object fsrc to file-like object fdst"""
    if rewind:
        fsrc.flush()
        fsrc.seek(0)

    while 1:
        buf = fsrc.read(length)
        if not buf:
            break
        fdst.write(buf)


class TempFile:
    """clone of tempfile.TemporaryFile to use with python < 2.0."""
    # Cache the unlinker so we don't get spurious errors at shutdown
    # when the module-level "os" is None'd out.  Note that this must
    # be referenced as self.unlink, because the name TempFile
    # may also get None'd out before __del__ is called.
    unlink = os.unlink

    def __init__(self):
        self.filename = tempfile.mktemp()
        self.file = open(self.filename,"w+b")
        self.close_called = 0

    def close(self):
        if not self.close_called:
            self.close_called = 1
            self.file.close()
            self.unlink(self.filename)

    def __del__(self):
        self.close()

    def read(self, size = -1):
        return self.file.read(size)

    def write(self, line):
        return self.file.write(line)

    def seek(self, offset):
        return self.file.seek(offset)

    def flush(self):
        return self.file.flush()


def mkstemp():
    """create a secure temporary file and return its object-like file"""
    major, minor = get_version_info()

    if major >= 2 and minor >= 0:
        return tempfile.TemporaryFile()
    else:
        return TempFile()
