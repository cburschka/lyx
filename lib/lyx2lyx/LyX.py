# This file is part of lyx2lyx
# -*- coding: iso-8859-1 -*-
# Copyright (C) 2002-2004 Dekel Tsur <dekel@lyx.org>, José Matos <jamatos@lyx.org>
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

from parser_tools import get_value, check_token, find_token, find_tokens, find_end_of, find_end_of_inset
import os.path
import gzip
import sys
import re
import string

##
# file format version
#
version = "1.4.0cvs"
default_debug_level = 2
format_re = re.compile(r"(\d)[\.,]?(\d\d)")
fileformat = re.compile(r"\\lyxformat\s*(\S*)")
original_version = re.compile(r"\#LyX (\S*)")

format_relation = [("0_10",  [210], ["0.10.7","0.10"]),
                   ("0_12",  [215], ["0.12","0.12.1","0.12"]),
                   ("1_0_0", [215], ["1.0.0","1.0"]),
                   ("1_0_1", [215], ["1.0.1","1.0.2","1.0.3","1.0.4", "1.1.2","1.1"]),
                   ("1_1_4", [215], ["1.1.4","1.1"]),
                   ("1_1_5", [216], ["1.1.5","1.1.5fix1","1.1.5fix2","1.1"]),
                   ("1_1_6", [217], ["1.1.6","1.1.6fix1","1.1.6fix2","1.1"]),
                   ("1_1_6fix3", [218], ["1.1.6fix3","1.1.6fix4","1.1"]),
                   ("1_2", [220], ["1.2.0","1.2.1","1.2.3","1.2.4","1.2"]),
                   ("1_3", [221], ["1.3.0","1.3.1","1.3.2","1.3.3","1.3.4","1.3.5","1.3"]),
                   ("1_4", range(223,238), ["1.4.0cvs","1.4"])]


def formats_list():
    formats = []
    for version in format_relation:
        for format in version[1]:
            if format not in formats:
                formats.append(format)
    return formats


def get_end_format():
    return format_relation[-1][1][-1]


def get_backend(textclass):
    if textclass == "linuxdoc" or textclass == "manpage":
        return "linuxdoc"
    if textclass[:7] == "docbook":
        return "docbook"
    return "latex"


