/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *
 *	     Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 *           This file is Copyright 1996-1999
 *           Lars Gullik Bjønnes
 *
 * ====================================================== 
 */

#include <config.h>

#include <fstream>
#include <iomanip>

#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <utime.h>

#include <algorithm>

#ifdef __GNUG__
#pragma implementation "buffer.h"
#endif

#include "buffer.h"
#include "bufferlist.h"
#include "lyx_main.h"
#include "lyx_gui_misc.h"
#include "LyXAction.h"
#include "lyxrc.h"
#include "lyxlex.h"
#include "tex-strings.h"
#include "layout.h"
#include "bufferview_funcs.h"
#include "minibuffer.h"
#include "lyxfont.h"
#include "version.h"
#include "mathed/formulamacro.h"
#include "insets/lyxinset.h"
#include "insets/inseterror.h"
#include "insets/insetlabel.h"
#include "insets/insetref.h"
#include "insets/inseturl.h"
#include "insets/insetinfo.h"
#include "insets/insetquotes.h"
#include "insets/insetlatexaccent.h"
#include "insets/insetbib.h" 
#include "insets/insetcite.h" 
#include "insets/insetexternal.h"
#include "insets/insetindex.h" 
#include "insets/insetinclude.h"
#include "insets/insettoc.h"
#include "insets/insetlof.h"
#include "insets/insetlot.h"
#include "insets/insetloa.h"
#include "insets/insetparent.h"
#include "insets/insetspecialchar.h"
#include "insets/figinset.h"
#include "insets/insettext.h"
#include "insets/insetert.h"
#include "insets/insetgraphics.h"
#include "insets/insetfoot.h"
#include "insets/insetmarginal.h"
#include "insets/insetminipage.h"
#include "insets/insetfloat.h"
#include "insets/insetlist.h"
#include "insets/insettabular.h"
#include "insets/insettheorem.h"
#include "insets/insetcaption.h"
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
#include "language.h"
#include "lyx_gui_misc.h"	// WarnReadonly()
#include "frontends/Dialogs.h"
#include "encoding.h"

using std::ostream;
using std::ofstream;
using std::ifstream;
using std::fstream;
using std::ios;
using std::setw;
using std::endl;
using std::pair;
using std::vector;
using std::max;
using std::set;
#ifdef HAVE_SSTREAM
using std::istringstream;
#endif

// all these externs should eventually be removed.
extern BufferList bufferlist;

extern void MenuExport(Buffer *, string const &);
extern LyXAction lyxaction;


static const float LYX_FORMAT = 2.16;

extern int tex_code_break_column;


Buffer::Buffer(string const & file, bool ronly)
{
	lyxerr[Debug::INFO] << "Buffer::Buffer()" << endl;
	filename = file;
	filepath = OnlyPath(file);
	paragraph = 0;
	lyx_clean = true;
	bak_clean = true;
	dep_clean = 0;
	read_only = ronly;
	unnamed = false;
	users = 0;
	lyxvc.buffer(this);
	if (read_only || (lyxrc.use_tempdir)) {
		tmppath = CreateBufferTmpDir();
	} else tmppath.erase();
}


Buffer::~Buffer()
{
	lyxerr[Debug::INFO] << "Buffer::~Buffer()" << endl;
	// here the buffer should take care that it is
	// saved properly, before it goes into the void.

	// make sure that views using this buffer
	// forgets it.
	if (users)
		users->buffer(0);
	
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
}


string Buffer::getLatexName(bool no_path) const
{
	if (no_path)
		return OnlyFilename(ChangeExtension(MakeLatexName(filename), 
						    ".tex"));
	else
		return ChangeExtension(MakeLatexName(filename), 
				       ".tex"); 
}


void Buffer::setReadonly(bool flag)
{
	if (read_only != flag) {
		read_only = flag; 
		updateTitles();
		users->owner()->getDialogs()->updateBufferDependent();
	}
	if (read_only) {
		WarnReadonly(filename);
	}
}


bool Buffer::saveParamsAsDefaults()
{
	string fname = AddName(AddPath(user_lyxdir, "templates/"),
			       "defaults.lyx");
	Buffer defaults = Buffer(fname);
	
	// Use the current buffer's parameters as default
	defaults.params = params;
	
	// add an empty paragraph. Is this enough?
	defaults.paragraph = new LyXParagraph;

	return defaults.writeFile(defaults.filename, false);
}


/// Update window titles of all users
// Should work on a list
void Buffer::updateTitles() const
{
	if (users) users->owner()->updateWindowTitle();
}


/// Reset autosave timer of all users
// Should work on a list
void Buffer::resetAutosaveTimers() const
{
	if (users) users->owner()->resetAutosaveTimer();
}


void Buffer::fileName(string const & newfile)
{
	filename = MakeAbsPath(newfile);
	filepath = OnlyPath(filename);
	setReadonly(IsFileWriteable(filename) == 0);
	updateTitles();
}


// candidate for move to BufferView
// (at least some parts in the beginning of the func)
//
// Uwe C. Schroeder
// changed to be public and have one parameter
// if par = 0 normal behavior
// else insert behavior
// Returns false if "\the_end" is not read for formats >= 2.13. (Asger)
bool Buffer::readLyXformat2(LyXLex & lex, LyXParagraph * par)
{
	string tmptok;
	int pos = 0;
	char depth = 0; // signed or unsigned?
#ifndef NEW_INSETS
	LyXParagraph::footnote_flag footnoteflag = LyXParagraph::NO_FOOTNOTE;
	LyXParagraph::footnote_kind footnotekind = LyXParagraph::FOOTNOTE;
#endif
	bool the_end_read = false;

	LyXParagraph * return_par = 0;
	LyXFont font(LyXFont::ALL_INHERIT, params.language_info);
	if (format < 2.16 && params.language == "hebrew")
		font.setLanguage(default_language);

	// If we are inserting, we cheat and get a token in advance
	bool has_token = false;
	string pretoken;

	if(!par) {
		par = new LyXParagraph;
	} else {
		users->text->BreakParagraph(users);
		return_par = users->text->FirstParagraph();
		pos = 0;
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
		the_end_read = parseSingleLyXformat2Token(lex, par, return_par,
							  token, pos, depth,
							  font
#ifndef NEW_INSETS
							  , footnoteflag,
							  footnotekind
#endif
			);
	}
   
	if (!return_par)
		return_par = par;

	paragraph = return_par;
	
	return the_end_read;
}


// We'll remove this later. (Lgb)
static string last_inset_read;


