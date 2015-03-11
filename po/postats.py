#! /usr/bin/env python
# -*- coding: utf-8 -*-
# Copyright (C) 2007 Michael Gerz <michael.gerz@teststep.org>
# Copyright (C) 2007 José Matos <jamatos@lyx.org>
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
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

"""
This script extracts some information from the po file headers (last
translator, revision date), generates the corresponding gmo files
to retrieve the number of translated/fuzzy/untranslated messages,
and generates a PHP web page.

Invocation:
   postats.py lyx_version po_files > "pathToWebPages"/i18n.inc
"""
from __future__ import print_function

# modify this when you change branch
# Note that an empty lyx_branch variable (ie svn trunk)
# will "do the right thing".
lyx_branch=""
# these po-files will be skipped:
ommitted = ('en.po')

import os
import sys

# Reset the locale
import locale
locale.setlocale(locale.LC_ALL, 'C') 
os.environ['LC_ALL'] = 'C'

def extract_number(line, issues, prop):
    """
    line is a string like
    '588 translated messages, 1248 fuzzy translations, 2 untranslated messages.'
    Any one of these substrings may not appear if the associated number is 0.

    issues is the set of words following the number to be extracted,
    ie, 'translated', 'fuzzy', or 'untranslated'.

    extract_number returns a list with those numbers, or sets it to
    zero if the word is not found in the string.
    """

    for issue in issues:
        i = line.find(issue)

        if i == -1:
            prop[issue] = 0
        else:
            prop[issue] = int(line[:i].split()[-1])


def read_pofile(pofile):
    """ Read the header of the pofile and return it as a dictionary"""
    header = {}
    read_header = False
    for line in open(pofile):
        line = line[:-1]
        if line[:5] == 'msgid':
            if read_header:
                break
            read_header = True
            continue

        if not line or line[0] == '#' or line == 'msgstr ""' or not read_header:
            continue

        line = line.strip('"')
        args = line.split(': ')
        if len(args) == 1:
            continue
        header[args[0]] = args[1].strip()[:-2]

    return header


def run_msgfmt(pofile):
    """ pofile is the name of the po file.
 The function runs msgfmt on it and returns corresponding php code.
"""
    if not pofile.endswith('.po'):
        print("%s is not a po file" % pofile, file=sys.stderr)
        sys.exit(1)

    dirname = os.path.dirname(pofile)
    gmofile = pofile.replace('.po', '.gmo')

    header = read_pofile(pofile)
    charset= header['Content-Type'].split('charset=')[1]

    # po file properties
    prop = {}
    prop["langcode"] = os.path.basename(pofile)[:-3]
    prop["date"] = header['PO-Revision-Date'].split()[0]
    prop["email"] = header['Last-Translator'].split('<')[1][:-1]
    prop["email"] = prop["email"].replace("@", " () ")
    prop["email"] = prop["email"].replace(".", " ! ")
    translator = header['Last-Translator'].split('<')[0].strip()
    try:
        prop["translator"] = translator.decode(charset).encode('ascii','xmlcharrefreplace')
    except LookupError:
        prop["translator"] = translator

    p_in, p_out = os.popen4("msgfmt --statistics -o %s %s" % (gmofile, pofile))
    extract_number(p_out.readline(),
                   ('translated', 'fuzzy', 'untranslated'),
                   prop)
    return """
array ( 'langcode' => '%(langcode)s', "date" => "%(date)s",
"msg_tr" => %(translated)d, "msg_fu" => %(fuzzy)d, "msg_nt" => %(untranslated)d,
"translator" => "%(translator)s", "email" => "%(email)s")""" % prop


if __name__ == "__main__":
    if lyx_branch:
        branch_tag = lyx_branch
    else:
        branch_tag = "master"


    print("""<?php
// The current version
$lyx_version = "%s";
// The branch tag
$branch_tag = "%s";

// The data itself
$podata = array (%s
)?>""" % (sys.argv[1], branch_tag, ",".join([run_msgfmt(po) for po in sys.argv[2:] if po not in ommitted])))
