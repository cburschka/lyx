# -*- coding: utf-8 -*-

# file layout2layout.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# author Georg Baum

# Full author contact details are available in file CREDITS

# This script will update a .layout file to current format

# The latest layout format is also defined in src/TextClass.cpp
currentFormat = 83


# Incremented to format 4, 6 April 2007, lasgouttes
# Introduction of generic "Provides" declaration

# Incremented to format 5, 22 August 2007 by vermeer
# InsetLayout material

# Incremented to format 6, 7 January 2008 by spitz
# Requires tag added to layout files

# Incremented to format 7, 24 March 2008 by rgh
# AddToPreamble tag added to layout files

# Incremented to format 8, 25 July 2008 by rgh
# UseModule tag added to layout files
# CopyStyle added to InsetLayout

# Incremented to format 9, 5 October 2008 by rgh
# ForcePlain and CustomPars tags added to InsetLayout

# Incremented to format 10, 6 October 2008 by rgh
# Change format of counters

# Incremented to format 11, 14 October 2008 by rgh
# Add ProvidesModule, ExcludesModule tags

# Incremented to format 12, 10 January 2009 by gb
# Add I18NPreamble tag

# Incremented to format 13, 5 February 2009 by rgh
# Add InToc tag for InsetLayout

# Incremented to format 14, 14 February 2009 by gb
# Rename I18NPreamble to BabelPreamble and add LangPreamble

# Incremented to format 15, 28 May 2009 by lasgouttes
# Add new tag OutputFormat; modules can be conditioned on feature
# "from->to".

# Incremented to format 16, 5 June 2009 by rgh
# Add new tags for Text Class:
#   HTMLPreamble, HTMLAddToPreamble
# For Layout:
#   HTMLTag, HTMLAttr, HTMLLabel, HTMLLabelAttr, HTMLItem, HTMLItemAttr
#   HTMLStyle, and HTMLPreamble
# For InsetLayout:
#   HTMLTag, HTMLAttr, HTMLStyle, and HTMLPreamble
# For Floats:
#   HTMLType, HTMLClass, HTMLStyle

# Incremented to format 17, 12 August 2009 by rgh
# Add IfStyle and IfCounter tags for layout.

# Incremented to format 18, 27 October 2009 by rgh
# Added some new tags for HTML output.

# Incremented to format 19, 17 November 2009 by rgh
# Added InPreamble tag.

# Incremented to format 20, 17 December 2009 by rgh
# Added ContentAsLabel tag.

# Incremented to format 21, 12 January 2010 by rgh
# Added HTMLTocLayout and HTMLTitle tags.

# Incremented to format 22, 20 January 2010 by rgh
# Added HTMLFormat tag to Counters.

# Incremented to format 23, 13 February 2010 by spitz
# Added Spellcheck tag.

# Incremented to format 24, 5 March 2010 by rgh
# Changed LaTeXBuiltin tag to NeedsFloatPkg and
# added new tag ListCommand.

# Incremented to format 25, 12 March 2010 by rgh
# Added RefPrefix tag for layouts and floats.

# Incremented to format 26, 29 March 2010 by rgh
# Added CiteFormat.

# Incremented to format 27, 4 June 2010 by rgh
# Added RequiredArgs tag.

# Incremented to format 28, 6 August 2010 by lasgouttes
# Added ParbreakIsNewline tag for Layout and InsetLayout.

# Incremented to format 29, 10 August 2010 by rgh
# Changed Custom:Style, CharStyle:Style, and Element:Style
# uniformly to Flex:Style.

# Incremented to format 30, 13 August 2010 by rgh
# Introduced ResetsFont tag for InsetLayout.

# Incremented to format 31, 12 January 2011 by rgh
# Introducted NoCounter tag.

# Incremented to format 32, 30 January 2011 by forenr
# Added Display tag for InsetLayout

# Incremented to format 33, 2 February 2011 by rgh
# Changed NeedsFloatPkg to UsesFloatPkg

# Incremented to format 34, 28 March 2011 by rgh
# Remove obsolete Fill_(Top|Bottom) tags

# Incremented to format 35, 28 March 2011 by rgh
# Try to add "Flex:" to any flex insets that don't have it.

# Incremented to format 36, 7 December 2011, by rgh
# Added HTMLStyles and AddToHTMLStyles tags.

# Incremented to format 37, 29 February 2012 by jrioux
# Implement the citation engine machinery in layouts.
# Change CiteFormat to CiteFormat (default|authoryear|numerical).

# Incremented to format 38, 08 April 2012 by gb
# Introduce LangPreamble and BabelPreamble for InsetLayout.

# Incremented to format 39, 15 April 2012 by sanda
# Introduce styling of branches via "InsetLayout Branch:".

# Incremented to format 40, 10 October 2012 by rgh
# Re-do layout names for layout categories

# Incremented to format 41, 20 November 2012 by spitz
# New Argument syntax

# Incremented to format 42, 22 December 2012 by spitz
# New Style tag "ItemCommand"

# Incremented to format 43, 30 December 2012 by spitz
# Extended InsetCaption format

# Incremented to format 44, 9 February 2013 by rgh
# Remove COUNTER label style; rename as STATIC
# Rename TOP_ENVIRONMENT to ABOVE and CENTERED_TOP_ENVIRONMENT to CENTERED

# Incremented to format 45, 12 February 2013 by rgh
# New Tag "NoInsetLayout"

# Incremented to format 46, 15 May 2013 by gb
# New Tag "ForceLocal"

# Incremented to format 47, 23 May 2013 by rgh
# Add PackageOptions tag

# Incremented to format 48, 31 May 2013 by rgh
# Add InitialValue tag for counters

# Incremented to format 49, 10 Feb 2014 by gb
# Change default of "ResetsFont" tag to false

# Incremented to format 50, 9 May 2014 by forenr
# Removal of "Separator" layouts

# Incremented to format 51, 29 May 2014 by spitz
# New Style tag "ToggleIndent"

