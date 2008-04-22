#! /usr/bin/env python
# -*- coding: utf-8 -*-
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

# This script is called by the script depend.py to create a "master table of contents"
# for a set of LyX docs.
# It does so by going through the files and printing out all of the
# chapter, section, and sub(sub)section headings out. (It numbers the
# sections sequentially; hopefully noone's using Section* in the docs.)
# It is called using this syntax:
# depend.py doc_toc.py SetOfDocuments
# where SetOfDocuments is a set of documents

import sys
import os

if __name__ == "__main__":
    srcdir = os.path.dirname(sys.argv[0])
    if srcdir == '':
        srcdir = '.'
    sys.path.insert(0, srcdir + "/../lyx2lyx")

# when doc_toc is imported by scons, sys.path is set over there
import parser_tools
import LyX
import depend

# Specific language information
# info["isoname"] = (language, language_quotes, TOC_translated)
info = { 'cs' : ('czech', 'german', u"Obsah dokumentace LyXu"),
         'da' : ('danish', 'german', u"Indholdsfortegnelse over LyX's dokumentation"),
         'de' : ('german', 'german', u"Inhaltsverzeichnis LyX Dokumentation"),
         'es' : ('spanish', 'spanish', u"Índice general LyX documentation"),
         'fr' : ('french', 'french', u"Plan de la documentation LyX"),
	 'ja' : ('japanese', 'japanese', u"LyX取扱説明書目次"),
         'ru' : ('russian', 'english', u"LyX Documentation Table of Contents"),
         'sl' : ('slovene', 'german', u"Kazalo dokumentacije LyXa"),
         'en' : ('english', 'english', u"LyX Documentation Table of Contents")}

def usage(pname):
    print """Usage: %s lang output

    lang is the language to build the TOC file,
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


def build_toc(output, documents, lang=None):
    # Determine existing translated documents for that language.
    toc_general = []
    for file in documents:
        file = LyX.File(input= file)
        file.convert()
        toc_general.extend(file.get_toc())

    # if lang is not given, guess from document names. (Used in scons build)
    if lang is None:
        lang = 'en'
        for file in documents:
            dir = file.split(os.sep)[-2]
            if dir in info.keys():
                lang = dir
    file = LyX.NewFile(output = output)
    data = info[lang]
    file.set_header(language = data[0], language_quotes = data[1], inputencoding = "auto")
    file.language = data[0]
    file.encoding = "utf-8"
    body = [ LyX.Paragraph('Title', [data[2]])]
    body.extend(build_from_toc(toc_general))
    file.set_body(body)
    file.write()

    
def main(argv):
    if len(argv) != 3:
        usage(argv[0])
        sys.exit(1)

    lang = argv[1]
    if not os.path.isdir(os.path.join(argv[2], lang)):
        # need to create lang dir if build dir != src dir
        os.mkdir(os.path.join(argv[2], lang))

    # choose language files
    if lang == 'en':
        output = os.path.join(argv[2], 'TOC.lyx')
    else:
        output = os.path.join(argv[2], lang, 'TOC.lyx')
        # fallback
        if lang not in info:
            lang = 'en'

    build_toc(output, depend.documents(srcdir, lang), lang)


if __name__ == "__main__":
    main(sys.argv)
