
# file lyxpreview_tools.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# author Angus Leeming
# Full author contact details are available in file CREDITS

# and with much help testing the code under Windows from
#   Paul A. Rubin, rubin@msu.edu.

# A repository of the following functions, used by the lyxpreview2xyz scripts.
# copyfileobj, error, find_exe, find_exe_or_terminate, make_texcolor, mkstemp,
# progress, run_command, run_latex, warning

# Requires python 2.4 or later (subprocess module).

import os, re, string, subprocess, sys, tempfile


# Control the output to stdout
debug = False
verbose = False

# Known flavors of latex and bibtex
bibtex_commands = ("bibtex", "bibtex8", "biber")
latex_commands = ("latex", "pplatex", "platex", "latex2e")
pdflatex_commands = ("pdflatex", "xelatex", "lualatex")

# Pre-compiled regular expressions
latex_file_re = re.compile(r"\.tex$")

# PATH and PATHEXT environment variables
path = os.environ["PATH"].split(os.pathsep)
extlist = ['']
if "PATHEXT" in os.environ:
    extlist += os.environ["PATHEXT"].split(os.pathsep)
extlist.append('.py')

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
        sys.stderr.write("Consider installing the PyWin extension modules " \
                         "if you're irritated by windows appearing briefly.\n")
        use_win32_modules = 0


def progress(message):
    global verbose
    if verbose:
        sys.stdout.write("Progress: %s\n" % message)


def warning(message):
    sys.stderr.write("Warning: %s\n" % message)


def error(message):
    sys.stderr.write("Error: %s\n" % message)
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


def find_exe(candidates):
    global extlist, path

    for command in candidates:
        prog = command.split()[0]
        for directory in path:
            for ext in extlist:
                full_path = os.path.join(directory, prog + ext)
                if os.access(full_path, os.X_OK):
                    # The thing is in the PATH already (or we wouldn't
                    # have found it). Return just the basename to avoid
                    # problems when the path to the executable contains
                    # spaces.
                    if full_path.lower().endswith('.py'):
                        return command.replace(prog, '"%s" "%s"'
                            % (sys.executable, full_path))
                    return command

    return None


def find_exe_or_terminate(candidates):
    exe = find_exe(candidates)
    if exe == None:
        error("Unable to find executable from '%s'" % string.join(candidates))

    return exe


def run_command_popen(cmd, stderr2stdout):
    if os.name == 'nt':
        unix = False
    else:
        unix = True
    if stderr2stdout:
        pipe = subprocess.Popen(cmd, shell=unix, close_fds=unix, stdin=subprocess.PIPE, \
            stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=True, env=os.environ)
        cmd_stdout = pipe.communicate()[0]
    else:
        pipe = subprocess.Popen(cmd, shell=unix, close_fds=unix, stdin=subprocess.PIPE, \
            stdout=subprocess.PIPE, universal_newlines=True, env=os.environ)
        (cmd_stdout, cmd_stderr) = pipe.communicate()
        if cmd_stderr:
            sys.stderr.write(cmd_stderr)
    cmd_status = pipe.returncode

    global debug
    if debug:
        sys.stdout.write(cmd_stdout)
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

    global debug
    if debug:
        sys.stdout.write(data)
    return 0, data


def run_command(cmd, stderr2stdout = True):
    progress("Running %s" % cmd)
    if use_win32_modules:
        return run_command_win32(cmd)
    else:
        return run_command_popen(cmd, stderr2stdout)


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

def write_metrics_info(metrics_info, metrics_file):
    metrics = open(metrics_file, 'w')
    for metric in metrics_info:
        metrics.write("Snippet %s %f\n" % metric)
    metrics.close()