bool
Buffer::parseSingleLyXformat2Token(LyXLex & lex, LyXParagraph *& par,
				   LyXParagraph *& return_par,
				   string const & token, int & pos,
				   char & depth, LyXFont & font
#ifndef NEW_INSETS
				   , LyXParagraph::footnote_flag & footnoteflag,
				   LyXParagraph::footnote_kind & footnotekind
#endif
	)
{
	bool the_end_read = false;
	
	if (token[0] != '\\') {
		for (string::const_iterator cit = token.begin();
		     cit != token.end(); ++cit) {
			par->InsertChar(pos, (*cit), font);
			++pos;
		}
	} else if (token == "\\i") {
		Inset * inset = new InsetLatexAccent;
		inset->Read(this, lex);
		par->InsertInset(pos, inset, font);
		++pos;
	} else if (token == "\\layout") {
		if (!return_par) 
			return_par = par;
		else {
			par->fitToSize();
			par = new LyXParagraph(par);
		}
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
		LyXLayout const & layout =
			textclasslist.Style(params.textclass,
					    par->layout); 
		if (!layout.obsoleted_by().empty())
			par->layout = 
				textclasslist.NumberOfLayout(params.textclass, 
							     layout.obsoleted_by()).second;
#ifndef NEW_INSETS
		par->footnoteflag = footnoteflag;
		par->footnotekind = footnotekind;
#endif
		par->depth = depth;
		font = LyXFont(LyXFont::ALL_INHERIT, params.language_info);
		if (format < 2.16 && params.language == "hebrew")
			font.setLanguage(default_language);
#ifndef NEW_INSETS
	} else if (token == "\\end_float") {
		if (!return_par) 
			return_par = par;
		else {
			par->fitToSize();
			par = new LyXParagraph(par);
		}
		footnotekind = LyXParagraph::FOOTNOTE;
		footnoteflag = LyXParagraph::NO_FOOTNOTE;
		pos = 0;
		lex.EatLine();
		par->layout = LYX_DUMMY_LAYOUT;
		font = LyXFont(LyXFont::ALL_INHERIT, params.language_info);
		if (format < 2.16 && params.language == "hebrew")
			font.setLanguage(default_language);
	} else if (token == "\\begin_float") {
		int tmpret = lex.FindToken(string_footnotekinds);
		if (tmpret == -1) ++tmpret;
		if (tmpret != LYX_LAYOUT_DEFAULT) 
			footnotekind = static_cast<LyXParagraph::footnote_kind>(tmpret); // bad
		if (footnotekind == LyXParagraph::FOOTNOTE
		    || footnotekind == LyXParagraph::MARGIN)
			footnoteflag = LyXParagraph::CLOSED_FOOTNOTE;
		else 
			footnoteflag = LyXParagraph::OPEN_FOOTNOTE;
#else
	} else if (token == "\\begin_float") {
		// This is the compability reader, unfinished but tested.
		// (Lgb)
		lex.next();
		string tmptok = lex.GetString();
		//lyxerr << "old float: " << tmptok << endl;
		
		Inset * inset = 0;
		string old_float;
		
		if (tmptok == "footnote") {
			inset = new InsetFoot;
		} else if (tmptok == "margin") {
			inset = new InsetMarginal;
		} else if (tmptok == "fig") {
			inset = new InsetFloat("figure");
			old_float += "placement htbp\n";
		} else if (tmptok == "tab") {
			inset = new InsetFloat("table");
			old_float += "placement htbp\n";
		} else if (tmptok == "alg") {
			inset = new InsetFloat("algorithm");
			old_float += "placement htbp\n";
		} else if (tmptok == "wide-fig") {
			InsetFloat * tmp = new InsetFloat("figure");
			tmp->wide(true);
			inset = tmp;
			old_float += "placement htbp\n";
		} else if (tmptok == "wide-tab") {
			InsetFloat * tmp = new InsetFloat("table");
			tmp->wide(true);
			inset = tmp;
			old_float += "placement htbp\n";
		}

		if (!inset) return false; // no end read yet
		
		old_float += "collapsed true\n";

		// Here we need to check for \end_deeper and handle that
		// before we do the footnote parsing.
		// This _is_ a hack! (Lgb)
		while(true) {
			lex.next();
			string tmp = lex.GetString();
			if (tmp == "\\end_deeper") {
				lyxerr << "\\end_deeper caught!" << endl;
				if (!depth) {
					lex.printError("\\end_deeper: "
						       "depth is already null");
				} else
					--depth;
				
			} else {
				old_float += tmp;
				old_float += ' ';
				break;
			}
		}
		
		old_float += lex.getLongString("\\end_float");
		old_float += "\n\\end_inset\n";
		//lyxerr << "float body: " << old_float << endl;

#ifdef HAVE_SSTREAM
		istringstream istr(old_float.c_str());
#else
		istrstream istr(old_float.c_str());
#endif
		
		LyXLex nylex(0, 0);
		nylex.setStream(istr);
		
		inset->Read(this, nylex);
		par->InsertInset(pos, inset, font);
		++pos;
#endif
	} else if (token == "\\begin_deeper") {
		++depth;
	} else if (token == "\\end_deeper") {
		if (!depth) {
			lex.printError("\\end_deeper: "
				       "depth is already null");
		}
		else
			--depth;
	} else if (token == "\\begin_preamble") {
		params.readPreamble(lex);
	} else if (token == "\\textclass") {
		lex.EatLine();
		pair<bool, LyXTextClassList::size_type> pp = 
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
		int tmpret = lex.FindToken(string_paragraph_separation);
		if (tmpret == -1) ++tmpret;
		if (tmpret != LYX_LAYOUT_DEFAULT) 
			params.paragraph_separation =
				static_cast<BufferParams::PARSEP>(tmpret);
	} else if (token == "\\defskip") {
		lex.nextToken();
		params.defskip = VSpace(lex.GetString());
	} else if (token == "\\epsfig") { // obsolete
		// Indeed it is obsolete, but we HAVE to be backwards
		// compatible until 0.14, because otherwise all figures
		// in existing documents are irretrivably lost. (Asger)
		params.readGraphicsDriver(lex);
	} else if (token == "\\quotes_language") {
		int tmpret = lex.FindToken(string_quotes_language);
		if (tmpret == -1) ++tmpret;
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
		int tmpret = lex.FindToken(string_papersize);
		if (tmpret == -1)
			++tmpret;
		else
			params.papersize2 = tmpret;
	} else if (token == "\\paperpackage") {
		int tmpret = lex.FindToken(string_paperpackages);
		if (tmpret == -1) {
			++tmpret;
			params.paperpackage = BufferParams::PACKAGE_NONE;
		} else
			params.paperpackage = tmpret;
	} else if (token == "\\use_geometry") {
		lex.nextToken();
		params.use_geometry = lex.GetInteger();
	} else if (token == "\\use_amsmath") {
		lex.nextToken();
		params.use_amsmath = lex.GetInteger();
	} else if (token == "\\paperorientation") {
		int tmpret = lex.FindToken(string_orientation);
		if (tmpret == -1) ++tmpret;
		if (tmpret != LYX_LAYOUT_DEFAULT) 
			params.orientation = static_cast<BufferParams::PAPER_ORIENTATION>(tmpret);
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
	} else if (token == "\\spacing") {
		lex.next();
		string tmp = strip(lex.GetString());
		Spacing::Space tmp_space = Spacing::Default;
		float tmp_val = 0.0;
		if (tmp == "single") {
			tmp_space = Spacing::Single;
		} else if (tmp == "onehalf") {
			tmp_space = Spacing::Onehalf;
		} else if (tmp == "double") {
			tmp_space = Spacing::Double;
		} else if (tmp == "other") {
			lex.next();
			tmp_space = Spacing::Other;
			tmp_val = lex.GetFloat();
		} else {
			lex.printError("Unknown spacing token: '$$Token'");
		}
		// Small hack so that files written with klyx will be
		// parsed correctly.
		if (return_par) {
			par->spacing.set(tmp_space, tmp_val);
		} else {
			params.spacing.set(tmp_space, tmp_val);
		}
	} else if (token == "\\paragraph_spacing") {
		lex.next();
		string tmp = strip(lex.GetString());
		if (tmp == "single") {
			par->spacing.set(Spacing::Single);
		} else if (tmp == "onehalf") {
			par->spacing.set(Spacing::Onehalf);
		} else if (tmp == "double") {
			par->spacing.set(Spacing::Double);
		} else if (tmp == "other") {
			lex.next();
			par->spacing.set(Spacing::Other,
					 lex.GetFloat());
		} else {
			lex.printError("Unknown spacing token: '$$Token'");
		}
	} else if (token == "\\float_placement") {
		lex.nextToken();
		params.float_placement = lex.GetString();
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
	} else if (token == "\\lang") {
		lex.next();
		string tok = lex.GetString();
		Languages::iterator lit = languages.find(tok);
		if (lit != languages.end()) {
			font.setLanguage(&(*lit).second);
		} else {
			font.setLanguage(params.language_info);
			lex.printError("Unknown language `$$Token'");
		}
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
		int tmpret = lex.FindToken(string_align);
		if (tmpret == -1) ++tmpret;
		if (tmpret != LYX_LAYOUT_DEFAULT) { // tmpret != 99 ???
			int tmpret2 = 1;
			for (; tmpret > 0; --tmpret)
				tmpret2 = tmpret2 * 2;
			par->align = LyXAlignment(tmpret2);
		}
	} else if (token == "\\added_space_top") {
		lex.nextToken();
		par->added_space_top = VSpace(lex.GetString());
	} else if (token == "\\added_space_bottom") {
		lex.nextToken();
		par->added_space_bottom = VSpace(lex.GetString());
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
		// do not delete this token, it is still needed!
	} else if (token == "\\end_inset") {
		lyxerr << "Solitary \\end_inset. Missing \\begin_inset?.\n"
		       << "Last inset read was: " << last_inset_read
		       << endl;
		// Simply ignore this. The insets do not have
		// to read this.
		// But insets should read it, it is a part of
		// the inset isn't it? Lgb.
	} else if (token == "\\begin_inset") {
#if 1
		readInset(lex, par, pos, font);
#else
		// Should be moved out into its own function/method. (Lgb)
		lex.next();
		string tmptok = lex.GetString();
		last_inset_read = tmptok;
		// test the different insets
		if (tmptok == "Quotes") {
			Inset * inset = new InsetQuotes;
			inset->Read(this, lex);
			par->InsertInset(pos, inset, font);
			++pos;
		} else if (tmptok == "External") {
			Inset * inset = new InsetExternal;
			inset->Read(this, lex);
			par->InsertInset(pos, inset, font);
			++pos;
		} else if (tmptok == "FormulaMacro") {
			Inset * inset = new InsetFormulaMacro;
			inset->Read(this, lex);
			par->InsertInset(pos, inset, font);
			++pos;
		} else if (tmptok == "Formula") {
			Inset * inset = new InsetFormula;
			inset->Read(this, lex);
			par->InsertInset(pos, inset, font);
			++pos;
		} else if (tmptok == "Figure") {
			Inset * inset = new InsetFig(100, 100, this);
			inset->Read(this, lex);
			par->InsertInset(pos, inset, font);
			++pos;
		} else if (tmptok == "Info") {
			Inset * inset = new InsetInfo;
			inset->Read(this, lex);
			par->InsertInset(pos, inset, font);
			++pos;
		} else if (tmptok == "Include") {
			Inset * inset = new InsetInclude(string(), this);
			inset->Read(this, lex);
			par->InsertInset(pos, inset, font);
			++pos;
		} else if (tmptok == "ERT") {
			Inset * inset = new InsetERT;
			inset->Read(this, lex);
			par->InsertInset(pos, inset, font);
			++pos;
		} else if (tmptok == "Tabular") {
			Inset * inset = new InsetTabular(this);
			inset->Read(this, lex);
			par->InsertInset(pos, inset, font);
			++pos;
		} else if (tmptok == "Text") {
			Inset * inset = new InsetText;
			inset->Read(this, lex);
			par->InsertInset(pos, inset, font);
			++pos;
		} else if (tmptok == "Foot") {
			Inset * inset = new InsetFoot;
			inset->Read(this, lex);
			par->InsertInset(pos, inset, font);
			++pos;
		} else if (tmptok == "Marginal") {
			Inset * inset = new InsetMarginal;
			inset->Read(this, lex);
			par->InsertInset(pos, inset, font);
			++pos;
		} else if (tmptok == "Minipage") {
			Inset * inset = new InsetMinipage;
			inset->Read(this, lex);
			par->InsertInset(pos, inset, font);
			++pos;
		} else if (tmptok == "Float") {
			lex.next();
			string tmptok = lex.GetString();
			Inset * inset = new InsetFloat(tmptok);
			inset->Read(this, lex);
			par->InsertInset(pos, inset, font);
			++pos;
		} else if (tmptok == "List") {
			Inset * inset = new InsetList;
			inset->Read(this, lex);
			par->InsertInset(pos, inset, font);
			++pos;
		} else if (tmptok == "Theorem") {
			Inset * inset = new InsetList;
			inset->Read(this, lex);
			par->InsertInset(pos, inset, font);
			++pos;
		} else if (tmptok == "Caption") {
			Inset * inset = new InsetCaption;
			inset->Read(this, lex);
			par->InsertInset(pos, inset, font);
			++pos;
		} else if (tmptok == "GRAPHICS") {
			Inset * inset = new InsetGraphics;
				//inset->Read(this, lex);
			par->InsertInset(pos, inset, font);
		} else if (tmptok == "LatexCommand") {
			InsetCommand inscmd;
			inscmd.Read(this, lex);
			Inset * inset = 0;
			if (inscmd.getCmdName() == "cite") {
				inset = new InsetCitation(inscmd.params());
			} else if (inscmd.getCmdName() == "bibitem") {
				lex.printError("Wrong place for bibitem");
				inset = inscmd.Clone();
			} else if (inscmd.getCmdName() == "BibTeX") {
				inset = new InsetBibtex(inscmd.getContents(), inscmd.getOptions(), this);
			} else if (inscmd.getCmdName() == "index") {
				inset = new InsetIndex(inscmd.getContents());
			} else if (inscmd.getCmdName() == "include") {
				inset = new InsetInclude(inscmd.getContents(), this);
			} else if (inscmd.getCmdName() == "label") {
				inset = new InsetLabel(inscmd.getCommand());
			} else if (inscmd.getCmdName() == "url"
				   || inscmd.getCmdName() == "htmlurl") {
				inset = new InsetUrl(inscmd.params());
			} else if (inscmd.getCmdName() == "ref"
				   || inscmd.getCmdName() == "pageref"
				   || inscmd.getCmdName() == "vref"
				   || inscmd.getCmdName() == "vpageref"
				   || inscmd.getCmdName() == "prettyref") {
				if (!inscmd.getOptions().empty() || !inscmd.getContents().empty()) {
					inset = new InsetRef(inscmd, this);
				}
			} else if (inscmd.getCmdName() == "tableofcontents") {
				inset = new InsetTOC(this);
			} else if (inscmd.getCmdName() == "listoffigures") {
				inset = new InsetLOF(this);
			} else if (inscmd.getCmdName() == "listofalgorithms") {
				inset = new InsetLOA(this);
			} else if (inscmd.getCmdName() == "listoftables") {
				inset = new InsetLOT(this);
			} else if (inscmd.getCmdName() == "printindex") {
				inset = new InsetPrintIndex(this);
			} else if (inscmd.getCmdName() == "lyxparent") {
				inset = new InsetParent(inscmd.getContents(), this);
			}
			       
			if (inset) {
				par->InsertInset(pos, inset, font);
				++pos;
			}
		}
#endif
	} else if (token == "\\SpecialChar") {
		LyXLayout const & layout =
			textclasslist.Style(params.textclass, 
					    par->GetLayout());

		// Insets don't make sense in a free-spacing context! ---Kayvan
		if (layout.free_spacing) {
			if (lex.IsOK()) {
				lex.next();
				string next_token = lex.GetString();
				if (next_token == "\\-") {
					par->InsertChar(pos, '-', font);
				} else if (next_token == "\\protected_separator"
					|| next_token == "~") {
					par->InsertChar(pos, ' ', font);
				} else {
					lex.printError("Token `$$Token' "
						       "is in free space "
						       "paragraph layout!");
					--pos;
				}
			}
		} else {
			Inset * inset = new InsetSpecialChar;
			inset->Read(this, lex);
			par->InsertInset(pos, inset, font);
		}
		++pos;
	} else if (token == "\\newline") {
		par->InsertChar(pos, LyXParagraph::META_NEWLINE, font);
		++pos;
	} else if (token == "\\LyXTable") {
#ifdef NEW_TABULAR
		Inset * inset = new InsetTabular(this);
		inset->Read(this, lex);
		par->InsertInset(pos, inset, font);
		++pos;
#else
		par->table = new LyXTable(lex);
#endif
	} else if (token == "\\hfill") {
		par->InsertChar(pos, LyXParagraph::META_HFILL, font);
		++pos;
	} else if (token == "\\protected_separator") { // obsolete
		// This is a backward compability thingie. (Lgb)
		// Remove it later some time...introduced with fileformat
		// 2.16. (Lgb)
		LyXLayout const & layout =
			textclasslist.Style(params.textclass, 
					    par->GetLayout());

		if (layout.free_spacing) {
			par->InsertChar(pos, ' ', font);
		} else {
			Inset * inset = new InsetSpecialChar(InsetSpecialChar::PROTECTED_SEPARATOR);
			par->InsertInset(pos, inset, font);
		}
		++pos;
	} else if (token == "\\bibitem") {  // ale970302
		if (!par->bibkey)
			par->bibkey = new InsetBibKey;
		par->bibkey->Read(this, lex);		        
	}else if (token == "\\backslash") {
		par->InsertChar(pos, '\\', font);
		++pos;
	}else if (token == "\\the_end") {
		the_end_read = true;
	} else {
		// This should be insurance for the future: (Asger)
		lex.printError("Unknown token `$$Token'. "
			       "Inserting as text.");
		for(string::const_iterator cit = token.begin();
		    cit != token.end(); ++cit) {
			par->InsertChar(pos, (*cit), font);
			++pos;
		}
	}
	return the_end_read;
}


