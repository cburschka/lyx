/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *	     Copyright 1995 Matthias Ettrich
 *           Copyright 1995-1999 The LyX Team.
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
	secnumdepth = 3;
	tocdepth = 3;
	language = "default";
	fonts = "default";
	inputenc = "latin1";
	graphicsDriver = "default";
	sides = LyXTextClass::OneSide;
	columns = 1;
	pagestyle = "default";
	for(int iter = 0; iter < 4; iter++) {
		user_defined_bullets[iter] = temp_bullets[iter] 
			                   = ITEMIZE_DEFAULTS[iter];
	}
	allowAccents = false;
}


void BufferParams::writeFile(FILE * file)
{
	// The top of the file is written by the buffer.
	// Prints out the buffer info into the .lyx file given by file

 	// the textclass
 	fprintf(file, "\\textclass %s\n",
 		textclasslist.NameOfClass(textclass).c_str());
	
	// then the the preamble
	if (!preamble.empty()) {
		fprintf(file, "\\begin_preamble\n");
		{
			// remove '\n' from the end of preamble
			preamble = strip(preamble, '\n');
			
			// write out the whole preamble  in one go
			fwrite(preamble.c_str(),
			       sizeof(char),
			       preamble.length(),
			       file);
			fprintf(file, "\n\\end_preamble\n");
		}
	}
      
	/* the options */ 
	if (!options.empty()) {
		fprintf(file,
			"\\options %s\n",
			options.c_str());
	}
   
	/* then the text parameters */ 
	fprintf(file, "\\language %s\n", language.c_str());
	fprintf(file, "\\inputencoding %s\n", inputenc.c_str());
	fprintf(file, "\\fontscheme %s\n", fonts.c_str());
	fprintf(file, "\\graphics %s\n", graphicsDriver.c_str());

	if (!float_placement.empty()) {
		fprintf(file,
			"\\float_placement %s\n",
			float_placement.c_str());
	}
	fprintf(file, "\\paperfontsize %s\n", fontsize.c_str());

	spacing.writeFile(file);

	fprintf(file, "\\papersize %s\n", string_papersize[papersize2]);
	fprintf(file, "\\paperpackage %s\n",
                string_paperpackages[paperpackage]);
	fprintf(file, "\\use_geometry %d\n", use_geometry);
	fprintf(file, "\\use_amsmath %d\n", use_amsmath);
	fprintf(file, "\\paperorientation %s\n",
		string_orientation[orientation]);
        if (!paperwidth.empty())
            fprintf(file, "\\paperwidth %s\n",
                    VSpace(paperwidth).asLyXCommand().c_str());
        if (!paperheight.empty())
            fprintf(file, "\\paperheight %s\n",
                    VSpace(paperheight).asLyXCommand().c_str());
        if (!leftmargin.empty())
            fprintf(file, "\\leftmargin %s\n",
                    VSpace(leftmargin).asLyXCommand().c_str());
        if (!topmargin.empty())
            fprintf(file, "\\topmargin %s\n",
                    VSpace(topmargin).asLyXCommand().c_str());
        if (!rightmargin.empty())
            fprintf(file, "\\rightmargin %s\n",
                    VSpace(rightmargin).asLyXCommand().c_str());
        if (!bottommargin.empty())
            fprintf(file, "\\bottommargin %s\n",
                    VSpace(bottommargin).asLyXCommand().c_str());
        if (!headheight.empty())
            fprintf(file, "\\headheight %s\n",
                    VSpace(headheight).asLyXCommand().c_str());
        if (!headsep.empty())
            fprintf(file, "\\headsep %s\n",
                    VSpace(headsep).asLyXCommand().c_str());
        if (!footskip.empty())
            fprintf(file, "\\footskip %s\n",
                    VSpace(footskip).asLyXCommand().c_str());
	fprintf(file, "\\secnumdepth %d\n", secnumdepth);
	fprintf(file, "\\tocdepth %d\n", tocdepth);
	fprintf(file, "\\paragraph_separation %s\n",
		string_paragraph_separation[paragraph_separation]);
	fprintf(file, "\\defskip %s\n", defskip.asLyXCommand().c_str());
	fprintf(file, "\\quotes_language %s\n",
		string_quotes_language[quotes_language]);
	switch(quotes_times) {
	case InsetQuotes::SingleQ: 
		fprintf(file, "\\quotes_times 1\n"); break;
	case InsetQuotes::DoubleQ: 
		fprintf(file, "\\quotes_times 2\n"); break;
	}		
	fprintf(file, "\\papercolumns %d\n", columns);
	fprintf(file, "\\papersides %d\n", sides);
	fprintf(file, "\\paperpagestyle %s\n", pagestyle.c_str());
	for (int i = 0; i < 4; ++i) {
		if (user_defined_bullets[i] != ITEMIZE_DEFAULTS[i]) {
			if (user_defined_bullets[i].getFont() != -1) {
				fprintf(file, "\\bullet %d\n\t%d\n\t%d\n\t%d\n\\end_bullet\n",
						i,
						user_defined_bullets[i].getFont(),
						user_defined_bullets[i].getCharacter(),
						user_defined_bullets[i].getSize());
			}
			else {
				fprintf(file, "\\bulletLaTeX %d\n\t%s\n\\end_bullet\n",
						i,
						user_defined_bullets[i].c_str());
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
	
	string tmptok = lex.GetString();
	string test;
	int n = 0;
	// check if tmptok is part of tex_babel in tex-defs.h
	while (true) {
		test = tex_babel[n++];
		
		if (test == tmptok) {
			language = tmptok;
			break;
		}
		else if (test.empty()) {
			lyxerr << "Warning: language `"
			       << tmptok << "' not recognized!\n"
			       << "         Setting language to `default'."
			       << endl;
			language = "default";
			break;	 
		}      
	}
}


void BufferParams::readGraphicsDriver(LyXLex & lex)
{
	string tmptok;
	string test;
	int n = 0;
	
	
	if (!lex.next()) return;
	
	tmptok = lex.GetString();
	// check if tmptok is part of tex_graphics in tex_defs.h
	while (true) {
		test = tex_graphics[n++];
		
		if (test == tmptok) {	 
			graphicsDriver = tmptok;
			break;
		}      
		else if (test == "last_item") {
			lex.printError(
				"Warning: graphics driver `$$Token' not recognized!\n"
				"         Setting graphics driver to `default'.\n");
			graphicsDriver = "default";
			break;
		}      
	}
}
