/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *
 *	     Copyright 1995 Matthias Ettrich
 *           Copyright 1995-1999 The LyX Team.
 *
 *           This file is Copyright 1996-1999
 *           Lars Gullik Bjønnes
 *
 * ======================================================
 */

// Change Log:
// ===========
// 23/03/98   Heinrich Bauer (heinrich.bauer@t-mobil.de)
// Spots marked "changed Heinrich Bauer, 23/03/98" modified due to the
// following bug: dvi file export did not work after printing (or previewing)
// and vice versa as long as the same file was concerned. This happened
// every time the LyX-file was left unchanged between the two actions mentioned
// above.

#include <config.h>

#include <cstdlib>
#include <unistd.h>

#ifdef __GNUG__
#pragma implementation "buffer.h"
#endif

#include "definitions.h"
#include "buffer.h"
#include "bufferlist.h"
#include "lyx_main.h"
#include "lyx_gui_misc.h"
#include "lyxrc.h"
#include "lyxlex.h"
#include "tex-strings.h"
#include "layout.h"
#include "lyx_cb.h"
#include "minibuffer.h"
#include "lyxfont.h"
#include "mathed/formulamacro.h"
#include "insets/lyxinset.h"
#include "insets/inseterror.h"
#include "insets/insetlabel.h"
#include "insets/insetref.h"
#include "insets/inseturl.h"
#include "insets/insetinfo.h"
#include "insets/insetquotes.h"
#include "insets/insetlatex.h"
#include "insets/insetlatexaccent.h"
#include "insets/insetbib.h" 
#include "insets/insetindex.h" 
#include "insets/insetinclude.h"
#include "insets/insettoc.h"
#include "insets/insetlof.h"
#include "insets/insetlot.h"
#include "insets/insetloa.h"
#include "insets/insetparent.h"
#include "insets/insetspecialchar.h"
#include "insets/figinset.h"
#include "support/filetools.h"
#include "support/path.h"
#include "LaTeX.h"
#include "Literate.h"
#include "Chktex.h"
#include "LyXView.h"
#include "debug.h"
#include "LaTeXFeatures.h"
#include "support/syscall.h"
#include "support/lyxlib.h"
#include "support/FileInfo.h"
#include "lyxtext.h"
#include "gettext.h"

// Uncomment this line to enable a workaround for the weird behaviour
// of the cursor between a displayed inset and last character
// at the upper line. (Alejandro 20.9.96)
// #define BREAK_BEFORE_INSET

/* Lars, when changing this file sometime in the future to a list,
 * please check out the figinsets-sideeffects described at the
 * beginning of figinset.C    Matthias (04.07.1996)
 */


// all these externs should eventually be removed.
extern BufferList bufferlist;
extern void SmallUpdate(signed char);
extern unsigned char GetCurrentTextClass();
extern void BeforeChange();

static const float LYX_FORMAT = 2.15;

extern int tex_code_break_column;

extern void FreeUpdateTimer();


Buffer::Buffer(string const & file, LyXRC *lyxrc, bool ronly)
{
	filename = file;
	filepath = OnlyPath(file);
	paragraph = 0;
	text = 0;
	the_locking_inset = 0;
	lyx_clean = true;
	bak_clean = true;
	dvi_clean_orgd = false;  // Heinrich Bauer, 23/03/98
	dvi_clean_tmpd = false;  // Heinrich Bauer, 23/03/98
	dep_clean = 0;
	read_only = ronly;
	inset_slept = false;
	users = 0;
	lyxvc.setBuffer(this);
	lyxerr.debug() << "Buffer::Buffer()" << endl;
	if (read_only || (lyxrc && lyxrc->use_tempdir)) {
		tmppath = CreateBufferTmpDir();
	} else tmppath.clear();
}


Buffer::~Buffer()
{
	lyxerr.debug() << "Buffer::~Buffer()" << endl;
	// here the buffer should take care that it is
	// saved properly, before it goes into the void.

	// make sure that views using this buffer
	// forgets it.
	if (users)
		users->setBuffer(0);
	
	if (!tmppath.empty()) {
		DestroyBufferTmpDir(tmppath);
	}
	
	LyXParagraph * par = paragraph;
	LyXParagraph * tmppar;
	while (par) {
		tmppar = par->next;
		delete par;
		par = tmppar;
	}
	paragraph = 0;
	delete text;
}


bool Buffer::saveParamsAsDefaults()
{
	string fname = AddName(AddPath(user_lyxdir, "templates/"),
				"defaults.lyx");
	Buffer defaults = Buffer(fname);
	
	// Use the current buffer's parameters as default
	defaults.params.Copy(params);
	// add an empty paragraph. Is this enough?
	defaults.paragraph = new LyXParagraph();

	return defaults.writeFile(defaults.filename,false);
}


/// Update window titles of all users
// Should work on a list
void Buffer::updateTitles()
{
	if (users) users->getOwner()->updateWindowTitle();
}


/// Reset autosave timer of all users
// Should work on a list
void Buffer::resetAutosaveTimers()
{
	if (users) users->getOwner()->resetAutosaveTimer();
}


void Buffer::setFileName(string const & newfile)
{
	filename = MakeAbsPath(newfile);
	filepath = OnlyPath(filename);
	setReadonly(IsFileWriteable(filename) == 0);
	updateTitles();
}

void Buffer::InsetUnlock()
{
	if (the_locking_inset) {
		if (!inset_slept) the_locking_inset->InsetUnlock();
		the_locking_inset = 0;
		text->FinishUndo();
		inset_slept = false;
	}
}


// Inserts a file into current document
bool Buffer::insertLyXFile(string const & filen)
	//
	// (c) CHT Software Service GmbH
	// Uwe C. Schroeder
	//
	// Insert a Lyxformat - file into current buffer
	//
	// Moved from lyx_cb.C (Lgb)
{
	if (filen.empty()) return false;

	string filename = MakeAbsPath(filen);

	// check if file exist
	FileInfo fi(filename);

	if (!fi.exist() || !fi.readable()) {
		WriteAlert(_("Error!"),
			   _("Cannot open specified file:"),
			   MakeDisplayPath(filename,50));
		return false;
	}
	
	BeforeChange();

	FilePtr myfile(filename, FilePtr::read);
	if (!myfile()) {
		WriteAlert(_("Error!"),
			   _("Cannot open specified file:"),
			   MakeDisplayPath(filename,50));
		return false;
	}
	LyXLex lex(0, 0);
	lex.setFile(myfile);
	int c = fgetc(myfile());
	ungetc(c, myfile);

	bool res = true;

	if (c == '#') {
		lyxerr.debug() << "Will insert file with header" << endl;
		res = readFile(lex, text->cursor.par);
	} else {
		lyxerr.debug() << "Will insert file without header" << endl;
		res = readLyXformat2(lex, text->cursor.par);
	}
	resize();
	return res;
}


