#!/usr/bin/env python
# -*- coding: utf-8 -*-

# file lyx_pot.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# \author Bo Peng
#
# Full author contact details are available in file CREDITS

# Usage: use
#     lyx_pot.py -h
# to get usage message

# This script will extract translatable strings from input files and write
# to output in gettext .pot format.
#
import sys, os, re, getopt

def relativePath(path, base):
    '''return relative path from top source dir'''
    # full pathname of path
    path1 = os.path.normpath(os.path.realpath(path)).split(os.sep)
    path2 = os.path.normpath(os.path.realpath(base)).split(os.sep)
    if path1[:len(path2)] != path2:
        print "Path %s is not under top source directory" % path
    path3 = os.path.join(*path1[len(path2):]);
    # replace all \ by / such that we get the same comments on Windows and *nix
    path3 = path3.replace('\\', '/')
    return path3


def writeString(outfile, infile, basefile, lineno, string):
    string = string.replace('\\', '\\\\').replace('"', '')
    if string == "":
        return
    print >> outfile, '#: %s:%d\nmsgid "%s"\nmsgstr ""\n' % \
        (relativePath(infile, basefile), lineno, string)


def ui_l10n(input_files, output, base):
    '''Generate pot file from lib/ui/*'''
    output = open(output, 'w')
    Submenu = re.compile(r'^[^#]*Submenu\s+"([^"]*)"')
    Popupmenu = re.compile(r'^[^#]*PopupMenu\s+"[^"]+"\s+"([^"]*)"')
    Toolbar = re.compile(r'^[^#]*Toolbar\s+"[^"]+"\s+"([^"]*)"')
    Item = re.compile(r'[^#]*Item\s+"([^"]*)"')
    TableInsert = re.compile(r'[^#]*TableInsert\s+"([^"]*)"')
    for src in input_files:
        input = open(src)
        for lineno, line in enumerate(input.readlines()):
            if Submenu.match(line):
                (string,) = Submenu.match(line).groups()
                string = string.replace('_', ' ')
            elif Popupmenu.match(line):
                (string,) = Popupmenu.match(line).groups()
            elif Toolbar.match(line):
                (string,) = Toolbar.match(line).groups()
            elif Item.match(line):
                (string,) = Item.match(line).groups()
            elif TableInsert.match(line):
                (string,) = TableInsert.match(line).groups()
            else:
                continue
            string = string.replace('"', '')
            if string != "":
                print >> output, '#: %s:%d\nmsgid "%s"\nmsgstr ""\n' % \
                    (relativePath(src, base), lineno+1, string)
        input.close()
    output.close()


def layouts_l10n(input_files, output, base):
    '''Generate pot file from lib/layouts/*.{layout,inc,module}'''
    out = open(output, 'w')
    Style = re.compile(r'^Style\s+(.*)')
    # include ???LabelString???, but exclude comment lines
    LabelString = re.compile(r'^[^#]*LabelString\S*\s+(.*)')
    GuiName = re.compile(r'\s*GuiName\s+(.*)')
    ListName = re.compile(r'\s*ListName\s+(.*)')
    CategoryName = re.compile(r'\s*Category\s+(.*)')
    NameRE = re.compile(r'DeclareLyXModule.*{(.*)}')
    InsetLayout = re.compile(r'^InsetLayout\s+(.*)')
    DescBegin = re.compile(r'#+\s*DescriptionBegin\s*$')
    DescEnd = re.compile(r'#+\s*DescriptionEnd\s*$')

    for src in input_files:
        readingDescription = False
        descStartLine = -1
        descLines = []
        lineno = 0
        for line in open(src).readlines():
            lineno += 1
            if readingDescription:
                res = DescEnd.search(line)
                if res != None:
                    readingDescription = False
                    desc = " ".join(descLines)
                    writeString(out, src, base, lineno + 1, desc)
                    continue
                descLines.append(line[1:].strip())
                continue
            res = DescBegin.search(line)
            if res != None:
                readingDescription = True
                descStartLine = lineno
                continue
            res = NameRE.search(line)
            if res != None:
                string = res.group(1)
                string = string.replace('\\', '\\\\').replace('"', '')
                if string != "":
                    print >> out, '#: %s:%d\nmsgid "%s"\nmsgstr ""\n' % \
                        (relativePath(src, base), lineno + 1, string)
                continue
            res = Style.search(line)
            if res != None:
                string = res.group(1)
                string = string.replace('_', ' ')
                writeString(out, src, base, lineno, string)
                continue
            res = LabelString.search(line)
            if res != None:
                string = res.group(1)
                writeString(out, src, base, lineno, string)
                continue
            res = GuiName.search(line)
            if res != None:
                string = res.group(1)
                writeString(out, src, base, lineno, string)
                continue
            res = CategoryName.search(line)
            if res != None:
                string = res.group(1)
                writeString(out, src, base, lineno, string)
                continue
            res = ListName.search(line)
            if res != None:
                string = res.group(1)
                writeString(out, src, base, lineno, string)
                continue
            res = InsetLayout.search(line)
            if res != None:
                string = res.group(1)
                string = string.replace('_', ' ')
                writeString(out, src, base, lineno, string)
                continue
    out.close()