void Buffer::readInset(LyXLex & lex, LyXParagraph *& par,
		       int & pos, LyXFont & font)
{
	// consistency check
	if (lex.GetString() != "\\begin_inset") {
		lyxerr << "Buffer::readInset: Consistency check failed."
		       << endl;
	}
	
	lex.next();
	string tmptok = lex.GetString();
	last_inset_read = tmptok;
	// test the different insets
	if (tmptok == "Quotes") {
		Inset * inset = new InsetQuotes;
		inset->Read(this, lex);
		par->InsertInset(pos, inset, font);
		++pos;
	} else if (tmptok == "External") {
		Inset * inset = new InsetExternal;
		inset->Read(this, lex);
		par->InsertInset(pos, inset, font);
		++pos;
	} else if (tmptok == "FormulaMacro") {
		Inset * inset = new InsetFormulaMacro;
		inset->Read(this, lex);
		par->InsertInset(pos, inset, font);
		++pos;
	} else if (tmptok == "Formula") {
		Inset * inset = new InsetFormula;
		inset->Read(this, lex);
		par->InsertInset(pos, inset, font);
		++pos;
	} else if (tmptok == "Figure") {
		Inset * inset = new InsetFig(100, 100, this);
		inset->Read(this, lex);
		par->InsertInset(pos, inset, font);
		++pos;
	} else if (tmptok == "Info") {
		Inset * inset = new InsetInfo;
		inset->Read(this, lex);
		par->InsertInset(pos, inset, font);
		++pos;
	} else if (tmptok == "Include") {
		Inset * inset = new InsetInclude(string(), this);
		inset->Read(this, lex);
		par->InsertInset(pos, inset, font);
		++pos;
	} else if (tmptok == "ERT") {
		Inset * inset = new InsetERT;
		inset->Read(this, lex);
		par->InsertInset(pos, inset, font);
		++pos;
	} else if (tmptok == "Tabular") {
		Inset * inset = new InsetTabular(this);
		inset->Read(this, lex);
		par->InsertInset(pos, inset, font);
		++pos;
	} else if (tmptok == "Text") {
		Inset * inset = new InsetText;
		inset->Read(this, lex);
		par->InsertInset(pos, inset, font);
		++pos;
	} else if (tmptok == "Foot") {
		Inset * inset = new InsetFoot;
		inset->Read(this, lex);
		par->InsertInset(pos, inset, font);
		++pos;
	} else if (tmptok == "Marginal") {
		Inset * inset = new InsetMarginal;
		inset->Read(this, lex);
		par->InsertInset(pos, inset, font);
		++pos;
	} else if (tmptok == "Minipage") {
		Inset * inset = new InsetMinipage;
		inset->Read(this, lex);
		par->InsertInset(pos, inset, font);
		++pos;
	} else if (tmptok == "Float") {
		lex.next();
		string tmptok = lex.GetString();
		Inset * inset = new InsetFloat(tmptok);
		inset->Read(this, lex);
		par->InsertInset(pos, inset, font);
		++pos;
	} else if (tmptok == "List") {
		Inset * inset = new InsetList;
		inset->Read(this, lex);
		par->InsertInset(pos, inset, font);
		++pos;
	} else if (tmptok == "Theorem") {
		Inset * inset = new InsetList;
		inset->Read(this, lex);
		par->InsertInset(pos, inset, font);
		++pos;
	} else if (tmptok == "Caption") {
		Inset * inset = new InsetCaption;
		inset->Read(this, lex);
		par->InsertInset(pos, inset, font);
		++pos;
	} else if (tmptok == "GRAPHICS") {
		Inset * inset = new InsetGraphics;
				//inset->Read(this, lex);
		par->InsertInset(pos, inset, font);
	} else if (tmptok == "LatexCommand") {
		InsetCommand inscmd;
		inscmd.Read(this, lex);
		Inset * inset = 0;
		if (inscmd.getCmdName() == "cite") {
			inset = new InsetCitation(inscmd.params());
		} else if (inscmd.getCmdName() == "bibitem") {
			lex.printError("Wrong place for bibitem");
			inset = inscmd.Clone();
		} else if (inscmd.getCmdName() == "BibTeX") {
			inset = new InsetBibtex(inscmd.getContents(),
						inscmd.getOptions(), this);
		} else if (inscmd.getCmdName() == "index") {
			inset = new InsetIndex(inscmd.getContents());
		} else if (inscmd.getCmdName() == "include") {
			inset = new InsetInclude(inscmd.getContents(), this);
		} else if (inscmd.getCmdName() == "label") {
			inset = new InsetLabel(inscmd.getCommand());
		} else if (inscmd.getCmdName() == "url"
			   || inscmd.getCmdName() == "htmlurl") {
			inset = new InsetUrl(inscmd.params());
		} else if (inscmd.getCmdName() == "ref"
			   || inscmd.getCmdName() == "pageref"
			   || inscmd.getCmdName() == "vref"
			   || inscmd.getCmdName() == "vpageref"
			   || inscmd.getCmdName() == "prettyref") {
			if (!inscmd.getOptions().empty()
			    || !inscmd.getContents().empty()) {
				inset = new InsetRef(inscmd, this);
			}
		} else if (inscmd.getCmdName() == "tableofcontents") {
			inset = new InsetTOC(this);
		} else if (inscmd.getCmdName() == "listoffigures") {
			inset = new InsetLOF(this);
		} else if (inscmd.getCmdName() == "listofalgorithms") {
			inset = new InsetLOA(this);
		} else if (inscmd.getCmdName() == "listoftables") {
			inset = new InsetLOT(this);
		} else if (inscmd.getCmdName() == "printindex") {
			inset = new InsetPrintIndex(this);
		} else if (inscmd.getCmdName() == "lyxparent") {
			inset = new InsetParent(inscmd.getContents(), this);
		}
		
		if (inset) {
			par->InsertInset(pos, inset, font);
			++pos;
		}
	}
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

				setPaperStuff();

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
	    	    


// Should probably be moved to somewhere else: BufferView? LyXView?
bool Buffer::save() const
{
	// We don't need autosaves in the immediate future. (Asger)
	resetAutosaveTimers();

	// make a backup
	string s;
	if (lyxrc.make_backup) {
		s = fileName() + '~';
		if (!lyxrc.backupdir_path.empty())
			s = AddName(lyxrc.backupdir_path,
				    subst(CleanupPath(s),'/','!'));

		// Rename is the wrong way of making a backup,
		// this is the correct way.
		/* truss cp fil fil2:
		   lstat("LyXVC3.lyx", 0xEFFFF898)                 Err#2 ENOENT
		   stat("LyXVC.lyx", 0xEFFFF688)                   = 0
		   open("LyXVC.lyx", O_RDONLY)                     = 3
		   open("LyXVC3.lyx", O_WRONLY|O_CREAT|O_TRUNC, 0600) = 4
		   fstat(4, 0xEFFFF508)                            = 0
		   fstat(3, 0xEFFFF508)                            = 0
		   read(3, " # T h i s   f i l e   w".., 8192)     = 5579
		   write(4, " # T h i s   f i l e   w".., 5579)    = 5579
		   read(3, 0xEFFFD4A0, 8192)                       = 0
		   close(4)                                        = 0
		   close(3)                                        = 0
		   chmod("LyXVC3.lyx", 0100644)                    = 0
		   lseek(0, 0, SEEK_CUR)                           = 46440
		   _exit(0)
		*/

		// Should proabaly have some more error checking here.
		// Should be cleaned up in 0.13, at least a bit.
		// Doing it this way, also makes the inodes stay the same.
		// This is still not a very good solution, in particular we
		// might loose the owner of the backup.
		FileInfo finfo(fileName());
		if (finfo.exist()) {
			mode_t fmode = finfo.getMode();
			struct utimbuf times = {
				finfo.getAccessTime(),
				finfo.getModificationTime() };

			ifstream ifs(fileName().c_str());
			ofstream ofs(s.c_str(), ios::out|ios::trunc);
			if (ifs && ofs) {
				ofs << ifs.rdbuf();
				ifs.close();
				ofs.close();
				::chmod(s.c_str(), fmode);
				
				if (::utime(s.c_str(), &times)) {
					lyxerr << "utime error." << endl;
				}
			} else {
				lyxerr << "LyX was not able to make "
					"backupcopy. Beware." << endl;
			}
		}
	}
	
	if (writeFile(fileName(), false)) {
		markLyxClean();

		// now delete the autosavefile
		string a = OnlyPath(fileName());
		a += '#';
		a += OnlyFilename(fileName());
		a += '#';
		FileInfo fileinfo(a);
		if (fileinfo.exist()) {
			if (::remove(a.c_str()) != 0) {
				WriteFSAlert(_("Could not delete "
					       "auto-save file!"), a);
			}
		}
	} else {
		// Saving failed, so backup is not backup
		if (lyxrc.make_backup) {
			::rename(s.c_str(), fileName().c_str());
		}
		return false;
	}
	return true;
}


// Returns false if unsuccesful
bool Buffer::writeFile(string const & fname, bool flag) const
{
	// if flag is false writeFile will not create any GUI
	// warnings, only cerr.
	// Needed for autosave in background or panic save (Matthias 120496)

	if (read_only && (fname == filename)) {
		// Here we should come with a question if we should
		// perform the write anyway.
		if (flag)
			lyxerr << _("Error! Document is read-only: ")
			       << fname << endl;
		else
			WriteAlert(_("Error! Document is read-only: "),
				   fname);
		return false;
	}

	FileInfo finfo(fname);
	if (finfo.exist() && !finfo.writable()) {
		// Here we should come with a question if we should
		// try to do the save anyway. (i.e. do a chmod first)
		if (flag)
			lyxerr << _("Error! Cannot write file: ")
			       << fname << endl;
		else
			WriteFSAlert(_("Error! Cannot write file: "),
				     fname);
		return false;
	}

	ofstream ofs(fname.c_str());
	if (!ofs) {
		if (flag)
			lyxerr << _("Error! Cannot open file: ")
			       << fname << endl;
		else
			WriteFSAlert(_("Error! Cannot open file: "),
				     fname);
		return false;
	}
	// The top of the file should not be written by params.

	// write out a comment in the top of the file
	ofs << '#' << LYX_DOCVERSION 
	    << " created this file. For more info see http://www.lyx.org/\n";
	ofs.setf(ios::showpoint|ios::fixed);
	ofs.precision(2);
	ofs << "\\lyxformat " << setw(4) <<  LYX_FORMAT << "\n";

	// now write out the buffer paramters.
	params.writeFile(ofs);

	char footnoteflag = 0;
	char depth = 0;

	// this will write out all the paragraphs
	// using recursive descent.
	paragraph->writeFile(this, ofs, params, footnoteflag, depth);

	// Write marker that shows file is complete
	ofs << "\n\\the_end" << endl;
	ofs.close();
	// how to check if close went ok?
	return true;
}


void Buffer::writeFileAscii(string const & fname, int linelen) 
{
	LyXFont font1, font2;
	Inset * inset;
	char c, footnoteflag = 0, depth = 0;
	string tmp;
	LyXParagraph::size_type i;
	int j;
	int ltype = 0;
	int ltype_depth = 0;
	int actcell = 0;
	int actpos = 0;
#ifndef NEW_TABULAR
	int h;
	int * clen = 0;
	int cell = 0;
	int cells = 0;
#endif
	int currlinelen = 0;
	long fpos = 0;
	bool ref_printed = false;

	ofstream ofs(fname.c_str());
	if (!ofs) {
		WriteFSAlert(_("Error: Cannot write file:"), fname);
		return;
	}

	string fname1 = TmpFileName();
	LyXParagraph * par = paragraph;
	while (par) {
		int noparbreak = 0;
		int islatex = 0;
		if (
#ifndef NEW_INSETS
			par->footnoteflag != LyXParagraph::NO_FOOTNOTE ||
#endif
		    !par->previous
#ifndef NEW_INSETS
		    || par->previous->footnoteflag == LyXParagraph::NO_FOOTNOTE
#endif
			){

#ifndef NEW_INSETS
			/* begins a footnote environment ? */ 
			if (footnoteflag != par->footnoteflag) {
				footnoteflag = par->footnoteflag;
				if (footnoteflag) {
					j = strlen(string_footnotekinds[par->footnotekind])+4;
					if (currlinelen + j > linelen)
						ofs << "\n";
					ofs << "(["
					    << string_footnotekinds[par->footnotekind] << "] ";
					currlinelen += j;
				}
			}
#endif
	 
			/* begins or ends a deeper area ?*/ 
			if (depth != par->depth) {
				if (par->depth > depth) {
					while (par->depth > depth) {
						++depth;
					}
				}
				else {
					while (par->depth < depth) {
						--depth;
					}
				}
			}
	 
			/* First write the layout */
			tmp = textclasslist.NameOfLayout(params.textclass, par->layout);
			if (tmp == "Itemize") {
				ltype = 1;
				ltype_depth = depth+1;
			} else if (tmp == "Enumerate") {
				ltype = 2;
				ltype_depth = depth+1;
			} else if (strstr(tmp.c_str(), "ection")) {
				ltype = 3;
				ltype_depth = depth+1;
			} else if (strstr(tmp.c_str(), "aragraph")) {
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
		} else {
#ifndef NEW_INSETS
			/* dummy layout, that means a footnote ended */ 
			footnoteflag = LyXParagraph::NO_FOOTNOTE;
			ofs << ") ";
			noparbreak = 1;
#else
			lyxerr << "Should this ever happen?" << endl;
#endif
		}
      
		//LyXLayout const & layout =
		//	textclasslist.Style(params.textclass, 
		//			    par->GetLayout()); // unused
		//bool free_spc = layout.free_spacing; //unused

#ifndef NEW_TABULAR
		/* It might be a table */ 
		if (par->table){
			cell = 1;
                        actcell = 0;
			cells = par->table->columns;
			clen = new int [cells];
			memset(clen, 0, sizeof(int) * cells);

			for (i = 0, j = 0, h = 1; i < par->size(); ++i, ++h) {
				c = par->GetChar(i);
				if (c == LyXParagraph::META_INSET) {
					if ((inset = par->GetInset(i))) {
#ifdef HAVE_SSTREAM
						std::ostringstream ost;
						inset->Ascii(this, ost);
						h += ost.str().length();
#else
						ostrstream ost;
						inset->Ascii(this, ost);
						ost << '\0';
						char * tmp = ost.str();
						string tstr(tmp);
						h += tstr.length();
						delete [] tmp;
#endif
					}
				} else if (c == LyXParagraph::META_NEWLINE) {
					if (clen[j] < h)
						clen[j] = h;
					h = 0;
					j = (++j) % par->table->NumberOfCellsInRow(actcell);
                                        ++actcell;
				}
			}
			if (clen[j] < h)
				clen[j] = h;
		}
#endif
		font1 = LyXFont(LyXFont::ALL_INHERIT, params.language_info);
                actcell = 0;
		for (i = 0, actpos = 1; i < par->size(); ++i, ++actpos) {
			if (!i && !footnoteflag && !noparbreak){
				ofs << "\n\n";
				for(j = 0; j < depth; ++j)
					ofs << "  ";
				currlinelen = depth * 2;
				switch(ltype) {
				case 0: /* Standard */
				case 4: /* (Sub)Paragraph */
                                case 5: /* Description */
					break;
				case 6: /* Abstract */
					ofs << "Abstract\n\n";
					break;
				case 7: /* Bibliography */
					if (!ref_printed) {
						ofs << "References\n\n";
						ref_printed = true;
					}
					break;
				default:
					ofs << par->labelstring << " ";
					break;
				}
				if (ltype_depth > depth) {
					for(j = ltype_depth - 1; j > depth; --j)
						ofs << "  ";
					currlinelen += (ltype_depth-depth)*2;
				}
#ifndef NEW_TABULAR
				if (par->table) {
					for(j = 0; j < cells; ++j) {
						ofs << '+';
						for(h = 0; h < (clen[j] + 1);
						    ++h)
							ofs << '-';
					}
					ofs << "+\n";
					for(j = 0; j < depth; ++j)
						ofs << "  ";
					currlinelen = depth * 2;
					if (ltype_depth > depth) {
						for(j = ltype_depth;
						    j > depth; --j)
							ofs << "  ";
						currlinelen += (ltype_depth-depth)*2;
					}
					ofs << "| ";
				}
#endif
			}
			font2 = par->GetFontSettings(params, i);
			if (font1.latex() != font2.latex()) {
				if (font2.latex() == LyXFont::OFF)
					islatex = 0;
				else
					islatex = 1;
			} else {
				islatex = 0;
			}
			c = par->GetChar(i);
			if (islatex)
				continue;
			switch (c) {
			case LyXParagraph::META_INSET:
				if ((inset = par->GetInset(i))) {
					fpos = ofs.tellp();
					inset->Ascii(this, ofs);
					currlinelen += (ofs.tellp() - fpos);
					actpos += (ofs.tellp() - fpos) - 1;
				}
				break;
			case LyXParagraph::META_NEWLINE:
#ifndef NEW_TABULAR
				if (par->table) {
					if (par->table->NumberOfCellsInRow(actcell) <= cell) {
						for(j = actpos; j < clen[cell - 1]; ++j)
							ofs << ' ';
						ofs << " |\n";
						for(j = 0; j < depth; ++j)
							ofs << "  ";
						currlinelen = depth*2;
						if (ltype_depth > depth) {
							for(j = ltype_depth; j > depth; --j)
								ofs << "  ";
							currlinelen += (ltype_depth-depth) * 2;
						}
						for(j = 0; j < cells; ++j) {
							ofs << '+';
							for(h = 0; h < (clen[j] + 1); ++h)
								ofs << '-';
						}
						ofs << "+\n";
						for(j = 0; j < depth; ++j)
							ofs << "  ";
						currlinelen = depth * 2;
						if (ltype_depth > depth) {
							for(j = ltype_depth;
							    j > depth; --j)
								ofs << "  ";
							currlinelen += (ltype_depth-depth)*2;
						}
						ofs << "| ";
						cell = 1;
					} else {
						for(j = actpos;
						    j < clen[cell - 1]; ++j)
							ofs << ' ';
						ofs << " | ";
						++cell;
					}
                                        ++actcell;
					currlinelen = actpos = 0;
				} else {
#endif
					ofs << "\n";
					for(j = 0; j < depth; ++j)
						ofs << "  ";
					currlinelen = depth * 2;
					if (ltype_depth > depth) {
						for(j = ltype_depth;
						    j > depth; --j)
							ofs << "  ";
						currlinelen += (ltype_depth - depth) * 2;
					}
#ifndef NEW_TABULAR
				}
#endif
				break;
			case LyXParagraph::META_HFILL: 
				ofs << "\t";
				break;
			case '\\':
				ofs << "\\";
				break;
			default:
				if (currlinelen > linelen - 10
                                    && c == ' ' && i + 2 < par->size()) {
					ofs << "\n";
					for(j = 0; j < depth; ++j)
						ofs << "  ";
					currlinelen = depth * 2;
					if (ltype_depth > depth) {
						for(j = ltype_depth;
						    j > depth; --j)
							ofs << "  ";
						currlinelen += (ltype_depth-depth)*2;
					}
				} else if (c != '\0')
					ofs << c;
				else if (c == '\0')
					lyxerr.debug() << "writeAsciiFile: NULL char in structure." << endl;
				++currlinelen;
				break;
			}
		}
#ifndef NEW_TABULAR
		if (par->table) {
			for(j = actpos; j < clen[cell - 1]; ++j)
				ofs << ' ';
			ofs << " |\n";
			for(j = 0; j < depth; ++j)
				ofs << "  ";
			currlinelen = depth * 2;
			if (ltype_depth > depth) {
				for(j = ltype_depth; j > depth; --j)
					ofs << "  ";
				currlinelen += (ltype_depth - depth) * 2;
			}
			for(j = 0; j < cells; ++j) {
				ofs << '+';
				for(h = 0; h < (clen[j] + 1); ++h)
					ofs << '-';
			}
			ofs << "+\n";
			delete [] clen;    
		}
#endif
		par = par->next;
	}
   
	ofs << "\n";
}


void Buffer::makeLaTeXFile(string const & fname, 
			   string const & original_path,
			   bool nice, bool only_body)
{
	lyxerr[Debug::LATEX] << "makeLaTeXFile..." << endl;
	
	niceFile = nice; // this will be used by Insetincludes.

	tex_code_break_column = lyxrc.ascii_linelen;

        LyXTextClass const & tclass =
		textclasslist.TextClass(params.textclass);

	ofstream ofs(fname.c_str());
	if (!ofs) {
		WriteFSAlert(_("Error: Cannot open file: "), fname);
		return;
	}
	
	// validate the buffer.
	lyxerr[Debug::LATEX] << "  Validating buffer..." << endl;
	LaTeXFeatures features(params, tclass.numLayouts());
	validate(features);
	lyxerr[Debug::LATEX] << "  Buffer validation done." << endl;
	
	texrow.reset();
	// The starting paragraph of the coming rows is the 
	// first paragraph of the document. (Asger)
	texrow.start(paragraph, 0);

	if (!only_body && nice) {
		ofs << "%% " LYX_DOCVERSION " created this file.  "
			"For more info, see http://www.lyx.org/.\n"
			"%% Do not edit unless you really know what "
			"you are doing.\n";
		texrow.newline();
		texrow.newline();
	}
	lyxerr.debug() << "lyx header finished" << endl;
	// There are a few differences between nice LaTeX and usual files:
	// usual is \batchmode and has a 
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
			ofs << "\\batchmode\n"; // changed
			// from \nonstopmode
			texrow.newline();
		}
		if (!original_path.empty()) {
			ofs << "\\makeatletter\n"
			    << "\\def\\input@path{{"
			    << original_path << "/}}\n"
			    << "\\makeatother\n";
			texrow.newline();
			texrow.newline();
			texrow.newline();
		}
		
		ofs << "\\documentclass";
		
		string options; // the document class options.
		
		if (tokenPos(tclass.opt_fontsize(),
			     '|', params.fontsize) >= 0) {
			// only write if existing in list (and not default)
			options += params.fontsize;
			options += "pt,";
		}
		
		
		if (!params.use_geometry &&
		    (params.paperpackage == BufferParams::PACKAGE_NONE)) {
			switch (params.papersize) {
			case BufferParams::PAPER_A4PAPER:
				options += "a4paper,";
				break;
			case BufferParams::PAPER_USLETTER:
				options += "letterpaper,";
				break;
			case BufferParams::PAPER_A5PAPER:
				options += "a5paper,";
				break;
			case BufferParams::PAPER_B5PAPER:
				options += "b5paper,";
				break;
			case BufferParams::PAPER_EXECUTIVEPAPER:
				options += "executivepaper,";
				break;
			case BufferParams::PAPER_LEGALPAPER:
				options += "legalpaper,";
				break;
			}
		}

		// if needed
		if (params.sides != tclass.sides()) {
			switch (params.sides) {
			case LyXTextClass::OneSide:
				options += "oneside,";
				break;
			case LyXTextClass::TwoSides:
				options += "twoside,";
				break;
			}

		}

		// if needed
		if (params.columns != tclass.columns()) {
			if (params.columns == 2)
				options += "twocolumn,";
			else
				options += "onecolumn,";
		}

		if (!params.use_geometry 
		    && params.orientation == BufferParams::ORIENTATION_LANDSCAPE)
			options += "landscape,";
		
		// language should be a parameter to \documentclass
		bool use_babel = false;
		if (params.language_info->lang() == "hebrew") // This seems necessary
			features.UsedLanguages.insert(default_language);
		if (params.language != "default" ||
		    !features.UsedLanguages.empty() ) {
			use_babel = true;
			for (LaTeXFeatures::LanguageList::const_iterator cit =
				     features.UsedLanguages.begin();
			     cit != features.UsedLanguages.end(); ++cit)
				options += (*cit)->lang() + ",";
			options += params.language_info->lang() + ',';
		}

		// the user-defined options
		if (!params.options.empty()) {
			options += params.options + ',';
		}
		
		if (!options.empty()){
			options = strip(options, ',');
			ofs << '[' << options << ']';
		}
		
		ofs << '{'
		    << textclasslist.LatexnameOfClass(params.textclass)
		    << "}\n";
		texrow.newline();
		// end of \documentclass defs
		
		// font selection must be done before loading fontenc.sty
		if (params.fonts != "default") {
			ofs << "\\usepackage{" << params.fonts << "}\n";
			texrow.newline();
		}
		// this one is not per buffer
		if (lyxrc.fontenc != "default") {
			ofs << "\\usepackage[" << lyxrc.fontenc
			    << "]{fontenc}\n";
			texrow.newline();
		}

		if (params.inputenc == "auto") {
			string doc_encoding =
				params.language_info->encoding()->LatexName();

			// Create a list with all the input encodings used 
			// in the document
			set<string> encodings;
			for (LaTeXFeatures::LanguageList::const_iterator it =
				     features.UsedLanguages.begin();
			     it != features.UsedLanguages.end(); ++it)
				if ((*it)->encoding()->LatexName() != doc_encoding)
					encodings.insert((*it)->encoding()->LatexName());

			ofs << "\\usepackage[";
			for (set<string>::const_iterator it = encodings.begin();
			     it != encodings.end(); ++it)
				ofs << *it << ",";
			ofs << doc_encoding << "]{inputenc}\n";
			texrow.newline();
		} else if (params.inputenc != "default") {
			ofs << "\\usepackage[" << params.inputenc
			    << "]{inputenc}\n";
			texrow.newline();
		}

		// At the very beginning the text parameters.
		if (params.paperpackage != BufferParams::PACKAGE_NONE) {
			switch (params.paperpackage) {
			case BufferParams::PACKAGE_A4:
				ofs << "\\usepackage{a4}\n";
				texrow.newline();
				break;
			case BufferParams::PACKAGE_A4WIDE:
				ofs << "\\usepackage{a4wide}\n";
				texrow.newline();
				break;
			case BufferParams::PACKAGE_WIDEMARGINSA4:
				ofs << "\\usepackage[widemargins]{a4}\n";
				texrow.newline();
				break;
			}
		}
		if (params.use_geometry) {
			ofs << "\\usepackage{geometry}\n";
			texrow.newline();
			ofs << "\\geometry{verbose";
			if (params.orientation == BufferParams::ORIENTATION_LANDSCAPE)
				ofs << ",landscape";
			switch (params.papersize2) {
			case BufferParams::VM_PAPER_CUSTOM:
				if (!params.paperwidth.empty())
					ofs << ",paperwidth="
					    << params.paperwidth;
				if (!params.paperheight.empty())
					ofs << ",paperheight="
					    << params.paperheight;
				break;
			case BufferParams::VM_PAPER_USLETTER:
				ofs << ",letterpaper";
				break;
			case BufferParams::VM_PAPER_USLEGAL:
				ofs << ",legalpaper";
				break;
			case BufferParams::VM_PAPER_USEXECUTIVE:
				ofs << ",executivepaper";
				break;
			case BufferParams::VM_PAPER_A3:
				ofs << ",a3paper";
				break;
			case BufferParams::VM_PAPER_A4:
				ofs << ",a4paper";
				break;
			case BufferParams::VM_PAPER_A5:
				ofs << ",a5paper";
				break;
			case BufferParams::VM_PAPER_B3:
				ofs << ",b3paper";
				break;
			case BufferParams::VM_PAPER_B4:
				ofs << ",b4paper";
				break;
			case BufferParams::VM_PAPER_B5:
				ofs << ",b5paper";
				break;
			default:
				// default papersize ie BufferParams::VM_PAPER_DEFAULT
				switch (lyxrc.default_papersize) {
				case BufferParams::PAPER_DEFAULT: // keep compiler happy
				case BufferParams::PAPER_USLETTER:
					ofs << ",letterpaper";
					break;
				case BufferParams::PAPER_LEGALPAPER:
					ofs << ",legalpaper";
					break;
				case BufferParams::PAPER_EXECUTIVEPAPER:
					ofs << ",executivepaper";
					break;
				case BufferParams::PAPER_A3PAPER:
					ofs << ",a3paper";
					break;
				case BufferParams::PAPER_A4PAPER:
					ofs << ",a4paper";
					break;
				case BufferParams::PAPER_A5PAPER:
					ofs << ",a5paper";
					break;
				case BufferParams::PAPER_B5PAPER:
					ofs << ",b5paper";
					break;
				}
			}
			if (!params.topmargin.empty())
				ofs << ",tmargin=" << params.topmargin;
			if (!params.bottommargin.empty())
				ofs << ",bmargin=" << params.bottommargin;
			if (!params.leftmargin.empty())
				ofs << ",lmargin=" << params.leftmargin;
			if (!params.rightmargin.empty())
				ofs << ",rmargin=" << params.rightmargin;
			if (!params.headheight.empty())
				ofs << ",headheight=" << params.headheight;
			if (!params.headsep.empty())
				ofs << ",headsep=" << params.headsep;
			if (!params.footskip.empty())
				ofs << ",footskip=" << params.footskip;
			ofs << "}\n";
			texrow.newline();
		}
		if (params.use_amsmath
		    && !tclass.provides(LyXTextClass::amsmath)) {
			ofs << "\\usepackage{amsmath}\n";
			texrow.newline();
		}

		if (tokenPos(tclass.opt_pagestyle(),
			     '|', params.pagestyle) >= 0) {
			if (params.pagestyle == "fancy") {
				ofs << "\\usepackage{fancyhdr}\n";
				texrow.newline();
			}
			ofs << "\\pagestyle{" << params.pagestyle << "}\n";
			texrow.newline();
		}

		// We try to load babel late, in case it interferes
		// with other packages.
		if (use_babel) {
			ofs << lyxrc.language_package << endl;
			texrow.newline();
		}

		if (params.secnumdepth != tclass.secnumdepth()) {
			ofs << "\\setcounter{secnumdepth}{"
			    << params.secnumdepth
			    << "}\n";
			texrow.newline();
		}
		if (params.tocdepth != tclass.tocdepth()) {
			ofs << "\\setcounter{tocdepth}{"
			    << params.tocdepth
			    << "}\n";
			texrow.newline();
		}
		
		if (params.paragraph_separation) {
			switch (params.defskip.kind()) {
			case VSpace::SMALLSKIP: 
				ofs << "\\setlength\\parskip{\\smallskipamount}\n";
				break;
			case VSpace::MEDSKIP:
				ofs << "\\setlength\\parskip{\\medskipamount}\n";
				break;
			case VSpace::BIGSKIP:
				ofs << "\\setlength\\parskip{\\bigskipamount}\n";
				break;
			case VSpace::LENGTH:
				ofs << "\\setlength\\parskip{"
				    << params.defskip.length().asLatexString()
				    << "}\n";
				break;
			default: // should never happen // Then delete it.
				ofs << "\\setlength\\parskip{\\medskipamount}\n";
				break;
			}
			texrow.newline();
			
			ofs << "\\setlength\\parindent{0pt}\n";
			texrow.newline();
		}

		// Now insert the LyX specific LaTeX commands...

		// The optional packages;
		string preamble(features.getPackages());

		// this might be useful...
		preamble += "\n\\makeatletter\n";

		// Some macros LyX will need
		string tmppreamble(features.getMacros());

		if (!tmppreamble.empty()) {
			preamble += "\n%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% "
				"LyX specific LaTeX commands.\n"
				+ tmppreamble + '\n';
		}

		// the text class specific preamble 
		tmppreamble = features.getTClassPreamble();
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

		preamble += "\\makeatother\n";

		// Itemize bullet settings need to be last in case the user
		// defines their own bullets that use a package included
		// in the user-defined preamble -- ARRae
		// Actually it has to be done much later than that
		// since some packages like frenchb make modifications
		// at \begin{document} time -- JMarc 
		string bullets_def;
		for (int i = 0; i < 4; ++i) {
			if (params.user_defined_bullets[i] != ITEMIZE_DEFAULTS[i]) {
				if (bullets_def.empty())
					bullets_def="\\AtBeginDocument{\n";
				bullets_def += "  \\renewcommand{\\labelitemi";
				switch (i) {
				// `i' is one less than the item to modify
				case 0:
					break;
				case 1:
					bullets_def += 'i';
					break;
				case 2:
					bullets_def += "ii";
					break;
				case 3:
					bullets_def += 'v';
					break;
				}
				bullets_def += "}{" + 
				  params.user_defined_bullets[i].getText() 
				  + "}\n";
			}
		}

		if (!bullets_def.empty())
		  preamble += bullets_def + "}\n\n";

		for (int j = countChar(preamble, '\n'); j-- ;) {
			texrow.newline();
		}

		ofs << preamble;

		// make the body.
		ofs << "\\begin{document}\n";
		texrow.newline();
	} // only_body
	lyxerr.debug() << "preamble finished, now the body." << endl;
	if (!lyxrc.language_auto_begin && params.language != "default") {
		ofs << subst(lyxrc.language_command_begin, "$$lang",
			     params.language)
		    << endl;
		texrow.newline();
	}
	
	latexParagraphs(ofs, paragraph, 0, texrow);

	// add this just in case after all the paragraphs
	ofs << endl;
	texrow.newline();

	if (!lyxrc.language_auto_end && params.language != "default") {
		ofs << subst(lyxrc.language_command_end, "$$lang",
			     params.language)
		    << endl;
		texrow.newline();
	}

	if (!only_body) {
		ofs << "\\end{document}\n";
		texrow.newline();
	
		lyxerr[Debug::LATEX] << "makeLaTeXFile...done" << endl;
	} else {
		lyxerr[Debug::LATEX] << "LaTeXFile for inclusion made."
				     << endl;
	}

	// Just to be sure. (Asger)
	texrow.newline();

	// tex_code_break_column's value is used to decide
	// if we are in batchmode or not (within mathed_write()
	// in math_write.C) so we must set it to a non-zero
	// value when we leave otherwise we save incorrect .lyx files.
	tex_code_break_column = lyxrc.ascii_linelen;

	ofs.close();
	if (ofs.fail()) {
		lyxerr << "File was not closed properly." << endl;
	}
	
	lyxerr.debug() << "Finished making latex file." << endl;
}


