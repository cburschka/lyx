#! /usr/bin/env python
# -*- coding: iso-8859-1 -*-

# file lyxpreview2ppm.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# author Angus Leeming
# Full author contact details are available in file CREDITS

# with much advice from members of the preview-latex project:
#   David Kastrup, dak@gnu.org and
#   Jan-Åke Larsson, jalar@mai.liu.se.
# and with much help testing the code under Windows from
#   Paul A. Rubin, rubin@msu.edu.

# This script takes a LaTeX file and generates a collection of
# ppm image files, one per previewed snippet.
# Example usage:
# lyxpreview2ppm.py 0lyxpreview.tex 128 ppm

# The script takes three arguments:
# TEXFILE:       the name of the .tex file to be converted.
# SCALEFACTOR:   a scale factor, used to ascertain the resolution of the
#                generated image which is then passed to gs.
# OUTPUTFORMAT:  the format of the output bitmap image files.
#                This particular script can produce only "ppm" format output.

# Decomposing TEXFILE's name as DIR/BASE.tex, this script will,
# if executed successfully, leave in DIR:
# * a (possibly large) number of image files with names
#   like BASE[0-9]+.ppm
# * a file BASE.metrics, containing info needed by LyX to position
#   the images correctly on the screen.

# The script uses several external programs and files:
# * A latex executable;
# * preview.sty;
# * dvips;
# * gs;
# * pnmcrop (optional).

# preview.sty is part of the preview-latex project
#   http://preview-latex.sourceforge.net/
# Alternatively, it can be obtained from
#   CTAN/support/preview-latex/

import glob, os, re, string, sys
import pipes, tempfile

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


# Pre-compiled regular expression.
latex_file_re = re.compile("\.tex$")


def usage(prog_name):
    return "Usage: %s <latex file> <dpi> ppm\n"\
           % prog_name


def warning(message):
    sys.stderr.write(message + '\n')


def error(message):
    sys.stderr.write(message + '\n')
    sys.exit(1)


def find_exe(candidates, path):
    for prog in candidates:
        for directory in path:
            if os.name == "nt":
                full_path = os.path.join(directory, prog + ".exe")
            else:
                full_path = os.path.join(directory, prog)

            if os.access(full_path, os.X_OK):
                return full_path

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


def extract_metrics_info(log_file, metrics_file):
    metrics = open(metrics_file, 'w')

    log_re = re.compile("Preview: [ST]")

    success = 0
    try:
        for line in open(log_file, 'r').readlines():
            match = log_re.match(line)
            if match != None:
                success = 1
                metrics.write("%s\n" % line)

    except:
        # Unable to open the file, but do nothing here because
        # the calling function will act on the value of 'success'.
        warning('Warning in extract_metrics_info! Unable to open "%s"' % log_file)
        warning(`sys.exc_type` + ',' + `sys.exc_value`)

    return success


def extract_resolution(log_file, dpi):
    fontsize_re = re.compile("Preview: Fontsize")
    magnification_re = re.compile("Preview: Magnification")
    extract_decimal_re = re.compile("([0-9\.]+)")
    extract_integer_re = re.compile("([0-9]+)")

    found_fontsize = 0
    found_magnification = 0

    # Default values
    magnification = 1000.0
    fontsize = 10.0

    try:
        for line in open(log_file, 'r').readlines():
            if found_fontsize and found_magnification:
                break

            if not found_fontsize:
                match = fontsize_re.match(line)
                if match != None:
                    match = extract_decimal_re.search(line)
                    if match == None:
                        error("Unable to parse: %s" % line)
                    fontsize = string.atof(match.group(1))
                    found_fontsize = 1
                    continue

            if not found_magnification:
                match = magnification_re.match(line)
                if match != None:
                    match = extract_integer_re.search(line)
                    if match == None:
                        error("Unable to parse: %s" % line)
                    magnification = string.atof(match.group(1))
                    found_magnification = 1
                    continue

    except:
        warning('Warning in extract_resolution! Unable to open "%s"' % log_file)
        warning(`sys.exc_type` + ',' + `sys.exc_value`)

    # This is safe because both fontsize and magnification have
    # non-zero default values.
    return dpi * (10.0 / fontsize) * (1000.0 / magnification)


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


def crop_files(pnmcrop, basename):
    t = pipes.Template()
    t.append('"%s" -left' % pnmcrop, '--')
    t.append('"%s" -right' % pnmcrop, '--')

    for file in glob.glob("%s*.ppm" % basename):
        tmp = mkstemp()
        new = t.open(file, "r")
        copyfileobj(new, tmp)
        if not new.close():
            copyfileobj(tmp, open(file,"wb"), 1)


def main(argv):
    # Parse and manipulate the command line arguments.
    if len(argv) != 4:
        error(usage(argv[0]))

    dir, latex_file = os.path.split(argv[1])
    if len(dir) != 0:
        os.chdir(dir)

    dpi = string.atoi(argv[2])

    output_format = argv[3]
    if output_format != "ppm":
        error("This script will generate ppm format images only.")

    # External programs used by the script.
    path = string.split(os.environ['PATH'], os.pathsep)
    latex   = find_exe_or_terminate(["pplatex", "latex2e", "latex"], path)
    dvips   = find_exe_or_terminate(["dvips"], path)
    gs      = find_exe_or_terminate(["gswin32", "gs"], path)
    pnmcrop = find_exe(["pnmcrop"], path)

    # Compile the latex file.
    latex_call = '"%s" "%s"' % (latex, latex_file)

    latex_status, latex_stdout = run_command(latex_call)
    if latex_status != None:
        error("%s failed to compile %s" \
              % (os.path.basename(latex), latex_file))

    # Run the dvi file through dvips.
    dvi_file = latex_file_re.sub(".dvi", latex_file)
    ps_file  = latex_file_re.sub(".ps",  latex_file)

    dvips_call = '"%s" -o "%s" "%s"' % (dvips, ps_file, dvi_file)

    dvips_status, dvips_stdout = run_command(dvips_call)
    if dvips_status != None:
        error("Failed: %s %s" % (os.path.basename(dvips), dvi_file))

    # Extract resolution data for gs from the log file.
    log_file = latex_file_re.sub(".log", latex_file)
    resolution = extract_resolution(log_file, dpi)

    # Older versions of gs have problems with a large degree of
    # anti-aliasing at high resolutions
    alpha = 4
    if resolution > 150:
        alpha = 2

    # Generate the bitmap images
    gs_call = '"%s" -dNOPAUSE -dBATCH -dSAFER -sDEVICE=pnmraw ' \
              '-sOutputFile="%s%%d.ppm" ' \
              '-dGraphicsAlphaBit=%d -dTextAlphaBits=%d ' \
              '-r%f "%s"' \
              % (gs, latex_file_re.sub("", latex_file), \
                 alpha, alpha, resolution, ps_file)

    gs_status, gs_stdout = run_command(gs_call)
    if gs_status != None:
        error("Failed: %s %s" % (os.path.basename(gs), ps_file))

    # Crop the images
    if pnmcrop != None:
        crop_files(pnmcrop, latex_file_re.sub("", latex_file))

    # Extract metrics info from the log file.
    metrics_file = latex_file_re.sub(".metrics", latex_file)
    if not extract_metrics_info(log_file, metrics_file):
        error("Failed to extract metrics info from %s" % log_file)

    return 0


if __name__ == "__main__":
    main(sys.argv)
