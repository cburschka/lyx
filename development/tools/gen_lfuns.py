#! /usr/bin/env python
# -*- coding: utf-8 -*-

# file gen_lfuns.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# author Ewan Davies

# Full author contact details are available in file CREDITS

# Usage:
# gen_lfuns.py <path/to/LyXAction.cpp> <where/to/save/LFUNs.lyx>

import sys,re,os.path
import io

def error(message):
    sys.stderr.write(message + '\n')
    sys.exit(1)

def usage(prog_name):
    return "Usage: %s <path/to/LyXAction.cpp> [<where/to/save/LFUNs.lyx>]" % prog_name

DOXYGEN_START = "/*!"
DOXYGEN_END = "},"

LYX_NEWLINE = u"\n\\begin_inset Newline newline\n\\end_inset\n\n"
LYX_BACKSLASH = u"\n\\backslash\n"

HTMLONLY_START = u"\\htmlonly"
HTMLONLY_END = u"\\endhtmlonly"
LFUN_NAME_ID = u"\\var lyx::FuncCode lyx::"
LFUN_ACTION_ID = u"\\li Action: "
LFUN_NOTION_ID = u"\\li Notion: "
LFUN_SYNTAX_ID = u"\\li Syntax: "
LFUN_PARAMS_ID = u"\\li Params: "
LFUN_SAMPLE_ID = u"\\li Sample: "
LFUN_ORIGIN_ID = u"\\li Origin: "
LFUN_ENDVAR = u"\\endvar"

ID_DICT = dict(name=LFUN_NAME_ID, action=LFUN_ACTION_ID, notion=LFUN_NOTION_ID, 
                syntax=LFUN_SYNTAX_ID, params=LFUN_PARAMS_ID, sample=LFUN_SAMPLE_ID, origin=LFUN_ORIGIN_ID)

LFUNS_HEADER = u"""# gen_lfuns.py generated this file. For more info see http://www.lyx.org/
\\lyxformat 506
\\begin_document
\\begin_header
\\save_transient_properties true
\\origin /systemlyxdir/doc/
\\textclass article
\\begin_preamble
\\renewcommand{\\descriptionlabel}[1]{\\hspace\\labelsep\\upshape\\bfseries #1:}
\\renewenvironment{description}{\\list{}{%
  \\setlength{\\itemsep}{-2pt}
  \\advance\\leftmargini6\\p@ \\itemindent-12\\p@
  \\labelwidth\\z@ \\let\\makelabel\\descriptionlabel}%
}{
  \\endlist
}
\\end_preamble
\\use_default_options false
\\maintain_unincluded_children false
\\begin_local_layout
Style Description
LabelIndent           MM
LeftMargin            MMMMMxx
End
\\end_local_layout
\\language english
\\language_package default
\\inputencoding auto
\\fontencoding global
\\font_roman "default" "default"
\\font_sans "default" "default"
\\font_typewriter "default" "default"
\\font_math "auto" "auto"
\\font_default_family default
\\use_non_tex_fonts false
\\font_sc false
\\font_osf false
\\font_sf_scale 100 100
\\font_tt_scale 100 100
\\graphics default
\\default_output_format default
\\output_sync 0
\\bibtex_command default
\\index_command default
\\paperfontsize default
\\spacing single
\\use_hyperref false
\\papersize default
\\use_geometry true
\\use_package amsmath 1
\\use_package amssymb 1
\\use_package cancel 0
\\use_package esint 1
\\use_package mathdots 0
\\use_package mathtools 0
\\use_package mhchem 1
\\use_package stackrel 0
\\use_package stmaryrd 0
\\use_package undertilde 0
\\cite_engine basic
\\cite_engine_type default
\\biblio_style plain
\\use_bibtopic false
\\use_indices false
\\paperorientation portrait
\\suppress_date false
\\justification true
\\use_refstyle 0
\\index Index
\\shortcut idx
\\color #008000
\\end_index
\\leftmargin 2.5cm
\\topmargin 2cm
\\rightmargin 3cm
\\bottommargin 2.5cm
\\secnumdepth 3
\\tocdepth 3
\\paragraph_separation indent
\\paragraph_indentation default
\\quotes_language english
\\papercolumns 1
\\papersides 1
\\paperpagestyle default
\\tracking_changes false
\\output_changes false
\\html_math_output 0
\\html_css_as_file 0
\\html_be_strict false
\\end_header

\\begin_body

\\begin_layout Title
\\SpecialChar LyX
 Functions (LFUNs)
\\end_layout

\\begin_layout Author
The \\SpecialChar LyX
 Team
\\end_layout

"""

