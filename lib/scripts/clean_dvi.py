#! /usr/bin/env python

'''
file clean_dvi.py
This file is part of LyX, the document processor.
Licence details can be found in the file COPYING
or at http://www.lyx.org/about/licence.php

author Angus Leeming
Full author contact details are available in the file CREDITS
or at http://www.lyx.org/about/credits.php

Usage:
    python clean_dvi.py infile.dvi outfile.dvi

clean_dvi modifies the input .dvi file so that
dvips and yap (a dvi viewer on Windows) can find
any embedded PostScript files whose names are protected
with "-quotes.

It works by:
1 translating the machine readable .dvi file to human
  readable .dtl form,
2 manipulating any references to external files
3 translating the .dtl file back to .dvi format.

It requires dv2dt and dt2dv from the DTL dviware package
http://www.ctan.org/tex-archive/dviware/dtl/
'''

import os, re, subprocess, sys

def usage(prog_name):
    return 'Usage: %s in.dvi out.dvi\n' \
           % os.path.basename(prog_name)


def warning(message):
    sys.stderr.write(message + '\n')


def error(message):
    sys.stderr.write(message + '\n')
    sys.exit(1)


def manipulated_dtl(data):
    psfile_re = re.compile(r'(special1 +)([0-9]+)( +\'PSfile=")(.*)(" llx=.*)')

    lines = data.split('\n')
    for i in range(len(lines)):
        line = lines[i]
        match = psfile_re.match(line)
        if match != None:
            file = match.group(4)
            filelen = len(file)
            file = file.replace('"', '')
            # Don't forget to update the length of the string too...
            strlen = int(match.group(2)) - (filelen - len(file))

            lines[i] = '%s%d%s%s%s' \
                       % ( match.group(1), strlen, match.group(3),
                           file, match.group(5) )

    return '\n'.join(lines)


def main(argv):
    # First establish that the expected information has
    # been input on the command line and whether the
    # required executables exist.
    if len(argv) != 3:
        error(usage(argv[0]))

    infile  = argv[1]
    outfile = argv[2]

    if not os.path.exists(infile):
        error('Unable to read "%s"\n' % infile)

    # Convert the input .dvi file to .dtl format.
    if os.name == 'nt':
        unix = False
    else:
        unix = True
    dv2dt_call = 'dv2dt "%s"' % infile
    dv2dt_pipe = subprocess.Popen(dv2dt_call, universal_newlines=True, \
        stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, \
        shell=unix, close_fds=unix)
    (dv2dt_stdin, dv2dt_stdout, dv2dt_stderr) = \
        (dv2dt_pipe.stdin, dv2dt_pipe.stdout, dv2dt_pipe.stderr)

    dv2dt_stdin.close()
    dv2dt_data   = dv2dt_stdout.read()
    dv2dt_status = dv2dt_stdout.close()

    if dv2dt_status != None or len(dv2dt_data) == 0:
        dv2dt_err = dv2dt_stderr.read()
        error("Failed: %s\n%s\n" % ( dv2dt_call, dv2dt_err) )

    # Manipulate the .dtl file.
    dtl_data = manipulated_dtl(dv2dt_data)
    if dtl_data == None:
        error("Failed to manipulate the dtl file")

    # Convert this .dtl file back to .dvi format.
    dt2dv_call = 'dt2dv -si "%s"' % outfile
    dt2dv_stdin = os.popen(dt2dv_call, 'w')
    dt2dv_stdin.write(dtl_data)


if __name__ == "__main__":
    main(sys.argv)
