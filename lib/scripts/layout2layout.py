# -*- coding: utf-8 -*-

# file layout2layout.py
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.

# author Georg Baum

# Full author contact details are available in file CREDITS

# This script will update a .layout file to current format


import os, re, string, sys

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

# Do not forget to document format change in Customization
# Manual (section "Declaring a new text class").

# You might also want to consider running the
# development/tools/updatelayouts.py script to update all
# layout files to the new format.

currentFormat = 54


def usage(prog_name):
    return ("Usage: %s inputfile outputfile\n" % prog_name +
            "or     %s <inputfile >outputfile" % prog_name)


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
    output.write(os.linesep.join(lines) + os.linesep)


# Concatenates old and new in an intelligent way:
# If old is wrapped in ", they are stripped. The result is wrapped in ".
def concatenate_label(old, new):
    # Don't use strip as long as we support python 1.5.2
    if old[0] == '"':
        return old[0:-1] + new + '"'
    else:
        return '"' + old + new + '"'

# appends a string to a list unless it's already there
def addstring(s, l):
    if l.count(s) > 0:
        return
    l.append(s)


def convert(lines):
    " Convert to new format."
    re_Comment = re.compile(r'^(\s*)#')
    re_Counter = re.compile(r'\s*Counter\s*', re.IGNORECASE)
    re_Name = re.compile(r'\s*Name\s+(\S+)\s*', re.IGNORECASE)
    re_UseMod = re.compile(r'^\s*UseModule\s+(.*)', re.IGNORECASE)
    re_Empty = re.compile(r'^(\s*)$')
    re_Format = re.compile(r'^(\s*)(Format)(\s+)(\S+)', re.IGNORECASE)
    re_Preamble = re.compile(r'^(\s*)Preamble', re.IGNORECASE)
    re_EndPreamble = re.compile(r'^(\s*)EndPreamble', re.IGNORECASE)
    re_LangPreamble = re.compile(r'^(\s*)LangPreamble', re.IGNORECASE)
    re_EndLangPreamble = re.compile(r'^(\s*)EndLangPreamble', re.IGNORECASE)
    re_BabelPreamble = re.compile(r'^(\s*)BabelPreamble', re.IGNORECASE)
    re_EndBabelPreamble = re.compile(r'^(\s*)EndBabelPreamble', re.IGNORECASE)
    re_MaxCounter = re.compile(r'^(\s*)(MaxCounter)(\s+)(\S+)', re.IGNORECASE)
    re_LabelType = re.compile(r'^(\s*)(LabelType)(\s+)(\S+)', re.IGNORECASE)
    re_LabelString = re.compile(r'^(\s*)(LabelString)(\s+)(("[^"]+")|(\S+))', re.IGNORECASE)
    re_LabelStringAppendix = re.compile(r'^(\s*)(LabelStringAppendix)(\s+)(("[^"]+")|(\S+))', re.IGNORECASE)
    re_LatexType = re.compile(r'^(\s*)(LatexType)(\s+)(\S+)', re.IGNORECASE)
    re_Style = re.compile(r'^(\s*)(Style)(\s+)(\S+)', re.IGNORECASE)
    re_CopyStyle = re.compile(r'^(\s*)(CopyStyle)(\s+)(\S+)', re.IGNORECASE)
    re_NoStyle = re.compile(r'^(\s*)(NoStyle)(\s+)(\S+)', re.IGNORECASE)
    re_End = re.compile(r'^(\s*)(End)(\s*)$', re.IGNORECASE)
    re_Provides = re.compile(r'^(\s*)Provides(\S+)(\s+)(\S+)', re.IGNORECASE)
    re_CharStyle = re.compile(r'^(\s*)CharStyle(\s+)(\S+)$', re.IGNORECASE)
    re_CiteFormat = re.compile(r'^(\s*)(CiteFormat)(?:(\s*)()|(\s+)(default|authoryear|numerical))', re.IGNORECASE)
    re_AMSMaths = re.compile(r'^\s*Input ams(?:math|def)s.inc\s*')
    re_AMSMathsPlain = re.compile(r'^\s*Input amsmaths-plain.inc\s*')
    re_AMSMathsSeq = re.compile(r'^\s*Input amsmaths-seq.inc\s*')
    re_TocLevel = re.compile(r'^(\s*)(TocLevel)(\s+)(\S+)', re.IGNORECASE)
    re_I18nPreamble = re.compile(r'^(\s*)I18nPreamble', re.IGNORECASE)
    re_EndI18nPreamble = re.compile(r'^(\s*)EndI18nPreamble', re.IGNORECASE)
    re_Float = re.compile(r'^\s*Float\s*$', re.IGNORECASE)
    re_Type = re.compile(r'\s*Type\s+(\w+)', re.IGNORECASE)
    re_Builtin = re.compile(r'^(\s*)LaTeXBuiltin\s+(\w*)', re.IGNORECASE)
    re_True = re.compile(r'^\s*(?:true|1)\s*$', re.IGNORECASE)
    re_InsetLayout = re.compile(r'^\s*InsetLayout\s+(?:Custom|CharStyle|Element):(\S+)\s*$', re.IGNORECASE)
    re_ResetsFont = re.compile(r'^(\s*)ResetsFont(\s+)(\S+)$', re.IGNORECASE)
    # with quotes
    re_QInsetLayout = re.compile(r'^\s*InsetLayout\s+"(?:Custom|CharStyle|Element):([^"]+)"\s*$', re.IGNORECASE)
    re_InsetLayout_CopyStyle = re.compile(r'^\s*CopyStyle\s+(?:Custom|CharStyle|Element):(\S+)\s*$', re.IGNORECASE)
    re_QInsetLayout_CopyStyle = re.compile(r'^\s*CopyStyle\s+"(?:Custom|CharStyle|Element):([^"]+)"\s*$', re.IGNORECASE)
    re_NeedsFloatPkg = re.compile(r'^(\s*)NeedsFloatPkg\s+(\w+)\s*$', re.IGNORECASE)
    re_Fill = re.compile(r'^\s*Fill_(?:Top|Bottom).*$', re.IGNORECASE)
    re_InsetLayout2 = re.compile(r'^\s*InsetLayout\s+(\S+)\s*$', re.IGNORECASE)
    # with quotes
    re_QInsetLayout2 = re.compile(r'^\s*InsetLayout\s+"([^"]+)"\s*$', re.IGNORECASE)
    re_IsFlex = re.compile(r'\s*LyXType.*$', re.IGNORECASE)
    re_CopyStyle2 = re.compile(r'(\s*CopyStyle\s+)"?([^"]+)"?\s*$')
    re_Separator = re.compile(r'^(?:(-*)|(\s*))(Separator|EndOfSlide)(?:(-*)|(\s*))$', re.IGNORECASE)
    # for categories
    re_Declaration = re.compile(r'^#\s*\\Declare\w+Class.*$')
    re_ExtractCategory = re.compile(r'^(#\s*\\Declare\w+Class(?:\[[^]]*?\])?){([^(]+?)\s+\(([^)]+?)\)\s*}\s*$')
    ConvDict = {"article": "Articles", "book" : "Books", "letter" : "Letters", "report": "Reports", \
                "presentation" : "Presentations", "curriculum vitae" : "Curricula Vitae", "handout" : "Handouts"}
    # Arguments
    re_OptArgs = re.compile(r'^(\s*)OptionalArgs(\s+)(\d+)\D*$', re.IGNORECASE)
    re_ReqArgs = re.compile(r'^(\s*)RequiredArgs(\s+)(\d+)\D*$', re.IGNORECASE)
    
    # various changes associated with changing how chapters are handled
    re_LabelTypeIsCounter = re.compile(r'^(\s*)LabelType(\s*)Counter\s*$', re.IGNORECASE)
    re_TopEnvironment = re.compile(r'^(\s*)LabelType(\s+)Top_Environment\s*$', re.IGNORECASE)
    re_CenteredEnvironment = re.compile(r'^(\s*)LabelType(\s+)Centered_Top_Environment\s*$', re.IGNORECASE)
    re_ChapterStyle = re.compile(r'^\s*Style\s+Chapter\s*$', re.IGNORECASE)


    # counters for sectioning styles (hardcoded in 1.3)
    counters = {"part"          : "\\Roman{part}",
                "chapter"       : "\\arabic{chapter}",
                "section"       : "\\arabic{section}",
                "subsection"    : "\\arabic{section}.\\arabic{subsection}",
                "subsubsection" : "\\arabic{section}.\\arabic{subsection}.\\arabic{subsubsection}",
                "paragraph"     : "\\arabic{section}.\\arabic{subsection}.\\arabic{subsubsection}.\\arabic{paragraph}",
                "subparagraph"  : "\\arabic{section}.\\arabic{subsection}.\\arabic{subsubsection}.\\arabic{paragraph}.\\arabic{subparagraph}"}

    # counters for sectioning styles in appendix (hardcoded in 1.3)
    appendixcounters = {"chapter"       : "\\Alph{chapter}",
                        "section"       : "\\Alph{section}",
                        "subsection"    : "\\arabic{section}.\\arabic{subsection}",
                        "subsubsection" : "\\arabic{section}.\\arabic{subsection}.\\arabic{subsubsection}",
                        "paragraph"     : "\\arabic{section}.\\arabic{subsection}.\\arabic{subsubsection}.\\arabic{paragraph}",
                        "subparagraph"  : "\\arabic{section}.\\arabic{subsection}.\\arabic{subsubsection}.\\arabic{paragraph}.\\arabic{subparagraph}"}

    # Value of TocLevel for sectioning styles
    toclevels = {"part"          : -1,
                 "chapter"       : 0,
                 "section"       : 1,
                 "subsection"    : 2,
                 "subsubsection" : 3,
                 "paragraph"     : 4,
                 "subparagraph"  : 5}

    i = 0
    only_comment = 1
    counter = ""
    toclevel = ""
    label = ""
    labelstring = ""
    labelstringappendix = ""
    space1 = ""
    labelstring_line = -1
    labelstringappendix_line = -1
    labeltype_line = -1
    latextype = ""
    latextype_line = -1
    style = ""
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
                  lines[i] = lpre + "{" + lnam + "}"
                  lines.insert(i+1, "#  \\DeclareCategory{" + lcat + "}")
                  i += 1 
          i += 1
          continue

        # insert file format if not already there
        if (only_comment):
            match = re_Format.match(lines[i])
            if match:
                formatline = i
                format = int(match.group(4))
                if format > 1 and format < currentFormat:
                    lines[i] = "Format %d" % (format + 1)
                    only_comment = 0
                elif format == currentFormat:
                    # nothing to do
                    return format
                else:
                    error('Cannot convert file format %s to %s' % (format, currentFormat))
            else:
                lines.insert(i, "Format 2")
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

        if format >= 50 and format <= 53:
            # nothing to do.
            i += 1
            continue

        if format == 49:
            separator = []

            # delete separator styles
            match = re_Style.match(lines[i])
            if match:
                style = string.lower(match.group(4))
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
                style = string.lower(match.group(4))
                if re_Separator.match(style):
                    del lines[i]
                    continue

            # replace the CopyStyle statement with the definition of the real
            # style. This may result in duplicate statements, but that is OK
            # since the second one will overwrite the first one.
            match = re_CopyStyle.match(lines[i])
            if match:
                style = string.lower(match.group(4))
                if re_Separator.match(style):
                    if len(separator) > 0:
                        lines[i:i+1] = separator
                    else:
                        # FIXME: If this style was redefined in an include file,
                        # we should replace the real style and not this default.
                        lines[i:i+1] = ['	Category              MainText',
                                        '	KeepEmpty             1',
                                        '	Margin                Dynamic',
                                        '	LatexType             Paragraph',
                                        '	LatexName             dummy',
                                        '	ParIndent             MM',
                                        '	Align                 Block',
                                        '	LabelType             Static',
                                        '	LabelString           "--- Separate Environment ---"',
                                        '	LabelFont',
                                        '	  Family              Roman',
                                        '	  Series              Medium',
                                        '	  Size                Normal',
                                        '	  Color               Blue',
                                        '	EndFont',
                                        '	HTMLLabel             NONE']
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

            name = string.lower(match.group(1))
            if name != "flex" and name != "\"flex\"" and name[0:5] != "flex:" and name [0:6] != "\"flex:":
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
              lines.insert(i, "\tResetsFont true")

            continue

        if format >= 44 and format <= 47:
            # nothing to do.
            i += 1
            continue

        if format == 43:
          match = re_LabelTypeIsCounter.match(lines[i])
          if match:
            if inchapter:
             lines[i] = match.group(1) + "LabelType" + match.group(2) + "Above"              
            else:
              lines[i] = match.group(1) + "LabelType" + match.group(2) + "Static"

          match = re_TopEnvironment.match(lines[i])
          if match:
            lines[i] = match.group(1) + "LabelType" + match.group(2) + "Above"

          match = re_CenteredEnvironment.match(lines[i])
          if match:
            lines[i] = match.group(1) + "LabelType" + match.group(2) + "Centered"

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
          if lines[i] == "InsetLayout Caption":
            lines[i] = "InsetLayout Caption:Standard"
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
                    lines[i] = space1 + "ResetArgs\t1"
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
                        newarg = [ '%sArgument 1' % (space1),
                                   '%s\tLabelString\t\"Optional Layout Argument\"' % (space1),
                                   '%sEndArgument' % (space1)]
                    elif opts > 1:
                        actopt = 1
                        while actopt < (opts + 1):
                            newarg += [ '%sArgument %d' % (space1, actopt),
                               '%s\tLabelString\t\"Optional Layout Argument %d\"' % (space1, actopt),
                               '%sEndArgument' % (space1)]
                            actopt += 1
                # Now the mandatories
                if reqs > 0:
                    actopt = opts + 1
                    while actopt < (opts +  reqs + 1):
                        newarg += [ '%sArgument %d' % (space1, actopt),
                           '%s\tLabelString\t"Required Layout Argument %d"' % (space1, actopt - opts),
                           '%s\tMandatory\t1' % (space1),
                           '%sEndArgument' % (space1)]
                        actopt += 1
                # Since we replace the "End" line, re-add this line
                if len(newarg) > 1:
                    newarg += ['End']
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
            match = re_CiteFormat.match(lines[i]);
            if match and match.group(4) == "":
                lines[i] = match.group(0) + " default"
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
              lines[i] = match.group(1) + "\"Flex:" + style + "\""
            i += 1
            continue

          name = match.group(1)
          names = name.split(":", 1)
          if len(names) > 1 and names[0] == "Flex":
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
            lines[i] = "InsetLayout \"Flex:" + name + "\""

          i += 1
          continue

        if format == 33:
          m = re_Fill.match(lines[i])
          if m:
            lines[i] = ""
          i += 1
          continue

        if format == 32:
          match = re_NeedsFloatPkg.match(lines[i])
          if match:
            space = match.group(1)
            val = match.group(2)
            lines[i] = space + "UsesFloatPkg " + val
            newval = 'true'
            if val == '1' or val.lower() == 'true':
              newval = 'false'
            lines.insert(i, space + "IsPredefined " + newval)
            i += 1
          i += 1
          continue

        # Only new features
        if format >= 29 and format <= 31:
          i += 1
          continue

        if format == 28:
          match = re_InsetLayout.match(lines[i])
          if match:
            lines[i] = "InsetLayout Flex:" + match.group(1)
          else:
            match = re_QInsetLayout.match(lines[i])
            if match:
              lines[i] = "InsetLayout \"Flex:" + match.group(1) + "\""
            else:
              match = re_InsetLayout_CopyStyle.match(lines[i])
              if match:
                lines[i] = "\tCopyStyle Flex:" + match.group(1)
              else:
                match = re_QInsetLayout_CopyStyle.match(lines[i])
                if match:
                  lines[i] = "\tCopyStyle \"Flex:" + match.group(1) + "\""
          i += 1
          continue
        
        # Only new features
        if format >= 24 and format <= 27:
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
          cmd = ""
          while True and i < len(lines):
            m1 = re_End.match(lines[i])
            if m1:
              if builtin and cmd:
                line = "    ListCommand " + cmd
                lines.insert(i, line)
                i += 1
              break
            m2 = re_Builtin.match(lines[i])
            if m2:
              builtin = True
              ws1 = m2.group(1)
              arg = m2.group(2)
              newarg = ""
              if re_True.match(arg):
                newarg = "false"
              else:
                newarg = "true"
              lines[i] = ws1 + "NeedsFloatPkg " + newarg
            m3 = re_Type.match(lines[i])
            if m3:
              fltype = m3.group(1)
              fltype = fltype.lower()
              if fltype == "table":
                cmd = "listoftables"
              elif fltype == "figure":
                cmd = "listoffigures"
              # else unknown, which is why we're doing this
            i += 1
          continue              
          
        # This just involved new features, not any changes to old ones
        if format >= 14 and format <= 22:
          i += 1
          continue

        # Rename I18NPreamble to BabelPreamble
        if format == 13:
            match = re_I18nPreamble.match(lines[i])
            if match:
                lines[i] = match.group(1) + "BabelPreamble"
                i += 1
                match = re_EndI18nPreamble.match(lines[i])
                while i < len(lines) and not match:
                    i += 1
                    match = re_EndI18nPreamble.match(lines[i])
                lines[i] = match.group(1) + "EndBabelPreamble"
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
                lines[i] = "DefaultModule " + module
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
                        lines[counterline] = "Counter %s" % name
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
                addstring("theorems-ams", usemodules)
                addstring("theorems-ams-extended", usemodules)
                addstring("theorems-sec", usemodules)
                lines.pop(i)
                continue
            match = re_AMSMathsPlain.match(lines[i])
            if match:
                addstring("theorems-starred", usemodules)
                lines.pop(i)
                continue
            match = re_AMSMathsSeq.match(lines[i])
            if match:
                addstring("theorems-ams", usemodules)
                addstring("theorems-ams-extended", usemodules)
                lines.pop(i)
                continue
            i += 1
            continue

        # These just involved new features, not any changes to old ones
        if format >= 5 and format <= 7:
          i += 1
          continue

        if format == 4:
            # Handle conversion to long CharStyle names
            match = re_CharStyle.match(lines[i])
            if match:
                lines[i] = "InsetLayout CharStyle:%s" % (match.group(3))
                i += 1
                lines.insert(i, "\tLyXType charstyle")
                i += 1
                lines.insert(i, "")
                lines[i] = "\tLabelString %s" % (match.group(3))
            i += 1
            continue

        if format == 3:
            # convert 'providesamsmath x',  'providesmakeidx x',  'providesnatbib x',  'providesurl x' to
            #         'provides amsmath x', 'provides makeidx x', 'provides natbib x', 'provides url x'
            # x is either 0 or 1
            match = re_Provides.match(lines[i])
            if match:
                lines[i] = "%sProvides %s%s%s" % (match.group(1), match.group(2).lower(),
                                                  match.group(3), match.group(4))
            i += 1
            continue

        if format == 2:
            caption = []

            # delete caption styles
            match = re_Style.match(lines[i])
            if match:
                style = string.lower(match.group(4))
                if style == "caption":
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
                style = string.lower(match.group(4))
                if style == "caption":
                    del lines[i]
                    continue

            # replace the CopyStyle statement with the definition of the real
            # style. This may result in duplicate statements, but that is OK
            # since the second one will overwrite the first one.
            match = re_CopyStyle.match(lines[i])
            if match:
                style = string.lower(match.group(4))
                if style == "caption":
                    if len(caption) > 0:
                        lines[i:i+1] = caption
                    else:
                        # FIXME: This style comes from an include file, we
                        # should replace the real style and not this default.
                        lines[i:i+1] = ['	Margin                First_Dynamic',
                                        '	LatexType             Command',
                                        '	LatexName             caption',
                                        '	NeedProtect           1',
                                        '	LabelSep              xx',
                                        '	ParSkip               0.4',
                                        '	TopSep                0.5',
                                        '	Align                 Center',
                                        '	AlignPossible         Center',
                                        '	LabelType             Sensitive',
                                        '	LabelString           "Senseless!"',
                                        '	OptionalArgs          1',
                                        '	LabelFont',
                                        '	  Series              Bold',
                                        '	EndFont']

            i += 1
            continue

        # Delete MaxCounter and remember the value of it
        match = re_MaxCounter.match(lines[i])
        if match:
            level = match.group(4)
            if string.lower(level) == "counter_chapter":
                maxcounter = 0
            elif string.lower(level) == "counter_section":
                maxcounter = 1
            elif string.lower(level) == "counter_subsection":
                maxcounter = 2
            elif string.lower(level) == "counter_subsubsection":
                maxcounter = 3
            elif string.lower(level) == "counter_paragraph":
                maxcounter = 4
            elif string.lower(level) == "counter_subparagraph":
                maxcounter = 5
            elif string.lower(level) == "counter_enumi":
                maxcounter = 6
            elif string.lower(level) == "counter_enumii":
                maxcounter = 7
            elif string.lower(level) == "counter_enumiii":
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
            if string.lower(label[:8]) == "counter_":
                counter = string.lower(label[8:])
                lines[i] = re_LabelType.sub(r'\1\2\3Counter', lines[i])

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
            latextype = string.lower(match.group(4))
            latextype_line = i

        # Remember the TocLevel line
        match = re_TocLevel.match(lines[i])
        if match:
            toclevel = string.lower(match.group(4))

        # Reset variables at the beginning of a style definition
        match = re_Style.match(lines[i])
        if match:
            style = string.lower(match.group(4))
            counter = ""
            toclevel = ""
            label = ""
            space1 = ""
            labelstring = ""
            labelstringappendix = ""
            labelstring_line = -1
            labelstringappendix_line = -1
            labeltype_line = -1
            latextype = ""
            latextype_line = -1

        if re_End.match(lines[i]):

            # Add a line "LatexType Bib_Environment" if LabelType is Bibliography
            # (or change the existing LatexType)
            if string.lower(label) == "bibliography":
                if (latextype_line < 0):
                    lines.insert(i, "%sLatexType Bib_Environment" % space1)
                    i += 1
                else:
                    lines[latextype_line] = re_LatexType.sub(r'\1\2\3Bib_Environment', lines[latextype_line])

            # Change "LabelType Static" to "LabelType Itemize" for itemize environments
            if latextype == "item_environment" and string.lower(label) == "static":
                lines[labeltype_line] = re_LabelType.sub(r'\1\2\3Itemize', lines[labeltype_line])

            # Change "LabelType Counter_EnumI" to "LabelType Enumerate" for enumerate environments
            if latextype == "item_environment" and string.lower(label) == "counter_enumi":
                lines[labeltype_line] = re_LabelType.sub(r'\1\2\3Enumerate', lines[labeltype_line])
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
            if counter != "":
                if style in counters:
                    if labelstring_line < 0:
                        lines.insert(i, '%sLabelString "%s"' % (space1, counters[style]))
                        i += 1
                    else:
                        new_labelstring = concatenate_label(labelstring, counters[style])
                        lines[labelstring_line] = re_LabelString.sub(
                                r'\1\2\3%s' % new_labelstring.replace("\\", "\\\\"),
                                lines[labelstring_line])
                if style in appendixcounters:
                    if labelstringappendix_line < 0:
                        lines.insert(i, '%sLabelStringAppendix "%s"' % (space1, appendixcounters[style]))
                        i += 1
                    else:
                        new_labelstring = concatenate_label(labelstring, appendixcounters[style])
                        lines[labelstringappendix_line] = re_LabelStringAppendix.sub(
                                r'\1\2\3%s' % new_labelstring.replace("\\", "\\\\"),
                                lines[labelstringappendix_line])

                # Now we can safely add the LabelCounter line
                lines.insert(labeltype_line + 1, "%sLabelCounter %s" % (space1, counter))
                i += 1

            # Add the TocLevel setting for sectioning styles
            if toclevel == "" and style in toclevels and maxcounter <= toclevels[style]:
                lines.insert(i, '%s\tTocLevel %d' % (space1, toclevels[style]))
                i += 1

        i += 1

    if only_comment:
        lines.insert(i, "Format 2")
    if usemodules:
        i = formatline + 1
        for mod in usemodules:
            lines.insert(i, "UseModule " + mod)
            i += 1

    return format + 1


def main(argv):

    # Open files
    if len(argv) == 1:
        source = sys.stdin
        output = sys.stdout
    elif len(argv) == 3:
        source = open(argv[1], 'rb')
        output = open(argv[2], 'wb')
    else:
        error(usage(argv[0]))

    # Do the real work
    lines = read(source)
    format = 1
    while (format < currentFormat):
        format = convert(lines)
    write(output, lines)

    # Close files
    if len(argv) == 3:
        source.close()
        output.close()

    return 0


if __name__ == "__main__":
    main(sys.argv)
