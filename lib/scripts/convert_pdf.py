# -*- coding: utf-8 -*-

# file convert_pdf.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# author Georg Baum
# Full author contact details are available in file CREDITS

# This script takes a PS or PDF file and creates a low resolution version.
# Example usage:
# convert_pdf.py big.pdf small.pdf ebook

# This script takes three arguments:
# INFILE:        the name of the .ps or .pdf file to be converted.
# OUTFILE:       the name of the .pdf file to be created.
# PDFSETTINGS:   any PDFSETTINGS supported by ghostscript:


import sys

from lyxpreview_tools import error, find_exe_or_terminate, run_command


def usage(prog_name):
    return "Usage: %s <ps or pdf input file> <pdf output file> <screen|ebook|printer|prepress>" \
        % prog_name


def main(argv):

    if len(argv) == 4:
        source = argv[1]
        output = argv[2]
        pdfsettings = argv[3]
    else:
        error(usage(argv[0]))

    gs = find_exe_or_terminate(["gswin32c", "gswin64c", "gs"])
    gs_call = '%s -dNOPAUSE -dBATCH -dSAFER -sDEVICE=pdfwrite ' \
              '-dCompatibilityLevel=1.4 -dPDFSETTINGS=/%s ' \
              '-sOutputFile="%s" "%s"' % (gs, pdfsettings, output, source)

    gs_status, gs_stdout = run_command(gs_call)
    if gs_stdout:
        sys.stdout.write(gs_stdout)
    return gs_status


if __name__ == "__main__":
    sys.exit(main(sys.argv))
