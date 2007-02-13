#!/usr/bin/env python
# -*- coding: utf-8 -*-

# file date.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# \author Enrico Forestieri

# Full author contact details are available in file CREDITS.

# Print the system date and time in the given format. See the python
# documentation for available formats (mostly the same as the POSIX std).
# This file is provided because the date command on Windows is not
# POSIX compliant.

import sys
from time import strftime

def main(argv):
    if len(argv) > 2:
        sys.stderr.write('Usage: python date.py [<format>]\n')
        sys.exit(1)

    if len(argv) == 2:
        format = argv[1]
    else:
        format = "%d-%m-%Y"

    print strftime(format)

if __name__ == "__main__":
    main(sys.argv)