##
# Class
#
class LyX_Base:
    """This class carries all the information of the LyX file."""
    def __init__(self, end_format = 0, input = "", output = "", error = "", debug = default_debug_level):
        if input and input != '-':
            self.input = self.open(input)
        else:
            self.input = sys.stdin
        if output:
            self.output = open(output, "w")
        else:
            self.output = sys.stdout

        if error:
            self.err = open(error, "w")
        else:
            self.err = sys.stderr

        self.debug = debug

        if end_format:
            self.end_format = self.lyxformat(end_format)
        else:
            self.end_format = get_end_format()

        self.backend = "latex"
        self.textclass = "article"
        self.header = []
        self.body = []


    def warning(self, message, debug_level= default_debug_level):
        if debug_level <= self.debug:
            self.err.write(message + "\n")


    def error(self, message):
        self.warning(message)
        self.warning("Quiting.")
        sys.exit(1)


    def read(self):
        """Reads a file into the self.header and self.body parts"""
        preamble = 0

        while 1:
            line = self.input.readline()
            if not line:
                self.error("Invalid LyX file.")

            line = line[:-1]
            # remove '\r' from line's end, if present
            if line[-1:] == '\r':
                line = line[:-1]

            if check_token(line, '\\begin_preamble'):
                preamble = 1
            if check_token(line, '\\end_preamble'):
                preamble = 0

            if not preamble:
                line = string.strip(line)

            if not line and not preamble:
                break

            self.header.append(line)

        while 1:
            line = self.input.readline()
            if not line:
                break
            # remove '\r' from line's end, if present
            if line[-2:-1] == '\r':
                self.body.append(line[:-2])
            else:
                self.body.append(line[:-1])

        self.textclass = get_value(self.header, "\\textclass", 0)
        self.backend = get_backend(self.textclass)
        self.format  = self.read_format()
        self.language = get_value(self.header, "\\language", 0)
        if self.language == "":
            self.language = "english"
        self.initial_version = self.read_version()


    def write(self):
        self.set_version()
        self.set_format()

        for line in self.header:
            self.output.write(line+"\n")
        self.output.write("\n")
        for line in self.body:
            self.output.write(line+"\n")


    def open(self, file):
        """Transparently deals with compressed files."""

        self.dir = os.path.dirname(os.path.abspath(file))
        try:
            gzip.open(file).readline()
            self.output = gzip.GzipFile("","wb",6,self.output)
            return gzip.open(file)
        except:
            return open(file)


    def lyxformat(self, format):
        result = format_re.match(format)
        if result:
            format = int(result.group(1) + result.group(2))
        else:
            self.error(str(format) + ": " + "Invalid LyX file.")

        if format in formats_list():
            return format

        self.error(str(format) + ": " + "Format not supported.")
        return None


    def read_version(self):
        for line in self.header:
            if line[0] != "#":
                return None

            result = original_version.match(line)
            if result:
                return result.group(1)
        return None


    def set_version(self):
        self.header[0] = "#LyX %s created this file. For more info see http://www.lyx.org/" % version
        if self.header[1][0] == '#':
            del self.header[1]


    def read_format(self):
        for line in self.header:
            result = fileformat.match(line)
            if result:
                return self.lyxformat(result.group(1))
        else:
            self.error("Invalid LyX File.")
        return None


    def set_format(self):
        if self.format <= 217:
            format = str(float(format)/100)
        else:
            format = str(self.format)
        i = find_token(self.header, "\\lyxformat", 0)
        self.header[i] = "\\lyxformat %s" % format


    def set_parameter(self, param, value):
        i = find_token(self.header, '\\' + param, 0)
        if i == -1:
            self.warning(3, 'Parameter not found in the header: %s' % param)
            return
        self.header[i] = '\\%s %s' % (param, str(value))


    def convert(self):
        "Convert from old to new format."
        mode, convertion_chain = self.chain()
        self.warning("convertion chain: " + str(convertion_chain), 3)

        for step in convertion_chain:
            convert_step = getattr(__import__("lyx_" + step), mode)
            convert_step(self)


    def chain(self):
        """ This is where all the decisions related with the convertion are taken"""

        self.start =  self.format
        format = self.format
        correct_version = 0

        for rel in format_relation:
            if self.initial_version in rel[2]:
                if format in rel[1]:
                    initial_step = rel[0]
                    correct_version = 1
                    break

        if not correct_version:
            if format <= 215:
                self.warning("Version does not match file format, discarding it.")
            for rel in format_relation:
                if format in rel[1]:
                    initial_step = rel[0]
                    break
            else:
                # This should not happen, really.
                self.error("Format not supported.")

        # Find the final step
        for rel in format_relation:
            if self.end_format in rel[1]:
                final_step = rel[0]
                break
        else:
            self.error("Format not supported.")

        # Convertion mode, back or forth
        steps = []
        if (initial_step, self.start) < (final_step, self.end_format):
            mode = "convert"
            first_step = 1
            for step in format_relation:
                if  initial_step <= step[0] <= final_step:
                    if first_step and len(step[1]) == 1:
                        first_step = 0
                        continue
                    steps.append(step[0])
        else:
            mode = "revert"
            relation_format = format_relation
            relation_format.reverse()
            last_step = None

            for step in relation_format:
                if  final_step <= step[0] <= initial_step:
                    steps.append(step[0])
                    last_step = step

            if last_step[1][-1] == self.end_format:
                steps.pop()

        return mode, steps


    def get_toc(self, depth = 4):
        " Returns the TOC of a lyx document."
        paragraphs_filter = {'Title' : 0,'Chapter' : 1, 'Section' : 2, 'Subsection' : 3, 'Subsubsection': 4}
        allowed_insets = ['Quotes']

        sections = []
        for section in paragraphs_filter.keys():
            sections.append('\\begin_layout %s' % section)

        toc_par = []
        i = 0
        while 1:
            i = find_tokens(self.body, sections, i)
            if i == -1:
                break

            j = find_end_of(self.body,  i + 1, '\\begin_layout', '\\end_layout')
            if j == -1:
                self.warning('Incomplete file.', 0)
                break

            section = string.split(self.body[i])[1]
            if section[-1] == '*':
                section = section[:-1]

            par = []

            k = i + 1
            # skip paragraph parameters
            while not self.body[k] or self.body[k][0] == '\\':
                k = k +1

            while k < j:
                if check_token(self.body[k], '\\begin_inset'):
                    inset = string.split(self.body[k])[1]
                    end = find_end_of_inset(self.body, k)
                    if end == -1 or end > j:
                        self.warning('Malformed file.', 0)

                    if inset in allowed_insets:
                        par.extend(self.body[k: end+1])
                    k = end + 1
                else:
                    par.append(self.body[k])
                    k = k + 1

            # trim empty lines in the end.
            while string.strip(par[-1]) == '' and par:
                par.pop()

            toc_par.append(Paragraph(section, par))

            i = j + 1

        return toc_par


class File(LyX_Base):
    def __init__(self, end_format = 0, input = "", output = "", error = "", debug = default_debug_level):
        LyX_Base.__init__(self, end_format, input, output, error, debug)
        self.read()


class NewFile(LyX_Base):
    def set_header(self, **params):
        # set default values
        self.header.extend([
            "#LyX xxxx created this file. For more info see http://www.lyx.org/",
            "\\lyxformat xxx",
            "\\begin_document",
            "\\begin_header",
            "\\textclass article",
            "\\language english",
            "\\inputencoding auto",
            "\\fontscheme default",
            "\\graphics default",
            "\\paperfontsize default",
            "\\papersize default",
            "\\paperpackage none",
            "\\use_geometry false",
            "\\use_amsmath 1",
            "\\cite_engine basic",
            "\\use_bibtopic false",
            "\\paperorientation portrait",
            "\\secnumdepth 3",
            "\\tocdepth 3",
            "\\paragraph_separation indent",
            "\\defskip medskip",
            "\\quotes_language english",
            "\\quotes_times 2",
            "\\papercolumns 1",
            "\\papersides 1",
            "\\paperpagestyle default",
            "\\tracking_changes false",
            "\\end_header"])

        self.format = get_end_format()
        for param in params:
            self.set_parameter(param, params[param])


    def set_body(self, paragraphs):
        self.body.extend(['\\begin_body',''])

        for par in paragraphs:
            self.body.extend(par.asLines())

        self.body.extend(['','\\end_body', '\\end_document'])


class Paragraph:
    def __init__(self, name, body=[], settings = [], child = []):
        self.name = name
        self.body = body
        self.settings = settings
        self.child = child

    def asLines(self):
        result = ['','\\begin_layout %s' % self.name]
        result.extend(self.settings)
        result.append('')
        result.extend(self.body)
        result.append('\\end_layout')

        if not self.child:
            return result

        result.append('\\begin_deeper')
        for node in self.child:
            result.extend(node.asLines())
        result.append('\\end_deeper')

        return result