# Incremented to format 52, 1 December 2014 by spitz
# New InsetLayout tag "ForceOwnlines"

# Incremented to format 53, 7 December 2014 by spitz
# New InsetLayout tag "ObsoletedBy"

# Incremented to format 54, 11 Jan 2014 by gb
# New InsetLayout tag "FixedWidthPreambleEncoding"

# Incremented to format 55, 20 April 2015 by spitz
# New InsetLayout and Layout tags "PassThruChars"

# Incremented to format 56, 20 May 2015 by spitz
# New Float tags "AllowedPlacement", "AllowsWide", "AllowsSideways"

# Incremented to format 57, 30 May 2015 by spitz
# New Layout tag "ParagraphGroup"

# Incremented to format 58, 5 December 2015, by rgh
# New Layout tag "ProvideStyle"
# Change "IfStyle" to "ModifyStyle"

# Incremented to format 59, 22 November 2015 by gm
# New Tag "OutlinerName"
# New Layout tags "AddToToc", "IsTocCaption"
# New Layout argument tag "IsTocCaption"

# Incremented to format 60, 25 March 2016 by lasgouttes
# Rename caption subtype LongTableNoNumber to Unnumbered

# Incremented to format 61, 14 October 2016 by spitz
# New Layout tags "ResumeCounter", "StepMasterCounter"

# Incremented to format 62, 21 October 2016 by spitz
# New Layout argument tag "PassThru"

# Incremented to format 63, 7 January 2017 by spitz
# - New textclass tags CiteFramework, MaxCiteNames (for cite engines)
# - Extended InsetCite syntax.

# Incremented to format 64, 30 August 2017 by rgh
# Strip leading and trailing spaces from LabelString,
# LabelStringAppendix, and EndLabelString, and LabelCounter,
# to conform to what we used to do.

# Incremented to format 65, 16 October 2017 by spitz
# Color collapsable -> collapsible

# Incremented to format 66, 28 December 2017 by spitz
# New Layout tags "AutoNests ... EndAutoNests" and
# "IsAutoNestedBy ... EndIsAutoNestedBy"

# Incremented to format 67, 14 April 2018 by spitz
# New Layout tag "NeedsCProtect"

# Incremented to format 68, 21 May 2018 by spitz
# New Layout tag "AddToCiteEngine"

# Incremented to format 69, 16 August 2018 by spitz
# New argument type "listpreamble"

# Incremented to format 70, 5 June 2018 by rkh
# New InsetLayout tag EditExternal

# Incremented to format 71, 12 March 2019 by spitz
# New [Inset]Layout tag NeedMBoxProtect

# Incremented to format 72, 26 March 2019 by spitz
# New TextClass tag TableStyle

# Incremented to format 73, 18 April 2019 by spitz
# New InsetLayout tag MenuString

# Incremented to format 74, 18 April 2019 by spitz
# New InsetLayout and Argument tag NewlineCmd

# Incremented to format 75, 2 June 2019 by spitz
# New Argument tags FreeSpacing, InsertOnNewline
# New InsetLayout tag ParbreakIgnored

# Incremented to format 76, 8 July 2019 by spitz
# New textclass tag BibInToc

# Incremented to format 77, 6 August 2019 by spitz
# New textclass tag PageSize (= default page size) 
# and textclass option PageSize (= list of available page sizes)

# Incremented to format 78, 6 August 2019 by spitz
# New textclass tag FontsizeFormat

# Incremented to format 79, 7 August 2019 by spitz
# New textclass tag PagesizeFormat

# Incremented to format 80, 12 August 2019 by spitz
# New float option Requires

# Incremented to format 81, 12 August 2019 by rikiheck
# New tag GuiName for counters

# Incremented to format 82, 4 June 2017 by dourouc05
# - Add new tags for Layout:
#   DocBookTag, DocBookAttr, DocBookInInfo,
#   DocBookWrapperTag, DocBookWrapperAttr,
#   DocBookItemWrapperTag, DocBookItemWrapperAttr,
#   DocBookItemTag, DocBookItemAttr,
#   DocBookLabelTag, DocBookLabelAttr
# - Removed tag Header from ClassOptionsClassOptions
# - Removed tag Element for flex insets

# Incremented to format 83, 2 August 2020 by dourouc05
# New tags DocBookWrapperMergeWithPrevious and DocBookAbstract

# Do not forget to document format change in Customization
# Manual (section "Declaring a new text class").

# You might also want to consider running the
# development/tools/updatelayouts.py script to update all
# layout files to the new format.


import os, re, sys
import argparse

# Provide support for both python 2 and 3
# (copied from lyx2lyx)
PY2 = sys.version_info[0] == 2
if PY2:
    # argparse returns strings in the commandline encoding, we need to convert.
    # sys.getdefaultencoding() would not always be correct, see
    # http://legacy.python.org/dev/peps/pep-0383/
    def cmd_arg(arg):
        return arg.decode(sys.getfilesystemencoding())
else:
    cmd_arg = str
# End of code to support for both python 2 and 3


def error(message):
    sys.stderr.write(message + '\n')
    sys.exit(1)


def trim_bom(line):
    " Remove byte order mark."
    if line[0:3] == "\357\273\277":
        return line[3:]
    else:
        return line


def read(source):
    " Read input file and strip lineendings."
    lines = source.read().splitlines() or ['']
    lines[0] = trim_bom(lines[0])
    return lines


def write(output, lines):
    " Write output file with native lineendings."
    output.write(os.linesep.encode('ascii').join(lines)
                 + os.linesep.encode('ascii'))


# Concatenates old and new in an intelligent way:
# If old is wrapped in ", they are stripped. The result is wrapped in ".
def concatenate_label(old, new):
    # Don't use strip as long as we support python 1.5.2
    if old[0] == b'"':
        return old[0:-1] + new + b'"'
    else:
        return b'"' + old + new + b'"'

# appends a string to a list unless it's already there
def addstring(s, l):
    if l.count(s) > 0:
        return
    l.append(s)


