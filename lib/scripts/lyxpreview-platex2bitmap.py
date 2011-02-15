#! /usr/bin/env python

# This script takes a pLaTeX file and generates a collection of
# png or ppm image files, one per previewed snippet.
# Example usage:
# lyxpreview-platex2bitmap.py ppm 0lyxpreview.tex 128 000000 faf0e6

# This script takes five arguments:
# FORMAT:   The desired output format. 'ppm'.
# TEXFILE:  the name of the .tex file to be converted.
# DPI:      a scale factor, used to ascertain the resolution of the
#           generated image which is then passed to gs.
# FG_COLOR: the foreground color as a hexadecimal string, eg '000000'.
# BG_COLOR: the background color as a hexadecimal string, eg 'faf0e6'.

import sys
from legacy_lyxpreview2ppm import legacy_conversion

def usage(prog_name):
    return "Usage: %s <format> <latex file> <dpi> <fg color> <bg color>\n"\
           "\twhere the colors are hexadecimal strings, eg 'faf0e6'"\
           % prog_name

def main(argv):
    # Parse and manipulate the command line arguments.
    if len(argv) != 6 and len(argv) != 7:
        error(usage(argv[0]))
    # The arguments of legacy_conversion are the same as 
    # those used in LyX 1.3.x, except for the 6th argument.
    # The 7th argument is just ignored, since we use platex always
    vec = [ argv[0], argv[2], argv[3], argv[1], argv[4], argv[5], "platex"]
    return legacy_conversion(vec)

if __name__ == "__main__":
    main(sys.argv)