//
// Uwe C. Schroeder
// changed to be public and have one parameter
// if par = 0 normal behavior
// else insert behavior
// Returns false if "\the_end" is not read for formats >= 2.13. (Asger)
bool Buffer::readLyXformat2(LyXLex &lex, LyXParagraph *par)
{
	string tmptok;
	Inset * inset = 0;
	int pos = 0;
	int tmpret, tmpret2;
	char depth = 0; // signed or unsigned?
	LyXParagraph::footnote_flag footnoteflag = LyXParagraph::NO_FOOTNOTE;
	LyXParagraph::footnote_kind footnotekind = LyXParagraph::FOOTNOTE;
	bool the_end_read = false;

	LyXParagraph *return_par = 0;
	LyXFont font = LyXFont(LyXFont::ALL_INHERIT);

	// If we are inserting, we cheat and get a token in advance
	bool has_token = false;
	string pretoken;

	if(!par) {
		par = new LyXParagraph();
	} else {
		text->BreakParagraph();
		return_par = text->FirstParagraph();
		pos=0;
		markDirty();
		// We don't want to adopt the parameters from the
		// document we insert, so we skip until the text begins:
		while (lex.IsOK()) {
			lex.nextToken();
			pretoken = lex.GetString();
			if (pretoken == "\\layout") {
				has_token = true;
				break;
			}
		}
	}

	while (lex.IsOK()) {
		if (has_token) {
			has_token = false;
		} else {
			lex.nextToken();
			pretoken = lex.GetString();
		}

		// Profiling show this should give a lot: (Asger)
		string const token = pretoken;

		if (token.empty())
			continue;
		else if (token[0] != '\\') {
			int n = token.length();
			for (int i=0; i < n; i++) {
				par->InsertChar(pos, token[i]);
				par->SetFont(pos, font);
				pos++;
			}
		} else if (token == "\\i") {
			inset = new InsetLatexAccent;
			inset->Read(lex);
			par->InsertChar(pos, LYX_META_INSET); 
			
			par->InsertInset(pos, inset);
			par->SetFont(pos, font);
			pos++;
		} else if (token == "\\layout") {
			if (!return_par) 
				return_par = par;
			else 
				par = new LyXParagraph(par);
			pos = 0;
			lex.EatLine();
			string layoutname = lex.GetString();
			pair<bool, LyXTextClass::LayoutList::size_type> pp
				= textclasslist.NumberOfLayout(params.textclass,
								layoutname);
			if (pp.first) {
				par->layout = pp.second;
			} else { // layout not found
				// use default layout "Standard" (0)
				par->layout = 0;
			}
			// Test whether the layout is obsolete.
			LyXLayout const & layout = textclasslist.Style(params.textclass,
							    par->layout); 
			if (!layout.obsoleted_by().empty())
				par->layout = 
					textclasslist.NumberOfLayout(params.textclass, 
								layout.obsoleted_by()).second;
			par->footnoteflag = footnoteflag;
			par->footnotekind = footnotekind;
			par->depth = depth;
			font = LyXFont(LyXFont::ALL_INHERIT);
		} else if (token == "\\end_float") {
			if (!return_par) 
				return_par = par;
			else 
				par = new LyXParagraph(par);
			footnotekind = LyXParagraph::FOOTNOTE;
			footnoteflag = LyXParagraph::NO_FOOTNOTE;
			pos = 0;
			lex.EatLine();
			par->layout = LYX_DUMMY_LAYOUT;
			font = LyXFont(LyXFont::ALL_INHERIT);
		} else if (token == "\\begin_float") {
			tmpret = lex.FindToken(string_footnotekinds);
			if (tmpret == -1) tmpret++;
			if (tmpret != LYX_LAYOUT_DEFAULT) 
				footnotekind = (LyXParagraph::footnote_kind)tmpret; // bad
			if (footnotekind == LyXParagraph::FOOTNOTE
			    || footnotekind == LyXParagraph::MARGIN)
				footnoteflag = LyXParagraph::CLOSED_FOOTNOTE;
			else 
				footnoteflag = LyXParagraph::OPEN_FOOTNOTE;
		} else if (token == "\\begin_deeper") {
			depth++;
		} else if (token == "\\end_deeper") {
			if (!depth) {
				lex.printError("\\end_deeper: "
					       "depth is already null");
			}
			else
				depth--;
		} else if (token == "\\begin_preamble") {
			params.readPreamble(lex);
		} else if (token == "\\textclass") {
			lex.EatLine();
			pair<bool, LyXTextClassList::ClassList::size_type> pp =
				textclasslist.NumberOfClass(lex.GetString());
			if (pp.first) {
				params.textclass = pp.second;
			} else {
				lex.printError("Unknown textclass `$$Token'");
				params.textclass = 0;
			}
			if (!textclasslist.Load(params.textclass)) {
				// if the textclass wasn't loaded properly
				// we need to either substitute another
				// or stop loading the file.
				// I can substitute but I don't see how I can
				// stop loading... ideas??  ARRae980418
				WriteAlert(_("Textclass Loading Error!"),
				           string(_("Can't load textclass ")) +
					   textclasslist.NameOfClass(params.textclass),
					   _("-- substituting default"));
				params.textclass = 0;
			}
		} else if (token == "\\options") {
			lex.EatLine();
			params.options = lex.GetString();
		} else if (token == "\\language") {
			params.readLanguage(lex);	    
		} else if (token == "\\fontencoding") {
			lex.EatLine();
		} else if (token == "\\inputencoding") {
			lex.EatLine();
			params.inputenc = lex.GetString();
		} else if (token == "\\graphics") {
			params.readGraphicsDriver(lex);
		} else if (token == "\\fontscheme") {
			lex.EatLine();
			params.fonts = lex.GetString();
		} else if (token == "\\noindent") {
			par->noindent = true;
		} else if (token == "\\fill_top") {
			par->added_space_top = VSpace(VSpace::VFILL);
		} else if (token == "\\fill_bottom") {
			par->added_space_bottom = VSpace(VSpace::VFILL);
		} else if (token == "\\line_top") {
			par->line_top = true;
		} else if (token == "\\line_bottom") {
			par->line_bottom = true;
		} else if (token == "\\pagebreak_top") {
			par->pagebreak_top = true;
		} else if (token == "\\pagebreak_bottom") {
			par->pagebreak_bottom = true;
		} else if (token == "\\start_of_appendix") {
			par->start_of_appendix = true;
		} else if (token == "\\paragraph_separation") {
			tmpret = lex.FindToken(string_paragraph_separation);
			if (tmpret == -1) tmpret++;
			if (tmpret != LYX_LAYOUT_DEFAULT) 
				params.paragraph_separation = tmpret;
		} else if (token == "\\defskip") {
			lex.nextToken();
			params.defskip = VSpace(lex.GetString());
		} else if (token == "\\no_isolatin1") {
			lex.nextToken();
		} else if (token == "\\no_babel") {
			lex.nextToken();
		} else if (token == "\\no_epsfig") {
			lex.nextToken();
		} else if (token == "\\epsfig") { // obsolete
			// Indeed it is obsolete, but we HAVE to be backwards
			// compatible until 0.14, because otherwise all figures
			// in existing documents are irretrivably lost. (Asger)
			params.readGraphicsDriver(lex);
		} else if (token == "\\quotes_language") {
			tmpret = lex.FindToken(string_quotes_language);
			if (tmpret == -1) tmpret++;
			if (tmpret != LYX_LAYOUT_DEFAULT) {
				InsetQuotes::quote_language tmpl =
					InsetQuotes::EnglishQ;
				switch(tmpret) {
				case 0:
					tmpl = InsetQuotes::EnglishQ;
					break;
				case 1:
					tmpl = InsetQuotes::SwedishQ;
					break;
				case 2:
					tmpl = InsetQuotes::GermanQ;
					break;
				case 3:
					tmpl = InsetQuotes::PolishQ;
					break;
				case 4:
					tmpl = InsetQuotes::FrenchQ;
					break;
				case 5:
					tmpl = InsetQuotes::DanishQ;
					break;	
				}
				params.quotes_language = tmpl;
			}
		} else if (token == "\\quotes_times") {
			lex.nextToken();
			switch(lex.GetInteger()) {
			case 1: 
				params.quotes_times = InsetQuotes::SingleQ; 
				break;
			case 2: 
				params.quotes_times = InsetQuotes::DoubleQ; 
				break;
			}
                } else if (token == "\\papersize") {
                        if (format > 2.13)
                                tmpret = lex.FindToken(string_papersize);
                        else
                                tmpret = lex.FindToken(string_oldpapersize);
			if (tmpret == -1)
                                tmpret++;
                        else
                                params.papersize2 = tmpret;
                } else if (token == "\\paperpackage") {
			tmpret = lex.FindToken(string_paperpackages);
			if (tmpret == -1) {
                                tmpret++;
                                params.paperpackage = PACKAGE_NONE;
                        } else
                                params.paperpackage = tmpret;
		} else if (token == "\\use_geometry") {
			lex.nextToken();
			params.use_geometry = lex.GetInteger();
		} else if (token == "\\use_amsmath") {
			lex.nextToken();
			params.use_amsmath = lex.GetInteger();
		} else if (token == "\\paperorientation") {
			tmpret = lex.FindToken(string_orientation);
			if (tmpret == -1) tmpret++;
			if (tmpret != LYX_LAYOUT_DEFAULT) 
				params.orientation = tmpret;
		} else if (token == "\\paperwidth") {
			lex.next();
			params.paperwidth = lex.GetString();
		} else if (token == "\\paperheight") {
			lex.next();
			params.paperheight = lex.GetString();
		} else if (token == "\\leftmargin") {
			lex.next();
			params.leftmargin = lex.GetString();
		} else if (token == "\\topmargin") {
			lex.next();
			params.topmargin = lex.GetString();
		} else if (token == "\\rightmargin") {
			lex.next();
			params.rightmargin = lex.GetString();
		} else if (token == "\\bottommargin") {
			lex.next();
			params.bottommargin = lex.GetString();
		} else if (token == "\\headheight") {
			lex.next();
			params.headheight = lex.GetString();
		} else if (token == "\\headsep") {
			lex.next();
			params.headsep = lex.GetString();
		} else if (token == "\\footskip") {
			lex.next();
			params.footskip = lex.GetString();
		} else if (token == "\\paperfontsize") {
			lex.nextToken();
			params.fontsize = strip(lex.GetString());
		} else if (token == "\\papercolumns") {
			lex.nextToken();
			params.columns = lex.GetInteger();
		} else if (token == "\\papersides") {
			lex.nextToken();
			switch(lex.GetInteger()) {
			default:
			case 1: params.sides = LyXTextClass::OneSide; break;
			case 2: params.sides = LyXTextClass::TwoSides; break;
			}
		} else if (token == "\\paperpagestyle") {
		        lex.nextToken();
			params.pagestyle = strip(lex.GetString());
		} else if (token == "\\bullet") {
			lex.nextToken();
			int index = lex.GetInteger();
			lex.nextToken();
			int temp_int = lex.GetInteger();
			params.user_defined_bullets[index].setFont(temp_int);
			params.temp_bullets[index].setFont(temp_int);
			lex.nextToken();
			temp_int = lex.GetInteger();
			params.user_defined_bullets[index].setCharacter(temp_int);
			params.temp_bullets[index].setCharacter(temp_int);
			lex.nextToken();
			temp_int = lex.GetInteger();
			params.user_defined_bullets[index].setSize(temp_int);
			params.temp_bullets[index].setSize(temp_int);
			lex.nextToken();
			string temp_str = lex.GetString();
			if (temp_str != "\\end_bullet") {
				// this element isn't really necessary for
				// parsing but is easier for humans
				// to understand bullets. Put it back and
				// set a debug message?
				lex.printError("\\end_bullet expected, got" + temp_str);
				//how can I put it back?
			}
		} else if (token == "\\bulletLaTeX") {
			lex.nextToken();
			int index = lex.GetInteger();
			lex.next();
			string temp_str = lex.GetString(), sum_str;
			while (temp_str != "\\end_bullet") {
				// this loop structure is needed when user
				// enters an empty string since the first
				// thing returned will be the \\end_bullet
				// OR
				// if the LaTeX entry has spaces. Each element
				// therefore needs to be read in turn
				sum_str += temp_str;
				lex.next();
				temp_str = lex.GetString();
			}
			params.user_defined_bullets[index].setText(sum_str);
			params.temp_bullets[index].setText(sum_str);
		} else if (token == "\\secnumdepth") {
			lex.nextToken();
			params.secnumdepth = lex.GetInteger();
		} else if (token == "\\tocdepth") {
			lex.nextToken();
			params.tocdepth = lex.GetInteger();
		} else if (token == "\\baselinestretch") { // now obsolete
			lex.nextToken(); // should not be used directly
			// anymore.
			// Will probably keep a kind of support just for
			// compability.
			params.spacing.set(Spacing::Other, lex.GetFloat());
		} else if (token == "\\spacing") {
			lex.next();
			string tmp = strip(lex.GetString());
			if (tmp == "single") {
				params.spacing.set(Spacing::Single);
			} else if (tmp == "onehalf") {
				params.spacing.set(Spacing::Onehalf);
			} else if (tmp == "double") {
				params.spacing.set(Spacing::Double);
			} else if (tmp == "other") {
				lex.next();
				params.spacing.set(Spacing::Other,
						   lex.GetFloat());
			} else {
				lex.printError("Unknown spacing token: '$$Token'");
			}
		} else if (token == "\\float_placement") {
			lex.nextToken();
			params.float_placement = lex.GetString();
		} else if (token == "\\cursor") {
			// this is obsolete, so we just skip it.
			lex.nextToken();
		} else if (token == "\\family") { 
			lex.next();
			font.setLyXFamily(lex.GetString());
		} else if (token == "\\series") {
			lex.next();
			font.setLyXSeries(lex.GetString());
		} else if (token == "\\shape") {
			lex.next();
			font.setLyXShape(lex.GetString());
		} else if (token == "\\size") {
			lex.next();
			font.setLyXSize(lex.GetString());
		} else if (token == "\\latex") {
			lex.next();
			string tok = lex.GetString();
			// This is dirty, but gone with LyX3. (Asger)
			if (tok == "no_latex")
				font.setLatex(LyXFont::OFF);
			else if (tok == "latex")
				font.setLatex(LyXFont::ON);
			else if (tok == "default")
				font.setLatex(LyXFont::INHERIT);
			else
				lex.printError("Unknown LaTeX font flag "
					       "`$$Token'");
		} else if (token == "\\emph") {
			lex.next();
			font.setEmph(font.setLyXMisc(lex.GetString()));
		} else if (token == "\\bar") {
			lex.next();
			string tok = lex.GetString();
			// This is dirty, but gone with LyX3. (Asger)
			if (tok == "under")
				font.setUnderbar(LyXFont::ON);
			else if (tok == "no")
				font.setUnderbar(LyXFont::OFF);
			else if (tok == "default")
				font.setUnderbar(LyXFont::INHERIT);
			else
				lex.printError("Unknown bar font flag "
					       "`$$Token'");
		} else if (token == "\\noun") {
			lex.next();
			font.setNoun(font.setLyXMisc(lex.GetString()));
		} else if (token == "\\color") {
			lex.next();
			font.setLyXColor(lex.GetString());
		} else if (token == "\\align") {
			tmpret = lex.FindToken(string_align);
			if (tmpret == -1) tmpret++;
			if (tmpret != LYX_LAYOUT_DEFAULT) {
				tmpret2 = 1;
				for (; tmpret>0; tmpret--)
					tmpret2 = tmpret2 * 2;
				par->align = LyXAlignment(tmpret2);
			}
		} else if (token == "\\added_space_top"){
			lex.nextToken();
			par->added_space_top = lex.GetString();
		} else if (token == "\\added_space_bottom") {
			lex.nextToken();
			par->added_space_bottom = lex.GetString();
                } else if (token == "\\pextra_type") {
                        lex.nextToken();
                        par->pextra_type = lex.GetInteger();
                } else if (token == "\\pextra_width") {
                        lex.nextToken();
			par->pextra_width = lex.GetString();
                } else if (token == "\\pextra_widthp") {
                        lex.nextToken();
			par->pextra_widthp = lex.GetString();
                } else if (token == "\\pextra_alignment") {
                        lex.nextToken();
                        par->pextra_alignment = lex.GetInteger();
                } else if (token == "\\pextra_hfill") {
                        lex.nextToken();
                        par->pextra_hfill = lex.GetInteger();
                } else if (token == "\\pextra_start_minipage") {
                        lex.nextToken();
                        par->pextra_start_minipage = lex.GetInteger();
		} else if (token == "\\labelwidthstring") {
			lex.EatLine();
			par->labelwidthstring = lex.GetString();
			/* do not delete this token, it is still needed! */ 
		} else if (token == "\\end_inset") {
			/* simple ignore this. The insets do not have
			 *  to read this */
			// but insets should read it, it is a part of
			//the inset isn't it? Lgb.
		} else if (token == "\\begin_inset") {
			lex.next();
			tmptok = lex.GetString();
			/* test the different insets */ 
			if (tmptok == "Quotes") {
				inset = new InsetQuotes;
				inset->Read(lex);
				par->InsertChar(pos, LYX_META_INSET); 
				par->InsertInset(pos, inset);
				par->SetFont(pos, font);
				pos++;
			} else if (tmptok == "Latex") {
				// This one is on its way out
				lex.EatLine();
				tmptok = strip(lex.GetString());
				//lyxerr <<string(tmptok[0]));
				if (tmptok[0] == '\\') {
					// then this latex is a
					// latex command
					InsetCommand *tmpinset =
						new InsetCommand();
					tmpinset->scanCommand(tmptok);
					inset = tmpinset;
				} else {
					// This should not use InsetLaTexDel
					// it should rather insert text into
					// the paragraph and mark it as tex.
					inset = new InsetLatex(tmptok);
				}
				par->InsertChar(pos, LYX_META_INSET); 
				par->InsertInset(pos, inset);
				par->SetFont(pos, font);
				pos++;
			} else if (tmptok == "LatexDel") {
				// This one is on its way out...
				lex.EatLine();
				tmptok = strip(lex.GetString());
				//lyxerr <<string(tmptok[0]));
				if (tmptok == "\\tableofcontents") {
					inset = new InsetTOC(this);
				} else if (tmptok == "\\listoffigures") {
					inset = new InsetLOF(this);
				} else if (tmptok == "\\listoftables") {
					inset = new InsetLOT(this);
				} else if (tmptok == "\\listofalgorithms") {
					inset = new InsetLOA(this);
				} else if (contains(tmptok, "\\ref{")
					   || contains(tmptok, "\\pageref{")) {
					inset = new InsetRef(tmptok, this);
				} else if (contains(tmptok, "\\url{")
					   || contains(tmptok, "\\htmlurl{")) {
					string cont,opt,tmptmptok,cmdname;
					lex.next();
					while(lex.IsOK() && lex.GetString() != "\\end_inset" ) {
						lex.next();
					}
					lex.next();
					while(lex.IsOK()) {
						tmptmptok=lex.GetString();
						if(tmptmptok[0] == '\\') {
							if( tmptmptok == "\\backslash")
								opt += '\\';
							else
								break;
						}
						else
							opt += tmptmptok;
						opt += ' ';
						lex.next();
					}
					while(lex.IsOK() && lex.GetString() != "\\end_inset" ) {
						lex.next();
					}
					lex.next();
					while(lex.IsOK()) {
						tmptmptok=lex.GetString();
						if(tmptmptok[0] == '\\') {
							if( tmptmptok == "\\backslash")
								cont += '\\';
							else
								break;
						}
						else
							cont += tmptmptok;
						cont += ' ';
						lex.next();
					}
					while(lex.IsOK() && lex.GetString() != "\\end_inset" ) {
						lex.next();
					}
					if(contains(tmptok, "\\url{"))
						cmdname = string("url");
					else
						cmdname = string("htmlurl");
					cont = strip(cont);
					opt = strip(opt);
					inset = new InsetUrl(cmdname,cont,opt);
				} else if (tmptok[0] == '\\') {
					// then this latex del is a
					// latex command
					InsetCommand *tmpinset =
						new InsetCommand();
					tmpinset->scanCommand(tmptok);
					inset = tmpinset;
				} 
				par->InsertChar(pos, LYX_META_INSET); 
				par->InsertInset(pos, inset);
				par->SetFont(pos, font);
				pos++;
			} else if (tmptok == "\\i") {
				inset = new InsetLatexAccent;
				inset->Read(lex);
				par->InsertChar(pos, LYX_META_INSET); 
				par->InsertInset(pos, inset);
				par->SetFont(pos, font);
				pos++;
			} else if (tmptok == "FormulaMacro") {
				inset = new InsetFormulaMacro;
				inset->Read(lex);
				par->InsertChar(pos, LYX_META_INSET); 
				par->InsertInset(pos, inset);
				par->SetFont(pos, font);
				pos++;
			} else if (tmptok == "Formula") {
				inset = new InsetFormula;
				inset->Read(lex);
				par->InsertChar(pos, LYX_META_INSET); 
				par->InsertInset(pos, inset);
				par->SetFont(pos, font);
				pos++;
			} else if (tmptok == "Figure") {
				inset = new InsetFig(100,100, this);
				inset->Read(lex);
				par->InsertChar(pos, LYX_META_INSET); 
				par->InsertInset(pos, inset);
				par->SetFont(pos, font);
				pos++;
			} else if (tmptok == "Label") {
				// Kept for compability. Remove in 0.13.
				if (lex.EatLine()) {
					string tmp = "\\label{";
					tmp += lex.GetString();
					tmp += '}';
					inset = new InsetLabel(tmp);
					par->InsertChar(pos, LYX_META_INSET); 
					par->InsertInset(pos, inset);
					par->SetFont(pos, font);
					pos++;
				}
			} else if (tmptok == "Info") {
				inset = new InsetInfo;
				inset->Read(lex);
				par->InsertChar(pos, LYX_META_INSET); 
				par->InsertInset(pos, inset);
				par->SetFont(pos, font);
				pos++;
			} else if (tmptok == "Include") {
				inset = new InsetInclude(string(), this);
				inset->Read(lex);
				par->InsertChar(pos, LYX_META_INSET); 
				par->InsertInset(pos, inset);
				par->SetFont(pos, font);
				pos++;
			} else if (tmptok == "LatexCommand") {
				InsetCommand inscmd;
				inscmd.Read(lex);
				if (inscmd.getCmdName()=="cite") {
					inset = new InsetCitation(inscmd.getContents(), inscmd.getOptions());
				} else if (inscmd.getCmdName()=="bibitem") {
					lex.printError("Wrong place for bibitem");
					inset = inscmd.Clone();
				} else if (inscmd.getCmdName()=="BibTeX") {
					inset = new InsetBibtex(inscmd.getContents(), inscmd.getOptions(), this);
				} else if (inscmd.getCmdName()=="index") {
					inset = new InsetIndex(inscmd.getContents());
				} else if (inscmd.getCmdName()=="include") {
					inset = new InsetInclude(inscmd.getContents(), this);
				} else if (inscmd.getCmdName()=="label") {
					inset = new InsetLabel(inscmd.getCommand());
				} else if (inscmd.getCmdName()=="url"
					   || inscmd.getCmdName()=="htmlurl") {
					inset = new InsetUrl(inscmd.getCommand());
				} else if (inscmd.getCmdName() == "ref"
					   || inscmd.getCmdName() == "pageref") {
					if (!inscmd.getOptions().empty() || !inscmd.getContents().empty()) {
						inset = new InsetRef(inscmd, this);
					}
					/* This condition comes from a temporary solution
					   to the latexdel ref inset that was transformed to an empty ref
					   inset plus the body surronded by latexdel insets */
					else {
						string cont,opt,tmptmptok,cmdname;
						lex.next();
						while(lex.IsOK() && lex.GetString() != "\\end_inset" ) {
							lex.next();
						}
						lex.next();
						while(lex.IsOK()) {
							tmptmptok=lex.GetString();
							if(tmptmptok[0] == '\\') {
								if( tmptmptok == "\\backslash")
									opt += '\\';
								else
									break;
							}
							else
								opt += tmptmptok;
							opt += ' ';
							lex.next();
						}
						while(lex.IsOK() && lex.GetString() != "\\end_inset" ) {
							lex.next();
						}
						lex.next();
						while(lex.IsOK()) {
							tmptmptok=lex.GetString();
							if(tmptmptok[0] == '\\') {
								if( tmptmptok == "\\backslash")
									cont += '\\';
								else
									break;
							}
							else
								cont += tmptmptok;
							cont += ' ';
							lex.next();
						}
						while(lex.IsOK() && lex.GetString() != "\\end_inset" ) {
							lex.next();
						}

						cont = strip(cont);
						opt = strip(opt);
						cmdname =  "\\" + inscmd.getCmdName();
						cmdname += "["  + cont  + "]";
						cmdname += "{"  + opt + "}";
						inset = new InsetRef(cmdname,this);
					}
				} else if (inscmd.getCmdName()=="tableofcontents") {
					inset = new InsetTOC(this);
				} else if (inscmd.getCmdName()=="listoffigures") {
					inset = new InsetLOF(this);
				} else if (inscmd.getCmdName()=="listofalgorithms") {
					inset = new InsetLOA(this);
				} else if (inscmd.getCmdName()=="listoftables") {
					inset = new InsetLOT(this);
				} else if (inscmd.getCmdName()=="printindex") {
					inset = new InsetPrintIndex(this);
				} else if (inscmd.getCmdName()=="lyxparent") {
					inset = new InsetParent(inscmd.getContents(),this);
				} else 
					// The following three are only for compatibility
					if (inscmd.getCmdName()=="-") {
						inset = new InsetSpecialChar(InsetSpecialChar::HYPHENATION);
					} else if (inscmd.getCmdName()=="@.") {
						inset = new InsetSpecialChar(InsetSpecialChar::END_OF_SENTENCE);
					} else if (inscmd.getCmdName()=="ldots") {
						inset = new InsetSpecialChar(InsetSpecialChar::LDOTS);
					} else
						inset = inscmd.Clone();
			       
				if (inset) {
					par->InsertChar(pos, LYX_META_INSET);
					par->InsertInset(pos, inset);
					par->SetFont(pos, font);
					pos++;
				}
			}
		} else if (token == "\\InsetQuotes") {
			inset = new InsetQuotes;
			inset->Read(lex);
			par->InsertChar(pos, LYX_META_INSET); 
			par->InsertInset(pos, inset);
			par->SetFont(pos, font);
			pos++;
		} else if (token == "\\InsetLatex") {
			inset = new InsetLatex;
			inset->Read(lex);
			par->InsertChar(pos, LYX_META_INSET); 
			par->InsertInset(pos, inset);
			par->SetFont(pos, font);
			pos++;
		} else if (token == "\\InsetLatexDel") {
			lex.printError(_("Warning: Ignoring Old Inset"));
		} else if (token == "\\InsetFormula") {
			inset = new InsetFormula;
			inset->Read(lex);
			par->InsertChar(pos, LYX_META_INSET); 
			par->InsertInset(pos, inset);
			par->SetFont(pos, font);
			pos++;
		} else if (token == "\\SpecialChar") {
			inset = new InsetSpecialChar;
			inset->Read(lex);
			par->InsertChar(pos, LYX_META_INSET); 
			par->InsertInset(pos, inset);
			par->SetFont(pos, font);
			pos++;
		} else if (token == "\\Figure") {
			inset = new InsetFig(100,100, this);
			inset->Read(lex);
			par->InsertChar(pos, LYX_META_INSET); 
			par->InsertInset(pos, inset);
			par->SetFont(pos, font);
			pos++;
		} else if (token == "\\newline") {
			par->InsertChar(pos, LYX_META_NEWLINE);
			par->SetFont(pos, font);
			pos++;
		} else if (token == "\\LyXTable") {
			par->table = new LyXTable(lex);
		} else if (token == "\\hfill") {
			par->InsertChar(pos, LYX_META_HFILL);
			par->SetFont(pos, font);
			pos++;
		} else if (token == "\\protected_separator") {
			par->InsertChar(pos, LYX_META_PROTECTED_SEPARATOR);
			par->SetFont(pos, font);
			pos++;
		} else if (token == "\\bibitem") {  // ale970302
		        if (!par->bibkey)
				par->bibkey = new InsetBibKey;
		        par->bibkey->Read(lex);		        
		}else if (token == "\\backslash") {
			par->InsertChar(pos, '\\');
			par->SetFont(pos, font);
			pos++;
		}else if (token == "\\the_end") {
			the_end_read = true;
		} else {
			// This should be insurance for the future: (Asger)
			lex.printError("Unknown token `$$Token'. "
				       "Inserting as text.");
			int n = token.length();
			for (int i=0; i < n; i++) {
				par->InsertChar(pos, token[i]);
				par->SetFont(pos, font);
				pos++;
			}
		}
	}
   
	if (!return_par)
		return_par = par;

	paragraph = return_par;
	
	return the_end_read;
}