//
// LaTeX all paragraphs from par to endpar, if endpar == 0 then to the end
//
void Buffer::latexParagraphs(ostream & ofs, LyXParagraph * par,
			     LyXParagraph * endpar, TexRow & texrow) const
{
	bool was_title = false;
	bool already_title = false;
#ifdef HAVE_SSTREAM
	std::ostringstream ftnote;
#else
	char * tmpholder = 0;
#endif
	TexRow ft_texrow;
	int ftcount = 0;

	// if only_body
	while (par != endpar) {
#ifndef HAVE_SSTREAM
		ostrstream ftnote;
		if (tmpholder) {
			ftnote << tmpholder;
			delete [] tmpholder;
			tmpholder = 0;
		}
#endif
#ifndef NEW_INSETS
		if (par->IsDummy())
			lyxerr[Debug::LATEX] << "Error in latexParagraphs."
					     << endl;
#endif
		LyXLayout const & layout =
			textclasslist.Style(params.textclass,
					    par->layout);
	    
	        if (layout.intitle) {
			if (already_title) {
				lyxerr <<"Error in latexParagraphs: You"
					" should not mix title layouts"
					" with normal ones." << endl;
			} else
				was_title = true;
	        } else if (was_title && !already_title) {
			ofs << "\\maketitle\n";
			texrow.newline();
			already_title = true;
			was_title = false;		    
		}
		// We are at depth 0 so we can just use
		// ordinary \footnote{} generation
		// flag this with ftcount
		ftcount = -1;
		if (layout.isEnvironment()
                    || par->pextra_type != LyXParagraph::PEXTRA_NONE) {
			par = par->TeXEnvironment(this, params, ofs, texrow
#ifndef NEW_INSETS
						  ,ftnote, ft_texrow, ftcount
#endif
				);
		} else {
			par = par->TeXOnePar(this, params, ofs, texrow, false
#ifndef NEW_INSETS
					     ,
					     ftnote, ft_texrow, ftcount
#endif
				);
		}

		// Write out what we've generated...
		if (ftcount >= 1) {
			if (ftcount > 1) {
				ofs << "\\addtocounter{footnote}{-"
				    << ftcount - 1
				    << '}';
			}
			ofs << ftnote.str();
			texrow += ft_texrow;
#ifdef HAVE_SSTREAM
			// The extra .c_str() is needed when we use
			// lyxstring instead of the STL string class. 
			ftnote.str(string().c_str());
#else
			delete [] ftnote.str();
#endif
			ft_texrow.reset();
			ftcount = 0;
		}
#ifndef HAVE_SSTREAM
		else {
			// I hate strstreams
			tmpholder = ftnote.str();
		}
#endif
	}
#ifndef HAVE_SSTREAM
	delete [] tmpholder;
#endif
	// It might be that we only have a title in this document
	if (was_title && !already_title) {
		ofs << "\\maketitle\n";
		texrow.newline();
	}
}


