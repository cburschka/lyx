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

version_lyx2lyx = "1.4.0cvs"
default_debug_level = 2

# Regular expressions used
format_re = re.compile(r"(\d)[\.,]?(\d\d)")
fileformat = re.compile(r"\\lyxformat\s*(\S*)")
original_version = re.compile(r"\#LyX (\S*)")

##
# file format information:
#  file, supported formats, stable release versions
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
                   ("1_4", range(223,239), ["1.4.0cvs","1.4"])]


def formats_list():
    " Returns a list with supported file formats."
    formats = []
    for version in format_relation:
        for format in version[1]:
            if format not in formats:
                formats.append(format)
    return formats


def get_end_format():
    " Returns the more recent file format available."
    return format_relation[-1][1][-1]


def get_backend(textclass):
    " For _textclass_ returns its backend."
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
        """Arguments:
        end_format: final format that the file should be converted. (integer)
        input: the name of the input source, if empty resort to standard input.
        output: the name of the output file, if empty use the standard output.
        error: the name of the error file, if empty use the standard error.
        debug: debug level, O means no debug, as its value increases be more verbose.
        """
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
        " Emits warning to self.error, if the debug_level is less than the self.debug."
        if debug_level <= self.debug:
            self.err.write(message + "\n")


    def error(self, message):
        " Emits a warning and exist incondicionally."
        self.warning(message)
        self.warning("Quiting.")
        sys.exit(1)


    def read(self):
        """Reads a file into the self.header and self.body parts, from self.input."""
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
        " Writes the LyX file to self.output."
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
        " Returns the file format representation, an integer."
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
        """ Searchs for clues of the LyX version used to write the file, returns the
        most likely value, or None otherwise."""
        for line in self.header:
            if line[0] != "#":
                return None

            result = original_version.match(line)
            if result:
                return result.group(1)
        return None


    def set_version(self):
        " Set the header with the version used."
        self.header[0] = "#LyX %s created this file. For more info see http://www.lyx.org/" % version_lyx2lyx
        if self.header[1][0] == '#':
            del self.header[1]


    def read_format(self):
        " Read from the header the fileformat of the present LyX file."
        for line in self.header:
            result = fileformat.match(line)
            if result:
                return self.lyxformat(result.group(1))
        else:
            self.error("Invalid LyX File.")
        return None


    def set_format(self):
        " Set the file format of the file, in the header."
        if self.format <= 217:
            format = str(float(self.format)/100)
        else:
            format = str(self.format)
        i = find_token(self.header, "\\lyxformat", 0)
        self.header[i] = "\\lyxformat %s" % format


    def set_parameter(self, param, value):
        " Set the value of the header parameter."
        i = find_token(self.header, '\\' + param, 0)
        if i == -1:
            self.warning(3, 'Parameter not found in the header: %s' % param)
            return
        self.header[i] = '\\%s %s' % (param, str(value))


    def convert(self):
        "Convert from current (self.format) to self.end_format."
        mode, convertion_chain = self.chain()
        self.warning("convertion chain: " + str(convertion_chain), 3)

        for step in convertion_chain:
            steps = getattr(__import__("lyx_" + step), mode)

            if not steps:
                    self.error("The convertion to an older format (%s) is not implemented." % self.format)

            if len(steps) == 1:
                version, table = steps[0]
                for conv in table:
                    conv(self)
                self.format = version
                continue

            for version, table in steps:
                if self.format >= version and mode == "convert":
                    continue
                if self.format <= version and mode == "revert":
                    continue
                for conv in table:
                    conv(self)
                self.format = version
                if self.end_format == self.format:
                    return


    def chain(self):
        """ This is where all the decisions related with the convertion are taken.
        It returns a list of modules needed to convert the LyX file from
        self.format to self.end_format"""

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
        " Returns the TOC of this LyX document."
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
    " This class reads existing LyX files."
    def __init__(self, end_format = 0, input = "", output = "", error = "", debug = default_debug_level):
        LyX_Base.__init__(self, end_format, input, output, error, debug)
        self.read()


class NewFile(LyX_Base):
    " This class is to create new LyX files."
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
    # unfinished implementation, it is missing the Text and Insets representation.
    " This class represents the LyX paragraphs."
    def __init__(self, name, body=[], settings = [], child = []):
        """ Parameters:
        name: paragraph name.
        body: list of lines of body text.
        child: list of paragraphs that descend from this paragraph.
        """
        self.name = name
        self.body = body
        self.settings = settings
        self.child = child

    def asLines(self):
        " Converts the paragraph to a list of strings, representing it in the LyX file."
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


class Inset:
    " This class represents the LyX insets."
    pass


class Text:
    " This class represents simple chuncks of text."
    pass
