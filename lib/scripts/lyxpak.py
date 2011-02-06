#! /usr/bin/env python
# -*- coding: utf-8 -*-

# file lyxpak.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# author Enrico Forestieri

# Full author contact details are available in file CREDITS

# This script creates a tar or zip archive with a lyx file and all included
# files (graphics and so on). The created archive is the standard type on a
# given platform, such that a zip archive is created on Windows and a gzip
# compressed tar archive on *nix.

import os, re, string, sys
if sys.version_info < (2, 4, 0):
    from sets import Set as set
if os.name == 'nt':
    import zipfile
else:
    import tarfile

# Replace with the actual path to the 1.5, 1.6, or 2.0 lyx2lyx.
# If left undefined and the LyX executable is in the path, the script will
# try to locate lyx2lyx by querying LyX about the system dir.
# Example for *nix:
# lyx2lyx = "/usr/share/lyx/lyx2lyx/lyx2lyx"
lyx2lyx = None

# Pre-compiled regular expressions.
re_lyxfile = re.compile("\.lyx$")
re_input = re.compile(r'^(.*)\\(input|include){(\s*)(\S+)(\s*)}.*$')
re_package = re.compile(r'^(.*)\\(usepackage){(\s*)(\S+)(\s*)}.*$')
re_class = re.compile(r'^(\\)(textclass)(\s+)(\S+)$')
re_norecur = re.compile(r'^(.*)\\(verbatiminput|lstinputlisting|includegraphics\[*.*\]*){(\s*)(\S+)(\s*)}.*$')
re_filename = re.compile(r'^(\s*)(filename)(\s+)(\S+)$')
re_options = re.compile(r'^(\s*)options(\s+)(\S+)$')
re_bibfiles = re.compile(r'^(\s*)bibfiles(\s+)(\S+)$')


def usage(prog_name):
    return "Usage: %s file.lyx [output_dir]\n" % prog_name


def error(message):
    sys.stderr.write(message + '\n')
    sys.exit(1)


def run_cmd(cmd):
    handle = os.popen(cmd, 'r')
    cmd_stdout = handle.read()
    cmd_status = handle.close()
    return cmd_status, cmd_stdout


def find_exe(candidates, extlist, path):
    for prog in candidates:
        for directory in path:
            for ext in extlist:
                full_path = os.path.join(directory, prog + ext)
                if os.access(full_path, os.X_OK):
                    return prog, full_path
    return None, None


def abspath(name):
    " Resolve symlinks and returns the absolute normalized name."
    newname = os.path.normpath(os.path.abspath(name))
    if os.name != 'nt':
        newname = os.path.realpath(newname)
    return newname


def gather_files(curfile, incfiles):
    " Recursively gather files."
    curdir = os.path.dirname(abspath(curfile))
    is_lyxfile = re_lyxfile.search(curfile)
    if is_lyxfile:
        lyx2lyx_cmd = 'python "%s" "%s"' % (lyx2lyx, curfile)
        l2l_status, l2l_stdout = run_cmd(lyx2lyx_cmd)
        if l2l_status != None:
            error('%s failed to convert "%s"' % (lyx2lyx, curfile))
        lines = l2l_stdout.splitlines()
    else:
        input = open(curfile, 'rU')
        lines = input.readlines()
        input.close()

    i = 0
    while i < len(lines):
        # Gather used files.
        recursive = True
        extlist = ['']
        match = re_filename.match(lines[i])
        if not match:
            match = re_input.match(lines[i])
            if not match:
                match = re_package.match(lines[i])
                extlist = ['.sty']
                if not match:
                    match = re_class.match(lines[i])
                    extlist = ['.cls']
                    if not match:
                        match = re_norecur.match(lines[i])
                        extlist = ['', '.eps', '.pdf', '.png', '.jpg']
                        recursive = False
        if match:
            file = match.group(4).strip('"')
            if not os.path.isabs(file):
                file = os.path.join(curdir, file)
            file_exists = False
            for ext in extlist:
                if os.path.exists(file + ext):
                    file = file + ext
                    file_exists = True
                    break
            if file_exists:
                incfiles.append(abspath(file))
                if recursive:
                    gather_files(file, incfiles)
            i += 1
            continue

        if not is_lyxfile:
            i += 1
            continue

        # Gather bibtex *.bst files.
        match = re_options.match(lines[i])
        if match:
            file = match.group(3).strip('"')
            if not os.path.isabs(file):
                file = os.path.join(curdir, file + '.bst')
            if os.path.exists(file):
                incfiles.append(abspath(file))
            i += 1
            continue

        # Gather bibtex *.bib files.
        match = re_bibfiles.match(lines[i])
        if match:
            bibfiles = match.group(3).strip('"').split(',')
            j = 0
            while j < len(bibfiles):
                if os.path.isabs(bibfiles[j]):
                    file = bibfiles[j]
                else:
                    file = os.path.join(curdir, bibfiles[j] + '.bib')
                if os.path.exists(file):
                    incfiles.append(abspath(file))
                j += 1
            i += 1
            continue

        i += 1

    return 0