bool Buffer::isLatex() const
{
	return textclasslist.TextClass(params.textclass).outputType() == LATEX;
}


bool Buffer::isLinuxDoc() const
{
	return textclasslist.TextClass(params.textclass).outputType() == LINUXDOC;
}


bool Buffer::isLiterate() const
{
	return textclasslist.TextClass(params.textclass).outputType() == LITERATE;
}


bool Buffer::isDocBook() const
{
	return textclasslist.TextClass(params.textclass).outputType() == DOCBOOK;
}


bool Buffer::isSGML() const
{
	return textclasslist.TextClass(params.textclass).outputType() == LINUXDOC ||
	       textclasslist.TextClass(params.textclass).outputType() == DOCBOOK;
}


void Buffer::sgmlOpenTag(ostream & os, int depth,
			 string const & latexname) const
{
	os << string(depth, ' ') << "<" << latexname << ">\n";
}


void Buffer::sgmlCloseTag(ostream & os, int depth,
			  string const & latexname) const
{
	os << string(depth, ' ') << "</" << latexname << ">\n";
}


void Buffer::makeLinuxDocFile(string const & fname, bool nice, bool body_only)
{
	LyXParagraph * par = paragraph;

	niceFile = nice; // this will be used by Insetincludes.

	string top_element = textclasslist.LatexnameOfClass(params.textclass);
	string environment_stack[10];
        string item_name;

	int depth = 0; // paragraph depth

	ofstream ofs(fname.c_str());

	if (!ofs) {
		WriteAlert(_("LYX_ERROR:"), _("Cannot write file"), fname);
		return;
	}

        LyXTextClass const & tclass =
		textclasslist.TextClass(params.textclass);

	LaTeXFeatures features(params, tclass.numLayouts());
	validate(features);

	//if(nice)
	tex_code_break_column = lyxrc.ascii_linelen;
	//else
	//tex_code_break_column = 0;

	texrow.reset();

	if (!body_only) {
		string sgml_includedfiles=features.getIncludedFiles();

		if (params.preamble.empty() && sgml_includedfiles.empty()) {
			ofs << "<!doctype linuxdoc system>\n\n";
		} else {
			ofs << "<!doctype linuxdoc system [ "
			    << params.preamble << sgml_includedfiles << " \n]>\n\n";
		}

		if(params.options.empty())
			sgmlOpenTag(ofs, 0, top_element);
		else {
			string top = top_element;
			top += " ";
			top += params.options;
			sgmlOpenTag(ofs, 0, top);
		}
	}

	ofs << "<!-- "  << LYX_DOCVERSION 
	    << " created this file. For more info see http://www.lyx.org/"
	    << " -->\n";

	while (par) {
		int desc_on = 0; // description mode
		LyXLayout const & style =
			textclasslist.Style(params.textclass,
					    par->layout);

		// treat <toc> as a special case for compatibility with old code
		if (par->GetChar(0) == LyXParagraph::META_INSET) {
		        Inset * inset = par->GetInset(0);
			Inset::Code lyx_code = inset->LyxCode();
			if (lyx_code == Inset::TOC_CODE){
				string temp = "toc";
				sgmlOpenTag(ofs, depth, temp);

				par = par->next;
#ifndef NEW_INSETS
				linuxDocHandleFootnote(ofs, par, depth);
#endif
				continue;
			}
		}

		// environment tag closing
		for( ; depth > par->depth; --depth) {
			sgmlCloseTag(ofs, depth, environment_stack[depth]);
			environment_stack[depth].erase();
		}

		// write opening SGML tags
		switch(style.latextype) {
		case LATEX_PARAGRAPH:
			if(depth == par->depth 
			   && !environment_stack[depth].empty()) {
				sgmlCloseTag(ofs, depth, environment_stack[depth]);
				environment_stack[depth].erase();
				if(depth) 
					--depth;
				else
				        ofs << "</p>";
			}
			sgmlOpenTag(ofs, depth, style.latexname());
			break;

		case LATEX_COMMAND:
			if (depth!= 0)
				LinuxDocError(par, 0,
					      _("Error : Wrong depth for"
						" LatexType Command.\n"));

			if (!environment_stack[depth].empty()){
				sgmlCloseTag(ofs, depth,
					     environment_stack[depth]);
				ofs << "</p>";
			}

			environment_stack[depth].erase();
			sgmlOpenTag(ofs, depth, style.latexname());
			break;

		case LATEX_ENVIRONMENT:
		case LATEX_ITEM_ENVIRONMENT:
			if(depth == par->depth 
			   && environment_stack[depth] != style.latexname()
			   && !environment_stack[depth].empty()) {

				sgmlCloseTag(ofs, depth,
					     environment_stack[depth]);
				environment_stack[depth].erase();
			}
			if (depth < par->depth) {
			       depth = par->depth;
			       environment_stack[depth].erase();
			}
			if (environment_stack[depth] != style.latexname()) {
				if(depth == 0) {
					string temp = "p";
					sgmlOpenTag(ofs, depth, temp);
				}
				environment_stack[depth] = style.latexname();
				sgmlOpenTag(ofs, depth,
					    environment_stack[depth]);
			}
			if(style.latextype == LATEX_ENVIRONMENT) break;

			desc_on = (style.labeltype == LABEL_MANUAL);

			if(desc_on)
				item_name = "tag";
			else
				item_name = "item";

			sgmlOpenTag(ofs, depth + 1, item_name);
			break;
		default:
			sgmlOpenTag(ofs, depth, style.latexname());
			break;
		}

#ifndef NEW_INSETS
		do {
#endif
			SimpleLinuxDocOnePar(ofs, par, desc_on, depth);

			par = par->next;
#ifndef NEW_INSETS
			linuxDocHandleFootnote(ofs, par, depth);
		}
		while(par && par->IsDummy());
#endif

		ofs << "\n";
		// write closing SGML tags
		switch(style.latextype) {
		case LATEX_COMMAND:
		case LATEX_ENVIRONMENT:
		case LATEX_ITEM_ENVIRONMENT:
			break;
		default:
			sgmlCloseTag(ofs, depth, style.latexname());
			break;
		}
	}
   
	// Close open tags
	for(; depth > 0; --depth)
	        sgmlCloseTag(ofs, depth, environment_stack[depth]);

	if(!environment_stack[depth].empty())
	        sgmlCloseTag(ofs, depth, environment_stack[depth]);

	if (!body_only) {
		ofs << "\n\n";
		sgmlCloseTag(ofs, 0, top_element);
	}

	ofs.close();
	// How to check for successful close
}