bool Buffer::readFile(LyXLex & lex, LyXParagraph * par)
{
	string token;

	if (lex.IsOK()) {
		lex.next();
		token = lex.GetString();
		if (token == "\\lyxformat") { // the first token _must_ be...
			lex.next();
			format = lex.GetFloat();
			if (format > 1) {
				if (LYX_FORMAT - format > 0.05) {
					printf(_("Warning: need lyxformat %.2f but found %.2f\n"),
					       LYX_FORMAT, format);
				}
				if (format - LYX_FORMAT > 0.05) {
					printf(_("ERROR: need lyxformat %.2f but found %.2f\n"),
					       LYX_FORMAT, format);
				}
				bool the_end = readLyXformat2(lex, par);
				// Formats >= 2.13 support "\the_end" marker
				if (format < 2.13)
					the_end = true;
                                // Formats >= 2.14 changed papersize stuff
                                if (format < 2.14) {
                                        setOldPaperStuff();
                                } else {
                                        setPaperStuff();
                                }
				if (!the_end)
					WriteAlert(_("Warning!"),
						   _("Reading of document is not complete"),
						   _("Maybe the document is truncated"));
				// We simulate a safe reading anyways to allow
				// users to take the chance... (Asger)
				return true;
			} // format < 1
			else {
				WriteAlert(_("ERROR!"),
					   _("Old LyX file format found. "
					     "Use LyX 0.10.x to read this!"));
				return false;
			}

		} else { // "\\lyxformat" not found
			WriteAlert(_("ERROR!"), _("Not a LyX file!"));
		}
	} else
		WriteAlert(_("ERROR!"), _("Unable to read file!"));
	return false;
}
	    	    

// Returns false if unsuccesful
bool Buffer::writeFile(string const & filename, bool flag)
{
	// if flag is false writeFile will not create any GUI
	// warnings, only cerr.
	// Needed for autosave in background or panic save (Matthias 120496)

	if (read_only && (filename == this->filename)) {
		// Here we should come with a question if we should
		// perform the write anyway.
		if (flag)
			lyxerr << _("Error! Document is read-only: ")
			       << filename << endl;
		else
			WriteAlert(_("Error! Document is read-only: "), filename);
		return false;
	}

	FileInfo finfo(filename);
	if (finfo.exist() && !finfo.writable()) {
		// Here we should come with a question if we should
		// try to do the save anyway. (i.e. do a chmod first)
		if (flag)
			lyxerr << _("Error! Cannot write file: ")
			       << filename << endl;
		else
			WriteFSAlert(_("Error! Cannot write file: "), filename);
		return false;
	}

	FilePtr file(filename, FilePtr::truncate);
	if (!file()) {
		if (flag)
			lyxerr << _("Error! Cannot write file: ")
			       << filename << endl;
		else
			WriteFSAlert(_("Error! Cannot write file: "), filename);
		return false;
	}
	// The top of the file should not be written by params.
	// collect some very important information
	string userName(getUserName()) ;

	// write out a comment in the top of the file
	fprintf(file,
		"#This file was created by <%s> %s",
		userName.c_str(),(char*)date());
	fprintf(file,
		"#LyX 1.0 (C) 1995-1999 Matthias Ettrich"
		" and the LyX Team\n");
	
	// at the very beginning the used lyx format
	fprintf(file, "\\lyxformat %.2f\n", LYX_FORMAT);

	// now write out the buffer parameters.
	params.writeFile(file);

	char footnoteflag = 0;
	char depth = 0;

	// this will write out all the paragraphs
	// using recursive descent.
	paragraph->writeFile(file, params, footnoteflag, depth);

	// Write marker that shows file is complete
	fprintf(file, "\n\\the_end\n");
	if (file.close()) {
		if (flag)
			lyxerr << _("Error! Could not close file properly: ")
			       << filename << endl;
		else
			WriteFSAlert(_("Error! Could not close file properly: "),
				     filename);
		return false;
	}
	return true;
}


