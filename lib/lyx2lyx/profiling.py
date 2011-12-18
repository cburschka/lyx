#! /usr/bin/env python
# -*- coding: utf-8 -*-
# Copyright (C) 2004 José Matos <jamatos@lyx.org>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

# We need all this because lyx2lyx does not have the .py termination
import imp
lyx2lyx = imp.load_source("lyx2lyx", "lyx2lyx", open("lyx2lyx"))

# Profiler used in the study
import hotshot, hotshot.stats

import sys
import os

"""
This program profiles lyx2lyx.
Usage:
        ./profiling.py option_to_lyx2lyx

Example:
        ./profiling.py -ou.lyx ../doc/UserGuide.lyx
"""

def main():
    # This will only work with python >= 2.2, the version where this module was added
    prof = hotshot.Profile("lyx2lyx.prof") # Use temporary file, here?
    benchtime = prof.runcall(lyx2lyx.main)
    prof.close()

    # After the tests, show the profile analysis.
    stats = hotshot.stats.load("lyx2lyx.prof")
    stats.strip_dirs()
    stats.sort_stats('time', 'calls')
    stats.print_stats(20)

    os.unlink("lyx2lyx.prof")


if __name__ == "__main__":
    main()