#ifndef NEW_INSETS
void Buffer::linuxDocHandleFootnote(ostream & os, LyXParagraph * & par,
				    int const depth)
{
	string tag = "footnote";

	while (par && par->footnoteflag != LyXParagraph::NO_FOOTNOTE) {
		sgmlOpenTag(os, depth + 1, tag);
		SimpleLinuxDocOnePar(os, par, 0, depth + 1);
		sgmlCloseTag(os, depth + 1, tag);
		par = par->next;
	}
}
#endif


void Buffer::DocBookHandleCaption(ostream & os, string & inner_tag,
				  int const depth, int desc_on,
				  LyXParagraph * & par)
{
	LyXParagraph * tpar = par;
	while (tpar
#ifndef NEW_INSETS
	       && (tpar->footnoteflag != LyXParagraph::NO_FOOTNOTE)
#endif
	       && (tpar->layout != textclasslist.NumberOfLayout(params.textclass,
							     "Caption").second))
		tpar = tpar->next;
	if (tpar &&
	    tpar->layout == textclasslist.NumberOfLayout(params.textclass,
							 "Caption").second) {
		sgmlOpenTag(os, depth + 1, inner_tag);
		string extra_par;
		SimpleDocBookOnePar(os, extra_par, tpar,
				    desc_on, depth + 2);
		sgmlCloseTag(os, depth+1, inner_tag);
		if(!extra_par.empty())
			os << extra_par;
	}
}


#ifndef NEW_INSETS
void Buffer::DocBookHandleFootnote(ostream & os, LyXParagraph * & par,
				   int const depth)
{
	string tag, inner_tag;
	string tmp_par, extra_par;
	bool inner_span = false;
	int desc_on = 4;

	// Someone should give this enum a proper name (Lgb)
	enum SOME_ENUM {
		NO_ONE,
		FOOTNOTE_LIKE,
		MARGIN_LIKE,
		FIG_LIKE,
		TAB_LIKE
	};
	SOME_ENUM last = NO_ONE;
	SOME_ENUM present = FOOTNOTE_LIKE;

	while (par && par->footnoteflag != LyXParagraph::NO_FOOTNOTE) {
		if(last == present) {
			if(inner_span) {
				if(!tmp_par.empty()) {
					os << tmp_par;
					tmp_par.erase();
					sgmlCloseTag(os, depth + 1, inner_tag);
					sgmlOpenTag(os, depth + 1, inner_tag);
				}
			} else {
				os << "\n";
			}
		} else {
			os << tmp_par;
			if(!inner_tag.empty()) sgmlCloseTag(os, depth + 1,
							    inner_tag);
			if(!extra_par.empty()) os << extra_par;
			if(!tag.empty()) sgmlCloseTag(os, depth, tag);
			extra_par.erase();

			switch (par->footnotekind) {
			case LyXParagraph::FOOTNOTE:
			case LyXParagraph::ALGORITHM:
				tag = "footnote";
				inner_tag = "para";
				present = FOOTNOTE_LIKE;
				inner_span = true;
				break;
			case LyXParagraph::MARGIN:
				tag = "sidebar";
				inner_tag = "para";
				present = MARGIN_LIKE;
				inner_span = true;
				break;
			case LyXParagraph::FIG:
			case LyXParagraph::WIDE_FIG:
				tag = "figure";
				inner_tag = "title";
				present = FIG_LIKE;
				inner_span = false;
				break;
			case LyXParagraph::TAB:
			case LyXParagraph::WIDE_TAB:
				tag = "table";
				inner_tag = "title";
				present = TAB_LIKE;
				inner_span = false;
				break;
			}
			sgmlOpenTag(os, depth, tag);
			if ((present == TAB_LIKE) || (present == FIG_LIKE)) {
				DocBookHandleCaption(os, inner_tag, depth,
						     desc_on, par);
				inner_tag.erase();
			} else {
				sgmlOpenTag(os, depth + 1, inner_tag);
			}
		}
		// ignore all caption here, we processed them above!!!
		if (par->layout != textclasslist
		    .NumberOfLayout(params.textclass,
				    "Caption").second) {
#ifdef HAVE_SSTREAM
			std::ostringstream ost;
#else
			ostrstream ost;
#endif
			SimpleDocBookOnePar(ost, extra_par, par,
					    desc_on, depth + 2);
#ifdef HAVE_SSTREAM
			tmp_par += ost.str().c_str();
#else
			ost << '\0';
			char * ctmp = ost.str();
			tmp_par += ctmp;
			delete [] ctmp;
#endif
		}
		tmp_par = frontStrip(strip(tmp_par));

		last = present;
		par = par->next;
	}
	os << tmp_par;
	if(!inner_tag.empty()) sgmlCloseTag(os, depth + 1, inner_tag);
	if(!extra_par.empty()) os << extra_par;
	if(!tag.empty()) sgmlCloseTag(os, depth, tag);
}
#endif


// push a tag in a style stack
void Buffer::push_tag(ostream & os, char const * tag,
		      int & pos, char stack[5][3])
{
	// pop all previous tags
	for (int j = pos; j >= 0; --j)
		os << "</" << stack[j] << ">";

	// add new tag
	sprintf(stack[++pos], "%s", tag);

	// push all tags
	for (int i = 0; i <= pos; ++i)
		os << "<" << stack[i] << ">";
}


void Buffer::pop_tag(ostream & os, char const * tag,
                     int & pos, char stack[5][3])
{
        int j;

        // pop all tags till specified one
        for (j = pos; (j >= 0) && (strcmp(stack[j], tag)); --j)
                os << "</" << stack[j] << ">";

        // closes the tag
        os << "</" << tag << ">";

        // push all tags, but the specified one
        for (j = j + 1; j <= pos; ++j) {
                os << "<" << stack[j] << ">";
                strcpy(stack[j-1], stack[j]);
        }
        --pos;
}


// Handle internal paragraph parsing -- layout already processed.

// checks, if newcol chars should be put into this line
// writes newline, if necessary.
static
void linux_doc_line_break(ostream & os, unsigned int & colcount,
			  const unsigned int newcol)
{
	colcount += newcol;
	if (colcount > lyxrc.ascii_linelen) {
		os << "\n";
		colcount = newcol; // assume write after this call
	}
}


void Buffer::SimpleLinuxDocOnePar(ostream & os, LyXParagraph * par,
				  int desc_on, int const /*depth*/)
{
	LyXFont font1, font2;
	char c;
	Inset * inset;
	LyXParagraph::size_type main_body;
	int j;
	LyXLayout const & style = textclasslist.Style(params.textclass,
						      par->GetLayout());

	char family_type = 0;               // family font flag 
	bool is_bold     = false;           // series font flag 
	char shape_type  = 0;               // shape font flag 
	bool is_em = false;                 // emphasis (italic) font flag 

	int stack_num = -1;          // style stack position
	// Can this be rewritten to use a std::stack, please. (Lgb)
	char stack[5][3];    	     // style stack 
        unsigned int char_line_count = 5;     // Heuristic choice ;-) 

	if (style.labeltype != LABEL_MANUAL)
		main_body = 0;
	else
		main_body = par->BeginningOfMainBody();

	// gets paragraph main font
	if (main_body > 0)
		font1 = style.labelfont;
	else
		font1 = style.font;

  
	// parsing main loop
	for (LyXParagraph::size_type i = 0;
	     i < par->size(); ++i) {

		// handle quote tag
		if (i == main_body
#ifndef NEW_INSETS
		    && !par->IsDummy()
#endif
			) {
			if (main_body > 0)
				font1 = style.font;
		}

		font2 = par->getFont(params, i);

		if (font1.family() != font2.family()) {
			switch(family_type) {
			case 0:
			       	if (font2.family() == LyXFont::TYPEWRITER_FAMILY) {
			        	push_tag(os, "tt", stack_num, stack);
					family_type = 1;
				}
				else if (font2.family() == LyXFont::SANS_FAMILY) {
				        push_tag(os, "sf", stack_num, stack);
					family_type = 2;
				}
				break;
			case 1:
				pop_tag(os, "tt", stack_num, stack);
				if (font2.family() == LyXFont::SANS_FAMILY) {
			        	push_tag(os, "sf", stack_num, stack);
					family_type = 2;
				} else {
					family_type = 0;
				}
				break;
			case 2:
				pop_tag(os, "sf", stack_num, stack);
				if (font2.family() == LyXFont::TYPEWRITER_FAMILY) {
			        	push_tag(os, "tt", stack_num, stack);
					family_type = 1;
				} else {
					family_type = 0;
				}
			}
		}

		// handle bold face
		if (font1.series() != font2.series()) {
		        if (font2.series() == LyXFont::BOLD_SERIES) {
			        push_tag(os, "bf", stack_num, stack);
				is_bold = true;
			} else if (is_bold) {
			        pop_tag(os, "bf", stack_num, stack);
				is_bold = false;
			}
		}

		// handle italic and slanted fonts
		if (font1.shape() != font2.shape()) {
			switch(shape_type) {
			case 0:
			       	if (font2.shape() == LyXFont::ITALIC_SHAPE) {
			        	push_tag(os, "it", stack_num, stack);
					shape_type = 1;
				} else if (font2.shape() == LyXFont::SLANTED_SHAPE) {
				        push_tag(os, "sl", stack_num, stack);
					shape_type = 2;
				}
				break;
			case 1:
				pop_tag(os, "it", stack_num, stack);
				if (font2.shape() == LyXFont::SLANTED_SHAPE) {
			        	push_tag(os, "sl", stack_num, stack);
					shape_type = 2;
				} else {
					shape_type = 0;
				}
				break;
			case 2:
				pop_tag(os, "sl", stack_num, stack);
				if (font2.shape() == LyXFont::ITALIC_SHAPE) {
			        	push_tag(os, "it", stack_num, stack);
					shape_type = 1;
				} else {
					shape_type = 0;
				}
			}
		}
		// handle <em> tag
		if (font1.emph() != font2.emph()) {
			if (font2.emph() == LyXFont::ON) {
				push_tag(os, "em", stack_num, stack);
				is_em = true;
			} else if (is_em) {
				pop_tag(os, "em", stack_num, stack);
				is_em = false;
			}
		}

		c = par->GetChar(i);

		if (c == LyXParagraph::META_INSET) {
			inset = par->GetInset(i);
			inset->Linuxdoc(this, os);
		}

		if (font2.latex() == LyXFont::ON) {
			// "TeX"-Mode on == > SGML-Mode on.
			if (c != '\0')
				os << c; // see LaTeX-Generation...
			++char_line_count;
		} else {
			string sgml_string;
			if (par->linuxDocConvertChar(c, sgml_string)
			    && !style.free_spacing) { // in freespacing
				                     // mode, spaces are
				                     // non-breaking characters
				// char is ' '
				if (desc_on == 1) {
					++char_line_count;
					linux_doc_line_break(os, char_line_count, 6);
					os << "</tag>";
					desc_on = 2;
				} else  {
					linux_doc_line_break(os, char_line_count, 1);
					os << c;
				}
			} else {
				os << sgml_string;
				char_line_count += sgml_string.length();
			}
		}
		font1 = font2;
	}

	// needed if there is an optional argument but no contents
	if (main_body > 0 && main_body == par->size()) {
		font1 = style.font;
	}

	// pop all defined Styles
	for (j = stack_num; j >= 0; --j) {
	        linux_doc_line_break(os, 
				     char_line_count, 
				     3 + strlen(stack[j]));
		os << "</" << stack[j] << ">";
	}

	// resets description flag correctly
	switch(desc_on){
	case 1:
		// <tag> not closed...
		linux_doc_line_break(os, char_line_count, 6);
		os << "</tag>";
		break;
	case 2:
	        // fprintf(file, "</p>");
		break;
	}
}


// Print an error message.
void Buffer::LinuxDocError(LyXParagraph * par, int pos,
			   char const * message) 
{
	// insert an error marker in text
	InsetError * new_inset = new InsetError(message);
	par->InsertInset(pos, new_inset);
}

// This constant defines the maximum number of 
// environment layouts that can be nesteded.
// The same applies for command layouts.
// These values should be more than enough.
//           José Matos (1999/07/22)

enum { MAX_NEST_LEVEL = 25};