def qt4_l10n(input_files, output, base):
    '''Generate pot file from src/frontends/qt4/ui/*.ui'''
    output = open(output, 'w')
    pat = re.compile(r'\s*<string>(.*)</string>')
    prop = re.compile(r'\s*<property.*name.*=.*shortcut')
    for src in input_files:
        input = open(src)
        skipNextLine = False
        for lineno, line in enumerate(input.readlines()):
            # skip the line after <property name=shortcut>
            if skipNextLine:
                skipNextLine = False
                continue
            if prop.match(line):
                skipNextLine = True
                continue
            # get lines that match <string>...</string>
            if pat.match(line):
                (string,) = pat.match(line).groups()
                string = string.replace('&amp;', '&').replace('&lt;', '<').replace('&gt;', '>')
                string = string.replace('\\', '\\\\').replace('"', r'\"')
                print >> output, '#: %s:%d\nmsgid "%s"\nmsgstr ""\n' % \
                    (relativePath(src, base), lineno+1, string) 
        input.close()
    output.close()


def languages_l10n(input_files, output, base):
    '''Generate pot file from lib/language'''
    output = open(output, 'w')
    # assuming only one language file
    reg = re.compile('[\w-]+\s+[\w"]+\s+"([\w \-\(\)]+)"\s+(true|false)\s+[\w-]+\s+\w+\s+"[^"]*"')
    input = open(input_files[0])
    for lineno, line in enumerate(input.readlines()):
        if line[0] == '#':
            continue
        # From:
        #   afrikaans   afrikaans	"Afrikaans"	false  iso8859-15 af_ZA	 ""
        # To:
        #   #: lib/languages:2
        #   msgid "Afrikaans"
        #   msgstr ""
        if reg.match(line):
            print >> output, '#: %s:%d\nmsgid "%s"\nmsgstr ""\n' % \
                (relativePath(input_files[0], base), lineno+1, reg.match(line).groups()[0])
        else:
            print "Error: Unable to handle line:"
            print line
            # No need to abort if the parsing fails (e.g. "ignore" language has no encoding)
            # sys.exit(1)
    input.close()
    output.close()


