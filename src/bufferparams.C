/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *	     Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#include <config.h>
#include <cstdlib>

#ifdef __GNUG__
#pragma implementation "bufferparams.h"
#endif

#include "bufferparams.h"
#include "tex-strings.h"
#include "layout.h"
#include "vspace.h"
#include "debug.h"
#include "support/lyxlib.h"
#include "support/lstrings.h"
#include "lyxrc.h"
#include "language.h"

using std::ostream;
using std::endl;

BufferParams::BufferParams()
{
	paragraph_separation = PARSEP_INDENT;
	defskip = VSpace(VSpace::MEDSKIP); 
	quotes_language = InsetQuotes::EnglishQ;
	quotes_times = InsetQuotes::DoubleQ;
	fontsize = "default";
	// Initialize textclass to point to article. if `first' is
	// true in the returned pair, then `second' is the textclass
	// number; if it is false, second is 0. In both cases, second
	// is what we want.
	textclass = textclasslist.NumberOfClass("article").second;

        /*  PaperLayout */
	papersize = PAPER_DEFAULT;
        papersize2 = VM_PAPER_DEFAULT; /* DEFAULT */
        paperpackage = PACKAGE_NONE;
	orientation = ORIENTATION_PORTRAIT;
        use_geometry = false;
        use_amsmath = false;
        use_natbib = false;
	use_numerical_citations = false;
	secnumdepth = 3;
	tocdepth = 3;
	language = default_language;
	fonts = "default";
	inputenc = "auto";
	graphicsDriver = "default";
	sides = LyXTextClass::OneSide;
	columns = 1;
	pagestyle = "default";
	for (int iter = 0; iter < 4; ++iter) {
		user_defined_bullets[iter] = temp_bullets[iter] 
			                   = ITEMIZE_DEFAULTS[iter];
	}
}


void BufferParams::writeFile(ostream & os) const
{
	// The top of the file is written by the buffer.
	// Prints out the buffer info into the .lyx file given by file

 	// the textclass
 	os << "\\textclass " << textclasslist.NameOfClass(textclass) << '\n';
	
	// then the the preamble
	if (!preamble.empty()) {
		// remove '\n' from the end of preamble
		string const tmppreamble = strip(preamble, '\n');
		os << "\\begin_preamble\n"
		   << tmppreamble
		   << "\n\\end_preamble\n";
	}
      
	/* the options */ 
	if (!options.empty()) {
		os << "\\options " << options << '\n';
	}
   
	/* then the text parameters */
	os << "\\language " << language->lang()
	   << "\n\\inputencoding " << inputenc
	   << "\n\\fontscheme " << fonts
	   << "\n\\graphics " << graphicsDriver << '\n';

	if (!float_placement.empty()) {
		os << "\\float_placement " << float_placement << '\n';
	}
	os << "\\paperfontsize " << fontsize << '\n';

	spacing.writeFile(os);

	os << "\\papersize " << string_papersize[papersize2]
	   << "\n\\paperpackage " << string_paperpackages[paperpackage]
	   << "\n\\use_geometry " << use_geometry
	   << "\n\\use_amsmath " << use_amsmath
	   << "\n\\use_natbib " << use_natbib
	   << "\n\\use_numerical_citations " << use_numerical_citations
	   << "\n\\paperorientation " << string_orientation[orientation]
	   << '\n';
        if (!paperwidth.empty())
		os << "\\paperwidth "
		   << VSpace(paperwidth).asLyXCommand() << '\n';
        if (!paperheight.empty())
		os << "\\paperheight "
		   << VSpace(paperheight).asLyXCommand() << '\n';
        if (!leftmargin.empty())
		os << "\\leftmargin "
		   << VSpace(leftmargin).asLyXCommand() << '\n';
        if (!topmargin.empty())
		os << "\\topmargin "
		   << VSpace(topmargin).asLyXCommand() << '\n';
        if (!rightmargin.empty())
		os << "\\rightmargin "
		   << VSpace(rightmargin).asLyXCommand() << '\n';
        if (!bottommargin.empty())
		os << "\\bottommargin "
		   << VSpace(bottommargin).asLyXCommand() << '\n';
        if (!headheight.empty())
		os << "\\headheight "
		   << VSpace(headheight).asLyXCommand() << '\n';
        if (!headsep.empty())
		os << "\\headsep "
		   << VSpace(headsep).asLyXCommand() << '\n';
        if (!footskip.empty())
		os << "\\footskip "
		   << VSpace(footskip).asLyXCommand() << '\n';
	os << "\\secnumdepth " << secnumdepth
	   << "\n\\tocdepth " << tocdepth
	   << "\n\\paragraph_separation "
	   << string_paragraph_separation[paragraph_separation]
	   << "\n\\defskip " << defskip.asLyXCommand()
	   << "\n\\quotes_language "
	   << string_quotes_language[quotes_language] << '\n';
	switch (quotes_times) {
		// An output operator for insetquotes would be nice
	case InsetQuotes::SingleQ:
		os << "\\quotes_times 1\n"; break;
	case InsetQuotes::DoubleQ: 
		os << "\\quotes_times 2\n"; break;
	}
	os << "\\papercolumns " << columns
	   << "\n\\papersides " << sides
	   << "\n\\paperpagestyle " << pagestyle << '\n';
	for (int i = 0; i < 4; ++i) {
		if (user_defined_bullets[i] != ITEMIZE_DEFAULTS[i]) {
			if (user_defined_bullets[i].getFont() != -1) {
				os << "\\bullet " << i
				   << "\n\t"
				   << user_defined_bullets[i].getFont()
				   << "\n\t"
				   << user_defined_bullets[i].getCharacter()
				   << "\n\t"
				   << user_defined_bullets[i].getSize()
				   << "\n\\end_bullet\n";
			}
			else {
				os << "\\bulletLaTeX " << i
				   << "\n\t"
				   << user_defined_bullets[i].getText()
				   << "\n\\end_bullet\n";
			}
		}
	}
}


void BufferParams::useClassDefaults() {
	LyXTextClass const & tclass = textclasslist.TextClass(textclass);

	sides = tclass.sides();
	columns = tclass.columns();
	pagestyle = tclass.pagestyle();
	options = tclass.options();
	secnumdepth = tclass.secnumdepth();
	tocdepth = tclass.tocdepth();
}


void BufferParams::readPreamble(LyXLex & lex)
{
	if (lex.GetString() != "\\begin_preamble")
		lyxerr << "Error (BufferParams::readPreamble):"
			"consistency check failed." << endl;

	preamble = lex.getLongString("\\end_preamble");
}


void BufferParams::readLanguage(LyXLex & lex)
{
	if (!lex.next()) return;
	
	string const tmptok = lex.GetString();

	// check if tmptok is part of tex_babel in tex-defs.h
	language = languages.getLanguage(tmptok);
	if (!language) {
		// Language tmptok was not found
		language = default_language;
		lyxerr << "Warning: Setting language `"
		       << tmptok << "' to `" << language->lang()
		       << "'." << endl;
	}
}


void BufferParams::readGraphicsDriver(LyXLex & lex)
{
	if (!lex.next()) return;
	
	string const tmptok = lex.GetString();
	// check if tmptok is part of tex_graphics in tex_defs.h
	int n = 0;
	while (true) {
		string const test = tex_graphics[n++];
		
		if (test == tmptok) {	 
			graphicsDriver = tmptok;
			break;
		} else if (test == "last_item") {
			lex.printError(
				"Warning: graphics driver `$$Token' not recognized!\n"
				"         Setting graphics driver to `default'.\n");
			graphicsDriver = "default";
			break;
		}      
	}
}