void Buffer::makeDocBookFile(string const & fname, bool nice, bool only_body)
{
	LyXParagraph * par = paragraph;

	niceFile = nice; // this will be used by Insetincludes.

	string top_element= textclasslist.LatexnameOfClass(params.textclass);
	// Please use a real stack.
	string environment_stack[MAX_NEST_LEVEL];
	string environment_inner[MAX_NEST_LEVEL];
	// Please use a real stack.
	string command_stack[MAX_NEST_LEVEL];
	bool command_flag= false;
	int command_depth= 0, command_base= 0, cmd_depth= 0;

        string item_name, command_name;
	string c_depth, c_params, tmps;

	int depth = 0; // paragraph depth
        LyXTextClass const & tclass =
		textclasslist.TextClass(params.textclass);

	LaTeXFeatures features(params, tclass.numLayouts());
	validate(features);

	//if(nice)
	tex_code_break_column = lyxrc.ascii_linelen;
	//else
	//tex_code_break_column = 0;

	ofstream ofs(fname.c_str());
	if (!ofs) {
		WriteAlert(_("LYX_ERROR:"), _("Cannot write file"), fname);
		return;
	}
   
	texrow.reset();

	if(!only_body) {
		string sgml_includedfiles=features.getIncludedFiles();

		ofs << "<!doctype " << top_element
		    << " public \"-//OASIS//DTD DocBook V3.1//EN\"";

		if (params.preamble.empty() && sgml_includedfiles.empty())
			ofs << ">\n\n";
		else
			ofs << "\n [ " << params.preamble 
			    << sgml_includedfiles << " \n]>\n\n";

		if(params.options.empty())
			sgmlOpenTag(ofs, 0, top_element);
		else {
			string top = top_element;
			top += " ";
			top += params.options;
			sgmlOpenTag(ofs, 0, top);
		}
	}

	ofs << "<!-- DocBook file was created by " << LYX_DOCVERSION 
	    << "\n  See http://www.lyx.org/ for more information -->\n";

	while (par) {
		int desc_on = 0; // description mode
		LyXLayout const & style =
			textclasslist.Style(params.textclass,
					    par->layout);

		// environment tag closing
		for( ; depth > par->depth; --depth) {
			if(environment_inner[depth] != "!-- --") {
				item_name= "listitem";
				sgmlCloseTag(ofs, command_depth + depth,
					     item_name);
				if( environment_inner[depth] == "varlistentry")
					sgmlCloseTag(ofs, depth+command_depth,
						     environment_inner[depth]);
			}
			sgmlCloseTag(ofs, depth + command_depth,
				     environment_stack[depth]);
			environment_stack[depth].erase();
			environment_inner[depth].erase();
		}

		if(depth == par->depth
		   && environment_stack[depth] != style.latexname()
		   && !environment_stack[depth].empty()) {
			if(environment_inner[depth] != "!-- --") {
				item_name= "listitem";
				sgmlCloseTag(ofs, command_depth+depth,
					     item_name);
				if( environment_inner[depth] == "varlistentry")
					sgmlCloseTag(ofs,
						     depth + command_depth,
						     environment_inner[depth]);
			}
			
			sgmlCloseTag(ofs, depth + command_depth,
				     environment_stack[depth]);
			
			environment_stack[depth].erase();
			environment_inner[depth].erase();
                }

		// Write opening SGML tags.
		switch(style.latextype) {
		case LATEX_PARAGRAPH:
			if(style.latexname() != "dummy")
                               sgmlOpenTag(ofs, depth+command_depth,
                                           style.latexname());
			break;

		case LATEX_COMMAND:
			if (depth!= 0)
				LinuxDocError(par, 0,
					      _("Error : Wrong depth for "
						"LatexType Command.\n"));
			
			command_name = style.latexname();
			
			tmps = style.latexparam();
			c_params = split(tmps, c_depth,'|');
			
			cmd_depth= atoi(c_depth.c_str());
			
			if(command_flag) {
				if(cmd_depth<command_base) {
					for(int j = command_depth;
					    j >= command_base; --j)
						if(!command_stack[j].empty())
							sgmlCloseTag(ofs, j, command_stack[j]);
					command_depth= command_base= cmd_depth;
				} else if(cmd_depth <= command_depth) {
					for(int j = command_depth;
					    j >= cmd_depth; --j)

						if(!command_stack[j].empty())
							sgmlCloseTag(ofs, j, command_stack[j]);
					command_depth= cmd_depth;
				} else
					command_depth= cmd_depth;
			} else {
				command_depth = command_base = cmd_depth;
				command_flag = true;
			}
			command_stack[command_depth]= command_name;

			// treat label as a special case for
			// more WYSIWYM handling.
			if (par->GetChar(0) == LyXParagraph::META_INSET) {
			        Inset * inset = par->GetInset(0);
				Inset::Code lyx_code = inset->LyxCode();
				if (lyx_code == Inset::LABEL_CODE){
					command_name += " id=\"";
					command_name += (static_cast<InsetCommand *>(inset))->getContents();
					command_name += "\"";
					desc_on = 3;
				}
			}

			sgmlOpenTag(ofs, depth + command_depth, command_name);
			item_name = "title";
			sgmlOpenTag(ofs, depth + 1 + command_depth, item_name);
			break;

		case LATEX_ENVIRONMENT:
		case LATEX_ITEM_ENVIRONMENT:
			if (depth < par->depth) {
				depth = par->depth;
				environment_stack[depth].erase();
			}

			if (environment_stack[depth] != style.latexname()) {
				environment_stack[depth] = style.latexname();
				environment_inner[depth] = "!-- --";
				sgmlOpenTag(ofs, depth + command_depth,
					    environment_stack[depth]);
			} else {
				if(environment_inner[depth] != "!-- --") {
					item_name= "listitem";
					sgmlCloseTag(ofs,
						     command_depth + depth,
						     item_name);
					if (environment_inner[depth] == "varlistentry")
						sgmlCloseTag(ofs,
							     depth + command_depth,
							     environment_inner[depth]);
				}
			}
			
			if(style.latextype == LATEX_ENVIRONMENT) {
				if(!style.latexparam().empty())
			  		sgmlOpenTag(ofs, depth + command_depth,
						    style.latexparam());
				break;
			}

			desc_on = (style.labeltype == LABEL_MANUAL);

			if(desc_on)
				environment_inner[depth]= "varlistentry";
			else
				environment_inner[depth]= "listitem";

			sgmlOpenTag(ofs, depth + 1 + command_depth,
				    environment_inner[depth]);

			if(desc_on) {
				item_name= "term";
				sgmlOpenTag(ofs, depth + 1 + command_depth,
					    item_name);
			} else {
				item_name= "para";
				sgmlOpenTag(ofs, depth + 1 + command_depth,
					    item_name);
			}
			break;
		default:
			sgmlOpenTag(ofs, depth + command_depth,
				    style.latexname());
			break;
		}

#ifndef NEW_INSETS
		do {
#endif
			string extra_par;
			SimpleDocBookOnePar(ofs, extra_par, par, desc_on,
					    depth + 1 + command_depth);
			par = par->next;
#ifndef NEW_INSETS
			DocBookHandleFootnote(ofs, par,
					      depth + 1 + command_depth);
		}
		while(par && par->IsDummy());
#endif
		string end_tag;
		// write closing SGML tags
		switch(style.latextype) {
		case LATEX_COMMAND:
			end_tag = "title";
			sgmlCloseTag(ofs, depth + command_depth, end_tag);
			break;
		case LATEX_ENVIRONMENT:
			if(!style.latexparam().empty())
				sgmlCloseTag(ofs, depth + command_depth,
					     style.latexparam());
			break;
		case LATEX_ITEM_ENVIRONMENT:
			if(desc_on == 1) break;
			end_tag= "para";
			sgmlCloseTag(ofs, depth + 1 + command_depth, end_tag);
			break;
		case LATEX_PARAGRAPH:
			if(style.latexname() != "dummy")
				sgmlCloseTag(ofs, depth + command_depth,
					     style.latexname());
			break;
		default:
			sgmlCloseTag(ofs, depth + command_depth,
				     style.latexname());
			break;
		}
	}

	// Close open tags
	for(; depth >= 0; --depth) {
		if(!environment_stack[depth].empty()) {
			if(environment_inner[depth] != "!-- --") {
				item_name= "listitem";
				sgmlCloseTag(ofs, command_depth + depth,
					     item_name);
                               if( environment_inner[depth] == "varlistentry")
				       sgmlCloseTag(ofs, depth + command_depth,
						    environment_inner[depth]);
			}
			
			sgmlCloseTag(ofs, depth + command_depth,
				     environment_stack[depth]);
		}
	}
	
	for(int j = command_depth; j >= command_base; --j)
		if(!command_stack[j].empty())
			sgmlCloseTag(ofs, j, command_stack[j]);

	if (!only_body) {
		ofs << "\n\n";
		sgmlCloseTag(ofs, 0, top_element);
	}

	ofs.close();
	// How to check for successful close
}


void Buffer::SimpleDocBookOnePar(ostream & os, string & extra,
				 LyXParagraph * par, int & desc_on,
				 int const depth) 
{
#ifndef NEW_TABULAR
	if (par->table) {
		par->SimpleDocBookOneTablePar(this,
					      os, extra, desc_on, depth);
		return;
	}
#endif

	bool emph_flag = false;

	LyXLayout const & style = textclasslist.Style(params.textclass,
						      par->GetLayout());

	LyXParagraph::size_type main_body;
	if (style.labeltype != LABEL_MANUAL)
		main_body = 0;
	else
		main_body = par->BeginningOfMainBody();

	// gets paragraph main font
	LyXFont font1 = main_body > 0 ? style.labelfont : style.font;
	
	int char_line_count = depth;
	if(!style.free_spacing)
		for (int j = 0; j < depth; ++j)
			os << ' ';

	// parsing main loop
	for (LyXParagraph::size_type i = 0;
	     i < par->size(); ++i) {
		LyXFont font2 = par->getFont(params, i);

		// handle <emphasis> tag
		if (font1.emph() != font2.emph() && i) {
			if (font2.emph() == LyXFont::ON) {
				os << "<emphasis>";
				emph_flag = true;
			}else {
				os << "</emphasis>";
				emph_flag = false;
			}
		}
      
		char c = par->GetChar(i);

		if (c == LyXParagraph::META_INSET) {
			Inset * inset = par->GetInset(i);
#ifdef HAVE_SSTREAM
			std::ostringstream ost;
			inset->DocBook(this, ost);
			string tmp_out = ost.str().c_str();
#else
			ostrstream ost;
			inset->DocBook(this, ost);
			ost << '\0';
			char * ctmp = ost.str();
			string tmp_out(ctmp);
			delete [] ctmp;
#endif
			//
			// This code needs some explanation:
			// Two insets are treated specially
			//   label if it is the first element in a command paragraph
			//         desc_on == 3
			//   graphics inside tables or figure floats can't go on
			//   title (the equivalente in latex for this case is caption
			//   and title should come first
			//         desc_on == 4
			//
			if(desc_on!= 3 || i!= 0) {
				if(!tmp_out.empty() && tmp_out[0] == '@') {
					if(desc_on == 4)
						extra += frontStrip(tmp_out, '@');
					else
						os << frontStrip(tmp_out, '@');
				}
				else
					os << tmp_out;
			}
		} else if (font2.latex() == LyXFont::ON) {
			// "TeX"-Mode on ==> SGML-Mode on.
			if (c != '\0')
				os << c;
			++char_line_count;
		} else {
			string sgml_string;
			if (par->linuxDocConvertChar(c, sgml_string)
			    && !style.free_spacing) { // in freespacing
				                     // mode, spaces are
				                     // non-breaking characters
				// char is ' '
				if (desc_on == 1) {
					++char_line_count;
					os << "\n</term><listitem><para>";
					desc_on = 2;
				} else {
					os << c;
				}
			} else {
				os << sgml_string;
			}
		}
		font1 = font2;
	}

	// needed if there is an optional argument but no contents
	if (main_body > 0 && main_body == par->size()) {
		font1 = style.font;
	}
	if (emph_flag) {
		os << "</emphasis>";
	}
	
	// resets description flag correctly
	switch(desc_on){
	case 1:
		// <term> not closed...
		os << "</term>";
		break;
	}
	os << '\n';
}


int Buffer::runLaTeX()
{
	if (!users->text) return 0;

	ProhibitInput(users);

	// get LaTeX-Filename
	string name = getLatexName();

	string path = OnlyPath(filename);

	string org_path = path;
	if (lyxrc.use_tempdir || (IsDirWriteable(path) < 1)) {
		path = tmppath;	 
	}

	Path p(path); // path to LaTeX file
	users->owner()->getMiniBuffer()->Set(_("Running LaTeX..."));   

	// Remove all error insets
	bool a = users->removeAutoInsets();

	// Always generate the LaTeX file
	makeLaTeXFile(name, org_path, false);

	// do the LaTex run(s)
	TeXErrors terr;
	string latex_command = lyxrc.pdf_mode ?
		lyxrc.pdflatex_command : lyxrc.latex_command;
	LaTeX latex(latex_command, name, filepath);
	int res = latex.run(terr,
			    users->owner()->getMiniBuffer()); // running latex

	// check return value from latex.run().
	if ((res & LaTeX::NO_LOGFILE)) {
		WriteAlert(_("LaTeX did not work!"),
			   _("Missing log file:"), name);
	} else if ((res & LaTeX::ERRORS)) {
		users->owner()->getMiniBuffer()->Set(_("Done"));
		// Insert all errors as errors boxes
		users->insertErrors(terr);
		
		// Dvi should also be kept dirty if the latex run
		// ends up with errors. However it should be possible
		// to view a dirty dvi too.
	} else {
		//no errors or any other things to think about so:
		users->owner()->getMiniBuffer()->Set(_("Done"));
	}

	// if we removed error insets before we ran LaTeX or if we inserted
	// error insets after we ran LaTeX this must be run:
        if (a || (res & LaTeX::ERRORS)){
                users->redraw();
                users->fitCursor();
                //users->updateScrollbar();
        }
        AllowInput(users);
 
        return latex.getNumErrors();
}