LFUNS_INTRO = u"""\\begin_layout Section*
About this manual
\\end_layout

\\begin_layout Standard
This manual documents the 
\\begin_inset Quotes eld
\\end_inset

LyX Functions
\\begin_inset Quotes erd
\\end_inset

 (abbreviated LFUNs).
 These are commands that are used to make \\SpecialChar LyX
 perform specific actions.
 \\SpecialChar LyX
 itself uses these functions internally, and every internal action is
 bound to an LFUN.
\\end_layout

\\begin_layout Standard
LFUNs are also used in the files that define keyboard shortcuts, menu or
 toolbar items.
 So if you want to change\\SpecialChar breakableslash
customize the user interface, you need to deal
 with LFUNs.
 Furthermore, external programs can use LFUNs to communicate with and 
\\begin_inset Quotes eld
\\end_inset

remote-control
\\begin_inset Quotes erd
\\end_inset

 \\SpecialChar LyX
 .
 Finally, you can also issue LFUNs directly via the so called mini-buffer
 which can be opened via 
\\begin_inset Info
type  "shortcuts"
arg   "command-execute"
\\end_inset

.
\\end_layout

\\begin_layout Standard
In the following, all LFUNs are listed, categorized by function.
\\end_layout

"""


LFUNS_FOOTER = u"""\\end_body
\\end_document
"""