def convert(lines, end_format):
    " Convert to new format."
    re_Comment = re.compile(b'^(\\s*)#')
    re_Counter = re.compile(b'\\s*Counter\\s*', re.IGNORECASE)
    re_Name = re.compile(b'\\s*Name\\s+(\\S+)\\s*', re.IGNORECASE)
    re_UseMod = re.compile(b'^\\s*UseModule\\s+(.*)', re.IGNORECASE)
    re_Empty = re.compile(b'^(\\s*)$')
    re_Format = re.compile(b'^(\\s*)(Format)(\\s+)(\\S+)', re.IGNORECASE)
    re_Preamble = re.compile(b'^(\\s*)Preamble', re.IGNORECASE)
    re_EndPreamble = re.compile(b'^(\\s*)EndPreamble', re.IGNORECASE)
    re_LangPreamble = re.compile(b'^(\\s*)LangPreamble', re.IGNORECASE)
    re_EndLangPreamble = re.compile(b'^(\\s*)EndLangPreamble', re.IGNORECASE)
    re_BabelPreamble = re.compile(b'^(\\s*)BabelPreamble', re.IGNORECASE)
    re_EndBabelPreamble = re.compile(b'^(\\s*)EndBabelPreamble', re.IGNORECASE)
    re_MaxCounter = re.compile(b'^(\\s*)(MaxCounter)(\\s+)(\\S+)', re.IGNORECASE)
    re_LabelType = re.compile(b'^(\\s*)(LabelType)(\\s+)(\\S+)', re.IGNORECASE)
    re_LabelString = re.compile(b'^(\\s*)(LabelString)(\\s+)(("[^"]+")|(\\S+))', re.IGNORECASE)
    re_LabelStringAppendix = re.compile(b'^(\\s*)(LabelStringAppendix)(\\s+)(("[^"]+")|(\\S+))', re.IGNORECASE)
    re_LatexType = re.compile(b'^(\\s*)(LatexType)(\\s+)(\\S+)', re.IGNORECASE)
    re_Style = re.compile(b'^(\\s*)(Style)(\\s+)(\\S+)', re.IGNORECASE)
    re_IfStyle = re.compile(b'^(\\s*)IfStyle(\\s+\\S+)', re.IGNORECASE)
    re_CopyStyle = re.compile(b'^(\\s*)(CopyStyle)(\\s+)(\\S+)', re.IGNORECASE)
    re_NoStyle = re.compile(b'^(\\s*)(NoStyle)(\\s+)(\\S+)', re.IGNORECASE)
    re_End = re.compile(b'^(\\s*)(End)(\\s*)$', re.IGNORECASE)
    re_Provides = re.compile(b'^(\\s*)Provides(\\S+)(\\s+)(\\S+)', re.IGNORECASE)
    re_CharStyle = re.compile(b'^(\\s*)CharStyle(\\s+)(\\S+)$', re.IGNORECASE)
    re_CiteFormat = re.compile(b'^(\\s*)(CiteFormat)(?:(\\s*)()|(\\s+)(default|authoryear|numerical))', re.IGNORECASE)
    re_AMSMaths = re.compile(b'^\\s*Input ams(?:math|def)s.inc\\s*')
    re_AMSMathsPlain = re.compile(b'^\\s*Input amsmaths-plain.inc\\s*')
    re_AMSMathsSeq = re.compile(b'^\\s*Input amsmaths-seq.inc\\s*')
    re_TocLevel = re.compile(b'^(\\s*)(TocLevel)(\\s+)(\\S+)', re.IGNORECASE)
    re_I18nPreamble = re.compile(b'^(\\s*)I18nPreamble', re.IGNORECASE)
    re_EndI18nPreamble = re.compile(b'^(\\s*)EndI18nPreamble', re.IGNORECASE)
    re_Float = re.compile(b'^\\s*Float\\s*$', re.IGNORECASE)
    re_Type = re.compile(b'\\s*Type\\s+(\\w+)', re.IGNORECASE)
    re_Builtin = re.compile(b'^(\\s*)LaTeXBuiltin\\s+(\\w*)', re.IGNORECASE)
    re_True = re.compile(b'^\\s*(?:true|1)\\s*$', re.IGNORECASE)
    re_InsetLayout = re.compile(b'^\\s*InsetLayout\\s+(?:Custom|CharStyle|Element):(\\S+)\\s*$', re.IGNORECASE)
    re_ResetsFont = re.compile(b'^(\\s*)ResetsFont(\\s+)(\\S+)$', re.IGNORECASE)
    # with quotes
    re_QInsetLayout = re.compile(b'^\\s*InsetLayout\\s+"(?:Custom|CharStyle|Element):([^"]+)"\\s*$', re.IGNORECASE)
    re_InsetLayout_CopyStyle = re.compile(b'^\\s*CopyStyle\\s+(?:Custom|CharStyle|Element):(\\S+)\\s*$', re.IGNORECASE)
    re_QInsetLayout_CopyStyle = re.compile(b'^\\s*CopyStyle\\s+"(?:Custom|CharStyle|Element):([^"]+)"\\s*$', re.IGNORECASE)
    re_NeedsFloatPkg = re.compile(b'^(\\s*)NeedsFloatPkg\\s+(\\w+)\\s*$', re.IGNORECASE)
    re_Fill = re.compile(b'^\\s*Fill_(?:Top|Bottom).*$', re.IGNORECASE)
    re_InsetLayout2 = re.compile(b'^\\s*InsetLayout\\s+(\\S+)\\s*$', re.IGNORECASE)
    # with quotes
    re_QInsetLayout2 = re.compile(b'^\\s*InsetLayout\\s+"([^"]+)"\\s*$', re.IGNORECASE)
    re_IsFlex = re.compile(b'\\s*LyXType.*$', re.IGNORECASE)
    re_CopyStyle2 = re.compile(b'(\\s*CopyStyle\\s+)"?([^"]+)"?\\s*$')
    re_Separator = re.compile(b'^(?:(-*)|(\\s*))(Separator|EndOfSlide)(?:(-*)|(\\s*))$', re.IGNORECASE)
    # for categories
    re_Declaration = re.compile(b'^#\\s*\\Declare\\w+Class.*$')
    re_ExtractCategory = re.compile(b'^(#\\s*\\Declare\\w+Class(?:\\[[^]]*?\\])?){([^(]+?)\\s+\\(([^)]+?)\\)\\s*}\\s*$')
    ConvDict = {"article": "Articles", "book" : "Books", "letter" : "Letters", "report": "Reports",
                "presentation" : "Presentations", "curriculum vitae" : "Curricula Vitae", "handout" : "Handouts"}
    # Arguments
    re_OptArgs = re.compile(b'^(\\s*)OptionalArgs(\\s+)(\\d+)\\D*$', re.IGNORECASE)
    re_ReqArgs = re.compile(b'^(\\s*)RequiredArgs(\\s+)(\\d+)\\D*$', re.IGNORECASE)

    # various changes associated with changing how chapters are handled
    re_LabelTypeIsCounter = re.compile(b'^(\\s*)LabelType(\\s*)Counter\\s*$', re.IGNORECASE)
    re_TopEnvironment = re.compile(b'^(\\s*)LabelType(\\s+)Top_Environment\\s*$', re.IGNORECASE)
    re_CenteredEnvironment = re.compile(b'^(\\s*)LabelType(\\s+)Centered_Top_Environment\\s*$', re.IGNORECASE)
    re_ChapterStyle = re.compile(b'^\\s*Style\\s+Chapter\\s*$', re.IGNORECASE)
    re_InsetLayout_CaptionLTNN = re.compile(b'^(\\s*InsetLayout\\s+)(Caption:LongTableNonumber)', re.IGNORECASE)
    # for format 64
    re_trimLabelString = re.compile(b'^(\\s*LabelString\s+)"\\s*(.*?)\\s*"\\s*$')
    re_trimLabelStringAppendix  = re.compile(b'^(\\s*LabelStringAppendix\s+)"\\s*(.*?)\\s*"\\s*$')
    re_trimEndLabelString = re.compile(b'^(\\s*EndLabelString\s+)"\\s*(.*?)\\s*"\\s*$')
    re_trimLabelCounter = re.compile(b'^(\\s*LabelCounter\s+)"\\s*(.*?)\\s*"\\s*$')


    # counters for sectioning styles (hardcoded in 1.3)
    counters = {b"part"          : b"\\Roman{part}",
                b"chapter"       : b"\\arabic{chapter}",
                b"section"       : b"\\arabic{section}",
                b"subsection"    : b"\\arabic{section}.\\arabic{subsection}",
                b"subsubsection" : b"\\arabic{section}.\\arabic{subsection}.\\arabic{subsubsection}",
                b"paragraph"     : b"\\arabic{section}.\\arabic{subsection}.\\arabic{subsubsection}.\\arabic{paragraph}",
                b"subparagraph"  : b"\\arabic{section}.\\arabic{subsection}.\\arabic{subsubsection}.\\arabic{paragraph}.\\arabic{subparagraph}"}

    # counters for sectioning styles in appendix (hardcoded in 1.3)
    appendixcounters = {b"chapter"       : b"\\Alph{chapter}",
                        b"section"       : b"\\Alph{section}",
                        b"subsection"    : b"\\arabic{section}.\\arabic{subsection}",
                        b"subsubsection" : b"\\arabic{section}.\\arabic{subsection}.\\arabic{subsubsection}",
                        b"paragraph"     : b"\\arabic{section}.\\arabic{subsection}.\\arabic{subsubsection}.\\arabic{paragraph}",
                        b"subparagraph"  : b"\\arabic{section}.\\arabic{subsection}.\\arabic{subsubsection}.\\arabic{paragraph}.\\arabic{subparagraph}"}

    # Value of TocLevel for sectioning styles
    toclevels = {b"part"          : -1,
                 b"chapter"       : 0,
                 b"section"       : 1,
                 b"subsection"    : 2,
                 b"subsubsection" : 3,
                 b"paragraph"     : 4,
                 b"subparagraph"  : 5}

    i = 0
    only_comment = 1
    counter = b""
    toclevel = b""
    label = b""
    labelstring = b""
    labelstringappendix = b""
    space1 = b""
    labelstring_line = -1
    labelstringappendix_line = -1
    labeltype_line = -1
    latextype = b""
    latextype_line = -1
    style = b""
    maxcounter = 0
    format = 1
    formatline = 0
    usemodules = []
    flexstyles = []
    opts = 0
    reqs = 0
    inchapter = False
    isflexlayout = False         # only used for 48 -> 49
    # Whether a style is inherited (works only for CopyStyle currently,
    # not for true inherited styles, see bug 8920
    inherited = False        # only used for 48 -> 49
    resetsfont_found = False # only used for 48 -> 49

    while i < len(lines):
        # Skip comments and empty lines
        if (re_Comment.match(lines[i]) or re_Empty.match(lines[i])):
          # We need to deal with this conversion here, because it happens
          # inside the initial comment block.
          if only_comment and format == 39:
              match = re_ExtractCategory.match(lines[i])
              if match:
                  lpre = match.group(1)
                  lcat = match.group(2)
                  lnam = match.group(3)
                  if lcat in ConvDict:
                      lcat = ConvDict[lcat]
                  lines[i] = lpre + b"{" + lnam + b"}"
                  lines.insert(i+1, b"#  \\DeclareCategory{" + lcat + b"}")
                  i += 1
          i += 1
          continue

        # insert file format if not already there
        if only_comment:
            match = re_Format.match(lines[i])
            if match:
                formatline = i
                format = int(match.group(4))
                if 1 < format < end_format:
                    lines[i] = b"Format %d" % (format + 1)
                    only_comment = 0
                elif format == end_format:
                    # nothing to do
                    return format
                else:
                    error('Cannot convert file format %s to %s' % (format, end_format))
            else:
                lines.insert(i, b"Format 2")
                only_comment = 0
                continue

        # Don't get confused by LaTeX code
        if re_Preamble.match(lines[i]):
            i += 1
            while i < len(lines) and not re_EndPreamble.match(lines[i]):
                i += 1
            continue
        if re_LangPreamble.match(lines[i]):
            i += 1
            while i < len(lines) and not re_EndLangPreamble.match(lines[i]):
                i += 1
            continue
        if re_BabelPreamble.match(lines[i]):
            i += 1
            while i < len(lines) and not re_EndBabelPreamble.match(lines[i]):
                i += 1
            continue

        if format == 81:
            match = re.compile(b'^(\\s*Header\\s+)("?\\w+"?)', re.IGNORECASE).match(lines[i])
            if match:
                del lines[i]
                continue

            match = re.compile(b'(\\s*LyXType\\s+)(\\w+)(\\s*Element\\s+)', re.IGNORECASE).match(lines[i])
            if match:
                del lines[i]
                continue

            i += 1
            continue

        if 65 <= format <= 80:
            # nothing to do.
            i += 1
            continue

        if format == 64:
            match = re.compile(b'(\\s*Color\\s+)(\\w+)', re.IGNORECASE).match(lines[i])
            if not match:
                i += 1
                continue
            col  = match.group(2)
            if col == "collapsable":
                lines[i] = match.group(1) + "collapsible"
            i += 1
            continue

        if format == 63:
            for r in (re_trimLabelString, re_trimLabelStringAppendix,\
              re_trimEndLabelString, re_trimLabelCounter):
                m = r.match(lines[i])
                if m:
                    lines[i] = m.group(1) + b'"' + m.group(2) + b'"'
            i += 1
            continue

        if 60 <= format <= 62:
            # nothing to do.
            i += 1
            continue

        if format == 59:
            match = re_InsetLayout_CaptionLTNN.match(lines[i])
            if not match:
                i += 1
                continue
            # '^(\s*InsetLayout\s+)(Caption:LongTableNonumber)'
            lead  = match.group(1)
            lines[i] = lead + b"Caption:Unnumbered"
            i += 1
            continue

        if format == 58:
            # nothing to do.
            i += 1
            continue

        if format == 57:
            match = re_IfStyle.match(lines[i])
            if not match:
                i += 1
                continue
            # b'^(\\s*)IfStyle(\\s+\\S+)
            lead  = match.group(1)
            trail = match.group(2)
            lines[i] = lead + b"ModifyStyle" + trail
            i += 1
            continue

        if 50 <= format <= 56:
            # nothing to do.
            i += 1
            continue

        if format == 49:
            separator = []

            # delete separator styles
            match = re_Style.match(lines[i])
            if match:
                style = match.group(4).lower()
                if re_Separator.match(style):
                    del lines[i]
                    while i < len(lines) and not re_End.match(lines[i]):
                        separator.append(lines[i])
                        del lines[i]
                    if i == len(lines):
                        error('Incomplete separator style.')
                    else:
                        del lines[i]
                        continue

            # delete undefinition of separator styles
            match = re_NoStyle.match(lines[i])
            if match:
                style = match.group(4).lower()
                if re_Separator.match(style):
                    del lines[i]
                    continue

            # replace the CopyStyle statement with the definition of the real
            # style. This may result in duplicate statements, but that is OK
            # since the second one will overwrite the first one.
            match = re_CopyStyle.match(lines[i])
            if match:
                style = match.group(4).lower()
                if re_Separator.match(style):
                    if len(separator) > 0:
                        lines[i:i+1] = separator
                    else:
                        # FIXME: If this style was redefined in an include file,
                        # we should replace the real style and not this default.
                        lines[i:i+1] = [b'	Category              MainText',
                                        b'	KeepEmpty             1',
                                        b'	Margin                Dynamic',
                                        b'	LatexType             Paragraph',
                                        b'	LatexName             dummy',
                                        b'	ParIndent             MM',
                                        b'	Align                 Block',
                                        b'	LabelType             Static',
                                        b'	LabelString           "--- Separate Environment ---"',
                                        b'	LabelFont',
                                        b'	  Family              Roman',
                                        b'	  Series              Medium',
                                        b'	  Size                Normal',
                                        b'	  Color               Blue',
                                        b'	EndFont',
                                        b'	HTMLLabel             NONE']
            i += 1
            continue

        if format == 48:
            # The default of ResetsFont in LyX changed from true to false,
            # because it is now used for all InsetLayouts, not only flex ones.
            # Therefore we need to set it to true for all flex insets which do
            # do not already have a ResetsFont.
            match = re_InsetLayout2.match(lines[i])
            if not match:
                i += 1
                continue

            name = match.group(1).lower()
            if name != b"flex" and name != b"\"flex\"" and name[0:5] != b"flex:" and name [0:6] != b"\"flex:":
                i += 1
                continue

            resetsfont_found = False
            inherited = False
            notdone = True
            while i < len(lines):
              match = re_ResetsFont.match(lines[i])
              if match:
                  resetsfont_found = True
              else:
                match = re_CopyStyle.match(lines[i])
                if match:
                  inherited = True
                else:
                  match = re_End.match(lines[i])
                  if match:
                    break
              i += 1
            if not resetsfont_found and not inherited:
              lines.insert(i, b"\tResetsFont true")

            continue

        if 44 <= format <= 47:
            # nothing to do.
            i += 1
            continue

        if format == 43:
          match = re_LabelTypeIsCounter.match(lines[i])
          if match:
            if inchapter:
             lines[i] = match.group(1) + b"LabelType" + match.group(2) + b"Above"
            else:
              lines[i] = match.group(1) + b"LabelType" + match.group(2) + b"Static"

          match = re_TopEnvironment.match(lines[i])
          if match:
            lines[i] = match.group(1) + b"LabelType" + match.group(2) + b"Above"

          match = re_CenteredEnvironment.match(lines[i])
          if match:
            lines[i] = match.group(1) + b"LabelType" + match.group(2) + b"Centered"

          if inchapter:
            match = re_Style.match(lines[i])
            if match:
              inchapter = False
          else:
            match = re_ChapterStyle.match(lines[i])
            if match:
              inchapter = True

          i += 1
          continue

        if format == 42:
          if lines[i] == b"InsetLayout Caption":
            lines[i] = b"InsetLayout Caption:Standard"
          i += 1
          continue

        if format == 41:
            # nothing to do.
            i += 1
            continue

        if format == 40:
            # reset counters on Style beginning
            match = re_Style.match(lines[i])
            if match:
                opts = 0
                reqs = 0
                i += 1
                continue
            match = re_OptArgs.match(lines[i])
            if match:
                # Save number of optional arguments
                space1 = match.group(1)
                opts = int(match.group(3))
                # OptionalArgs 0 > ResetArgs 1
                if opts == 0:
                    lines[i] = space1 + b"ResetArgs\t1"
                    i += 1
                else:
                    del lines[i]
                continue
            match = re_ReqArgs.match(lines[i])
            if match:
                # Save number of required arguments
                space1 = match.group(1)
                reqs = int(match.group(3))
                del lines[i]
                continue
            # Insert the required number of arguments at the end of the style definition
            match = re_End.match(lines[i])
            if match:
                newarg = ['']
                # First the optionals (this is the required order pre 2.1)
                if opts > 0:
                    if opts == 1:
                        newarg = [ b'%sArgument 1' % (space1),
                                   b'%s\tLabelString\t\"Optional Layout Argument\"' % (space1),
                                   b'%sEndArgument' % (space1)]
                    elif opts > 1:
                        actopt = 1
                        while actopt < (opts + 1):
                            newarg += [ b'%sArgument %d' % (space1, actopt),
                               b'%s\tLabelString\t\"Optional Layout Argument %d\"' % (space1, actopt),
                               b'%sEndArgument' % (space1)]
                            actopt += 1
                # Now the mandatories
                if reqs > 0:
                    actopt = opts + 1
                    while actopt < (opts +  reqs + 1):
                        newarg += [ b'%sArgument %d' % (space1, actopt),
                           b'%s\tLabelString\t"Required Layout Argument %d"' % (space1, actopt - opts),
                           b'%s\tMandatory\t1' % (space1),
                           b'%sEndArgument' % (space1)]
                        actopt += 1
                # Since we replace the "End" line, re-add this line
                if len(newarg) > 1:
                    newarg += [b'End']
                    lines[i:i+1] = newarg
                    i += len(newarg)
                # Reset the counters
                opts = 0
                reqs = 0
            i += 1
            continue

        if format == 39:
            # There is a conversion with format 40, but it is done within the
            # initial comment block and so is above.
            i += 1
            continue

        if format == 37 or format == 38:
            i += 1
            continue

        if format == 36:
            match = re_CiteFormat.match(lines[i])
            if match and match.group(4) == b"":
                lines[i] = match.group(0) + b" default"
            i += 1
            continue

        if format == 35:
          i += 1
          continue

        if format == 34:
          match = re_QInsetLayout2.match(lines[i])
          if not match:
            match = re_InsetLayout2.match(lines[i])
          if not match:
            match = re_CopyStyle2.match(lines[i])
            if not match:
              i += 1
              continue
            style = match.group(2)

            if flexstyles.count(style):
              lines[i] = match.group(1) + b"\"Flex:" + style + b"\""
            i += 1
            continue

          name = match.group(1)
          names = name.split(b":", 1)
          if len(names) > 1 and names[0] == b"Flex":
            i += 1
            continue

          isflex = False
          for j in range(i + 1, len(lines)):
            if re_IsFlex.match(lines[j]):
              isflex = True
              break
            if re_End.match(lines[j]):
              break

          if isflex:
            flexstyles.append(name)
            lines[i] = b"InsetLayout \"Flex:" + name + b"\""

          i += 1
          continue

        if format == 33:
          m = re_Fill.match(lines[i])
          if m:
            lines[i] = b""
          i += 1
          continue

        if format == 32:
          match = re_NeedsFloatPkg.match(lines[i])
          if match:
            space = match.group(1)
            val = match.group(2)
            lines[i] = space + b"UsesFloatPkg " + val
            newval = b'true'
            if val == b'1' or val.lower() == b'true':
              newval = b'false'
            lines.insert(i, space + b"IsPredefined " + newval)
            i += 1
          i += 1
          continue

        # Only new features
        if 29 <= format <= 31:
          i += 1
          continue

        if format == 28:
          match = re_InsetLayout.match(lines[i])
          if match:
            lines[i] = b"InsetLayout Flex:" + match.group(1)
          else:
            match = re_QInsetLayout.match(lines[i])
            if match:
              lines[i] = b"InsetLayout \"Flex:" + match.group(1) + b"\""
            else:
              match = re_InsetLayout_CopyStyle.match(lines[i])
              if match:
                lines[i] = b"\tCopyStyle Flex:" + match.group(1)
              else:
                match = re_QInsetLayout_CopyStyle.match(lines[i])
                if match:
                  lines[i] = b"\tCopyStyle \"Flex:" + match.group(1) + b"\""
          i += 1
          continue

        # Only new features
        if 24 <= format <= 27:
          i += 1
          continue

        if format == 23:
          match = re_Float.match(lines[i])
          i += 1
          if not match:
            continue
          # we need to do two things:
          # (i)  Convert Builtin to NeedsFloatPkg
          # (ii) Write ListCommand lines for the builtin floats table and figure
          builtin = False
          cmd = b""
          while True and i < len(lines):
            m1 = re_End.match(lines[i])
            if m1:
              if builtin and cmd:
                line = b"    ListCommand " + cmd
                lines.insert(i, line)
                i += 1
              break
            m2 = re_Builtin.match(lines[i])
            if m2:
              builtin = True
              ws1 = m2.group(1)
              arg = m2.group(2)
              newarg = b""
              if re_True.match(arg):
                newarg = b"false"
              else:
                newarg = b"true"
              lines[i] = ws1 + b"NeedsFloatPkg " + newarg
            m3 = re_Type.match(lines[i])
            if m3:
              fltype = m3.group(1)
              fltype = fltype.lower()
              if fltype == b"table":
                cmd = b"listoftables"
              elif fltype == b"figure":
                cmd = b"listoffigures"
              # else unknown, which is why we're doing this
            i += 1
          continue

        # This just involved new features, not any changes to old ones
        if 14 <= format <= 22:
          i += 1
          continue

        # Rename I18NPreamble to BabelPreamble
        if format == 13:
            match = re_I18nPreamble.match(lines[i])
            if match:
                lines[i] = match.group(1) + b"BabelPreamble"
                i += 1
                match = re_EndI18nPreamble.match(lines[i])
                while i < len(lines) and not match:
                    i += 1
                    match = re_EndI18nPreamble.match(lines[i])
                lines[i] = match.group(1) + b"EndBabelPreamble"
                i += 1
                continue

        # These just involved new features, not any changes to old ones
        if format == 11 or format == 12:
          i += 1
          continue

        if format == 10:
            match = re_UseMod.match(lines[i])
            if match:
                module = match.group(1)
                lines[i] = b"DefaultModule " + module
            i += 1
            continue

        if format == 9:
            match = re_Counter.match(lines[i])
            if match:
                counterline = i
                i += 1
                while i < len(lines):
                    namem = re_Name.match(lines[i])
                    if namem:
                        name = namem.group(1)
                        lines.pop(i)
                        lines[counterline] = b"Counter %s" % name
                        # we don't need to increment i
                        continue
                    endem = re_End.match(lines[i])
                    if endem:
                        i += 1
                        break
                    i += 1
            i += 1
            continue

        if format == 8:
            # We want to scan for ams-type includes and, if we find them,
            # add corresponding UseModule tags to the layout.
            match = re_AMSMaths.match(lines[i])
            if match:
                addstring(b"theorems-ams", usemodules)
                addstring(b"theorems-ams-extended", usemodules)
                addstring(b"theorems-sec", usemodules)
                lines.pop(i)
                continue
            match = re_AMSMathsPlain.match(lines[i])
            if match:
                addstring(b"theorems-starred", usemodules)
                lines.pop(i)
                continue
            match = re_AMSMathsSeq.match(lines[i])
            if match:
                addstring(b"theorems-ams", usemodules)
                addstring(b"theorems-ams-extended", usemodules)
                lines.pop(i)
                continue
            i += 1
            continue

        # These just involved new features, not any changes to old ones
        if 5 <= format <= 7:
          i += 1
          continue

        if format == 4:
            # Handle conversion to long CharStyle names
            match = re_CharStyle.match(lines[i])
            if match:
                lines[i] = b"InsetLayout CharStyle:%s" % (match.group(3))
                i += 1
                lines.insert(i, b"\tLyXType charstyle")
                i += 1
                lines.insert(i, b"")
                lines[i] = b"\tLabelString %s" % (match.group(3))
            i += 1
            continue

        if format == 3:
            # convert 'providesamsmath x',  'providesmakeidx x',  'providesnatbib x',  'providesurl x' to
            #         'provides amsmath x', 'provides makeidx x', 'provides natbib x', 'provides url x'
            # x is either 0 or 1
            match = re_Provides.match(lines[i])
            if match:
                lines[i] = b"%sProvides %s%s%s" % (match.group(1), match.group(2).lower(),
                                                  match.group(3), match.group(4))
            i += 1
            continue

        if format == 2:
            caption = []

            # delete caption styles
            match = re_Style.match(lines[i])
            if match:
                style = match.group(4).lower()
                if style == b"caption":
                    del lines[i]
                    while i < len(lines) and not re_End.match(lines[i]):
                        caption.append(lines[i])
                        del lines[i]
                    if i == len(lines):
                        error('Incomplete caption style.')
                    else:
                        del lines[i]
                        continue

            # delete undefinition of caption styles
            match = re_NoStyle.match(lines[i])
            if match:
                style = match.group(4).lower()
                if style == b"caption":
                    del lines[i]
                    continue

            # replace the CopyStyle statement with the definition of the real
            # style. This may result in duplicate statements, but that is OK
            # since the second one will overwrite the first one.
            match = re_CopyStyle.match(lines[i])
            if match:
                style = match.group(4).lower()
                if style == b"caption":
                    if len(caption) > 0:
                        lines[i:i+1] = caption
                    else:
                        # FIXME: This style comes from an include file, we
                        # should replace the real style and not this default.
                        lines[i:i+1] = [b'	Margin                First_Dynamic',
                                        b'	LatexType             Command',
                                        b'	LatexName             caption',
                                        b'	NeedProtect           1',
                                        b'	LabelSep              xx',
                                        b'	ParSkip               0.4',
                                        b'	TopSep                0.5',
                                        b'	Align                 Center',
                                        b'	AlignPossible         Center',
                                        b'	LabelType             Sensitive',
                                        b'	LabelString           "Senseless!"',
                                        b'	OptionalArgs          1',
                                        b'	LabelFont',
                                        b'	  Series              Bold',
                                        b'	EndFont']

            i += 1
            continue

        # Delete MaxCounter and remember the value of it
        match = re_MaxCounter.match(lines[i])
        if match:
            level = match.group(4).lower()
            if level == b"counter_chapter":
                maxcounter = 0
            elif level == b"counter_section":
                maxcounter = 1
            elif level == b"counter_subsection":
                maxcounter = 2
            elif level == b"counter_subsubsection":
                maxcounter = 3
            elif level == b"counter_paragraph":
                maxcounter = 4
            elif level == b"counter_subparagraph":
                maxcounter = 5
            elif level == b"counter_enumi":
                maxcounter = 6
            elif level == b"counter_enumii":
                maxcounter = 7
            elif level == b"counter_enumiii":
                maxcounter = 8
            del lines[i]
            continue

        # Replace line
        #
        # LabelType Counter_EnumI
        #
        # with two lines
        #
        # LabelType Counter
        # LabelCounter EnumI
        #
        match = re_LabelType.match(lines[i])
        if match:
            label = match.group(4)
            # Remember indenting space for later reuse in added lines
            space1 = match.group(1)
            # Remember the line for adding the LabelCounter later.
            # We can't do it here because it could shift latextype_line etc.
            labeltype_line = i
            if label[:8].lower() == b"counter_":
                counter = label[8:].lower()
                lines[i] = re_LabelType.sub(b'\\1\\2\\3Counter', lines[i])

        # Remember the LabelString line
        match = re_LabelString.match(lines[i])
        if match:
            labelstring = match.group(4)
            labelstring_line = i

        # Remember the LabelStringAppendix line
        match = re_LabelStringAppendix.match(lines[i])
        if match:
            labelstringappendix = match.group(4)
            labelstringappendix_line = i

        # Remember the LatexType line
        match = re_LatexType.match(lines[i])
        if match:
            latextype = match.group(4).lower()
            latextype_line = i

        # Remember the TocLevel line
        match = re_TocLevel.match(lines[i])
        if match:
            toclevel = match.group(4).lower()

        # Reset variables at the beginning of a style definition
        match = re_Style.match(lines[i])
        if match:
            style = match.group(4).lower()
            counter = b""
            toclevel = b""
            label = b""
            space1 = b""
            labelstring = b""
            labelstringappendix = b""
            labelstring_line = -1
            labelstringappendix_line = -1
            labeltype_line = -1
            latextype = b""
            latextype_line = -1

        if re_End.match(lines[i]):

            # Add a line "LatexType Bib_Environment" if LabelType is Bibliography
            # (or change the existing LatexType)
            if label.lower() == b"bibliography":
                if (latextype_line < 0):
                    lines.insert(i, b"%sLatexType Bib_Environment" % space1)
                    i += 1
                else:
                    lines[latextype_line] = re_LatexType.sub(b'\\1\\2\\3Bib_Environment', lines[latextype_line])

            # Change "LabelType Static" to "LabelType Itemize" for itemize environments
            if latextype == b"item_environment" and label.lower() == b"static":
                lines[labeltype_line] = re_LabelType.sub(b'\\1\\2\\3Itemize', lines[labeltype_line])

            # Change "LabelType Counter_EnumI" to "LabelType Enumerate" for enumerate environments
            if latextype == b"item_environment" and label.lower() == b"counter_enumi":
                lines[labeltype_line] = re_LabelType.sub(b'\\1\\2\\3Enumerate', lines[labeltype_line])
                # Don't add the LabelCounter line later
                counter = ""

            # Replace
            #
            # LabelString "Chapter"
            #
            # with
            #
            # LabelString "Chapter \arabic{chapter}"
            #
            # if this style has a counter. Ditto for LabelStringAppendix.
            # This emulates the hardcoded article style numbering of 1.3
            #
            if counter != b"":
                if style in counters:
                    if labelstring_line < 0:
                        lines.insert(i, b'%sLabelString "%s"' % (space1, counters[style]))
                        i += 1
                    else:
                        new_labelstring = concatenate_label(labelstring, counters[style])
                        lines[labelstring_line] = re_LabelString.sub(
                                b'\\1\\2\\3%s' % new_labelstring.replace(b"\\", b"\\\\"),
                                lines[labelstring_line])
                if style in appendixcounters:
                    if labelstringappendix_line < 0:
                        lines.insert(i, b'%sLabelStringAppendix "%s"' % (space1, appendixcounters[style]))
                        i += 1
                    else:
                        new_labelstring = concatenate_label(labelstring, appendixcounters[style])
                        lines[labelstringappendix_line] = re_LabelStringAppendix.sub(
                                b'\\1\\2\\3%s' % new_labelstring.replace(b"\\", b"\\\\"),
                                lines[labelstringappendix_line])

                # Now we can safely add the LabelCounter line
                lines.insert(labeltype_line + 1, b"%sLabelCounter %s" % (space1, counter))
                i += 1

            # Add the TocLevel setting for sectioning styles
            if toclevel == b"" and style in toclevels and maxcounter <= toclevels[style]:
                lines.insert(i, b'%s\tTocLevel %d' % (space1, toclevels[style]))
                i += 1

        i += 1

    if only_comment:
        lines.insert(i, b"Format 2")
    if usemodules:
        i = formatline + 1
        for mod in usemodules:
            lines.insert(i, b"UseModule " + mod)
            i += 1

    return format + 1