int Buffer::runLiterate()
{
	if (!users->text) return 0;

	ProhibitInput(users);

	// get LaTeX-Filename
	string name = getLatexName();
        // get Literate-Filename
        string lit_name = OnlyFilename(ChangeExtension (getLatexName(), 
					   lyxrc.literate_extension));

	string path = OnlyPath(filename);

	string org_path = path;
	if (lyxrc.use_tempdir || (IsDirWriteable(path) < 1)) {
		path = tmppath;	 
	}

	Path p(path); // path to Literate file
	users->owner()->getMiniBuffer()->Set(_("Running Literate..."));   

	// Remove all error insets
	bool a = users->removeAutoInsets();

	// generate the Literate file if necessary
	makeLaTeXFile(lit_name, org_path, false);

	string latex_command = lyxrc.pdf_mode ?
		lyxrc.pdflatex_command : lyxrc.latex_command;
        Literate literate(latex_command, name, filepath, 
			  lit_name,
			  lyxrc.literate_command, lyxrc.literate_error_filter,
			  lyxrc.build_command, lyxrc.build_error_filter);
	TeXErrors terr;
	int res = literate.weave(terr, users->owner()->getMiniBuffer());

	// check return value from literate.weave().
	if ((res & Literate::NO_LOGFILE)) {
		WriteAlert(_("Literate command did not work!"),
			   _("Missing log file:"), name);
	} else if ((res & Literate::ERRORS)) {
		users->owner()->getMiniBuffer()->Set(_("Done"));
		// Insert all errors as errors boxes
		users->insertErrors(terr);
		
		// Dvi should also be kept dirty if the latex run
		// ends up with errors. However it should be possible
		// to view a dirty dvi too.
	} else {
		//no errors or any other things to think about so:
		users->owner()->getMiniBuffer()->Set(_("Done"));
	}

	// if we removed error insets before we ran LaTeX or if we inserted
	// error insets after we ran LaTeX this must be run:
        if (a || (res & Literate::ERRORS)){
                users->redraw();
                users->fitCursor();
                //users->updateScrollbar();
        }
        AllowInput(users);
 
        return literate.getNumErrors();
}


int Buffer::buildProgram()
{
        if (!users->text) return 0;
 
        ProhibitInput(users);
 
        // get LaTeX-Filename
        string name = getLatexName();
        // get Literate-Filename
        string lit_name = OnlyFilename(ChangeExtension(getLatexName(), 
						       lyxrc.literate_extension));
 
        string path = OnlyPath(filename);
 
        string org_path = path;
        if (lyxrc.use_tempdir || (IsDirWriteable(path) < 1)) {
                path = tmppath;  
        }
 
        Path p(path); // path to Literate file
        users->owner()->getMiniBuffer()->Set(_("Building Program..."));   
 
        // Remove all error insets
        bool a = users->removeAutoInsets();
 
        // generate the LaTeX file if necessary
        if (!isNwClean() || a) {
                makeLaTeXFile(lit_name, org_path, false);
                markNwDirty();
        }

	string latex_command = lyxrc.pdf_mode ?
		lyxrc.pdflatex_command : lyxrc.latex_command;
        Literate literate(latex_command, name, filepath, 
			  lit_name,
			  lyxrc.literate_command, lyxrc.literate_error_filter,
			  lyxrc.build_command, lyxrc.build_error_filter);
        TeXErrors terr;
        int res = literate.build(terr, users->owner()->getMiniBuffer());
 
        // check return value from literate.build().
        if ((res & Literate::NO_LOGFILE)) {
                WriteAlert(_("Build did not work!"),
                           _("Missing log file:"), name);
        } else if ((res & Literate::ERRORS)) {
                users->owner()->getMiniBuffer()->Set(_("Done"));
                // Insert all errors as errors boxes
		users->insertErrors(terr);
                
                // Literate files should also be kept dirty if the literate 
                // command run ends up with errors.
        } else {
                //no errors or any other things to think about so:
                users->owner()->getMiniBuffer()->Set(_("Done"));
                markNwClean();
        }
 
        // if we removed error insets before we ran Literate/Build or
	// if we inserted error insets after we ran Literate/Build this
	// must be run:
	if (a || (res & Literate::ERRORS)){
		users->redraw();
		users->fitCursor();
		//users->updateScrollbar();
	}
	AllowInput(users);

	return literate.getNumErrors();
}


// This should be enabled when the Chktex class is implemented. (Asger)
// chktex should be run with these flags disabled: 3, 22, 25, 30, 38(?)
// Other flags: -wall -v0 -x
int Buffer::runChktex()
{
	if (!users->text) return 0;

	ProhibitInput(users);

	// get LaTeX-Filename
	string name = getLatexName();
	string path = OnlyPath(filename);

	string org_path = path;
	if (lyxrc.use_tempdir || (IsDirWriteable(path) < 1)) {
		path = tmppath;	 
	}

	Path p(path); // path to LaTeX file
	users->owner()->getMiniBuffer()->Set(_("Running chktex..."));

	// Remove all error insets
	bool removedErrorInsets = users->removeAutoInsets();

	// Generate the LaTeX file if neccessary
	makeLaTeXFile(name, org_path, false);

	TeXErrors terr;
	Chktex chktex(lyxrc.chktex_command, name, filepath);
	int res = chktex.run(terr); // run chktex

	if (res == -1) {
		WriteAlert(_("chktex did not work!"),
			   _("Could not run with file:"), name);
	} else if (res > 0) {
		// Insert all errors as errors boxes
		users->insertErrors(terr);
	}

	// if we removed error insets before we ran chktex or if we inserted
	// error insets after we ran chktex, this must be run:
	if (removedErrorInsets || res){
		users->redraw();
		users->fitCursor();
		//users->updateScrollbar();
	}
	AllowInput(users);

	return res;
}


void Buffer::validate(LaTeXFeatures & features) const
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
			else if ((font >= 2 && font <= 5)) {
				features.pifont = true;
			}
		}
	}
	
	if (lyxerr.debugging(Debug::LATEX)) {
		features.showStruct();
	}
}


void Buffer::setPaperStuff()
{
	params.papersize = BufferParams::PAPER_DEFAULT;
	char c1 = params.paperpackage;
	if (c1 == BufferParams::PACKAGE_NONE) {
		char c2 = params.papersize2;
		if (c2 == BufferParams::VM_PAPER_USLETTER)
			params.papersize = BufferParams::PAPER_USLETTER;
		else if (c2 == BufferParams::VM_PAPER_USLEGAL)
			params.papersize = BufferParams::PAPER_LEGALPAPER;
		else if (c2 == BufferParams::VM_PAPER_USEXECUTIVE)
			params.papersize = BufferParams::PAPER_EXECUTIVEPAPER;
		else if (c2 == BufferParams::VM_PAPER_A3)
			params.papersize = BufferParams::PAPER_A3PAPER;
		else if (c2 == BufferParams::VM_PAPER_A4)
			params.papersize = BufferParams::PAPER_A4PAPER;
		else if (c2 == BufferParams::VM_PAPER_A5)
			params.papersize = BufferParams::PAPER_A5PAPER;
		else if ((c2 == BufferParams::VM_PAPER_B3) || (c2 == BufferParams::VM_PAPER_B4) ||
			 (c2 == BufferParams::VM_PAPER_B5))
			params.papersize = BufferParams::PAPER_B5PAPER;
	} else if ((c1 == BufferParams::PACKAGE_A4) || (c1 == BufferParams::PACKAGE_A4WIDE) ||
		   (c1 == BufferParams::PACKAGE_WIDEMARGINSA4))
		params.papersize = BufferParams::PAPER_A4PAPER;
}


// This function should be in Buffer because it's a buffer's property (ale)
string Buffer::getIncludeonlyList(char delim)
{
	string lst;
	for (inset_iterator it = inset_iterator_begin();
	    it != inset_iterator_end(); ++it) {
		if ((*it)->LyxCode() == Inset::INCLUDE_CODE) {
			InsetInclude * insetinc = 
				static_cast<InsetInclude *>(*it);
			if (insetinc->isInclude() 
			    && insetinc->isNoLoad()) {
				if (!lst.empty())
					lst += delim;
				lst += OnlyFilename(ChangeExtension(insetinc->getContents(), string()));
			}
		}
	}
	lyxerr.debug() << "Includeonly(" << lst << ')' << endl;
	return lst;
}


vector<string> Buffer::getLabelList()
{
	/// if this is a child document and the parent is already loaded
	/// Use the parent's list instead  [ale990407]
	if (!params.parentname.empty()
	    && bufferlist.exists(params.parentname)) {
		Buffer * tmp = bufferlist.getBuffer(params.parentname);
		if (tmp)
			return tmp->getLabelList();
	}

	vector<string> label_list;
	for (inset_iterator it = inset_iterator_begin();
	     it != inset_iterator_end(); ++it) {
		vector<string> l = (*it)->getLabelList();
		label_list.insert(label_list.end(), l.begin(), l.end());
	}
	return label_list;
}


vector<vector<Buffer::TocItem> > Buffer::getTocList()
{
	vector<vector<TocItem> > l(4);
	LyXParagraph * par = paragraph;
	while (par) {
#ifndef NEW_INSETS
		if (par->footnoteflag != LyXParagraph::NO_FOOTNOTE) {
			if (textclasslist.Style(params.textclass, 
						par->GetLayout()).labeltype
			    == LABEL_SENSITIVE) {
				TocItem tmp;
				tmp.par = par;
				tmp.depth = 0;
				tmp.str =  par->String(this, false);
				switch (par->footnotekind) {
				case LyXParagraph::FIG:
				case LyXParagraph::WIDE_FIG:
					l[TOC_LOF].push_back(tmp);
					break;
				case LyXParagraph::TAB:
				case LyXParagraph::WIDE_TAB:
					l[TOC_LOT].push_back(tmp);
					break;
				case LyXParagraph::ALGORITHM:
					l[TOC_LOA].push_back(tmp);
					break;
				case LyXParagraph::FOOTNOTE:
				case LyXParagraph::MARGIN:
					break;
				}
			}
		} else if (!par->IsDummy()) {
#endif
			char labeltype = textclasslist.Style(params.textclass, 
							     par->GetLayout()).labeltype;
      
			if (labeltype >= LABEL_COUNTER_CHAPTER
			    && labeltype <= LABEL_COUNTER_CHAPTER + params.tocdepth) {
				// insert this into the table of contents
				TocItem tmp;
				tmp.par = par;
				tmp.depth = max(0,
						labeltype - 
						textclasslist.TextClass(params.textclass).maxcounter());
				tmp.str =  par->String(this, true);
				l[TOC_TOC].push_back(tmp);
			}
#ifndef NEW_INSETS
		}
#endif
		par = par->next;
	}
	return l;
}

// This is also a buffer property (ale)
vector<pair<string,string> > Buffer::getBibkeyList()
{
	/// if this is a child document and the parent is already loaded
	/// Use the parent's list instead  [ale990412]
        if (!params.parentname.empty() && bufferlist.exists(params.parentname)) {
		Buffer * tmp = bufferlist.getBuffer(params.parentname);
		if (tmp)
			return tmp->getBibkeyList();
	}

	vector<pair<string, string> > keys;
	LyXParagraph * par = paragraph;
	while (par) {
		if (par->bibkey)
			keys.push_back(pair<string, string>(par->bibkey->getContents(),
							   par->String(this, false)));
		par = par->next;
	}

	// Might be either using bibtex or a child has bibliography
	if (keys.empty()) {
		for (inset_iterator it = inset_iterator_begin();
			it != inset_iterator_end(); ++it) {
			// Search for Bibtex or Include inset
			if ((*it)->LyxCode() == Inset::BIBTEX_CODE) {
				vector<pair<string,string> > tmp =
					static_cast<InsetBibtex*>(*it)->getKeys();
				keys.insert(keys.end(), tmp.begin(), tmp.end());
			} else if ((*it)->LyxCode() == Inset::INCLUDE_CODE) {
				vector<pair<string,string> > tmp =
					static_cast<InsetInclude*>(*it)->getKeys();
				keys.insert(keys.end(), tmp.begin(), tmp.end());
			}
		}
	}
 
	return keys;
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
			item->next = 0;
		}
	}
}


bool Buffer::Dispatch(string const & command)
{
	// Split command string into command and argument
	string cmd, line = frontStrip(command);
	string arg = strip(frontStrip(split(line, cmd, ' ')));

	return Dispatch(lyxaction.LookupFunc(cmd.c_str()), arg.c_str());
}


bool Buffer::Dispatch(int action, string const & argument)
{
	bool dispatched = true;
	switch (action) {
		case LFUN_EXPORT: 
			MenuExport(this, argument);
			break;

		default:
			dispatched = false;
	}
	return dispatched;
}


void Buffer::resize()
{
	/// resize the BufferViews!
	if (users)
		users->resize();
}


void Buffer::resizeInsets(BufferView * bv)
{
	/// then remove all LyXText in text-insets
	LyXParagraph * par = paragraph;
	for(;par;par = par->next) {
	    par->resizeInsetsLyXText(bv);
	}
}

void Buffer::ChangeLanguage(Language const * from, Language const * to)
{

	LyXParagraph * par = paragraph;
	while (par) {
		par->ChangeLanguage(params, from, to);
		par = par->next;
	}
}


bool Buffer::isMultiLingual()
{

	LyXParagraph * par = paragraph;
	while (par) {
		if (par->isMultiLingual(params))
			return true;
		par = par->next;
	}
	return false;
}


Buffer::inset_iterator::inset_iterator(LyXParagraph * paragraph,
				       LyXParagraph::size_type pos)
	: par(paragraph) {
	it = par->InsetIterator(pos);
	if (it == par->inset_iterator_end()) {
		par = par->next;
		SetParagraph();
	}
}


void Buffer::inset_iterator::SetParagraph() {
	while (par) {
		it = par->inset_iterator_begin();
		if (it != par->inset_iterator_end())
			return;
		par = par->next;
	}
	//it = 0;
	// We maintain an invariant that whenever par = 0 then it = 0
}
