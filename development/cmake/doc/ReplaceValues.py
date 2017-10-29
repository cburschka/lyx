#! /usr/bin/env python

from __future__ import print_function

# file ReplaceValues.py
#
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# author: Kornel Benko, kornel@lyx.org
#
# Syntax: ReplaceValues.py [<var1>=<Subst1> [<var2>=<Subst> ...]] <Inputfile> [<Inputfile> ...]

import sys
import re
import codecs

Subst = {}  # map of desired substitutions
prog = re.compile("")

def createProg():
    matchingS = u"\\b|\\b".join(Subst.keys())
    pattern = u"".join(["(.*)(\\b", matchingS, "\\b)(.*)"])
    return re.compile(pattern)

def SubstituteDataInLine(line):
    m = prog.match(line)
    if m:
        return "".join([SubstituteDataInLine(m.group(1)),
                        Subst[m.group(2)],
                        SubstituteDataInLine(m.group(3))])
    return line


def SubstituteDataInFile(InFile):
    for line in codecs.open(InFile, 'r', 'UTF-8'):
        print(SubstituteDataInLine(line[:-1]))

##########################################

# ensure standard output with UTF8 encoding:
if sys.version_info < (3,0):
    sys.stdout = codecs.getwriter('UTF-8')(sys.stdout)
else:
    sys.stdout = codecs.getwriter('UTF-8')(sys.stdout.buffer)

for arg in sys.argv[1:]:         # skip first arg (name of this script)
    if sys.version_info < (3,0):
        # support non-ASCII characters in arguments
        arg = arg.decode(sys.stdin.encoding or 'UTF-8')
    entry = arg.split("=", 1)
    if len(entry) == 2:
        key, val = entry
        if len(key) > 0:
            Subst[key] = val
    else:
        prog = createProg()
        SubstituteDataInFile(arg)