def parse_lfun(str):
    """Takes a comment block (str) and parses it for fields describing the LFUN. Returns a dict containing the fields."""
    
    lfun = dict(action="", notion="", syntax="", params="", sample="", origin="")
    field = ""
    lines = str.splitlines()
    # strip leading whitespace and * from the lines of the comment to get 
    # rid of unimportant characters
    for i in range(0, len(lines)):
        lines[i] = lines[i].strip(" *")
    
    for i in range(0, len(lines) - 1):
        # work out what field is being read if none of these is found, the line will be added
        #     to the last field edited
        # since the field identifier is not included skip it out if it's found, otherwise skip
        #     nothing as an existing field is being added to
        # if a field id is found, then its the first line of the field so set the pre_space to ""
        #     so that the first line isn't prespaced
        if lines[i].startswith(LFUN_ACTION_ID):
            field = "action"
            pre_space = ""
            skip = len(ID_DICT[field])
        elif lines[i].startswith(LFUN_NOTION_ID):
            field = "notion"
            pre_space = ""
            skip = len(ID_DICT[field])
        elif lines[i].startswith(LFUN_SYNTAX_ID):
            field = "syntax"
            pre_space = ""
            skip = len(ID_DICT[field])
        elif lines[i].startswith(LFUN_PARAMS_ID):
            field = "params"
            pre_space = ""
            skip = len(ID_DICT[field])
        elif lines[i].startswith(LFUN_SAMPLE_ID):
            field = "sample"
            pre_space = ""
            skip = len(ID_DICT[field])
        elif lines[i].startswith(LFUN_ORIGIN_ID):
            field = "origin"
            pre_space = ""
            skip = len(ID_DICT[field])
        elif lines[i].startswith(LFUN_ENDVAR):
            break
        else:
            skip = 0
            # if a manual line break was found last line, don't prespace this line
            if i > 1 and lines[i-1].endswith("\\n"):
                pre_space = ""
            else:
                pre_space = " "
        
        # add the line to the field, processing it for \ characters and \n
        # which, if occurring at the end of a line, must become a LYX_NEWLINE
        line = lines[i][skip:]
        
        # deal with \htmlonly
        # TODO: convert chars found in htmlonly to unicode
        start = line.find(HTMLONLY_START)
        if start > 0:
            # if removing the htmlonly element leaves a double space, go back one to remove it
            if line[start-1] == " ":
                start = start - 1
            end = line.find(HTMLONLY_END)
            if end > start:
                end = line.find(HTMLONLY_END) + len(HTMLONLY_END)
                line = line[:start] + line[end:]
            #else:
            # TODO: if HTMLONLY_END is not found, look on the next line
            # TODO: in the current LyXAction.cpp there are no htmlonly fields which go over a line break
        
        # deal with \ but leave \n if at the end of the line
        slash_idx = line.find("\\")
        while slash_idx >= 0:
            if slash_idx < len(line)-2 \
            or slash_idx == len(line)-1:
                # true when the \ is not the last or second last char
                #      or when the slash is the last char of the line
                
                # slash must be interpreted literaly so swap it for a LYX_BACKSLASH
                line = line[:slash_idx] + LYX_BACKSLASH + line[slash_idx+1:]
                # skip the index ahead beyond the added text
                slash_idx = slash_idx + len(LYX_BACKSLASH)
            elif line[slash_idx+1] != "n": # only evaluated if the line ends "\x" where 'x' != 'n'
                line = line[:slash_idx] + LYX_BACKSLASH + line[slash_idx+1:]
                # skip the index ahead beyond the added text
                slash_idx = slash_idx + len(LYX_BACKSLASH) 
            # look for the next \
            slash_idx = line.find("\\", slash_idx+1)
            
        # \n at the end of lines will not be processed by the above while loop
        # so sort those out now
        # sometime lines end " \n" so chop the space if its there
        if line.endswith(" \\n"):
            line = line[:len(line)-3] + LYX_NEWLINE
        elif line.endswith("\\n"):
            line = line[:len(line)-2] + LYX_NEWLINE
        
        # any references to other LFUNs need the # removing
        # TODO: actually insert a cross-reference here
        line = line.replace("#LFUN", "LFUN")
        
        # handle the few #lyx:: cases
        line = line.replace("#lyx::", "lyx::")

        # the first line might not have a field in it in which
        # case the variable field won't have a value, so check
        # to avoid an error
        if field != "":
            lfun[field] = lfun[field] + pre_space + line
        
        # TODO: sort out chopping lines of more that 80 chars in length
        
    return lfun

def write_fields(file, lfun):
    """Writes the LFUN contained in the dict lfun to the file. Does not write a the file header or footer"""
    # add lfun to LFUNs.lyx
    file.write(u"\\begin_layout Subsection*\n")
    file.write(lfun["name"] + "\n")
    file.write(u"\\end_layout\n")
    file.write(u"\n")
    if lfun["action"] != "":
        file.write(u"\\begin_layout Description\n")
        file.write("Action " + lfun["action"] + "\n")
        file.write(u"\\end_layout\n")
        file.write(u"\n")
    if lfun["notion"] != "":
        file.write(u"\\begin_layout Description\n")
        file.write("Notion " + lfun["notion"] + "\n")
        file.write(u"\\end_layout\n")
        file.write(u"\n")
    if lfun["syntax"] != "":
        file.write(u"\\begin_layout Description\n")
        file.write("Syntax " + lfun["syntax"] + "\n")
        file.write(u"\\end_layout\n")
        file.write(u"\n")
    if lfun["params"] != "":
        file.write(u"\\begin_layout Description\n")
        file.write("Params " + lfun["params"] + "\n")
        file.write(u"\\end_layout\n")
        file.write(u"\n")
    if lfun["sample"] != "":
        file.write(u"\\begin_layout Description\n")
        file.write("Sample " + lfun["sample"] + "\n")
        file.write(u"\\end_layout\n")
        file.write(u"\n")
    if lfun["origin"] != "":
        file.write(u"\\begin_layout Description\n")
        file.write("Origin " + lfun["origin"] + "\n")
        file.write(u"\\end_layout\n")
        file.write(u"\n")

