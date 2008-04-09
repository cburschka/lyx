#! /usr/bin/env python
# -*- coding: utf-8 -*-

# file lyxpak.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# author Enrico Forestieri

# Full author contact details are available in file CREDITS

# This script creates a tar or zip archive with a lyx file and all included
# files (graphics and so on). A zip archive is created only if tar is not
# found in the path. The tar archive is then compressed with gzip or bzip2.

import os, re, string, sys
from sets import Set

# Replace with the actual path to the 1.5.x or 1.6.x lyx2lyx.
# If left undefined and the LyX executable is in the path, the script will
# try to locate lyx2lyx by querying LyX about the system dir.
# Example for *nix:
# lyx2lyx = /usr/share/lyx/lyx2lyx/lyx2lyx
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

    if len(argv) >= 2 and len(argv) <= 3:
        lyxfile = argv[1]
        if not os.path.exists(lyxfile):
            error('File "%s" not found.' % lyxfile)

        # Check that it actually is a LyX document
        input = open(lyxfile, 'rU')
        line = input.readline()
        input.close()
        if not (line and line.startswith('#LyX')):
            error('File "%s" is not a LyX document.' % lyxfile)

        # Either tar or zip must be available
        extlist = ['']
        if os.environ.has_key("PATHEXT"):
            extlist = extlist + os.environ["PATHEXT"].split(os.pathsep)
        path = string.split(os.environ["PATH"], os.pathsep)
        archiver, full_path = find_exe(["tar", "zip"], extlist, path)

        if archiver == "tar":
            ar_cmd = "tar cf"
            ar_name = re_lyxfile.sub(".tar", abspath(lyxfile))
            # Archive will be compressed if either gzip or bzip2 are available
            compress, full_path = find_exe(["gzip", "bzip2"], extlist, path)
            if compress == "gzip":
                ext = ".gz"
            elif compress == "bzip2":
                ext = ".bz2"
        elif archiver == "zip":
            ar_cmd = "zip"
            ar_name = re_lyxfile.sub(".zip", abspath(lyxfile))
            compress = None
        else:
            error("Unable to find neither tar nor zip.")

        if len(argv) == 3:
            outdir = argv[2]
            if not os.path.isdir(outdir):
                error('Error: "%s" is not a directory.' % outdir)
            ar_name = os.path.join(abspath(outdir), os.path.basename(ar_name))
    else:
        error(usage(argv[0]))

    # Try to find the location of the lyx2lyx script
    global lyx2lyx
    if lyx2lyx == None:
        lyx_exe, full_path = find_exe(["lyxc", "lyx"], extlist, path)
        if lyx_exe == None:
            error('Cannot find the LyX executable in the path.')
        else:
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
        incfiles[i] = string.replace(incfiles[i], topdir, '')
        i += 1

    # Remove duplicates and sort the list
    incfiles = list(Set(incfiles))
    incfiles.sort()

    # Build the archive command
    ar_cmd = '%s "%s"' % (ar_cmd, ar_name)
    for file in incfiles:
        print file
        ar_cmd = ar_cmd + ' "' + file + '"'

    # Create the archive
    if topdir != '':
        os.chdir(topdir)
    cmd_status, cmd_stdout = run_cmd(ar_cmd)
    if cmd_status != None:
        error('Failed to create LyX archive "%s"' % ar_name)

    # If possible, compress the archive
    if compress != None:
        compress_cmd = '%s "%s"' % (compress, ar_name)
        cmd_status, cmd_stdout = run_cmd(compress_cmd)
        if cmd_status != None:
            error('Failed to compress LyX archive "%s"' % ar_name)
        ar_name = ar_name + ext

    print 'LyX archive "%s" created successfully.' % ar_name
    return 0


if __name__ == "__main__":
    main(sys.argv)
