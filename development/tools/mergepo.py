#! /usr/bin/env python
# -*- coding: utf-8 -*-

# file mergepo.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# author Georg Baum

# Full author contact details are available in file CREDITS

# This script takes missing translations from another set of po files and
# merges them into the po files in this source tree.


import os, re, string, sys
import polib
from optparse import OptionParser


# we do unix/windows line trimming ourselves since it can happen that we
# are on unix, but the file has been written on windows or vice versa.
def trim_eol(line):
    " Remove end of line char(s)."
    if line[-2:-1] == '\r':
        return line[:-2]
    elif line[-1:] == '\r' or line[-1:] == '\n':
        return line[:-1]
    else:
        # file with no EOL in last line
        return line


def read(input):
    " Read utf8 input file and strip lineendings."
    lines = list()
    while 1:
        line = input.readline()
        if not line:
            break
        line = trim_eol(line)
        lines.append(line.decode('UTF-8'))
    return lines


def parse_msg(lines):
    " Extracts msgid or msgstr from lines."
    if len(lines) < 1:
        return ''
    i = lines[0].find('"')
    if i < 0:
        return ''
    msg = lines[0][i:].strip('"')
    for i in range(1, len(lines)):
        j = lines[i].find('"')
        if j < 0:
            return ''
        msg = msg + lines[i][j:].strip('"')
    return polib.unescape(msg)


def translate(msgid, msgstr_lines, po2, options):
    msgstr = parse_msg(msgstr_lines)
    if options.overwrite:
        other = po2.find(msgid)
        if not other:
            return 0
        if not other.translated():
            return 0
        if msgstr == other.msgstr:
            return 0
    else:
        if msgstr != '':
            return 0
        other = po2.find(msgid)
        if not other:
            return 0
        if not other.translated():
            return 0
    if options.nonnull and other.msgstr == other.msgid:
        return 0
    msgstr = other.msgstr
    obsolete = (msgstr_lines[0].find('#~') == 0)
    j = msgstr_lines[0].find('"')
    # must not assign to msgstr_lines, because that would not be seen by our caller
    new_lines = polib.wrap(msgstr_lines[0][0:j+1] + polib.escape(msgstr), 76, drop_whitespace = False)
    del msgstr_lines[:]
    for i in range(0, len(new_lines)):
        if i == 0:
            msgstr_lines.append(new_lines[i] + '"')
        elif obsolete:
            msgstr_lines.append('#~ "' + new_lines[i] + '"')
        else:
            msgstr_lines.append('"' + new_lines[i] + '"')
    return 1


def mergepo_polib(target, source, options):
    changed = 0
    po1 = polib.pofile(target)
    po2 = polib.pofile(source)
    if options.overwrite:
        for entry in po1.entries():
            other = po2.find(entry.msgid, include_obsolete_entries=True)
            if not other:
                continue
            if options.nonnull and other.msgstr == other.msgid:
                continue
            if other.translated() and other.msgstr != entry.msgstr:
                entry.msgstr = other.msgstr
                changed = changed + 1
    else:
        for entry in po1.untranslated_entries():
            other = po2.find(entry.msgid, include_obsolete_entries=True)
            if not other:
                continue
            if options.nonnull and other.msgstr == other.msgid:
                continue
            if other.translated():
                entry.msgstr = other.msgstr
                changed = changed + 1
    if changed > 0:
        po1.save(target)
    return changed


