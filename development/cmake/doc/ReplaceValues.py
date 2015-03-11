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

Subst = {}  # map of desired substitutions
prog = re.compile("")

def createProg():
    matchingS = "\\b|\\b".join(Subst.keys())
    pattern = "".join(["(.*)(\\b", matchingS, "\\b)(.*)"])
    return re.compile(pattern)

def SubstituteDataInLine(line):
    result = line
    m = prog.match(result)
    if m:
        return "".join([SubstituteDataInLine(m.group(1)),
                        Subst[m.group(2)],
                        SubstituteDataInLine(m.group(3))])
    return line


def SubstituteDataInFile(InFile):
    for line in open(InFile):
        print(SubstituteDataInLine(line[:-1]))

##########################################


args = sys.argv

del args[0] # we don't need the name ot this script
while len(args) > 0:
    arg = args[0]
    entry = args[0].split("=",1)
    if len(entry) == 2:
        key=entry[0]
        val=entry[1]
        if len(key) > 0:
            Subst[key] = val
    else:
        prog = createProg()
        SubstituteDataInFile(args[0])
    del args[0]