def main(argv):

    if len(argv) < 2 and len(argv) > 3:
        error(usage(argv[0]))

    lyxfile = argv[1]
    if not os.path.exists(lyxfile):
        error('File "%s" not found.' % lyxfile)

    outdir = ""
    if len(argv) == 3:
        outdir = argv[2]
        if not os.path.isdir(outdir):
            error('Error: "%s" is not a directory.' % outdir)

    # Check that it actually is a LyX document
    input = open(lyxfile, 'rU')
    line = input.readline()
    input.close()
    if not (line and line.startswith('#LyX')):
        error('File "%s" is not a LyX document.' % lyxfile)

    # Create a tar archive on *nix and a zip archive on Windows
    extlist = ['']
    ar_ext = ".tar.gz"
    if os.name == 'nt':
        ar_ext = ".zip"
        if os.environ.has_key("PATHEXT"):
            extlist = extlist + os.environ["PATHEXT"].split(os.pathsep)

    ar_name = re_lyxfile.sub(ar_ext, abspath(lyxfile))
    if outdir:
        ar_name = os.path.join(abspath(outdir), os.path.basename(ar_name))

    path = string.split(os.environ["PATH"], os.pathsep)

    # Try to find the location of the lyx2lyx script
    global lyx2lyx
    if lyx2lyx == None:
        # first we will see if the script is roughly where we are
        # i.e., we will assume we are in $SOMEDIR/scripts and look
        # for $SOMEDIR/lyx2lyx/lyx2lyx.
        ourpath = os.path.dirname(abspath(argv[0]))
        (upone, discard) = os.path.split(ourpath)
        tryit = os.path.join(upone, "lyx2lyx", "lyx2lyx")
        if os.path.exists(tryit):
            lyx2lyx = tryit
        else:
          lyx_exe, full_path = find_exe(["lyxc", "lyx"], extlist, path)
          if lyx_exe == None:
              error('Cannot find the LyX executable in the path.')
          cmd_status, cmd_stdout = run_cmd("%s -version 2>&1" % lyx_exe)
          if cmd_status != None:
              error('Cannot query LyX about the lyx2lyx script.')
          re_msvc = re.compile(r'^(\s*)(Host type:)(\s+)(win32)$')
          re_sysdir = re.compile(r'^(\s*)(LyX files dir:)(\s+)(\S+)$')
          lines = cmd_stdout.splitlines()
          for line in lines:
              match = re_msvc.match(line)
              if match:
                  # The LyX executable was built with MSVC, so the
                  # "LyX files dir:" line is unusable
                  basedir = os.path.dirname(os.path.dirname(full_path))
                  lyx2lyx = os.path.join(basedir, 'Resources', 'lyx2lyx', 'lyx2lyx')
                  break
              match = re_sysdir.match(line)
              if match:
                  lyx2lyx = os.path.join(match.group(4), 'lyx2lyx', 'lyx2lyx')
                  break
          if not os.access(lyx2lyx, os.X_OK):
              error('Unable to find the lyx2lyx script.')

    # Initialize the list with the specified LyX file and recursively
    # gather all required files (also from child documents).
    incfiles = [abspath(lyxfile)]
    gather_files(lyxfile, incfiles)

    # Find the topmost dir common to all files
    if len(incfiles) > 1:
        topdir = os.path.commonprefix(incfiles)
    else:
        topdir = os.path.dirname(incfiles[0]) + os.path.sep

    # Remove the prefix common to all paths in the list
    i = 0
    while i < len(incfiles):
        incfiles[i] = string.replace(incfiles[i], topdir, '', 1)
        i += 1

    # Remove duplicates and sort the list
    incfiles = list(set(incfiles))
    incfiles.sort()

    if topdir != '':
        os.chdir(topdir)

    # Create the archive
    try:
        if os.name == 'nt':
            zip = zipfile.ZipFile(ar_name, "w", zipfile.ZIP_DEFLATED)
            for file in incfiles:
                zip.write(file)
            zip.close()
        else:
            tar = tarfile.open(ar_name, "w:gz")
            for file in incfiles:
                tar.add(file)
            tar.close()
    except:
        error('Failed to create LyX archive "%s"' % ar_name)

    print 'LyX archive "%s" created successfully.' % ar_name
    return 0


if __name__ == "__main__":
    main(sys.argv)