def main(argv):
    args = {}
    args["description"] = "Convert layout file <inputfile> to a newer format."

    parser = argparse.ArgumentParser(**args)

    parser.add_argument("-t", "--to", type=int, dest="format", default= currentFormat,
                        help=("destination layout format, default %i (latest)") % currentFormat)
    parser.add_argument("input_file", nargs='?', type=cmd_arg, default=None,
                        help="input file (default stdin)")
    parser.add_argument("output_file", nargs='?', type=cmd_arg, default=None,
                        help="output file (default stdout)")

    options = parser.parse_args(argv[1:])

    # Open files
    if options.input_file:
        source = open(options.input_file, 'rb')
    elif PY2:
        source = sys.stdin
    else:
        source = sys.stdin.buffer

    if options.output_file:
        output = open(options.output_file, 'wb')
    elif PY2:
        output = sys.stdout
    else:
        output = sys.stdout.buffer

    if options.format > currentFormat:
        error("Format %i does not exist" % options.format);

    # Do the real work
    lines = read(source)
    format = 1
    while (format < options.format):
        format = convert(lines, options.format)
    write(output, lines)

    # Close files
    if options.input_file:
        source.close()
    if options.output_file:
        output.close()

    return 0


if __name__ == "__main__":
    main(sys.argv)