# Reads a .tex files and create an identical file but only with
# pages whose index is in pages_to_keep
def filter_pages(source_path, destination_path, pages_to_keep):
    source_file = open(source_path, "r")
    destination_file = open(destination_path, "w")

    page_index = 0
    skip_page = False
    for line in source_file:
        # We found a new page
        if line.startswith("\\begin{preview}"):
            page_index += 1
            # If the page index isn't in pages_to_keep we don't copy it
            skip_page = page_index not in pages_to_keep

        if not skip_page:
            destination_file.write(line)

        # End of a page, we reset the skip_page bool
        if line.startswith("\\end{preview}"):
            skip_page = False

    destination_file.close()
    source_file.close()

# Joins two metrics list, that is a list of tuple (page_index, metric)
# new_page_indexes contains the original page number of the pages in new_metrics
# e.g. new_page_indexes[3] == 14 means that the 4th item in new_metrics is the 15th in the original counting
# original_bitmap and destination_bitmap are file name models used to rename the new files
# e.g. image_new%d.png and image_%d.png
def join_metrics_and_rename(original_metrics, new_metrics, new_page_indexes, original_bitmap, destination_bitmap):
    legacy_index = 0
    for (index, metric) in new_metrics:
        # If the file exists we rename it
        if os.path.isfile(original_bitmap % (index)):
            os.rename(original_bitmap % (index), destination_bitmap % new_page_indexes[index-1])

        # Extract the original page index
        index = new_page_indexes[index-1]
        # Goes through the array until the end is reached or the correct index is found
        while legacy_index < len(original_metrics) and original_metrics[legacy_index][0] < index:
            legacy_index += 1

        # Add or update the metric for this page
        if legacy_index < len(original_metrics) and original_metrics[legacy_index][0] == index:
            original_metrics[legacy_index] = (index, metric)
        else:
            original_metrics.insert(legacy_index, (index, metric))


def run_latex(latex, latex_file, bibtex = None):
    # Run latex
    latex_status, latex_stdout = run_tex(latex, latex_file)

    if bibtex is None:
        return latex_status, latex_stdout

    # The aux and log output file names
    aux_file = latex_file_re.sub(".aux", latex_file)
    log_file = latex_file_re.sub(".log", latex_file)

    # Run bibtex/latex if necessary
    progress("Checking if a bibtex run is necessary")
    if string_in_file(r"\bibdata", aux_file):
        bibtex_status, bibtex_stdout = run_tex(bibtex, aux_file)
        latex_status, latex_stdout = run_tex(latex, latex_file)
    # Rerun latex if necessary
    progress("Checking if a latex rerun is necessary")
    if string_in_file("Warning: Citation", log_file):
        latex_status, latex_stdout = run_tex(latex, latex_file)

    return latex_status, latex_stdout


def run_tex(tex, tex_file):
    tex_call = '%s "%s"' % (tex, tex_file)

    tex_status, tex_stdout = run_command(tex_call)
    if tex_status:
        warning("%s had problems compiling %s" \
            % (os.path.basename(tex), tex_file))
    return tex_status, tex_stdout


def string_in_file(string, infile):
    if not os.path.isfile(infile):
        return False
    f = open(infile, 'r')
    for line in f.readlines():
        if string in line:
            f.close()
            return True
    f.close()
    return False


# Returns a list of indexes of pages giving errors extracted from the latex log
def check_latex_log(log_file):

    error_re = re.compile("^! ")
    snippet_re = re.compile("^Preview: Snippet ")
    data_re = re.compile("([0-9]+) ([0-9]+) ([0-9]+) ([0-9]+)")

    found_error = False
    error_pages = []

    try:
        for line in open(log_file, 'r').readlines():
            if not found_error:
                match = error_re.match(line)
                if match != None:
                    found_error = True
                continue
            else:
                match = snippet_re.match(line)
                if match == None:
                    continue

                found_error = False
                match = data_re.search(line)
                if match == None:
                    error("Unexpected data in %s\n%s" % (log_file, line))

                error_pages.append(int(match.group(1)))

    except:
        warning('check_latex_log: Unable to open "%s"' % log_file)
        warning(`sys.exc_type` + ',' + `sys.exc_value`)

    return error_pages