void Buffer::writeFileAscii(string const & filename, int linelen) 
{
	FilePtr	file(filename, FilePtr::write);
	LyXFont
		font1, font2;
	Inset * inset;
	LyXParagraph * par = paragraph;
	char
		c,
		footnoteflag = 0,
		depth = 0;

	string
		fname1,
		tmp;

#ifdef NEW_TEXT
	LyXParagraph::size_type i;
#else
	int i;
#endif
	int
		j,h,
		ltype=0,
		ltype_depth=0,
		noparbreak=0,
		islatex=0,
		*clen=0,
		actcell=0,
		actpos=0,
		cell=0,
	        cells=0,
	        currlinelen=0;
	long
		fpos=0;
	bool
		ref_printed = false;
   
   
	if (!file()) {
		WriteFSAlert(_("Error: Cannot write file:"), filename);
		return;
	}
	fname1=TmpFileName();
	while (par) {
		noparbreak = 0;
		islatex = 0;
		if (par->footnoteflag != LyXParagraph::NO_FOOTNOTE ||
		    !par->previous ||
		    par->previous->footnoteflag == LyXParagraph::NO_FOOTNOTE){
	 
			/* begins a footnote environment ? */ 
			if (footnoteflag != par->footnoteflag) {
				footnoteflag = par->footnoteflag;
				if (footnoteflag) {
					j=strlen(string_footnotekinds[par->footnotekind])+4;
					if (currlinelen+j > linelen)
						fprintf(file,"\n");
					fprintf(file, "([%s] ", 
						string_footnotekinds[par->footnotekind]);
					currlinelen += j;
				}
			}
	 
			/* begins or ends a deeper area ?*/ 
			if (depth != par->depth) {
				if (par->depth > depth) {
					while (par->depth > depth) {
						depth++;
					}
				}
				else {
					while (par->depth < depth) {
						depth--;
					}
				}
			}
	 
			/* First write the layout */
			tmp = textclasslist.NameOfLayout(params.textclass,par->layout);
			if (tmp == "Itemize") {
				ltype = 1;
				ltype_depth = depth+1;
			} else if (tmp =="Enumerate") {
				ltype = 2;
				ltype_depth = depth+1;
			} else if (strstr(tmp.c_str(),"ection")) {
				ltype = 3;
				ltype_depth = depth+1;
			} else if (strstr(tmp.c_str(),"aragraph")) {
				ltype = 4;
				ltype_depth = depth+1;
			} else if (tmp == "Description") {
				ltype = 5;
				ltype_depth = depth+1;
			} else if (tmp == "Abstract") {
				ltype = 6;
				ltype_depth = 0;
			} else if (tmp == "Bibliography") {
				ltype = 7;
				ltype_depth = 0;
			} else {
				ltype = 0;
				ltype_depth = 0;
			}
	 
			/* maybe some vertical spaces */ 

			/* the labelwidthstring used in lists */ 
	 
			/* some lines? */ 
	 
			/* some pagebreaks? */ 
	 
			/* noindent ? */ 
	 
			/* what about the alignment */ 
		}
		else {
			/* dummy layout, that means a footnote ended */ 
			footnoteflag = LyXParagraph::NO_FOOTNOTE;
			fprintf(file, ") ");
			noparbreak = 1;
		}
      
		/* It might be a table */ 
		if (par->table){
			if (!lyxrc->ascii_roff_command.empty() &&
                            lyxrc->ascii_roff_command != "none") {
				RoffAsciiTable(file,par);
				par = par->next;
				continue;
			}
			cell = 1;
                        actcell = 0;
			cells = par->table->columns;
			clen = new int [cells];
			memset(clen,0,sizeof(int)*cells);
#ifdef NEW_TEXT
			for (i = 0, j = 0, h = 1; i < par->size(); ++i, ++h) {
#else
			for (i = 0, j = 0, h=1; i < par->last; i++, h++) {
#endif
				c = par->GetChar(i);
				if (c == LYX_META_INSET) {
					if ((inset = par->GetInset(i))) {
						FilePtr fp(fname1,
							   FilePtr::write);
						if (!fp()) {
							WriteFSAlert(_("Error: Cannot open temporary file:"), fname1);
							return;
						}
						inset->Latex(fp,-1);
						h += ftell(fp) - 1;
						remove(fname1.c_str());
					}
				} else if (c == LYX_META_NEWLINE) {
					if (clen[j] < h)
						clen[j] = h;
					h = 0;
					j = (++j) % par->table->NumberOfCellsInRow(actcell);
                                        actcell++;
				}
			}
			if (clen[j] < h)
				clen[j] = h;
		}
      
		font1 = LyXFont(LyXFont::ALL_INHERIT);
                actcell=0;
#ifdef NEW_TEXT
		for (i = 0, actpos = 1; i < par->size(); ++i, ++actpos) {
#else
		for (i = 0,actpos=1; i < par->last; i++, actpos++) {
#endif
			if (!i && !footnoteflag && !noparbreak){
				fprintf(file, "\n\n");
				for(j=0; j<depth; j++)
					fprintf(file, "  ");
				currlinelen = depth*2;
				switch(ltype) {
				case 0: /* Standart */
				case 4: /* (Sub)Paragraph */
                                case 5: /* Description */
					break;
				case 6: /* Abstract */
					fprintf(file, "Abstract\n\n");
					break;
				case 7: /* Bibliography */
					if (!ref_printed) {
						fprintf(file, "References\n\n");
						ref_printed = true;
					}
					break;
				default:
					fprintf(file,"%s ",par->labelstring.c_str());
					break;
				}
				if (ltype_depth > depth) {
					for(j=ltype_depth-1; j>depth; j--)
						fprintf(file, "  ");
					currlinelen += (ltype_depth-depth)*2;
				}
				if (par->table) {
					for(j=0;j<cells;j++) {
						fputc('+',file);
						for(h=0; h<(clen[j]+1); h++)
							fputc('-',file);
					}
					fprintf(file,"+\n");
					for(j=0; j<depth; j++)
						fprintf(file, "  ");
					currlinelen = depth*2;
					if (ltype_depth > depth) {
						for(j=ltype_depth; j>depth; j--)
							fprintf(file, "  ");
						currlinelen += (ltype_depth-depth)*2;
					}
					fprintf(file,"| ");
				}
			}
			font2 = par->GetFontSettings(i);
			if (font1.latex() != font2.latex()) {
				if (font2.latex() == LyXFont::OFF)
					islatex = 0;
				else
					islatex = 1;
			}
			else {
				islatex = 0;
			}
			c = par->GetChar(i);
			if (islatex)
				continue;
			switch (c) {
			case LYX_META_INSET:
				if ((inset = par->GetInset(i))) {
					fpos = ftell(file);
					inset->Latex(file,-1);
					currlinelen += (ftell(file) - fpos);
					actpos += (ftell(file) - fpos) - 1;
				}
				break;
			case LYX_META_NEWLINE:
				if (par->table) {
					if (par->table->NumberOfCellsInRow(actcell) <= cell) {
						for(j=actpos;j<clen[cell-1];j++)
							fputc(' ',file);
						fprintf(file," |\n");
						for(j=0; j<depth; j++)
							fprintf(file, "  ");
						currlinelen = depth*2;
						if (ltype_depth > depth) {
							for(j=ltype_depth; j>depth; j--)
								fprintf(file, "  ");
							currlinelen += (ltype_depth-depth)*2;
						}
						for(j=0;j<cells;j++) {
							fputc('+',file);
							for(h=0; h<(clen[j]+1); h++)
								fputc('-',file);
						}
						fprintf(file,"+\n");
						for(j=0; j<depth; j++)
							fprintf(file, "  ");
						currlinelen = depth*2;
						if (ltype_depth > depth) {
							for(j=ltype_depth; j>depth; j--)
								fprintf(file, "  ");
							currlinelen += (ltype_depth-depth)*2;
						}
						fprintf(file,"| ");
						cell = 1;
					} else {
						for(j=actpos; j<clen[cell-1]; j++)
							fputc(' ',file);
						fprintf(file, " | ");
						cell++;
					}
                                        actcell++;
					currlinelen = actpos = 0;
				} else {
					fprintf(file, "\n");
					for(j=0; j<depth; j++)
						fprintf(file, "  ");
					currlinelen = depth*2;
					if (ltype_depth > depth) {
						for(j=ltype_depth; j>depth; j--)
							fprintf(file, "  ");
						currlinelen += (ltype_depth-depth)*2;
					}
				}
				break;
			case LYX_META_HFILL: 
				fprintf(file, "\t");
				break;
			case LYX_META_PROTECTED_SEPARATOR:
				fprintf(file, " ");
				break;
			case '\\': 
				fprintf(file, "\\");
				break;
			default:
#ifdef NEW_TEXT
				if (currlinelen > linelen - 10
                                    && c == ' ' && i + 2 < par->size()) {
#else
				if (currlinelen > (linelen-10) \
                                    && c==' ' && (i+2 < par->last)) {
#endif
					fprintf(file, "\n");
					for(j = 0; j < depth; ++j)
						fprintf(file, "  ");
					currlinelen = depth * 2;
					if (ltype_depth > depth) {
						for(j = ltype_depth;
						    j > depth; --j)
							fprintf(file, "  ");
						currlinelen += (ltype_depth-depth)*2;
					}
				} else if (c != '\0')
					fprintf(file, "%c", c);
				else if (c == '\0')
					lyxerr.debug() << "writeAsciiFile: NULL char in structure." << endl;
				currlinelen++;
				break;
			}
		}
		if (par->table) {
			for(j=actpos;j<clen[cell-1];j++)
				fputc(' ',file);
			fprintf(file," |\n");
			for(j=0; j<depth; j++)
				fprintf(file, "  ");
			currlinelen = depth*2;
			if (ltype_depth > depth) {
				for(j=ltype_depth; j>depth; j--)
					fprintf(file, "  ");
				currlinelen += (ltype_depth-depth)*2;
			}
			for(j=0;j<cells;j++) {
				fputc('+',file);
				for(h=0; h<(clen[j]+1); h++)
					fputc('-',file);
			}
			fprintf(file,"+\n");
			delete [] clen;    
		}      
		par = par->next;
	}
   
	fprintf(file, "\n");
}


void Buffer::makeLaTeXFile(string const & filename, 
			   string const & original_path,
			   bool nice, bool only_body)
{
	lyxerr[Debug::LATEX] << "makeLaTeXFile..." << endl;
	params.textclass = GetCurrentTextClass();
	niceFile = nice; // this will be used by Insetincludes.

	tex_code_break_column = lyxrc->ascii_linelen;

        LyXTextClass const & tclass = textclasslist.TextClass(params.textclass);
  
	FilePtr file(filename, FilePtr::write);
	if (!file()) {
		WriteFSAlert(_("Error: Cannot write file:"),filename);
		return;
	}

	// validate the buffer.
	lyxerr[Debug::LATEX] << "  Validating buffer..." << endl;
	LaTeXFeatures features(tclass.numLayouts());
	validate(features);
	lyxerr[Debug::LATEX] << "  Buffer validation done." << endl;
	
	texrow.reset();
	// The starting paragraph of the coming rows is the 
	// first paragraph of the document. (Asger)
	texrow.start(paragraph, 0);

	string userName(getUserName());
	string LFile;
	
	if (!only_body && nice) {
		LFile += "%% This LaTeX-file was created by <";
		LFile += userName + "> " + (char*)date();
		LFile += "%% LyX 1.0 (C) 1995-1999 by Matthias Ettrich and the LyX Team\n";
		LFile += "\n%% Do not edit this file unless you know what you are doing.\n";
		texrow.newline();
		texrow.newline();
		texrow.newline();
		texrow.newline();
	}
	lyxerr.debug() << "lyx header finished" << endl;
	// There are a few differences between nice LaTeX and usual files:
	// usual is \batchmode, uses \listfiles and has a 
	// special input@path to allow the including of figures
	// with either \input or \includegraphics (what figinsets do).
	// batchmode is not set if there is a tex_code_break_column.
	// In this case somebody is interested in the generated LaTeX,
	// so this is OK. input@path is set when the actual parameter
	// original_path is set. This is done for usual tex-file, but not
	// for nice-latex-file. (Matthias 250696)
	if (!only_body) {
		if (!nice){
			// code for usual, NOT nice-latex-file
			LFile += "\\batchmode\n"; // changed
			// from \nonstopmode
			texrow.newline();
			// We don't need listfiles anymore
			//LFile += "\\listfiles\n";
			//texrow.newline();
		}
		if (!original_path.empty()) {
			LFile += "\\makeatletter\n";
			texrow.newline();
			LFile += "\\def\\input@path{{" + original_path
				 + "/}}\n";
			texrow.newline();
			LFile += "\\makeatother\n";
			texrow.newline();
		}
		
		LFile += "\\documentclass";
		
		string options; // the document class options.
		
		if (tokenPos(tclass.opt_fontsize(), '|',params.fontsize) >= 0) {
			// only write if existing in list (and not default)
			options += params.fontsize;
			options += "pt,";
		}
		
		
		if (!params.use_geometry &&
		    (params.paperpackage == PACKAGE_NONE)) {
			switch (params.papersize) {
			case PAPER_A4PAPER:
				options += "a4paper,";
				break;
			case PAPER_USLETTER:
				options += "letterpaper,";
				break;
			case PAPER_A5PAPER:
				options += "a5paper,";
				break;
			case PAPER_B5PAPER:
				options += "b5paper,";
				break;
			case PAPER_EXECUTIVEPAPER:
				options += "executivepaper,";
				break;
			case PAPER_LEGALPAPER:
				options += "legalpaper,";
				break;
			}
		}

		// if needed
		if (params.sides != tclass.sides()) {
			if (params.sides == 2)
				options += "twoside,";
			else
				options += "oneside,";
		}

		// if needed
		if (params.columns != tclass.columns()) {
			if (params.columns == 2)
				options += "twocolumn,";
			else
				options += "onecolumn,";
		}

		if (!params.use_geometry && params.orientation == ORIENTATION_LANDSCAPE)
			options += "landscape,";
		
		// language should be a parameter to \documentclass
		if (params.language != "default") {
			options += params.language + ',';
		}
		
		// the user-defined options
		if (!params.options.empty()) {
			options += params.options + ',';
		}
		
		if (!options.empty()){
			options = strip(options, ',');
			LFile += '[';
			LFile += options;
			LFile += ']';
		}
		
		LFile += '{';
		LFile += textclasslist.LatexnameOfClass(params.textclass);
		LFile += "}\n";
		texrow.newline();
		// end of \documentclass defs
		
		// font selection must be done before loading fontenc.sty
		if (params.fonts != "default") {
			LFile += "\\usepackage{" + params.fonts + "}\n";
			texrow.newline();
		}
		// this one is not per buffer
		if (lyxrc->fontenc != "default") {
			LFile += "\\usepackage[" + lyxrc->fontenc
				 + "]{fontenc}\n";
			texrow.newline();
		}
		if (params.inputenc != "default") {
			LFile += "\\usepackage[" + params.inputenc
				 + "]{inputenc}\n";
			texrow.newline();
		}
		
		/* at the very beginning the text parameters */
		if (params.paperpackage != PACKAGE_NONE) {
			switch (params.paperpackage) {
			case PACKAGE_A4:
				LFile += "\\usepackage{a4}\n";
				texrow.newline();
				break;
			case PACKAGE_A4WIDE:
				LFile += "\\usepackage{a4wide}\n";
				texrow.newline();
				break;
			case PACKAGE_WIDEMARGINSA4:
				LFile += "\\usepackage[widemargins]{a4}\n";
				texrow.newline();
				break;
			}
		}
		if (params.use_geometry) {
			LFile += "\\usepackage{geometry}\n";
			texrow.newline();
			LFile += "\\geometry{verbose";
			if (params.orientation == ORIENTATION_LANDSCAPE)
				LFile += ",landscape";
			switch (params.papersize2) {
			case VM_PAPER_CUSTOM:
				if (!params.paperwidth.empty())
					LFile += ",paperwidth="
						 + params.paperwidth;
				if (!params.paperheight.empty())
					LFile += ",paperheight="
						 + params.paperheight;
				break;
			case VM_PAPER_USLETTER:
				LFile += ",letterpaper";
				break;
			case VM_PAPER_USLEGAL:
				LFile += ",legalpaper";
				break;
			case VM_PAPER_USEXECUTIVE:
				LFile += ",executivepaper";
				break;
			case VM_PAPER_A3:
				LFile += ",a3paper";
				break;
			case VM_PAPER_A4:
				LFile += ",a4paper";
				break;
			case VM_PAPER_A5:
				LFile += ",a5paper";
				break;
			case VM_PAPER_B3:
				LFile += ",b3paper";
				break;
			case VM_PAPER_B4:
				LFile += ",b4paper";
				break;
			case VM_PAPER_B5:
				LFile += ",b5paper";
				break;
			default:
				// default papersize ie VM_PAPER_DEFAULT
				switch (lyxrc->default_papersize) {
				case PAPER_DEFAULT: // keep compiler happy
				case PAPER_USLETTER:
					LFile += ",letterpaper";
					break;
				case PAPER_LEGALPAPER:
					LFile += ",legalpaper";
					break;
				case PAPER_EXECUTIVEPAPER:
					LFile += ",executivepaper";
					break;
				case PAPER_A3PAPER:
					LFile += ",a3paper";
					break;
				case PAPER_A4PAPER:
					LFile += ",a4paper";
					break;
				case PAPER_A5PAPER:
					LFile += ",a5paper";
					break;
				case PAPER_B5PAPER:
					LFile += ",b5paper";
					break;
				}
			}
			if (!params.topmargin.empty())
				LFile += ",tmargin=" + params.topmargin;
			if (!params.bottommargin.empty())
				LFile += ",bmargin=" + params.bottommargin;
			if (!params.leftmargin.empty())
				LFile += ",lmargin=" + params.leftmargin;
			if (!params.rightmargin.empty())
				LFile += ",rmargin=" + params.rightmargin;
			if (!params.headheight.empty())
				LFile += ",headheight=" + params.headheight;
			if (!params.headsep.empty())
				LFile += ",headsep=" + params.headsep;
			if (!params.footskip.empty())
				LFile += ",footskip=" + params.footskip;
			LFile += "}\n";
			texrow.newline();
		}
		if (params.use_amsmath
		    && !prefixIs(textclasslist.LatexnameOfClass(params.textclass), "ams")) {
			LFile += "\\usepackage{amsmath}\n";
		}

		if (tokenPos(tclass.opt_pagestyle(), '|',params.pagestyle) >= 0) {
			if (params.pagestyle == "fancy") {
				LFile += "\\usepackage{fancyhdr}\n";
				texrow.newline();
			}
			LFile += "\\pagestyle{" + params.pagestyle + "}\n";
			texrow.newline();
		}

		// We try to load babel late, in case it interferes
		// with other packages.
		if (params.language != "default") {
			LFile += "\\usepackage{babel}\n";
			texrow.newline();
		}

		if (params.secnumdepth != tclass.secnumdepth()) {
			LFile += "\\setcounter{secnumdepth}{";
			LFile += tostr(params.secnumdepth);
			LFile += "}\n";
			texrow.newline();
		}
		if (params.tocdepth != tclass.tocdepth()) {
			LFile += "\\setcounter{tocdepth}{";
			LFile += tostr(params.tocdepth);
			LFile += "}\n";
			texrow.newline();
		}
		
		if (params.paragraph_separation) {
			switch (params.defskip.kind()) {
			case VSpace::SMALLSKIP: 
				LFile += "\\setlength\\parskip{\\smallskipamount}\n";
				break;
			case VSpace::MEDSKIP:
				LFile += "\\setlength\\parskip{\\medskipamount}\n";
				break;
			case VSpace::BIGSKIP:
				LFile += "\\setlength\\parskip{\\bigskipamount}\n";
				break;
			case VSpace::LENGTH:
				LFile += "\\setlength\\parskip{"
					 + params.defskip.length().asLatexString()
					 + "}\n";
				break;
			default: // should never happen // Then delete it.
				LFile += "\\setlength\\parskip{\\medskipamount}\n";
				break;
			}
			texrow.newline();
			
			LFile += "\\setlength\\parindent{0pt}\n";
			texrow.newline();
		}

		// Write out what we've generated so far...and reset LFile
		fwrite(LFile.c_str(), sizeof(char), LFile.length(), file);
		LFile.clear(); 

		// Now insert the LyX specific LaTeX commands...
		string preamble, tmppreamble;

		// The optional packages;
		preamble = features.getPackages(params);

		// this might be useful...
		preamble += "\n\\makeatletter\n\n";

		// Some macros LyX will need
		tmppreamble = features.getMacros(params);

		if (!tmppreamble.empty()) {
			preamble += "\n%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% "
				"LyX specific LaTeX commands.\n"
				+ tmppreamble + '\n';
		}

		// the text class specific preamble 
		tmppreamble = features.getTClassPreamble(params);
		if (!tmppreamble.empty()) {
			preamble += "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% "
				"Textclass specific LaTeX commands.\n"
				+ tmppreamble + '\n';
		}

		/* the user-defined preamble */
		if (!params.preamble.empty()) {
			preamble += "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% "
				"User specified LaTeX commands.\n"
				+ params.preamble + '\n';
		}

		preamble += "\\makeatother\n\n";

		// Itemize bullet settings need to be last in case the user
		// defines their own bullets that use a package included
		// in the user-defined preamble -- ARRae
		for (int i = 0; i < 4; ++i) {
			if (params.user_defined_bullets[i] != ITEMIZE_DEFAULTS[i]) {
				preamble += "\\renewcommand\\labelitemi";
				switch (i) {
					// `i' is one less than the item to modify
				case 0:
					break;
				case 1:
					preamble += 'i';
					break;
				case 2:
					preamble += "ii";
					break;
				case 3:
					preamble += 'v';
					break;
				}
				preamble += "[0]{" + params.user_defined_bullets[i].getText() + "}\n";
			}
		}

		for (int j = countChar(preamble, '\n'); j-- ;) {
			texrow.newline();
		}

		// A bit faster than printing a char at a time I think.
		fwrite(preamble.c_str(), sizeof(char),
		       preamble.length(), file);

		// make the body.
		LFile += "\\begin{document}\n\n";
		texrow.newline();
		texrow.newline();
	} // only_body
	lyxerr.debug() << "preamble finished, now the body." << endl;
	
	bool was_title = false;
	bool already_title = false;
	string ftnote;
	TexRow ft_texrow;
	int ftcount = 0;
	int loop_count = 0;

	LyXParagraph * par = paragraph;

	// if only_body
	while (par) {
		++loop_count;
		if (par->IsDummy())
			lyxerr[Debug::LATEX] << "Error in MakeLateXFile."
					     << endl;
		LyXLayout const & layout = textclasslist.Style(params.textclass,
						    par->layout);
	    
	        if (layout.intitle) {
			if (already_title) {
				lyxerr <<"Error in MakeLatexFile: You"
					" should not mix title layouts"
					" with normal ones." << endl;
			} else
				was_title = true;
	        } else if (was_title && !already_title) {
			LFile += "\\maketitle\n";
			texrow.newline();
			already_title = true;
			was_title = false;		    
		}
		// We are at depth 0 so we can just use
		// ordinary \footnote{} generation
		// flag this with ftcount
		ftcount = -1;
		if (layout.isEnvironment()
                    || par->pextra_type != PEXTRA_NONE) {
			par = par->TeXEnvironment(LFile, texrow,
						  ftnote, ft_texrow, ftcount);
		} else {
			par = par->TeXOnePar(LFile, texrow,
					     ftnote, ft_texrow, ftcount);
		}

		// Write out what we've generated...and reset LFile
		if (ftcount >= 1) {
			if (ftcount > 1) {
				LFile += "\\addtocounter{footnote}{-";
				LFile += tostr(ftcount - 1);
				LFile += '}';
			}
			LFile += ftnote;
			texrow += ft_texrow;
			ftnote.clear();
			ft_texrow.reset();
			ftcount = 0;
		}
		if (loop_count == 2) {
			// fwrite()ing every second time through the loop
			// gains a few extra % of speed; going higher than
			// 2 will slow things down again.  I'll look at
			// LFile.length() in a future revision.  ARRae
			fwrite(LFile.c_str(), sizeof(char),
			       LFile.length(), file);
			LFile.clear();
			loop_count = 0;
		}
	}

	// It might be that we only have a title in this document
	if (was_title && !already_title) {
		LFile += "\\maketitle\n";
		texrow.newline();
	}

	if (!only_body) {
		LFile += "\\end{document}\n";
		texrow.newline();
	
		lyxerr[Debug::LATEX] << "makeLaTeXFile...done" << endl;
	} else {
		lyxerr[Debug::LATEX] << "LaTeXFile for inclusion made."
				     << endl;
	}

	// Just to be sure. (Asger)
	texrow.newline();

	// Write out what we've generated...and reset LFile
	fwrite(LFile.c_str(), sizeof(char), LFile.length(), file);
	LFile.clear();

	// tex_code_break_column's value is used to decide
	// if we are in batchmode or not (within mathed_write()
	// in math_write.C) so we must set it to a non-zero
	// value when we leave otherwise we save incorrect .lyx files.
	tex_code_break_column = lyxrc->ascii_linelen;

	if (file.close()) {
		WriteFSAlert(_("Error! Could not close file properly:"), filename);
	}
	lyxerr.debug() << "Finished making latex file." << endl;
}


bool Buffer::isLatex()
{
	return textclasslist.TextClass(params.textclass).outputType() == LATEX;
}


bool Buffer::isLinuxDoc()
{
	return textclasslist.TextClass(params.textclass).outputType() == LINUXDOC;
}


bool Buffer::isLiterate()
{
	return textclasslist.TextClass(params.textclass).outputType() == LITERATE;
}


bool Buffer::isDocBook()
{
	return textclasslist.TextClass(params.textclass).outputType() == DOCBOOK;
}


bool Buffer::isSGML()
{
	return textclasslist.TextClass(params.textclass).outputType() == LINUXDOC ||
	       textclasslist.TextClass(params.textclass).outputType() == DOCBOOK;
}


void Buffer::sgmlOpenTag(FILE * file, int depth, string const & latexname) const
{
	static char *space[] = {" ","  ","   ","    ","     ","      ","       ",
			 "        ","         ","          ","          "};

	fprintf(file, "%s<%s>\n", space[depth], latexname.c_str());
}


void Buffer::sgmlCloseTag(FILE * file, int depth, string const & latexname) const
{
	static char *space[] = {" ","  ","   ","    ","     ","      ","       ",
			 "        ","         ","          ","          "};

	fprintf(file, "%s</%s>\n", space[depth], latexname.c_str());
}


void Buffer::makeLinuxDocFile(string const & filename, int column)
{
	LyXParagraph * par = paragraph;

	string top_element=textclasslist.LatexnameOfClass(params.textclass);
	string environment_stack[10];
        string item_name;

	int depth = 0;              /* paragraph depth */

	FilePtr file(filename, FilePtr::write);
	tex_code_break_column = column; 

	if (!file()) {
		WriteAlert(_("LYX_ERROR:"), _("Cannot write file"), filename);
		return;
	}
   
	//ResetTexRow();
	texrow.reset();
   
	if (params.preamble.empty()) {
		fprintf(file, "<!doctype linuxdoc system>\n\n");
	}
	else {
		fprintf(file, "<!doctype linuxdoc system \n [ %s \n]>\n\n", 
			params.preamble.c_str() );
	}

        string userName(getUserName());
	fprintf(file, "<!-- LinuxDoc file was created by LyX 1.0 (C) 1995-1999 ");
	fprintf(file, "by <%s> %s -->\n", userName.c_str(), (char *)date());

	if(params.options.empty())
		sgmlOpenTag(file,0,top_element);
	else {
		string top = top_element;
		top += " ";
		top += params.options;
		sgmlOpenTag(file,0,top);
	}

	while (par) {
		int desc_on=0;            /* description mode*/
		LyXLayout const & style=textclasslist.Style(GetCurrentTextClass(), par->layout);
		par->AutoDeleteInsets();

		/* treat <toc> as a special case for compatibility with old code */
		if (par->GetChar(0) == LYX_META_INSET) {
		        Inset *inset = par->GetInset(0);
			char  lyx_code = inset->LyxCode();
			if (lyx_code ==Inset::TOC_CODE){
				string temp= "toc";
				sgmlOpenTag(file, depth, temp);

				par = par->next;
				linuxDocHandleFootnote(file, par, depth);
				continue;
			}
		}

		/* environment tag closing */
		for( ;depth > par->depth; depth--) {
			sgmlCloseTag(file,depth,environment_stack[depth]);
			environment_stack[depth].clear();
		}

		/* write opening SGML tags */
		switch(style.latextype) {
		case LATEX_PARAGRAPH:
			if(depth == par->depth 
			   && !environment_stack[depth].empty()) {
				sgmlCloseTag(file,depth,environment_stack[depth]);
				environment_stack[depth].clear();
				if(depth) 
					depth--;
				else
				        fprintf(file,"</p>");
			}
			sgmlOpenTag(file,depth,style.latexname());
			break;

		case LATEX_COMMAND:
			if (depth!=0)
				LinuxDocError(par, 0, _("Error : Wrong depth for LatexType Command.\n"));

			if (!environment_stack[depth].empty()){
				sgmlCloseTag(file,depth,environment_stack[depth]);
				fprintf(file, "</p>");
			}

			environment_stack[depth].clear();
			sgmlOpenTag(file,depth, style.latexname());
			break;

		case LATEX_ENVIRONMENT:
		case LATEX_ITEM_ENVIRONMENT:
			if(depth == par->depth 
			   && environment_stack[depth] != style.latexname()
			   && !environment_stack[depth].empty()) {

				sgmlCloseTag(file,depth,environment_stack[depth]);
				environment_stack[depth].clear();
			}
			if (depth < par->depth) {
			       depth = par->depth;
			       environment_stack[depth].clear();
			}
			if (environment_stack[depth] != style.latexname()) {
				if(depth==0) {
					string temp="p";
					sgmlOpenTag(file,depth,temp);
				}
				environment_stack[depth] = style.latexname();
				sgmlOpenTag(file,depth,environment_stack[depth]);
			}
			if(style.latextype == LATEX_ENVIRONMENT) break;

			desc_on =(style.labeltype == LABEL_MANUAL);

			if(desc_on)
				item_name="tag";
			else
				item_name="item";

			sgmlOpenTag(file,depth+1,item_name);
			break;
		default:
			sgmlOpenTag(file, depth, style.latexname());
			break;
		}

		do {
			SimpleLinuxDocOnePar(file, par, desc_on, depth);

			par = par->next;
			linuxDocHandleFootnote(file,par,depth);
		}
		while(par && par->IsDummy());

		fprintf(file,"\n");
		/* write closing SGML tags */
		switch(style.latextype) {
		case LATEX_COMMAND:
		case LATEX_ENVIRONMENT:
		case LATEX_ITEM_ENVIRONMENT:
			break;
		default:
			sgmlCloseTag(file, depth, style.latexname());
			break;
		}


	}
   
	
	/* Close open tags */
	for(;depth>0;depth--)
	        sgmlCloseTag(file,depth,environment_stack[depth]);

	if(!environment_stack[depth].empty())
	        sgmlCloseTag(file,depth,environment_stack[depth]);

	fprintf(file, "\n\n");
	sgmlCloseTag(file,0,top_element);

	if (file.close()) {
		WriteFSAlert(_("Error! Could not close file properly:"),
			     filename);
	}
}


void Buffer::linuxDocHandleFootnote(FILE *file,LyXParagraph* &par, int const depth)
{
	string tag="footnote";

	while (par && par->footnoteflag != LyXParagraph::NO_FOOTNOTE) {
		sgmlOpenTag(file,depth+1,tag);
		SimpleLinuxDocOnePar(file, par, 0,depth+1);
		sgmlCloseTag(file,depth+1,tag);
		par = par->next;
	}
}

void Buffer::DocBookHandleCaption(FILE *file, string &inner_tag,
				  int const depth, int desc_on,
				  LyXParagraph* &par)
{
	LyXParagraph *tpar = par;
	string tmp_par, extra_par;
	while (tpar && (tpar->footnoteflag != LyXParagraph::NO_FOOTNOTE) &&
	       (tpar->layout != textclasslist.NumberOfLayout(params.textclass,"Caption").second))
		tpar = tpar->next;
	if (tpar &&
	    tpar->layout==textclasslist.NumberOfLayout(params.textclass,"Caption").second) {
		sgmlOpenTag(file,depth+1,inner_tag);
	    SimpleDocBookOnePar(tmp_par,extra_par,tpar,desc_on,depth+2);
               tmp_par = strip(tmp_par);
               tmp_par = frontStrip(tmp_par);
               fprintf(file,"%s",tmp_par.c_str());
               sgmlCloseTag(file,depth+1,inner_tag);
               if(!extra_par.empty())
                       fprintf(file,"%s",extra_par.c_str());
       }
}

void Buffer::DocBookHandleFootnote(FILE *file,LyXParagraph* &par, int const depth)
{
	string tag,inner_tag;
	string tmp_par,extra_par;
	bool inner_span = false;
	int desc_on=4;

	// This is not how I like to see enums. They should not be anonymous
	// and variables of its type should not be declared right after the
	// last brace. (Lgb)
	enum {
		NO_ONE,
		FOOTNOTE_LIKE,
		MARGIN_LIKE,
		FIG_LIKE,
		TAB_LIKE
	} last=NO_ONE, present=FOOTNOTE_LIKE;

	while (par && par->footnoteflag != LyXParagraph::NO_FOOTNOTE) {
		if(last==present) {
			if(inner_span) {
				if(!tmp_par.empty()) {
					fprintf(file,"%s",tmp_par.c_str());
					tmp_par.clear();
					sgmlCloseTag(file,depth+1,inner_tag);
					sgmlOpenTag(file,depth+1,inner_tag);
				}
			}
			else
				fprintf(file,"\n");
		} else {
			fprintf(file,"%s",tmp_par.c_str());
			if(!inner_tag.empty()) sgmlCloseTag(file,depth+1,inner_tag);
			if(!extra_par.empty()) fprintf(file,"%s",extra_par.c_str());
			if(!tag.empty()) sgmlCloseTag(file,depth,tag);
			extra_par.clear();

			switch (par->footnotekind) {
			case LyXParagraph::FOOTNOTE:
			case LyXParagraph::ALGORITHM:
				tag="footnote";
				inner_tag="para";
				present=FOOTNOTE_LIKE;
				inner_span=true;
				break;
			case LyXParagraph::MARGIN:
				tag="sidebar";
				inner_tag="para";
				present=MARGIN_LIKE;
				inner_span=true;
				break;
			case LyXParagraph::FIG:
			case LyXParagraph::WIDE_FIG:
				tag="figure";
				inner_tag="title";
				present=FIG_LIKE;
				inner_span=false;
				break;
			case LyXParagraph::TAB:
			case LyXParagraph::WIDE_TAB:
				tag="table";
				inner_tag="title";
				present=TAB_LIKE;
				inner_span=false;
				break;
			}
			sgmlOpenTag(file,depth,tag);
			if ((present == TAB_LIKE) || (present == FIG_LIKE)) {
				DocBookHandleCaption(file, inner_tag, depth,
						     desc_on, par);
				inner_tag.clear();
			} else {
				sgmlOpenTag(file,depth+1,inner_tag);
			}
		}
		// ignore all caption here, we processed them above!!!
		if (par->layout != textclasslist.NumberOfLayout(params.textclass,
							   "Caption").second) {
			SimpleDocBookOnePar(tmp_par,extra_par,par,
					    desc_on,depth+2);
		}
		tmp_par = frontStrip(strip(tmp_par));

		last=present;
		par = par->next;
	}
	fprintf(file,"%s",tmp_par.c_str());
	if(!inner_tag.empty()) sgmlCloseTag(file,depth+1,inner_tag);
	if(!extra_par.empty()) fprintf(file,"%s",extra_par.c_str());
	if(!tag.empty()) sgmlCloseTag(file,depth,tag);

}


/* push a tag in a style stack */
void Buffer::push_tag(FILE *file, char const *tag,
		      int& pos, char stack[5][3])
{
	int j;

	/* pop all previous tags */
	for (j=pos; j>=0; j--)
		fprintf(file, "</%s>", stack[j]); 

	/* add new tag */
	sprintf(stack[++pos], "%s", tag);

	/* push all tags */
	for (j=0; j<=pos; j++)
		fprintf(file, "<%s>", stack[j]);
}


// pop a tag from a style stack
void Buffer::pop_tag(FILE *file, char const * tag,
		     int& pos, char stack[5][3])
{
	int j;

	// pop all tags till specified one
	for (j=pos; (j>=0) && (strcmp(stack[j], tag)); j--)
		fprintf(file, "</%s>", stack[j]);

	// closes the tag
	fprintf(file, "</%s>", tag);

	// push all tags, but the specified one
	for (j=j+1; j<=pos; j++) {
		fprintf(file, "<%s>", stack[j]);
		strcpy(stack[j-1], stack[j]);
	}
	pos --;
}


/* handle internal paragraph parsing -- layout already processed */

// checks, if newcol chars should be put into this line
// writes newline, if necessary.
static
void linux_doc_line_break(FILE *file, unsigned int &colcount, const unsigned int newcol)
{
	colcount += newcol;
	if (colcount > lyxrc->ascii_linelen) {
		fprintf(file, "\n");
		colcount = newcol; // assume write after this call
	}
}


void Buffer::SimpleLinuxDocOnePar(FILE *file, LyXParagraph *par, int desc_on, int const depth)
{
	LyXFont font1,font2;
	char c;
	Inset * inset;
#ifdef NEW_TEXT
	LyXParagraph::size_type main_body;
	int j;
#else
	int main_body, j;
#endif
	LyXLayout const & style = textclasslist.Style(params.textclass, par->GetLayout());

	char family_type = 0;               // family font flag 
	bool is_bold     = false;           // series font flag 
	char shape_type  = 0;               // shape font flag 
	bool is_em = false;                 // emphasis (italic) font flag 

	int stack_num = -1;          // style stack position 
	char stack[5][3];    	     // style stack 
        unsigned int char_line_count = 5;     // Heuristic choice ;-) 

	if (style.labeltype != LABEL_MANUAL)
		main_body = 0;
	else
		main_body = par->BeginningOfMainBody();

	/* gets paragraph main font */
	if (main_body > 0)
		font1 = style.labelfont;
	else
		font1 = style.font;

  
	/* parsing main loop */
#ifdef NEW_TEXT
	for (LyXParagraph::size_type i = 0;
	     i < par->size(); ++i) {
#else
	for (int i = 0; i < par->last; i++) {
#endif

		/* handle quote tag */
		if (i == main_body && !par->IsDummy()) {
			if (main_body > 0)
				font1 = style.font;
		}

		font2 = par->getFont(i);

		if (font1.family() != font2.family()) {
			switch(family_type) {
			case 0:
			       	if (font2.family() == LyXFont::TYPEWRITER_FAMILY) {
			        	push_tag(file, "tt", stack_num, stack);
					family_type=1;
				}
				else if (font2.family() == LyXFont::SANS_FAMILY) {
				        push_tag(file, "sf", stack_num, stack);
					family_type=2;
				}
				break;
			case 1:
				pop_tag(file, "tt", stack_num, stack);
				if (font2.family() == LyXFont::SANS_FAMILY) {
			        	push_tag(file, "sf", stack_num, stack);
					family_type=2;
				}
				else {
					family_type=0;
				}
				break;
			case 2:
				pop_tag(file, "sf", stack_num, stack);
				if (font2.family() == LyXFont::TYPEWRITER_FAMILY) {
			        	push_tag(file, "tt", stack_num, stack);
					family_type=1;
				}
				else {
					family_type=0;
				}
			}
		}

		/* handle bold face */
		if (font1.series() != font2.series()) {
		        if (font2.series() == LyXFont::BOLD_SERIES) {
			        push_tag(file, "bf", stack_num, stack);
				is_bold = true;
			}
			else if (is_bold) {
			        pop_tag(file, "bf", stack_num, stack);
				is_bold = false;
			}
		}

		/* handle italic and slanted fonts */
		if (font1.shape() != font2.shape()) {
			switch(shape_type) {
			case 0:
			       	if (font2.shape() == LyXFont::ITALIC_SHAPE) {
			        	push_tag(file, "it", stack_num, stack);
					shape_type=1;
				}
				else if (font2.shape() == LyXFont::SLANTED_SHAPE) {
				        push_tag(file, "sl", stack_num, stack);
					shape_type=2;
				}
				break;
			case 1:
				pop_tag(file, "it", stack_num, stack);
				if (font2.shape() == LyXFont::SLANTED_SHAPE) {
			        	push_tag(file, "sl", stack_num, stack);
					shape_type=2;
				}
				else {
					shape_type=0;
				}
				break;
			case 2:
				pop_tag(file, "sl", stack_num, stack);
				if (font2.shape() == LyXFont::ITALIC_SHAPE) {
			        	push_tag(file, "it", stack_num, stack);
					shape_type=1;
				}
				else {
					shape_type=0;
				}
			}
		}
		/* handle <em> tag */
		if (font1.emph() != font2.emph()) {
			if (font2.emph() == LyXFont::ON) {
				push_tag(file, "em", stack_num, stack);
				is_em = true;
			} else if (is_em) {
				pop_tag(file, "em", stack_num, stack);
				is_em = false;
			}
		}

		c = par->GetChar(i);
      
		if (font2.latex() == LyXFont::ON) {
			// "TeX"-Mode on ==> SGML-Mode on.
			if (c!='\0')
				fprintf(file, "%c", c); // see LaTeX-Generation...
			char_line_count++;
		} else if (c == LYX_META_INSET) {
			inset = par->GetInset(i);
			string tmp_out;
			inset->Linuxdoc(tmp_out);
			fprintf(file,"%s",tmp_out.c_str());
		}
		else {
			string sgml_string;
			if (par->linuxDocConvertChar(c, sgml_string)
			    && !style.free_spacing) { // in freespacing
				                     // mode, spaces are
				                     // non-breaking characters
				// char is ' '
				if (desc_on == 1) {
					char_line_count++;
					linux_doc_line_break(file, char_line_count, 6);
					fprintf(file, "</tag>");
					desc_on = 2;
				}
				else  {
					linux_doc_line_break(file, char_line_count, 1);
					fprintf(file, "%c", c);
				}
			}
			else {
				fprintf(file, "%s", sgml_string.c_str());
				char_line_count += sgml_string.length();
			}
		}
		font1 = font2;
	}

	/* needed if there is an optional argument but no contents */
#ifdef NEW_TEXT
	if (main_body > 0 && main_body == par->size()) {
		font1 = style.font;
	}
#else
	if (main_body > 0 && main_body == par->last) {
		font1 = style.font;
	}
#endif
	/* pop all defined Styles */
	for (j = stack_num; j >= 0; j--) {
	        linux_doc_line_break(file, 
				     char_line_count, 
				     3+strlen(stack[j]));
		fprintf(file, "</%s>", stack[j]);
	}

	/* resets description flag correctly */
	switch(desc_on){
	case 1:
		/* <tag> not closed... */
		linux_doc_line_break(file, char_line_count, 6);
		fprintf(file, "</tag>");
		break;
	case 2:
	        /* fprintf(file, "</p>");*/
		break;
	}
}


/* print an error message */
void Buffer::LinuxDocError(LyXParagraph * par, int pos,
			   char const * message) 
{
	InsetError * new_inset;

	/* insert an error marker in text */
	new_inset = new InsetError(message);
	par->InsertChar(pos, LYX_META_INSET);
	par->InsertInset(pos, new_inset);
}

// This constant defines the maximum number of 
// environment layouts that can be nesteded.
// The same applies for command layouts.
// These values should be more than enough.
//           José Matos (1999/07/22)

enum { MAX_NEST_LEVEL = 25};

void Buffer::makeDocBookFile(string const & filename, int column)
{
	LyXParagraph * par = paragraph;

	string top_element=textclasslist.LatexnameOfClass(params.textclass);
	string environment_stack[MAX_NEST_LEVEL];
	string environment_inner[MAX_NEST_LEVEL];
	string command_stack[MAX_NEST_LEVEL];
	bool command_flag=false;
	int command_depth=0,command_base=0,cmd_depth=0;

        string item_name,command_name;
	string c_depth,c_params,tmps;

	int depth=0;              /* paragraph depth */

	FilePtr file(filename, FilePtr::write);
	tex_code_break_column = column; 

	if (!file()) {
		WriteAlert(_("LYX_ERROR:"), _("Cannot write file"), filename);
		return;
	}
   
	//ResetTexRow();
	texrow.reset();

	fprintf(file,
		"<!doctype %s public \"-//OASIS//DTD DocBook V3.1//EN\"",
		top_element.c_str());

	if (params.preamble.empty())
		fprintf(file, ">\n\n");
	else
		fprintf(file, "\n [ %s \n]>\n\n",params.preamble.c_str() );

        string userName(getUserName());
	fprintf(file,
		"<!-- DocBook file was created by LyX 1.0 (C) 1995-1999\n");
	fprintf(file, "by <%s> %s -->\n", userName.c_str(), (char *)date());

	if(params.options.empty())
		sgmlOpenTag(file,0,top_element);
	else {
		string top = top_element;
		top += " ";
		top += params.options;
		sgmlOpenTag(file,0,top);
	}

	while (par) {
		int desc_on=0;            /* description mode*/
		LyXLayout const & style = textclasslist.Style(GetCurrentTextClass(),
						   par->layout);
		par->AutoDeleteInsets();

		/* environment tag closing */
		for( ;depth > par->depth; depth--) {
			if(environment_inner[depth] != "!-- --") {
				item_name="listitem";
				sgmlCloseTag(file,command_depth+depth,
					     item_name);
				if( environment_inner[depth] == "varlistentry")
					sgmlCloseTag(file,depth+command_depth,
						     environment_inner[depth]);
			}
			sgmlCloseTag(file,depth+command_depth,
				     environment_stack[depth]);
			environment_stack[depth].clear();
			environment_inner[depth].clear();
		}

		if(depth == par->depth
		   && environment_stack[depth] != style.latexname()
		   && !environment_stack[depth].empty()) {
			if(environment_inner[depth] != "!-- --") {
				item_name="listitem";
				sgmlCloseTag(file,command_depth+depth,
					     item_name);
				if( environment_inner[depth] == "varlistentry")
					sgmlCloseTag(file,depth+command_depth,
						     environment_inner[depth]);
			}
			
			sgmlCloseTag(file,depth+command_depth,
				     environment_stack[depth]);
			
			environment_stack[depth].clear();
			environment_inner[depth].clear();
                }

		// Write opening SGML tags.
		switch(style.latextype) {
		case LATEX_PARAGRAPH:
			if(style.latexname() != "dummy")
                               sgmlOpenTag(file, depth+command_depth,
                                           style.latexname());
			break;

		case LATEX_COMMAND:
			if (depth!=0)
				LinuxDocError(par, 0,
					      _("Error : Wrong depth for "
						"LatexType Command.\n"));
			
			command_name = style.latexname();
			
			tmps = style.latexparam();
			c_params = split(tmps, c_depth,'|');
			
			cmd_depth=atoi(c_depth.c_str());
			
			if(command_flag) {
				if(cmd_depth<command_base) {
					for(int j = command_depth;
					    j >= command_base; j--)
						if(!command_stack[j].empty())
							sgmlCloseTag(file,j,command_stack[j]);
					command_depth=command_base=cmd_depth;
				}
				else if(cmd_depth<=command_depth) {
					for(int j= command_depth;
					    j >= cmd_depth; j--)

						if(!command_stack[j].empty())
							sgmlCloseTag(file,j,command_stack[j]);
					command_depth=cmd_depth;
				}
				else
					command_depth=cmd_depth;
			}
			else {
				command_depth = command_base = cmd_depth;
				command_flag = true;
			}
			command_stack[command_depth]=command_name;

			// treat label as a special case for
			// more WYSIWYM handling.
			if (par->GetChar(0) == LYX_META_INSET) {
			        Inset *inset = par->GetInset(0);
				char  lyx_code = inset->LyxCode();
				if (lyx_code ==Inset::LABEL_CODE){
					command_name+= " id=\"";
					command_name+=((InsetCommand *) inset)->getContents();
					command_name+="\"";
					desc_on=3;
				}
			}

			sgmlOpenTag(file,depth+command_depth, command_name);
			item_name="title";
			sgmlOpenTag(file,depth+1+command_depth,item_name);
			break;

		case LATEX_ENVIRONMENT:
		case LATEX_ITEM_ENVIRONMENT:
			if (depth < par->depth) {
				depth = par->depth;
				environment_stack[depth].clear();
			}

			if (environment_stack[depth] != style.latexname()) {
				environment_stack[depth]= style.latexname();
				environment_inner[depth]= "!-- --";
				sgmlOpenTag(file, depth + command_depth,
					    environment_stack[depth]);
			} else {
				if(environment_inner[depth] != "!-- --") {
					item_name="listitem";
					sgmlCloseTag(file,
						     command_depth + depth,
						     item_name);
					if (environment_inner[depth] == "varlistentry")
						sgmlCloseTag(file,
							     depth+command_depth,
							     environment_inner[depth]);
				}
			}
			
			if(style.latextype == LATEX_ENVIRONMENT) {
				if(!style.latexparam().empty())
			  		sgmlOpenTag(file, depth+command_depth,
						    style.latexparam());
				break;
			}

			desc_on =(style.labeltype == LABEL_MANUAL);

			if(desc_on)
				environment_inner[depth]="varlistentry";
			else
				environment_inner[depth]="listitem";

			sgmlOpenTag(file,depth+1+command_depth,
				    environment_inner[depth]);

			if(desc_on) {
				item_name="term";
				sgmlOpenTag(file,depth+1+command_depth,
					    item_name);
			}
			else {
				item_name="para";
				sgmlOpenTag(file,depth+1+command_depth,
					    item_name);
			}
			break;
		default:
			sgmlOpenTag(file, depth + command_depth,
				    style.latexname());
			break;
		}

		do {
			string tmp_par,extra_par;

			SimpleDocBookOnePar(tmp_par,extra_par, par, desc_on,
					    depth+1+command_depth);
			fprintf(file,"%s",tmp_par.c_str());

			par = par->next;
			DocBookHandleFootnote(file,par, depth+1+command_depth);
		}
		while(par && par->IsDummy());

		string end_tag;
		/* write closing SGML tags */
		switch(style.latextype) {
		case LATEX_COMMAND:
			end_tag = "title";
			sgmlCloseTag(file, depth + command_depth, end_tag);
			break;
		case LATEX_ENVIRONMENT:
			if(!style.latexparam().empty())
				sgmlCloseTag(file, depth + command_depth,
					     style.latexparam());
			break;
		case LATEX_ITEM_ENVIRONMENT:
			if(desc_on==1) break;
			end_tag="para";
			sgmlCloseTag(file,depth+1+command_depth,end_tag);
			break;
		case LATEX_PARAGRAPH:
			if(style.latexname() != "dummy")
				sgmlCloseTag(file, depth + command_depth,
					     style.latexname());
			break;
		default:
			sgmlCloseTag(file,depth+command_depth,
				     style.latexname());
			break;
		}
	}

	// Close open tags
	for(;depth>=0;depth--) {
		if(!environment_stack[depth].empty()) {
			if(environment_inner[depth] != "!-- --") {
				item_name="listitem";
				sgmlCloseTag(file,command_depth+depth,
					     item_name);
                               if( environment_inner[depth] == "varlistentry")
				       sgmlCloseTag(file,depth+command_depth,
						    environment_inner[depth]);
			}
			
			sgmlCloseTag(file,depth+command_depth,
				     environment_stack[depth]);
		}
	}
	
	for(int j=command_depth;j>=command_base;j--)
		if(!command_stack[j].empty())
			sgmlCloseTag(file,j,command_stack[j]);

	fprintf(file, "\n\n");
	sgmlCloseTag(file,0,top_element);

	if (file.close()) {
		WriteFSAlert(_("Error! Could not close file properly:"),
			     filename);
	}
}


void Buffer::SimpleDocBookOnePar(string & file, string & extra,
				 LyXParagraph * par, int & desc_on,
				 int const depth) 
{
	if (par->table) {
		par->SimpleDocBookOneTablePar(file, extra, desc_on, depth);
		return;
	}
	LyXFont font1,font2;
	char c;
	Inset *inset;
#ifdef NEW_TEXT
	LyXParagraph::size_type main_body;
	int j;
#else
	int main_body, j;
#endif
	string emph="emphasis";
	bool emph_flag=false;
	int char_line_count=0;

	LyXLayout const & style = textclasslist.Style(params.textclass, par->GetLayout());

	if (style.labeltype != LABEL_MANUAL)
		main_body = 0;
	else
		main_body = par->BeginningOfMainBody();

	/* gets paragraph main font */
	if (main_body > 0)
		font1 = style.labelfont;
	else
		font1 = style.font;

	char_line_count = depth;
	if(!style.free_spacing)
		for (j=0;j< depth;j++)
			file += ' ';

	/* parsing main loop */
#ifdef NEW_TEXT
	for (LyXParagraph::size_type i = 0;
	     i < par->size(); ++i) {
#else
	for (int i = 0; i < par->last; i++) {
#endif
		font2 = par->getFont(i);

		/* handle <emphasis> tag */
		if (font1.emph() != font2.emph() && i) {
			if (font2.emph() == LyXFont::ON) {
				file += "<emphasis>";
				emph_flag=true;
			}else {
				file += "</emphasis>";
				emph_flag=false;
			}
		}
      
		c = par->GetChar(i);

		if (c == LYX_META_INSET) {
			inset = par->GetInset(i);
			string tmp_out;
			inset->DocBook(tmp_out);
			//
			// This code needs some explanation:
			// Two insets are treated specially
			//   label if it is the first element in a command paragraph
			//         desc_on==3
			//   graphics inside tables or figure floats can't go on
			//   title (the equivalente in latex for this case is caption
			//   and title should come first
			//         desc_on==4
			//
			if(desc_on!=3 || i!=0) {
				if(tmp_out[0]=='@') {
					if(desc_on==4)
						extra += frontStrip(tmp_out, '@');
					else
						file += frontStrip(tmp_out, '@');
				}
				else
					file += tmp_out;
			}
		} else if (font2.latex() == LyXFont::ON) {
			// "TeX"-Mode on ==> SGML-Mode on.
			if (c!='\0')
				file += c;
			char_line_count++;
		}
		else {
			string sgml_string;
			if (par->linuxDocConvertChar(c, sgml_string)
			    && !style.free_spacing) { // in freespacing
				                     // mode, spaces are
				                     // non-breaking characters
				// char is ' '
				if (desc_on == 1) {
					char_line_count++;
					file += '\n';
					file += "</term><listitem><para>";
					desc_on = 2;
				}
				else  {
					file += c;
				}
			}
			else {
				file += sgml_string;
			}
		}
		font1 = font2;
	}

	/* needed if there is an optional argument but no contents */
#ifdef NEW_TEXT
	if (main_body > 0 && main_body == par->size()) {
		font1 = style.font;
	}
#else
	if (main_body > 0 && main_body == par->last) {
		font1 = style.font;
	}
#endif
	if (emph_flag) {
		file += "</emphasis>";
	}
	
	/* resets description flag correctly */
	switch(desc_on){
	case 1:
		/* <term> not closed... */
		file += "</term>";
		break;
	}
	file += '\n';
}


bool Buffer::removeAutoInsets()
{
	LyXParagraph *par = paragraph;

	LyXCursor cursor = text->cursor;
	LyXCursor tmpcursor = cursor;
	cursor.par = tmpcursor.par->ParFromPos(tmpcursor.pos);
	cursor.pos = tmpcursor.par->PositionInParFromPos(tmpcursor.pos);

	bool a = false;
	while (par) {
		if (par->AutoDeleteInsets()){
			a = true;
			if (par->footnoteflag != LyXParagraph::CLOSED_FOOTNOTE){
				/* this is possible now, since SetCursor takes
				   care about footnotes */
				text->SetCursorIntern(par, 0);
				text->RedoParagraphs(text->cursor, text->cursor.par->Next());
				text->FullRebreak();
			}
		}
		par = par->next;
	}
	/* avoid forbidden cursor positions caused by error removing */ 
	if (cursor.pos > cursor.par->Last())
		cursor.pos = cursor.par->Last();
	text->SetCursorIntern(cursor.par, cursor.pos);

	return a;
}


int Buffer::runLaTeX()
{
	if (!text) return 0;

	ProhibitInput();

	// get LaTeX-Filename
	string name = SpaceLess(ChangeExtension (filename, ".tex", true));

	string path = OnlyPath(filename);

	string org_path = path;
	if (lyxrc->use_tempdir || (IsDirWriteable(path) < 1)) {
		path = tmppath;	 
	}

	Path p(path); // path to LaTeX file
	users->getOwner()->getMiniBuffer()->Set(_("Running LaTeX..."));   

	// Remove all error insets
	bool a = removeAutoInsets();

	// Always generate the LaTeX file
	makeLaTeXFile(name, org_path, false);
	markDviDirty();

	// do the LaTex run(s)
	TeXErrors terr;
	LaTeX latex(lyxrc->latex_command, name, filepath);
	int res = latex.run(terr,users->getOwner()->getMiniBuffer()); // running latex

	// check return value from latex.run().
	if ((res & LaTeX::NO_LOGFILE)) {
		WriteAlert(_("LaTeX did not work!"),
			   _("Missing log file:"), name);
	} else if ((res & LaTeX::ERRORS)) {
		users->getOwner()->getMiniBuffer()->Set(_("Done"));
		// Insert all errors as errors boxes
		insertErrors(terr);
		
		// Dvi should also be kept dirty if the latex run
		// ends up with errors. However it should be possible
		// to view a dirty dvi too.
	} else {
		//no errors or any other things to think about so:
		users->getOwner()->getMiniBuffer()->Set(_("Done"));
		markDviClean();
	}

	// if we removed error insets before we ran LaTeX or if we inserted
	// error insets after we ran LaTeX this must be run:
        if (a || (res & LaTeX::ERRORS)){
                users->redraw();
                users->fitCursor();
                users->updateScrollbar();
        }
        AllowInput();
 
        return latex.getNumErrors();
}


int Buffer::runLiterate()
{
	if (!text) return 0;

	ProhibitInput();

	// get LaTeX-Filename
	string name = SpaceLess(ChangeExtension (filename, ".tex", true));
        // get Literate-Filename
        string lit_name = SpaceLess(ChangeExtension (filename, lyxrc->literate_extension, true));

	string path = OnlyPath(filename);

	string org_path = path;
	if (lyxrc->use_tempdir || (IsDirWriteable(path) < 1)) {
		path = tmppath;	 
	}

	Path p(path); // path to Literate file
	users->getOwner()->getMiniBuffer()->Set(_("Running Literate..."));   

	// Remove all error insets
	bool a = removeAutoInsets();

	// generate the Literate file if necessary
	if (!isDviClean() || a) {
		makeLaTeXFile(lit_name, org_path, false);
		markDviDirty();
	}

        Literate literate(lyxrc->latex_command, name, filepath, 
			  lit_name,
			  lyxrc->literate_command, lyxrc->literate_error_filter,
			  lyxrc->build_command, lyxrc->build_error_filter);
	TeXErrors terr;
	int res = literate.weave(terr, users->getOwner()->getMiniBuffer());

	// check return value from literate.weave().
	if ((res & Literate::NO_LOGFILE)) {
		WriteAlert(_("Literate command did not work!"),
			   _("Missing log file:"), name);
	} else if ((res & Literate::ERRORS)) {
		users->getOwner()->getMiniBuffer()->Set(_("Done"));
		// Insert all errors as errors boxes
		insertErrors(terr);
		
		// Dvi should also be kept dirty if the latex run
		// ends up with errors. However it should be possible
		// to view a dirty dvi too.
	} else {
		//no errors or any other things to think about so:
		users->getOwner()->getMiniBuffer()->Set(_("Done"));
		markDviClean();
	}

	// if we removed error insets before we ran LaTeX or if we inserted
	// error insets after we ran LaTeX this must be run:
        if (a || (res & Literate::ERRORS)){
                users->redraw();
                users->fitCursor();
                users->updateScrollbar();
        }
        AllowInput();
 
        return literate.getNumErrors();
}


int Buffer::buildProgram()
{
        if (!text) return 0;
 
        ProhibitInput();
 
        // get LaTeX-Filename
        string name = SpaceLess(ChangeExtension (filename, ".tex", true));
        // get Literate-Filename
        string lit_name = SpaceLess(ChangeExtension (filename, lyxrc->literate_extension, true));
 
        string path = OnlyPath(filename);
 
        string org_path = path;
        if (lyxrc->use_tempdir || (IsDirWriteable(path) < 1)) {
                path = tmppath;  
        }
 
        Path p(path); // path to Literate file
        users->getOwner()->getMiniBuffer()->Set(_("Building Program..."));   
 
        // Remove all error insets
        bool a = removeAutoInsets();
 
        // generate the LaTeX file if necessary
        if (!isNwClean() || a) {
                makeLaTeXFile(lit_name, org_path, false);
                markNwDirty();
        }
 
        Literate literate(lyxrc->latex_command, name, filepath, 
			  lit_name,
			  lyxrc->literate_command, lyxrc->literate_error_filter,
			  lyxrc->build_command, lyxrc->build_error_filter);
        TeXErrors terr;
        int res = literate.build(terr, users->getOwner()->getMiniBuffer());
 
        // check return value from literate.build().
        if ((res & Literate::NO_LOGFILE)) {
                WriteAlert(_("Build did not work!"),
                           _("Missing log file:"), name);
        } else if ((res & Literate::ERRORS)) {
                users->getOwner()->getMiniBuffer()->Set(_("Done"));
                // Insert all errors as errors boxes
                insertErrors(terr);
                
                // Literate files should also be kept dirty if the literate 
                // command run ends up with errors.
        } else {
                //no errors or any other things to think about so:
                users->getOwner()->getMiniBuffer()->Set(_("Done"));
                markNwClean();
        }
 
        // if we removed error insets before we ran Literate/Build or if we inserted
        // error insets after we ran Literate/Build this must be run:
	if (a || (res & Literate::ERRORS)){
		users->redraw();
		users->fitCursor();
		users->updateScrollbar();
	}
	AllowInput();

	return literate.getNumErrors();
}


// This should be enabled when the Chktex class is implemented. (Asger)
// chktex should be run with these flags disabled: 3, 22, 25, 30, 38(?)
// Other flags: -wall -v0 -x
int Buffer::runChktex()
{
	if (!text) return 0;

	ProhibitInput();

	// get LaTeX-Filename
	string name = SpaceLess(ChangeExtension (filename, ".tex", true));
	string path = OnlyPath(filename);

	string org_path = path;
	if (lyxrc->use_tempdir || (IsDirWriteable(path) < 1)) {
		path = tmppath;	 
	}

	Path p(path); // path to LaTeX file
	users->getOwner()->getMiniBuffer()->Set(_("Running chktex..."));

	// Remove all error insets
	bool a = removeAutoInsets();

	// Generate the LaTeX file if neccessary
	if (!isDviClean() || a) {
		makeLaTeXFile(name, org_path, false);
		markDviDirty();
	}

	TeXErrors terr;
	Chktex chktex(lyxrc->chktex_command, name, filepath);
	int res = chktex.run(terr); // run chktex

	if (res == -1) {
		WriteAlert(_("chktex did not work!"),
			   _("Could not run with file:"), name);
	} else if (res > 0) {
		// Insert all errors as errors boxes
		insertErrors(terr);
	}

	// if we removed error insets before we ran chktex or if we inserted
	// error insets after we ran chktex, this must be run:
	if (a || res){
		users->redraw();
		users->fitCursor();
		users->updateScrollbar();
	}
	AllowInput();

	return res;
}


extern void AllFloats(char, char);


void Buffer::insertErrors(TeXErrors & terr)
{
	// Save the cursor position
	LyXCursor cursor = text->cursor;

	// This is drastic, but it's the only fix, I could find. (Asger)
	AllFloats(1,0);
	AllFloats(1,1);

	for (TeXErrors::Errors::const_iterator cit = terr.begin();
	     cit != terr.end();
	     ++cit) {
		string desctext((*cit).error_desc);
		string errortext((*cit).error_text);
		string msgtxt = desctext + '\n' + errortext;
		int errorrow = (*cit).error_in_line;

		// Insert error string for row number
		int tmpid = -1; 
		int tmppos = -1;

		texrow.getIdFromRow(errorrow, tmpid, tmppos);

		LyXParagraph* texrowpar;

		if (tmpid == -1) {
			texrowpar = text->FirstParagraph();
			tmppos = 0;
		} else {
			texrowpar = text->GetParFromID(tmpid);
		}

		if (texrowpar == 0)
			continue;

		InsetError *new_inset = new InsetError(msgtxt);

		text->SetCursorIntern(texrowpar, tmppos);
		text->InsertInset(new_inset);
		text->FullRebreak();
	}
	// Restore the cursor position
	text->SetCursorIntern(cursor.par, cursor.pos);
}


void Buffer::setCursorFromRow (int row)
{
	int tmpid = -1; 
	int tmppos = -1;

	texrow.getIdFromRow(row, tmpid, tmppos);

	LyXParagraph* texrowpar;

	if (tmpid == -1) {
		texrowpar = text->FirstParagraph();
		tmppos = 0;
	} else {
		texrowpar = text->GetParFromID(tmpid);
	}
	text->SetCursor(texrowpar, tmppos);
}


void Buffer::RoffAsciiTable(FILE * file, LyXParagraph * par)
{
	LyXFont
		font1 =  LyXFont(LyXFont::ALL_INHERIT),
		font2;
	Inset * inset;
#ifdef NEW_TEXT
	LyXParagraph::size_type i;
#else
	int i;
#endif
	int
		j,
		cell = 0;
	char
		c;
	FILE
		* fp, * fp2;
	
	string fname1 = TmpFileName(string(),"RAT1");
	string fname2 = TmpFileName(string(),"RAT2");
	if (!(fp=fopen(fname1.c_str(),"w"))) {
		WriteAlert(_("LYX_ERROR:"),
			   _("Cannot open temporary file:"), fname1);
		return;
	}
	par->table->RoffEndOfCell(fp, -1);
#ifdef NEW_TEXT
	for (i = 0; i < par->size(); ++i) {
#else
	for (i = 0; i < par->last; ++i) {
#endif
		c = par->GetChar(i);
		if (par->table->IsContRow(cell)) {
			if (c == LYX_META_NEWLINE)
				cell++;
			continue;
		}
		font2 = par->GetFontSettings(i);
		if (font1.latex() != font2.latex()) {
			if (font2.latex() != LyXFont::OFF)
				continue;
		}
		switch (c) {
		case LYX_META_INSET:
			if ((inset = par->GetInset(i))) {
				if (!(fp2=fopen(fname2.c_str(),"w+"))) {
					WriteAlert(_("LYX_ERROR:"),
						   _("Cannot open temporary file:"), fname2);
					fclose(fp);
					remove(fname1.c_str());
					return;
				}
				inset->Latex(fp2,-1);
				rewind(fp2);
				c = fgetc(fp2);
				while(!feof(fp2)) {
					if (c == '\\')
						fprintf(fp,"\\\\");
					else
						fputc(c,fp);
					c = fgetc(fp2);
				}
				fclose(fp2);
			}
			break;
		case LYX_META_NEWLINE:
			if (par->table->CellHasContRow(cell)>=0)
				par->RoffContTableRows(fp, i+1,cell);
			par->table->RoffEndOfCell(fp, cell);
			cell++;
			break;
		case LYX_META_HFILL: 
			break;
		case LYX_META_PROTECTED_SEPARATOR:
			break;
		case '\\': 
			fprintf(fp, "\\\\");
			break;
		default:
			if (c != '\0')
				fprintf(fp, "%c", c);
			else if (c == '\0')
				lyxerr.debug()
					<< "RoffAsciiTable:"
					" NULL char in structure." << endl;
			break;
		}
	}
	par->table->RoffEndOfCell(fp, cell);
	fclose(fp);
	string cmd = lyxrc->ascii_roff_command + " >" + fname2;
	cmd = subst(cmd, "$$FName", fname1);
	Systemcalls one(Systemcalls::System, cmd);
	if (!(lyxerr.debugging(Debug::ROFF))) {
		remove(fname1.c_str());
	}
	if (!(fp=fopen(fname2.c_str(),"r"))) {
		WriteFSAlert(_("Error! Can't open temporary file:"), fname2);
		return;
	}
	// now output the produced file
	fprintf(file, "\n\n");
	c = fgetc(fp);
	if (feof(fp))
		WriteAlert(_("Error!"),
			   _("Error executing *roff command on table"));
	// overread leading blank lines
	while(!feof(fp) && (c == '\n'))
		c = fgetc(fp);
	while(!feof(fp)) {
		for(j=0; j<par->depth; j++)
			fprintf(file, "  ");
		while(!feof(fp) && (c != '\n')) {
			fputc(c,file);
			c = fgetc(fp);
		}
		fputc('\n',file);
		// overread trailing blank lines
		while(!feof(fp) && (c == '\n'))
			c = fgetc(fp);
	}
	fclose(fp);
	remove(fname2.c_str());
}

	
/// changed Heinrich Bauer, 23/03/98
bool Buffer::isDviClean()
{
  if (lyxrc->use_tempdir)
    return dvi_clean_tmpd;
  else
    return dvi_clean_orgd;
}

 
/// changed Heinrich Bauer, 23/03/98
void Buffer::markDviClean()
{
  if (lyxrc->use_tempdir)
    dvi_clean_tmpd = true;
  else
    dvi_clean_orgd = true;
}


/// changed Heinrich Bauer, 23/03/98
void Buffer::markDviDirty()
{
  if (lyxrc->use_tempdir)
    dvi_clean_tmpd = false;
  else
    dvi_clean_orgd = false;
}


void Buffer::update(signed char f)
{
	if (!users) return;
	
	users->getOwner()->updateLayoutChoice();

	if (!text->selection && f > -3)
		text->sel_cursor = text->cursor;
	
	FreeUpdateTimer();
	text->FullRebreak();
	users->update();

	if (f != 3 && f != -3) {
		users->fitCursor();
		users->updateScrollbar();
      	}

	if (f==1 || f==-1) {
		if (isLyxClean()) {
			markDirty();
			users->getOwner()->getMiniBuffer()->setTimer(4);
		} else {
			markDirty();
		}
	}
}


void Buffer::validate(LaTeXFeatures & features)
{
	LyXParagraph * par = paragraph;
        LyXTextClass const & tclass =
		textclasslist.TextClass(params.textclass);
    
        // AMS Style is at document level
    
        features.amsstyle = (params.use_amsmath ||
			     tclass.provides(LyXTextClass::amsmath));
    
	while (par) {
		// We don't use "lyxerr.debug" because of speed. (Asger)
		if (lyxerr.debugging(Debug::LATEX))
			lyxerr << "Paragraph: " <<  par << endl;

		// Now just follow the list of paragraphs and run
		// validate on each of them.
		par->validate(features);

		// and then the next paragraph
		par = par->next;
	}

	// the bullet shapes are buffer level not paragraph level
	// so they are tested here
	for (int i = 0; i < 4; ++i) {
		if (params.user_defined_bullets[i] != ITEMIZE_DEFAULTS[i]) {
			int font = params.user_defined_bullets[i].getFont();
			if (font == 0) {
				int c = params
					.user_defined_bullets[i]
					.getCharacter();
				if (c == 16
				   || c == 17
				   || c == 25
				   || c == 26
				   || c == 31) {
					features.latexsym = true;
				}
			}
			if (font == 1) {
				features.amssymb = true;
			}
			else if ((font >= 2 && font <=5)) {
				features.pifont = true;
			}
		}
	}
	
	if (lyxerr.debugging(Debug::LATEX)) {
		features.showStruct(params);
	}
}


void Buffer::setPaperStuff()
{
	params.papersize = PAPER_DEFAULT;
	char c1 = params.paperpackage;
	if (c1 == PACKAGE_NONE) {
		char c2 = params.papersize2;
		if (c2 == VM_PAPER_USLETTER)
			params.papersize = PAPER_USLETTER;
		else if (c2 == VM_PAPER_USLEGAL)
			params.papersize = PAPER_LEGALPAPER;
		else if (c2 == VM_PAPER_USEXECUTIVE)
			params.papersize = PAPER_EXECUTIVEPAPER;
		else if (c2 == VM_PAPER_A3)
			params.papersize = PAPER_A3PAPER;
		else if (c2 == VM_PAPER_A4)
			params.papersize = PAPER_A4PAPER;
		else if (c2 == VM_PAPER_A5)
			params.papersize = PAPER_A5PAPER;
		else if ((c2 == VM_PAPER_B3) || (c2 == VM_PAPER_B4) ||
			 (c2 == VM_PAPER_B5))
			params.papersize = PAPER_B5PAPER;
	} else if ((c1 == PACKAGE_A4) || (c1 == PACKAGE_A4WIDE) ||
		   (c1 == PACKAGE_WIDEMARGINSA4))
		params.papersize = PAPER_A4PAPER;
}


void Buffer::setOldPaperStuff()
{
	char c = params.papersize = params.papersize2;
	params.papersize2 = VM_PAPER_DEFAULT;
	params.paperpackage = PACKAGE_NONE;
	if (c == OLD_PAPER_A4PAPER)
		params.papersize2 = VM_PAPER_A4;
	else if (c == OLD_PAPER_A4)
		params.paperpackage = PACKAGE_A4;
	else if (c == OLD_PAPER_A4WIDE)
		params.paperpackage = PACKAGE_A4WIDE;
	else if (c == OLD_PAPER_WIDEMARGINSA4)
		params.paperpackage = PACKAGE_WIDEMARGINSA4;
	else if (c == OLD_PAPER_USLETTER)
		params.papersize2 = VM_PAPER_USLETTER;
	else if (c == OLD_PAPER_A5PAPER)
		params.papersize2 = VM_PAPER_A5;
	else if (c == OLD_PAPER_B5PAPER)
		params.papersize2 = VM_PAPER_B5;
	else if (c == OLD_PAPER_EXECUTIVEPAPER)
		params.papersize2 = VM_PAPER_USEXECUTIVE;
	else if (c == OLD_PAPER_LEGALPAPER)
		params.papersize2 = VM_PAPER_USLEGAL;
	setPaperStuff();
}


void Buffer::insertInset(Inset * inset, string const & lout,
			 bool no_table)
{
	// check for table/list in tables
	if (no_table && text->cursor.par->table){
		WriteAlert(_("Impossible Operation!"),
			   _("Cannot insert table/list in table."),
			   _("Sorry."));
		return;
	}
	// not quite sure if we want this...
	text->SetCursorParUndo();
	text->FreezeUndo();
	
	BeforeChange();
	if (!lout.empty()) {
		update(-2);
		text->BreakParagraph();
		update(-1);
		
		if (text->cursor.par->Last()) {
			text->CursorLeft();
			
			text->BreakParagraph();
			update(-1);
		}

		int lay = textclasslist.NumberOfLayout(params.textclass,
						       lout).second;
		if (lay == -1) // layout not found
			// use default layout "Standard" (0)
			lay = 0;
		
		text->SetLayout(lay);
		
		text->SetParagraph(0, 0,
				   0, 0,
				   VSpace(VSpace::NONE), VSpace(VSpace::NONE),
				   LYX_ALIGN_LAYOUT, 
				   string(),
				   0);
		update(-1);
		
		text->current_font.setLatex(LyXFont::OFF);
	}
	
	text->InsertInset(inset);
	update(-1);

	text->UnFreezeUndo();	
}


// Open and lock an updatable inset
void Buffer::open_new_inset(UpdatableInset * new_inset)
{
	BeforeChange();
	text->FinishUndo();
	insertInset(new_inset);
	text->CursorLeft();
	update(1);
    	new_inset->Edit(0,0);
}


/* This function should be in Buffer because it's a buffer's property (ale) */
string Buffer::getIncludeonlyList(char delim)
{
	string lst;
	LyXParagraph * par = paragraph;
#ifdef NEW_TEXT
	LyXParagraph::size_type pos;
#else
	int pos;
#endif
	Inset * inset;
	while (par){
		pos = -1;
		while ((inset = par->ReturnNextInsetPointer(pos))){
			if (inset->LyxCode()==Inset::INCLUDE_CODE) {
				InsetInclude * insetinc =
					static_cast<InsetInclude*>(inset);
				if (insetinc->isInclude() 
				    && insetinc->isNoLoad()) {
					if (!lst.empty())
						lst += delim;
					lst += ChangeExtension(insetinc->getContents(), string(), true);
				}
			}
			pos++;
		} 
		par = par->next;
	}
	lyxerr.debug() << "Includeonly(" << lst << ')' << endl;
	return lst;
}


/* This is also a buffer property (ale) */ 
string Buffer::getReferenceList(char delim)
{
	/// if this is a child document and the parent is already loaded
	/// Use the parent's list instead  [ale990407]
	if (!params.parentname.empty() && bufferlist.exists(params.parentname)) {
		Buffer *tmp = bufferlist.getBuffer(params.parentname);
		if (tmp)
		  return tmp->getReferenceList(delim);
	}

	LyXParagraph *par = paragraph;
#ifdef NEW_TEXT
	LyXParagraph::size_type pos;
#else
	int pos;
#endif
	Inset * inset;
        string lst;
	while (par){
		pos = -1;
		while ((inset = par->ReturnNextInsetPointer(pos))){     
			for (int i = 0; i < inset->GetNumberOfLabels(); i++) {
				if (!lst.empty())
					lst += delim;
				lst += inset->getLabel(i);
			}
			pos++;
		} 
		par = par->next;
	}
	lyxerr.debug() << "References(" <<  lst << ")" << endl;
	return lst;
}


/* This is also a buffer property (ale) */ 
string Buffer::getBibkeyList(char delim)
{
	/// if this is a child document and the parent is already loaded
	/// Use the parent's list instead  [ale990412]
        if (!params.parentname.empty() && bufferlist.exists(params.parentname)) {
		Buffer *tmp = bufferlist.getBuffer(params.parentname);
		if (tmp)
			return tmp->getBibkeyList(delim);
	}

	string bibkeys;
	LyXParagraph * par = paragraph;
	while (par) {
		if (par->bibkey) {
			if (!bibkeys.empty())
				bibkeys += delim;
			bibkeys += par->bibkey->getContents();
		}
		par = par->next;
	}

	// Might be either using bibtex or a child has bibliography
	if (bibkeys.empty()) {
		par = paragraph;
		while (par) {
			Inset * inset;
#ifdef NEW_TEXT
			LyXParagraph::size_type pos = -1;
#else
			int pos = -1;
#endif

			// Search for Bibtex or Include inset
			while ((inset = par->ReturnNextInsetPointer(pos))) {
				if (inset-> LyxCode()==Inset::BIBTEX_CODE) {
					if (!bibkeys.empty())
						bibkeys += delim;
					bibkeys += ((InsetBibtex*)inset)->getKeys();
				} else if (inset-> LyxCode()==Inset::INCLUDE_CODE) {
					string bk = ((InsetInclude*)inset)->getKeys();
					if (!bk.empty()) {
						if (!bibkeys.empty())
							bibkeys += delim;
						bibkeys += bk;
					}
				}
				pos++;
			}
			par = par->next;
		}
	}
 
	lyxerr.debug() << "Bibkeys(" << bibkeys << ")" << endl;
	return bibkeys;
}


/* This is also a buffer property (ale) */
// Not so sure about that. a goto Label function can not be buffer local, just
// think how this will work in a multiwindo/buffer environment, all the
// cursors in all the views showing this buffer will move. (Lgb)
// OK, then no cursor action should be allowed in buffer. (ale)
bool Buffer::gotoLabel(string const & label)

{
        LyXParagraph * par = paragraph;
#ifdef NEW_TEXT
        LyXParagraph::size_type pos;
#else
        int pos;
#endif
        Inset * inset;
        while (par) {
                pos = -1;
                while ((inset = par->ReturnNextInsetPointer(pos))){     
                        for (int i = 0; i < inset->GetNumberOfLabels(); i++) {
				if (label==inset->getLabel(i)) {
					BeforeChange();
					text->SetCursor(par, pos);
					text->sel_cursor = text->cursor;
					update(0);
					return true;
				}
			}
                        pos++;
                } 
                par = par->next;
	}
	return false;
}


bool Buffer::isDepClean(string const & name) const
{
	DEPCLEAN * item = dep_clean;
	while (item && item->master != name)
		item = item->next;
	if (!item) return true;
	return item->clean;
}


void Buffer::markDepClean(string const & name)
{
	if (!dep_clean) {
		dep_clean = new DEPCLEAN;
		dep_clean->clean = true;
		dep_clean->master = name;
		dep_clean->next = 0;
	} else {
		DEPCLEAN* item = dep_clean;
		while (item && item->master != name)
			item = item->next;
		if (item) {
			item->clean = true;
		} else {
			item = new DEPCLEAN;
			item->clean = true;
			item->master = name;
			item->next = 0;;
		}
	}
}