def write_sections(file,lfuns):
    """Write sections of LFUNs"""
    sections = ["Layout", "Edit", "Math", "Buffer", "System", "Hidden"]
    section_headings = {
        "Layout":  u"Layout Functions (Font, Layout and Textclass related)",
        "Edit":    u"Editing Functions (Cursor and Mouse Movement, Copy/Paste etc.)",
        "Math":    u"Math Editor Functions",
        "Buffer":  u"Buffer Fuctions (File and Window related)",
        "System":  u"System Functions (Preferences, LyX Server etc.)",
        "Hidden":  u"Hidden Functions (not listed for configuration)"
        }
        # write the lfuns to the file
    for val in sections:
        file.write(u"\\begin_layout Section\n")
        file.write(section_headings[val] + "\n")
        file.write(u"\\end_layout\n")
        file.write(u"\n")
        for lf in lfuns:
            if lf["type"] == val:
                write_fields(file, lf)
    
def main(argv):
    # parse command line arguments
    script_path, script_name = os.path.split(argv[0])
    if len(argv) < 2:
        error(usage(script_name))
    # input file
    lyxaction_path = argv[1]
    if not os.path.isfile(lyxaction_path):
        error(script_name + ": %s is not a valid path" % lyxaction_path)

    # output file
    if len(argv) == 3:
        lfuns_path = argv[2]
        if os.path.isdir(lfuns_path):
            lfuns_path = lfuns_path + "LFUNs.lyx"
        elif os.path.exists(lfuns_path):
            error(script_name + ": %s already exists, delete it and rerun the script" % lfuns_path)
        lfuns_file = io.open(lfuns_path, 'w', encoding='utf_8')
    else:
        lfuns_file = sys.stdout

    sys.stderr.write(script_name + ": Start processing " + argv[1] + '\n')
    # Read the input file and write the output file
    lyxaction_file = io.open(lyxaction_path, 'r', encoding='utf_8')

    lyxaction_text = lyxaction_file.read()

    lfuns_file.write(LFUNS_HEADER)
    
    # An introductory section
    lfuns_file.write(LFUNS_INTRO)

    # seek to the important bit of LyXAction.cpp
    try:
        start = lyxaction_text.index("ev_item const items[] = {")
    except ValueError:
        lyxaction_file.close()
        lfuns_file.close()
        error(script_name + ": LFUNs not found in " + lyxaction_file)

    done = count = 0

    lfun_list_unsorted = []

    while done == 0:
        # look for a doxygen comment
        start = lyxaction_text.find(DOXYGEN_START, start)
        end = lyxaction_text.find(DOXYGEN_END, start) + len(DOXYGEN_END)
        name = ""
        atype = ""
        snippet = lyxaction_text[start:end]
        defline = snippet.replace("\n", "")
        match = re.match(r'.*\s*\{\s*(.+),\s*"(.*)",\s*([\w\|\s]+),\s*(\w+)\s*\},.*$', defline)
        if match:
            name = match.group(2)
            atype = match.group(4)
        # parse the lfun if it is found
        if start > 0:
            if name:
                count = count + 1
                lfun = parse_lfun(snippet)
                lfun["name"] = name
                lfun["type"] = atype
                # save the lfun (we sort it before writing)
                lfun_list_unsorted.append(lfun)
            # get the next one
            start = end
        else:
            # if no more lfuns are found, EOF reached
            done = 1

    lfun_list = sorted(lfun_list_unsorted, key=lambda k: k['name'])
    
    # write the lfuns to the file
    write_sections(lfuns_file, lfun_list)

    sys.stderr.write(script_name + ": Created documentation for " + str(count) + " LFUNs\n")

    # write the last part of LFUNs.lyx
    lfuns_file.write(LFUNS_FOOTER)
    
    lyxaction_file.close()
    lfuns_file.close()
    
    sys.stderr.write(script_name + ": Finished\n")
    
if __name__ == "__main__":
    main(sys.argv)
