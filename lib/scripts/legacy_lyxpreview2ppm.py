#! /usr/bin/env python

# file legacy_lyxpreview2ppm.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# author Angus Leeming

# Full author contact details are available in file CREDITS

# This script converts a LaTeX file to a bunch of ppm files using the
# deprecated dvi->ps->ppm conversion route.

# If possible, please grab 'dvipng'; it's faster and more robust.
# This legacy support will be removed one day...

import glob, os, re, string, sys
import pipes, tempfile


# Pre-compiled regular expressions.
latex_file_re = re.compile("\.tex$")


def usage(prog_name):
    return "Usage: %s <latex file> <dpi> <fg color> <bg color>\n"\
           "\twhere the colors are hexadecimal strings, eg 'faf0e6'"\
           % prog_name


def error(message):
    sys.stderr.write(message + '\n')
    sys.exit(1)


def find_exe(candidates, path):
    for prog in candidates:
        for directory in path:
            full_path = os.path.join(directory, prog)
            if os.access(full_path, os.X_OK):
                return full_path

    return None


def find_exe_or_terminate(candidates, path):
    exe = find_exe(candidates, path)
    if exe == None:
        error("Unable to find executable from '%s'" % string.join(candidates))

    return exe


def run_command(cmd):
    handle = os.popen(cmd, 'r')
    cmd_stdout = handle.read()
    cmd_status = handle.close()

    return cmd_status, cmd_stdout


def extract_metrics_info(log_file, metrics_file):
    metrics = open(metrics_file, 'w')

    log_re = re.compile("Preview: ([ST])")
    data_re = re.compile("(-?[0-9]+) (-?[0-9]+) (-?[0-9]+) (-?[0-9]+)")

    tp_ascent  = 0.0
    tp_descent = 0.0

    success = 0
    for line in open(log_file, 'r').readlines():
        match = log_re.match(line)
        if match == None:
            continue

        snippet = (match.group(1) == 'S')
        success = 1
        match = data_re.search(line)
        if match == None:
            error("Unexpected data in %s\n%s" % (log_file, line))

        if snippet:
            ascent  = string.atof(match.group(2)) + tp_ascent
            descent = string.atof(match.group(3)) - tp_descent

            frac = 0.5
            if abs(ascent + descent) > 0.1:
                frac = ascent / (ascent + descent)

                metrics.write("Snippet %s %f\n" % (match.group(1), frac))

        else:
            tp_descent = string.atof(match.group(2))
            tp_ascent  = string.atof(match.group(4))

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
    fontsize = 0.0

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


def legacy_latex_file(latex_file, fg_color, bg_color):
    use_preview_re = re.compile("(\\\\usepackage\[[^]]+)(\]{preview})")

    tmp = mkstemp()

    success = 0
    for line in open(latex_file, 'r').readlines():
        match = use_preview_re.match(line)
        if match == None:
            tmp.write(line)
            continue

        success = 1
        tmp.write("%s,dvips,tightpage%s\n\n" \
                  "\\AtBeginDocument{\\AtBeginDvi{%%\n" \
                  "\\special{!userdict begin/bop-hook{//bop-hook exec\n" \
                  "<%s%s>{255 div}forall setrgbcolor\n" \
                  "clippath fill setrgbcolor}bind def end}}}\n" \
                  % (match.group(1), match.group(2), fg_color, bg_color))

    if success:
        copyfileobj(tmp, open(latex_file,"wb"), 1)

    return success


def crop_files(pnmcrop, basename):
    t = pipes.Template()
    t.append("%s -left" % pnmcrop, '--')
    t.append("%s -right" % pnmcrop, '--')

    for file in glob.glob("%s*.ppm" % basename):
        tmp = mkstemp()
        new = t.open(file, "r")
        copyfileobj(new, tmp)
        if not new.close():
            copyfileobj(tmp, open(file,"wb"), 1)


def legacy_conversion(argv):
    # Parse and manipulate the command line arguments.
    if len(argv) != 6:
        error(usage(argv[0]))

    # Ignore argv[1]

    dir, latex_file = os.path.split(argv[2])
    if len(dir) != 0:
        os.chdir(dir)

    dpi = string.atoi(argv[3])
    fg_color = argv[4]
    bg_color = argv[5]

    # External programs used by the script.
    path = string.split(os.getenv("PATH"), os.pathsep)
    latex   = find_exe_or_terminate(["pplatex", "latex2e", "latex"], path)
    dvips   = find_exe_or_terminate(["dvips"], path)
    gs      = find_exe_or_terminate(["gswin32", "gs"], path)
    pnmcrop = find_exe(["pnmcrop"], path)

    # Move color information into the latex file.
    if not legacy_latex_file(latex_file, fg_color, bg_color):
        error("Unable to move color info into the latex file")

    # Compile the latex file.
    latex_call = "%s %s" % (latex, latex_file)

    latex_status, latex_stdout = run_command(latex_call)
    if latex_status != None:
        error("%s failed to compile %s" \
              % (os.path.basename(latex), latex_file))

    # Run the dvi file through dvips.
    dvi_file = latex_file_re.sub(".dvi", latex_file)
    ps_file  = latex_file_re.sub(".ps",  latex_file)

    dvips_call = "%s -o %s %s" % (dvips, ps_file, dvi_file)
    
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
    gs_call = "%s -dNOPAUSE -dBATCH -dSAFER -sDEVICE=pnmraw " \
              "-sOutputFile=%s%%d.ppm " \
              "-dGraphicsAlphaBit=%d -dTextAlphaBits=%d " \
              "-r%f %s" \
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