def external_l10n(input_files, output, base):
    '''Generate pot file from lib/external_templates'''
    output = open(output, 'w')
    Template = re.compile(r'^Template\s+(.*)')
    GuiName = re.compile(r'\s*GuiName\s+(.*)')
    HelpTextStart = re.compile(r'\s*HelpText\s')
    HelpTextSection = re.compile(r'\s*(\S.*)\s*$')
    HelpTextEnd = re.compile(r'\s*HelpTextEnd\s')
    i = -1
    for src in input_files:
        input = open(src)
        inHelp = False
        hadHelp = False
        prev_help_string = ''
        for lineno, line in enumerate(input.readlines()):
            if Template.match(line):
                (string,) = Template.match(line).groups()
            elif GuiName.match(line):
                (string,) = GuiName.match(line).groups()
            elif inHelp:
                if HelpTextEnd.match(line):
                    if hadHelp:
                        print >> output, '\nmsgstr ""\n'
                    inHelp = False
                    hadHelp = False
                    prev_help_string = ''
                elif HelpTextSection.match(line):
                    (help_string,) = HelpTextSection.match(line).groups()
                    help_string = help_string.replace('"', '')
                    if help_string != "" and prev_help_string == '':
                        print >> output, '#: %s:%d\nmsgid ""\n"%s\\n"' % \
                            (relativePath(src, base), lineno+1, help_string)
                        hadHelp = True
                    elif help_string != "":
                        print >> output, '"%s\\n"' % help_string
                    prev_help_string = help_string
            elif HelpTextStart.match(line):
                inHelp = True
                prev_help_string = ''
            else:
                continue
            string = string.replace('"', '')
            if string != "" and not inHelp:
                print >> output, '#: %s:%d\nmsgid "%s"\nmsgstr ""\n' % \
                    (relativePath(src, base), lineno+1, string)
        input.close()
    output.close()


def formats_l10n(input_files, output, base):
    '''Generate pot file from configure.py'''
    output = open(output, 'w')
    GuiName = re.compile(r'.*\Format\s+\S+\s+\S+\s+"([^"]*)"\s+(\S*)\s+.*')
    GuiName2 = re.compile(r'.*\Format\s+\S+\s+\S+\s+([^"]\S+)\s+(\S*)\s+.*')
    input = open(input_files[0])
    for lineno, line in enumerate(input.readlines()):
        label = ""
        labelsc = ""
        if GuiName.match(line):
            label = GuiName.match(line).group(1)
            shortcut = GuiName.match(line).group(2).replace('"', '')
        elif GuiName2.match(line):
            label = GuiName2.match(line).group(1)
            shortcut = GuiName2.match(line).group(2).replace('"', '')
        else:
            continue
        label = label.replace('\\', '\\\\').replace('"', '')
        if shortcut != "":
            labelsc = label + "|" + shortcut
        if label != "":
            print >> output, '#: %s:%d\nmsgid "%s"\nmsgstr ""\n' % \
                (relativePath(input_files[0], base), lineno+1, label)
        if labelsc != "":
            print >> output, '#: %s:%d\nmsgid "%s"\nmsgstr ""\n' % \
                (relativePath(input_files[0], base), lineno+1, labelsc)
    input.close()
    output.close()


Usage = '''
lyx_pot.py [-b|--base top_src_dir] [-o|--output output_file] [-h|--help] -t|--type input_type input_files

where 
    --base:
        path to the top source directory. default to '.'
    --output:
        output pot file, default to './lyx.pot'
    --input_type can be
        ui: lib/ui/*
        layouts: lib/layouts/*
        qt4: qt4 ui files
        languages: file lib/languages
        external: external templates file
        formats: formats predefined in lib/configure.py
'''

if __name__ == '__main__':
    input_type = None
    output = 'lyx.pot'
    base = '.'
    #
    optlist, args = getopt.getopt(sys.argv[1:], 'ht:o:b:',
        ['help', 'type=', 'output=', 'base='])
    for (opt, value) in optlist:
        if opt in ['-h', '--help']:
            print Usage
            sys.exit(0)
        elif opt in ['-o', '--output']:
            output = value
        elif opt in ['-b', '--base']:
            base = value
        elif opt in ['-t', '--type']:
            input_type = value
    if input_type not in ['ui', 'layouts', 'modules', 'qt4', 'languages', 'external', 'formats'] or output is None:
        print 'Wrong input type or output filename.'
        sys.exit(1)
    if input_type == 'ui':
        ui_l10n(args, output, base)
    elif input_type == 'layouts':
        layouts_l10n(args, output, base)
    elif input_type == 'qt4':
        qt4_l10n(args, output, base)
    elif input_type == 'external':
        external_l10n(args, output, base)
    elif input_type == 'formats':
        formats_l10n(args, output, base)
    else:
        languages_l10n(args, output, base)

