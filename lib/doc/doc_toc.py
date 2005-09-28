#! /usr/bin/env python
# -*- coding: iso-8859-1 -*-
# This file is part of the LyX Documentation
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
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

# This script creates a "master table of contents" for a set of LyX docs.
# It does so by going through the files and printing out all of the
# chapter, section, and sub(sub)section headings out. (It numbers the
# sections sequentially; hopefully noone's using Section* in the docs.)

import sys
import os

srcdir = os.path.dirname(sys.argv[0])
if srcdir == '':
    srcdir = '.'
sys.path.insert(0, srcdir + "/../lyx2lyx")

import parser_tools
import LyX
import depend

# Specific language information
# info["isoname"] = (language, language_quotes, enconding, TOC_translated)
info = { 'da' : ('danish', 'german', 'latin1', "Indholdsfortegnelse over LyX's dokumentation"),
         'de' : ('german', 'german', 'latin1', "Inhaltsverzeichnis LyX Dokumentation"),
         'fr' : ('french', 'french', 'latin1', "Plan de la documentation"),
         'ru' : ('russian', 'english', 'koi8-r', "LyX Documentation Table of Contents"),
         'sl' : ('slovene', 'german', 'latin2', "Kazalo dokumentacije LyXa"),
         'en' : ('english', 'english', 'latin1', "LyX Documentation Table of Contents")}

def usage(pname):
    print """Usage: %s [lang]

    lang is the language to build the TOC file, if not present use english.
""" % pname


transform_table = {'Title' : 'Section*', 'Chapter': 'Enumerate',
                   'Section':'Enumerate', 'Subsection': 'Enumerate',
                   'Subsubsection' : 'Enumerate'}

def build_from_toc(par_list):
    if not par_list:
        return []

    if par_list[0].name == 'Title':
        par = par_list[0]
        par.name = transform_table[par.name]

        if len(par_list) == 1:
            return par_list

        for i in range(1, len(par_list)):
            if par_list[i].name == 'Title':
                return [par] + \
            		build_from_toc(par_list[1:i]) + \
                        build_from_toc(par_list[i:])

        return [par] + build_from_toc(par_list[1:])

    if par_list[0].name in ('Chapter', 'Section', 'Subsection', 'Subsubsection'):
        return nest_struct(par_list[0].name, par_list)


def nest_struct(name, par_list):
    if par_list[0].name == name:
        par = par_list[0]
        par.name = transform_table[par.name]

        if len(par_list) == 1:
            return par_list

        for i in range(1, len(par_list)):
            if par_list[i].name == name:
                par.child = build_from_toc(par_list[1:i])
                return [par] + build_from_toc(par_list[i:])
        par.child = build_from_toc(par_list[1:])
        return [ par ]


def main(argv):
    if len(argv) > 2:
        usage()
        sys.exit(1)

    # choose language and prefix for files
    if len(argv) == 1:
        lang = "en"
        pref = ""
    else:
        lang = argv[1]
        pref = lang + '_'
        # fallback
        if lang not in info:
            lang = 'en'

    # Determine existing translated documents for that language.
    toc_general = []
    for file in depend.documents(srcdir, pref):
        file = LyX.File(input= file)
        file.convert()
        toc_general.extend(file.get_toc())

    file = LyX.NewFile(output= pref + 'TOC.lyx')
    data = info[lang]
    file.set_header(language = data[0], language_quotes = data[1], inputencoding = data[2])
    body = [ LyX.Paragraph('Title', [data[3]])]
    body.extend(build_from_toc(toc_general))
    file.set_body(body)
    file.write()


if __name__ == "__main__":
    main(sys.argv)
