/*
  This file is part of
  ======================================================
  
  LyX, The Document Processor
  
  Copyright (C) 1995 Matthias Ettrich
  Copyright (C) 1995-1998 The LyX Team.
  ======================================================
  */

#include <config.h>

#include "lyx_sty.h"

//\NeedsTeXFormat{LaTeX2e}
//\ProvidesPackage{lyx}[1996/01/11 LLE v0.2 (LyX LaTeX Extensions)]
//\message{LyX LaTeX Extensions (LLE v0.2) of 11-Jan-1996.} 

// include this always
LString const lyx_def =
	"\\providecommand{\\LyX}{L\\kern-.1667em\\lower.25em\\hbox{Y}\\kern-.125emX\\@}";
	
LString const lyxline_def =
	"\\newcommand{\\lyxline}[1]{\n"
	"  {#1 \\vspace{1ex} \\hrule width \\columnwidth \\vspace{1ex}}\n"
	"}";

LString const noun_def = "\\newcommand{\\noun}[1]{\\textsc{#1}}";

LString const lyxarrow_def = 
      "\\newcommand{\\lyxarrow}{\\leavevmode\\,$\\triangleright$\\,\\allowbreak}";

// for quotes without babel. This does not give perfect results, but
// anybody serious about non-english quotes should use babel (JMarc).



LString const quotedblbase_def =
	"\\ProvideTextCommandDefault{\\quotedblbase}{%\n"
	"  \\raisebox{-1.4ex}[1ex][.5ex]{\\textquotedblright}%\n"
	"  \\penalty10000\\hskip0em\\relax%\n"
	"}";

LString const quotesinglbase_def =
	"\\ProvideTextCommandDefault{\\quotesinglbase}{%\n"
	"  \\raisebox{-1.4ex}[1ex][.5ex]{\\textquoteright}%\n"
	"  \\penalty10000\\hskip0em\\relax%\n"
	"}";

LString const guillemotleft_def =
        "\\ProvideTextCommandDefault{\\guillemotleft}{%\n"
        "  {\\usefont{U}{lasy}{m}{n}\\char'50\\kern-.15em\\char'50}%\n"
        "\\penalty10000\\hskip0pt\\relax%\n"
        "}";

LString const guillemotright_def =
        "\\ProvideTextCommandDefault{\\guillemotright}{%\n"
        "  \\penalty10000\\hskip0pt%\n"
        "  {\\usefont{U}{lasy}{m}{n}\\char'51\\kern-.15em\\char'51}%\n"
        "}";

LString const guilsinglleft_def =
        "\\ProvideTextCommandDefault{\\guilsinglleft}{%\n"
        "  {\\usefont{U}{lasy}{m}{n}\\char'50}%\n"
        "  \\penalty10000\\hskip0pt\\relax%\n"
        "}";

LString const guilsinglright_def =
        "\\ProvideTextCommandDefault{\\guilsinglright}{%\n"
        "  \\penalty10000\\hskip0pt%\n"
        "  {\\usefont{U}{lasy}{m}{n}\\char'51}%\n"
        "}";

LString const paragraphindent_def =
	"\\newenvironment{LyXParagraphIndent}[1]%\n"
	"{\n"
	"  \\begin{list}{}{%\n"
	"    \\setlength\\topsep{0pt}%\n"
	"    \\addtolength{\\leftmargin}{#1}\n"
// 	"%%    \\addtolength{\\leftmargin}{#1\\textwidth}\n"
// 	"%%    \\setlength{\\textwidth}{#2\\textwidth}\n"
// 	"%%    \\setlength\\listparindent\\parindent%\n"
// 	"%%    \\setlength\\itemindent\\parindent%\n"
	"    \\setlength\\parsep{0pt plus 1pt}%\n"
	"  }\n"
	"  \\item[]\n"
	"}\n"
	"{\\end{list}}\n";

LString const floatingfootnote_def =
	"%% Special footnote code from the package 'stblftnt.sty'\n"
	"%% Author: Robin Fairbairns -- Last revised Dec 13 1996\n"
	"\\let\\SF@@footnote\\footnote\n"
	"\\def\\footnote{\\ifx\\protect\\@typeset@protect\n"
	"    \\expandafter\\SF@@footnote\n"
	"  \\else\n"
	"    \\expandafter\\SF@gobble@opt\n"
	"  \\fi\n"
	"}\n"
	"\\expandafter\\def\\csname SF@gobble@opt \\endcsname{\\@ifnextchar[%]\n"
	"  \\SF@gobble@twobracket\n"
	"  \\@gobble\n"
	"}\n"
	"\\edef\\SF@gobble@opt{\\noexpand\\protect\n"
	"  \\expandafter\\noexpand\\csname SF@gobble@opt \\endcsname}\n"
	"\\def\\SF@gobble@twobracket[#1]#2{}\n";

LString const minipageindent_def =
	"%% This length is the backup for minipages of the \\parindent\n"
	"\\newlength{\\LyXMinipageIndent}\n"
	"\\setlength{\\LyXMinipageIndent}{\\parindent}\n";

LString const boldsymbol_def =
	"%% Bold symbol macro for standard LaTeX users\n"
	"\\newcommand{\\boldsymbol}[1]{\\mbox{\\boldmath $#1$}}\n";

LString const binom_def =
	"%% Binom macro for standard LaTeX users\n"
	"\\newcommand{\\binom}[2]{{#1 \\choose #2}}\n";