def mergepo_minimaldiff(target, source, options):
    changed = 0
    po2 = polib.pofile(source)
    target_enc = polib.detect_encoding(target)
    # for utf8 files we can use our self written parser to minimize diffs,
    # otherwise we need to use polib
    if target_enc != 'UTF-8':
        raise
    po1 = open(target, 'rb')
    oldlines = read(po1)
    po1.close()
    newlines = []
    in_msgid = False
    in_msgstr = False
    msgstr_lines = []
    msgid_lines = []
    msgid = ''
    for line in oldlines:
        if in_msgid:
            if line.find('"') == 0 or line.find('#~ "') == 0:
                msgid_lines.append(line)
            else:
                in_msgid = False
                msgid = parse_msg(msgid_lines)
                newlines.extend(msgid_lines)
                msgid_lines = []
        elif in_msgstr:
            if line.find('"') == 0 or line.find('#~ "') == 0:
                msgstr_lines.append(line)
            else:
                in_msgstr = False
                changed = changed + translate(msgid, msgstr_lines, po2, options)
                newlines.extend(msgstr_lines)
                msgstr_lines = []
                msgid = ''
        if not in_msgid and not in_msgstr:
            if line.find('msgid') == 0 or line.find('#~ msgid') == 0:
                msgid_lines.append(line)
                in_msgid = True
            elif line.find('msgstr') == 0 or line.find('#~ msgstr') == 0:
                if line.find('msgstr[') == 0 or line.find('#~ msgstr[') == 0:
                    # plural forms are not implemented
                    raise
                msgstr_lines.append(line)
                in_msgstr = True
            else:
                newlines.append(line)
    if msgid != '':
        # the file ended with a msgstr
        changed = changed + translate(msgid, msgstr_lines, po2, options)
        newlines.extend(msgstr_lines)
        msgstr_lines = []
        msgid = ''
    if changed > 0:
        # we store .po files with unix line ends in git,
        # so do always write them even on windows
        po1 = open(target, 'wb')
        for line in newlines:
            po1.write(line.encode('UTF-8') + '\n')
    return changed


def mergepo(target, source, options):
    if not os.path.exists(source):
        sys.stderr.write('Skipping %s since %s does not exist.\n' % (target, source))
        return
    if not os.path.exists(target):
        sys.stderr.write('Skipping %s since %s does not exist.\n' % (target, target))
        return
    sys.stderr.write('Merging %s into %s: ' % (source, target))
    try:
        changed = mergepo_minimaldiff(target, source, options)
        sys.stderr.write('Updated %d translations with minimal diff.\n' % changed)
    except:
        changed = mergepo_polib(target, source, options)
        sys.stderr.write('Updated %d translations using polib.\n' % changed)


def main(argv):

    parser = OptionParser(description = """This script reads translations from .po files in the given source directory
and adds all translations that do not already exist to the corresponding .po
files in the target directory. It is recommended to remerge strings from the
source code before running this script. Otherwise translations that are not
yet in the target .po files are not updated.""", usage = "Usage: %prog [options] sourcedir")
    parser.add_option("-t", "--target", dest="target",
                      help="target directory containing .po files. If missing, it is determined from the script location.")
    parser.add_option("-l", "--language", dest="language",
                      help="language for which translations are merged (if missing, all languages are merged)")
    parser.add_option("-o", "--overwrite", action="store_true", dest="overwrite", default=False,
                      help="overwrite existing target translations with source translations (if missing, only new translations are added)")
    parser.add_option("-n", "--nonnull", action="store_true", dest="nonnull", default=False,
                      help="do not update target translations with source translations that are identical to the untranslated text)")
    (options, args) = parser.parse_args(argv)
    if len(args) <= 1:
        parser.print_help()
        return 0

    toolsdir = os.path.dirname(args[0])
    if options.target:
        podir1 = os.path.abspath(options.target)
    else:
        podir1 = os.path.normpath(os.path.join(toolsdir, '../../po'))
    podir2 = os.path.abspath(args[1])

    if options.language:
        name = options.language + '.po'
        mergepo(os.path.join(podir1, name), os.path.join(podir2, name), options)
    else:
        for i in os.listdir(podir1):
            (base, ext) = os.path.splitext(i)
            if ext != ".po":
                continue
            mergepo(os.path.join(podir1, i), os.path.join(podir2, i), options)

    return 0


if __name__ == "__main__":
    main(sys.argv)
