/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *
 *	     Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 *           This file is Copyright 1996-2001
 *           Lars Gullik Bjønnes
 *
 * ====================================================== 
 */

#include <config.h>

#include <fstream>
#include <iomanip>
#include <map>
#include <stack>
#include <list>

#include <cstdlib>
#include <cmath>
#include <unistd.h>
#include <sys/types.h>
#include <utime.h>

#include <algorithm>

#ifdef HAVE_LOCALE
#include <locale>
#endif

#ifdef __GNUG__
#pragma implementation
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
#include "lyxfont.h"
#include "version.h"
#include "mathed/formulamacro.h"
#include "mathed/formula.h"
#include "insets/inset.h"
#include "insets/inseterror.h"
#include "insets/insetlabel.h"
#include "insets/insetref.h"
#include "insets/inseturl.h"
#include "insets/insetnote.h"
#include "insets/insetquotes.h"
#include "insets/insetlatexaccent.h"
#include "insets/insetbib.h" 
#include "insets/insetcite.h" 
#include "insets/insetexternal.h"
#include "insets/insetindex.h" 
#include "insets/insetinclude.h"
#include "insets/insettoc.h"
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
#include "insets/insettabular.h"
#if 0
#include "insets/insettheorem.h"
#include "insets/insetlist.h"
#endif
#include "insets/insetcaption.h"
#include "insets/insetfloatlist.h"
#include "support/textutils.h"
#include "support/filetools.h"
#include "support/path.h"
#include "support/os.h"
#include "LaTeX.h"
#include "Chktex.h"
#include "LyXView.h"
#include "debug.h"
#include "LaTeXFeatures.h"
#include "support/syscall.h"
#include "support/lyxlib.h"
#include "support/FileInfo.h"
#include "support/lyxmanip.h"
#include "lyxtext.h"
#include "gettext.h"
#include "language.h"
#include "lyx_gui_misc.h"	// WarnReadonly()
#include "frontends/Dialogs.h"
#include "encoding.h"
#include "exporter.h"
#include "Lsstream.h"
#include "converter.h"
#include "BufferView.h"
#include "ParagraphParameters.h"
#include "iterators.h"

using std::ostream;
using std::ofstream;
using std::ifstream;
using std::fstream;
using std::ios;
using std::setw;
using std::endl;
using std::pair;
using std::make_pair;
using std::vector;
using std::map;
using std::max;
using std::set;
using std::stack;
using std::list;

// all these externs should eventually be removed.
extern BufferList bufferlist;

extern LyXAction lyxaction;

namespace {

const int LYX_FORMAT = 220;

} // namespace anon

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
	
	Paragraph * par = paragraph;
	Paragraph * tmppar;
	while (par) {
		tmppar = par->next();
		delete par;
		par = tmppar;
	}
	paragraph = 0;
}


string const Buffer::getLatexName(bool no_path) const
{
	string name = ChangeExtension(MakeLatexName(filename), ".tex");
	if (no_path)
		return OnlyFilename(name);
	else
		return name;
}


pair<Buffer::LogType, string> const Buffer::getLogName(void) const
{
	string const filename = getLatexName(false);

	if (filename.empty())
		return make_pair(Buffer::latexlog, string());

	string path = OnlyPath(filename);

	if (lyxrc.use_tempdir || !IsDirWriteable(path))
		path = tmppath;

	string const fname = AddName(path,
				     OnlyFilename(ChangeExtension(filename,
								  ".log")));
	string const bname =
		AddName(path, OnlyFilename(
			ChangeExtension(filename,
					formats.extension("literate") + ".out")));

	// If no Latex log or Build log is newer, show Build log

	FileInfo const f_fi(fname);
	FileInfo const b_fi(bname);

	if (b_fi.exist() &&
	    (!f_fi.exist() || f_fi.getModificationTime() < b_fi.getModificationTime())) {
		lyxerr[Debug::FILES] << "Log name calculated as : " << bname << endl;
		return make_pair(Buffer::buildlog, bname);
	}
	lyxerr[Debug::FILES] << "Log name calculated as : " << fname << endl;
	return make_pair(Buffer::latexlog, fname);
}


void Buffer::setReadonly(bool flag)
{
	if (read_only != flag) {
		read_only = flag; 
		updateTitles();
		users->owner()->getDialogs()->updateBufferDependent(false);
	}
	if (read_only) {
		WarnReadonly(filename);
	}
}


bool Buffer::saveParamsAsDefaults() // const
{
	string const fname = AddName(AddPath(user_lyxdir, "templates/"),
			       "defaults.lyx");
	Buffer defaults = Buffer(fname);
	
	// Use the current buffer's parameters as default
	defaults.params = params;
	
	// add an empty paragraph. Is this enough?
	defaults.paragraph = new Paragraph;

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


void Buffer::setFileName(string const & newfile)
{
	filename = MakeAbsPath(newfile);
	filepath = OnlyPath(filename);
	setReadonly(IsFileWriteable(filename) == 0);
	updateTitles();
}


// We'll remove this later. (Lgb)
namespace {

string last_inset_read;

#ifndef NO_COMPABILITY
struct ErtComp 
{
	ErtComp() : active(false), in_tabular(false) {
	}
	string contents;
	bool active;
	bool in_tabular;
};

std::stack<ErtComp> ert_stack;
ErtComp ert_comp;
#endif

#warning And _why_ is this here? (Lgb)
int unknown_layouts;

} // anon


// candidate for move to BufferView
// (at least some parts in the beginning of the func)
//
// Uwe C. Schroeder
// changed to be public and have one parameter
// if par = 0 normal behavior
// else insert behavior
// Returns false if "\the_end" is not read for formats >= 2.13. (Asger)
bool Buffer::readLyXformat2(LyXLex & lex, Paragraph * par)
{
	unknown_layouts = 0;
#ifndef NO_COMPABILITY
	ert_comp.contents.erase();
	ert_comp.active = false;
	ert_comp.in_tabular = false;
#endif
	int pos = 0;
	Paragraph::depth_type depth = 0; 
	bool the_end_read = false;

	Paragraph * first_par = 0;
	LyXFont font(LyXFont::ALL_INHERIT, params.language);
	if (file_format < 216 && params.language->lang() == "hebrew")
		font.setLanguage(default_language);

	if (!par) {
		par = new Paragraph;
	} else {
		// We are inserting into an existing document
		users->text->breakParagraph(users);
		first_par = users->text->firstParagraph();
		pos = 0;
		markDirty();
		// We don't want to adopt the parameters from the
		// document we insert, so we skip until the text begins:
		while (lex.isOK()) {
			lex.nextToken();
			string const pretoken = lex.getString();
			if (pretoken == "\\layout") {
				lex.pushToken(pretoken);
				break;
			}
		}
	}

	while (lex.isOK()) {
		lex.nextToken();
		string const token = lex.getString();

		if (token.empty()) continue;

		lyxerr[Debug::PARSER] << "Handling token: `"
				      << token << "'" << endl;
		
		the_end_read =
			parseSingleLyXformat2Token(lex, par, first_par,
						   token, pos, depth,
						   font);
	}
   
	if (!first_par)
		first_par = par;

	paragraph = first_par;

	if (unknown_layouts > 0) {
		string s = _("Couldn't set the layout for ");
		if (unknown_layouts == 1) {
			s += _("one paragraph");
		} else {
			s += tostr(unknown_layouts);
			s += _(" paragraphs");
		}
		WriteAlert(_("Textclass Loading Error!"), s,
			   _("When reading " + fileName()));
	}	

	return the_end_read;
}


#ifndef NO_COMPABILITY
void Buffer::insertErtContents(Paragraph * par, int & pos,
			       LyXFont const & font, bool set_inactive) 
{
	if (!ert_comp.contents.empty()) {
		lyxerr[Debug::INSETS] << "ERT contents:\n"
		       << ert_comp.contents << endl;
		Inset * inset = new InsetERT(ert_comp.contents, true);
		par->insertInset(pos++, inset, font);
		ert_comp.contents.erase();
	}
	if (set_inactive) {
		ert_comp.active = false;
	}
}
#endif


bool
Buffer::parseSingleLyXformat2Token(LyXLex & lex, Paragraph *& par,
				   Paragraph *& first_par,
				   string const & token, int & pos,
				   Paragraph::depth_type & depth, 
				   LyXFont & font
	)
{
	bool the_end_read = false;
#ifndef NO_COMPABILITY
#ifndef NO_PEXTRA_REALLY
	// This is super temporary but is needed to get the compability
	// mode for minipages work correctly together with new tabulars.
	static int call_depth;
	++call_depth;
	bool checkminipage = false;
	static Paragraph * minipar;
	static Paragraph * parBeforeMinipage;
#endif
#endif
	if (token[0] != '\\') {
#ifndef NO_COMPABILITY
		if (ert_comp.active) {
			ert_comp.contents += token;
		} else {
#endif
		for (string::const_iterator cit = token.begin();
		     cit != token.end(); ++cit) {
			par->insertChar(pos, (*cit), font);
			++pos;
		}
#ifndef NO_COMPABILITY
		}
#endif
	} else if (token == "\\i") {
		Inset * inset = new InsetLatexAccent;
		inset->read(this, lex);
		par->insertInset(pos, inset, font);
		++pos;
	} else if (token == "\\layout") {
#ifndef NO_COMPABILITY
		ert_comp.in_tabular = false;
		// Do the insetert.
		insertErtContents(par, pos, font);
#endif
                lex.eatLine();
                string const layoutname = lex.getString();
                pair<bool, LyXTextClass::LayoutList::size_type> pp
                        = textclasslist.NumberOfLayout(params.textclass,
                                                       layoutname);

#ifndef NO_COMPABILITY
		if (compare_no_case(layoutname, "latex") == 0) {
			ert_comp.active = true;
		}
#endif
#ifdef USE_CAPTION
		// The is the compability reading of layout caption.
		// It can be removed in LyX version 1.3.0. (Lgb)
		if (compare_no_case(layoutname, "caption") == 0) {
			// We expect that the par we are now working on is
			// really inside a InsetText inside a InsetFloat.
			// We also know that captions can only be
			// one paragraph. (Lgb)
			
			// We should now read until the next "\layout"
			// is reached.
			// This is probably not good enough, what if the
			// caption is the last par in the document (Lgb)
			istream & ist = lex.getStream();
			stringstream ss;
			string line;
			int begin = 0;
			while (true) {
				getline(ist, line);
				if (prefixIs(line, "\\layout")) {
					lex.pushToken(line);
					break;
				}
				if (prefixIs(line, "\\begin_inset"))
					++begin;
				if (prefixIs(line, "\\end_inset")) {
					if (begin)
						--begin;
					else {
						lex.pushToken(line);
						break;
					}
				}
				
				ss << line << '\n';
			}
			// Now we should have the whole layout in ss
			// we should now be able to give this to the
			// caption inset.
			ss << "\\end_inset\n";
			
			// This seems like a bug in stringstream.
			// We really should be able to use ss
			// directly. (Lgb)
			istringstream is(ss.str());
			LyXLex tmplex(0, 0);
			tmplex.setStream(is);
			Inset * inset = new InsetCaption;
			inset->Read(this, tmplex);
			par->InsertInset(pos, inset, font);
			++pos;
		} else {
#endif
			if (!first_par)
				first_par = par;
			else {
				par = new Paragraph(par);
			}
			pos = 0;
			if (pp.first) {
				par->layout = pp.second;
#ifndef NO_COMPABILITY
			} else if (ert_comp.active) {
				par->layout = 0;
#endif
			} else {
				// layout not found
				// use default layout "Standard" (0)
				par->layout = 0;
				++unknown_layouts;
				string const s = _("Layout had to be changed from\n")
					+ layoutname + _(" to ")
					+ textclasslist.NameOfLayout(params.textclass, par->layout);
				InsetError * new_inset = new InsetError(s);
				par->insertInset(0, new_inset);
			}
			// Test whether the layout is obsolete.
			LyXLayout const & layout =
				textclasslist.Style(params.textclass,
						    par->layout);
			if (!layout.obsoleted_by().empty())
				par->layout = textclasslist
					.NumberOfLayout(params.textclass,
							layout.obsoleted_by())
					.second;
			par->params().depth(depth);
			font = LyXFont(LyXFont::ALL_INHERIT, params.language);
			if (file_format < 216
			    && params.language->lang() == "hebrew")
				font.setLanguage(default_language);
#if USE_CAPTION
                }
#endif

#ifndef NO_COMPABILITY
	} else if (token == "\\begin_float") {
		insertErtContents(par, pos, font);
		//insertErtContents(par, pos, font, false);
		//ert_stack.push(ert_comp);
		//ert_comp = ErtComp();
		
		// This is the compability reader. It can be removed in
		// LyX version 1.3.0. (Lgb)
		lex.next();
		string const tmptok = lex.getString();
		//lyxerr << "old float: " << tmptok << endl;
		
		Inset * inset = 0;
		stringstream old_float;
		
		if (tmptok == "footnote") {
			inset = new InsetFoot;
			old_float << "collapsed true\n";
		} else if (tmptok == "margin") {
			inset = new InsetMarginal;
			old_float << "collapsed true\n";
		} else if (tmptok == "fig") {
			inset = new InsetFloat("figure");
			old_float << "placement htbp\n"
				  << "wide false\n"
				  << "collapsed false\n";
		} else if (tmptok == "tab") {
			inset = new InsetFloat("table");
			old_float << "placement htbp\n"
				  << "wide false\n"
				  << "collapsed false\n";
		} else if (tmptok == "alg") {
			inset = new InsetFloat("algorithm");
			old_float << "placement htbp\n"
				  << "wide false\n"
				  << "collapsed false\n";
		} else if (tmptok == "wide-fig") {
			inset = new InsetFloat("figure");
			//InsetFloat * tmp = new InsetFloat("figure");
			//tmp->wide(true);
			//inset = tmp;
			old_float << "placement htbp\n"
				  << "wide true\n"
				  << "collapsed false\n";
		} else if (tmptok == "wide-tab") {
			inset = new InsetFloat("table");
			//InsetFloat * tmp = new InsetFloat("table");
			//tmp->wide(true);
			//inset = tmp;
			old_float << "placement htbp\n"
				  << "wide true\n"
				  << "collapsed false\n";
		}

		if (!inset) {
#ifndef NO_PEXTRA_REALLY
			--call_depth;
#endif
			return false; // no end read yet
		}

		// Here we need to check for \end_deeper and handle that
		// before we do the footnote parsing.
		// This _is_ a hack! (Lgb)
		while (true) {
			lex.next();
			string const tmp = lex.getString();
			if (tmp == "\\end_deeper") {
				//lyxerr << "\\end_deeper caught!" << endl;
				if (!depth) {
					lex.printError("\\end_deeper: "
						       "depth is already null");
				} else
					--depth;
				
			} else {
				old_float << tmp << ' ';
				break;
			}
		}
		
		old_float << lex.getLongString("\\end_float")
			  << "\n\\end_inset\n";
		//lyxerr << "Float Body:\n" << old_float.str() << endl;
		// That this does not work seems like a bug
		// in stringstream. (Lgb)
		istringstream istr(old_float.str());
		LyXLex nylex(0, 0);
		nylex.setStream(istr);
		inset->read(this, nylex);
		par->insertInset(pos, inset, font);
		++pos;
		insertErtContents(par, pos, font);
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
		lex.eatLine();
		pair<bool, LyXTextClassList::size_type> pp = 
			textclasslist.NumberOfClass(lex.getString());
		if (pp.first) {
			params.textclass = pp.second;
		} else {
			WriteAlert(string(_("Textclass error")), 
				string(_("The document uses an unknown textclass \"")) + 
				lex.getString() + string("\"."),
				string(_("LyX will not be able to produce output correctly.")));
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
		lex.eatLine();
		params.options = lex.getString();
	} else if (token == "\\language") {
		params.readLanguage(lex);    
	} else if (token == "\\fontencoding") {
		lex.eatLine();
	} else if (token == "\\inputencoding") {
		lex.eatLine();
		params.inputenc = lex.getString();
	} else if (token == "\\graphics") {
		params.readGraphicsDriver(lex);
	} else if (token == "\\fontscheme") {
		lex.eatLine();
		params.fonts = lex.getString();
	} else if (token == "\\noindent") {
		par->params().noindent(true);
	} else if (token == "\\fill_top") {
		par->params().spaceTop(VSpace(VSpace::VFILL));
	} else if (token == "\\fill_bottom") {
		par->params().spaceBottom(VSpace(VSpace::VFILL));
	} else if (token == "\\line_top") {
		par->params().lineTop(true);
	} else if (token == "\\line_bottom") {
		par->params().lineBottom(true);
	} else if (token == "\\pagebreak_top") {
		par->params().pagebreakTop(true);
	} else if (token == "\\pagebreak_bottom") {
		par->params().pagebreakBottom(true);
	} else if (token == "\\start_of_appendix") {
		par->params().startOfAppendix(true);
	} else if (token == "\\paragraph_separation") {
		int tmpret = lex.findToken(string_paragraph_separation);
		if (tmpret == -1) ++tmpret;
		if (tmpret != LYX_LAYOUT_DEFAULT) 
			params.paragraph_separation =
				static_cast<BufferParams::PARSEP>(tmpret);
	} else if (token == "\\defskip") {
		lex.nextToken();
		params.defskip = VSpace(lex.getString());
	} else if (token == "\\epsfig") { // obsolete
		// Indeed it is obsolete, but we HAVE to be backwards
		// compatible until 0.14, because otherwise all figures
		// in existing documents are irretrivably lost. (Asger)
		params.readGraphicsDriver(lex);
	} else if (token == "\\quotes_language") {
		int tmpret = lex.findToken(string_quotes_language);
		if (tmpret == -1) ++tmpret;
		if (tmpret != LYX_LAYOUT_DEFAULT) {
			InsetQuotes::quote_language tmpl = 
				InsetQuotes::EnglishQ;
			switch (tmpret) {
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
		switch (lex.getInteger()) {
		case 1: 
			params.quotes_times = InsetQuotes::SingleQ; 
			break;
		case 2: 
			params.quotes_times = InsetQuotes::DoubleQ; 
			break;
		}
	} else if (token == "\\papersize") {
		int tmpret = lex.findToken(string_papersize);
		if (tmpret == -1)
			++tmpret;
		else
			params.papersize2 = tmpret;
	} else if (token == "\\paperpackage") {
		int tmpret = lex.findToken(string_paperpackages);
		if (tmpret == -1) {
			++tmpret;
			params.paperpackage = BufferParams::PACKAGE_NONE;
		} else
			params.paperpackage = tmpret;
	} else if (token == "\\use_geometry") {
		lex.nextToken();
		params.use_geometry = lex.getInteger();
	} else if (token == "\\use_amsmath") {
		lex.nextToken();
		params.use_amsmath = lex.getInteger();
	} else if (token == "\\use_natbib") {
		lex.nextToken();
		params.use_natbib = lex.getInteger();
	} else if (token == "\\use_numerical_citations") {
		lex.nextToken();
		params.use_numerical_citations = lex.getInteger();
	} else if (token == "\\paperorientation") {
		int tmpret = lex.findToken(string_orientation);
		if (tmpret == -1) ++tmpret;
		if (tmpret != LYX_LAYOUT_DEFAULT) 
			params.orientation = static_cast<BufferParams::PAPER_ORIENTATION>(tmpret);
	} else if (token == "\\paperwidth") {
		lex.next();
		params.paperwidth = lex.getString();
	} else if (token == "\\paperheight") {
		lex.next();
		params.paperheight = lex.getString();
	} else if (token == "\\leftmargin") {
		lex.next();
		params.leftmargin = lex.getString();
	} else if (token == "\\topmargin") {
		lex.next();
		params.topmargin = lex.getString();
	} else if (token == "\\rightmargin") {
		lex.next();
		params.rightmargin = lex.getString();
	} else if (token == "\\bottommargin") {
		lex.next();
		params.bottommargin = lex.getString();
	} else if (token == "\\headheight") {
		lex.next();
		params.headheight = lex.getString();
	} else if (token == "\\headsep") {
		lex.next();
		params.headsep = lex.getString();
	} else if (token == "\\footskip") {
		lex.next();
		params.footskip = lex.getString();
	} else if (token == "\\paperfontsize") {
		lex.nextToken();
		params.fontsize = strip(lex.getString());
	} else if (token == "\\papercolumns") {
		lex.nextToken();
		params.columns = lex.getInteger();
	} else if (token == "\\papersides") {
		lex.nextToken();
		switch (lex.getInteger()) {
		default:
		case 1: params.sides = LyXTextClass::OneSide; break;
		case 2: params.sides = LyXTextClass::TwoSides; break;
		}
	} else if (token == "\\paperpagestyle") {
		lex.nextToken();
		params.pagestyle = strip(lex.getString());
	} else if (token == "\\bullet") {
		lex.nextToken();
		int const index = lex.getInteger();
		lex.nextToken();
		int temp_int = lex.getInteger();
		params.user_defined_bullets[index].setFont(temp_int);
		params.temp_bullets[index].setFont(temp_int);
		lex.nextToken();
		temp_int = lex.getInteger();
		params.user_defined_bullets[index].setCharacter(temp_int);
		params.temp_bullets[index].setCharacter(temp_int);
		lex.nextToken();
		temp_int = lex.getInteger();
		params.user_defined_bullets[index].setSize(temp_int);
		params.temp_bullets[index].setSize(temp_int);
		lex.nextToken();
		string const temp_str = lex.getString();
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
		int const index = lex.getInteger();
		lex.next();
		string temp_str = lex.getString();
		string sum_str;
		while (temp_str != "\\end_bullet") {
				// this loop structure is needed when user
				// enters an empty string since the first
				// thing returned will be the \\end_bullet
				// OR
				// if the LaTeX entry has spaces. Each element
				// therefore needs to be read in turn
			sum_str += temp_str;
			lex.next();
			temp_str = lex.getString();
		}
		params.user_defined_bullets[index].setText(sum_str);
		params.temp_bullets[index].setText(sum_str);
	} else if (token == "\\secnumdepth") {
		lex.nextToken();
		params.secnumdepth = lex.getInteger();
	} else if (token == "\\tocdepth") {
		lex.nextToken();
		params.tocdepth = lex.getInteger();
	} else if (token == "\\spacing") {
		lex.next();
		string const tmp = strip(lex.getString());
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
			tmp_val = lex.getFloat();
		} else {
			lex.printError("Unknown spacing token: '$$Token'");
		}
		// Small hack so that files written with klyx will be
		// parsed correctly.
		if (first_par) {
			par->params().spacing(Spacing(tmp_space, tmp_val));
		} else {
			params.spacing.set(tmp_space, tmp_val);
		}
	} else if (token == "\\paragraph_spacing") {
		lex.next();
		string const tmp = strip(lex.getString());
		if (tmp == "single") {
			par->params().spacing(Spacing(Spacing::Single));
		} else if (tmp == "onehalf") {
			par->params().spacing(Spacing(Spacing::Onehalf));
		} else if (tmp == "double") {
			par->params().spacing(Spacing(Spacing::Double));
		} else if (tmp == "other") {
			lex.next();
			par->params().spacing(Spacing(Spacing::Other,
					 lex.getFloat()));
		} else {
			lex.printError("Unknown spacing token: '$$Token'");
		}
	} else if (token == "\\float_placement") {
		lex.nextToken();
		params.float_placement = lex.getString();
	} else if (token == "\\family") { 
		lex.next();
		font.setLyXFamily(lex.getString());
	} else if (token == "\\series") {
		lex.next();
		font.setLyXSeries(lex.getString());
	} else if (token == "\\shape") {
		lex.next();
		font.setLyXShape(lex.getString());
	} else if (token == "\\size") {
		lex.next();
		font.setLyXSize(lex.getString());
#ifndef NO_COMPABILITY
	} else if (token == "\\latex") {
		lex.next();
		string const tok = lex.getString();
		if (tok == "no_latex") {
			// Do the insetert.
			insertErtContents(par, pos, font);
		} else if (tok == "latex") {
			ert_comp.active = true;
		} else if (tok == "default") {
			// Do the insetert.
			insertErtContents(par, pos, font);
		} else {
			lex.printError("Unknown LaTeX font flag "
				       "`$$Token'");
		}
#endif
	} else if (token == "\\lang") {
		lex.next();
		string const tok = lex.getString();
		Language const * lang = languages.getLanguage(tok);
		if (lang) {
			font.setLanguage(lang);
		} else {
			font.setLanguage(params.language);
			lex.printError("Unknown language `$$Token'");
		}
	} else if (token == "\\numeric") {
		lex.next();
		font.setNumber(font.setLyXMisc(lex.getString()));
	} else if (token == "\\emph") {
		lex.next();
		font.setEmph(font.setLyXMisc(lex.getString()));
	} else if (token == "\\bar") {
		lex.next();
		string const tok = lex.getString();
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
		font.setNoun(font.setLyXMisc(lex.getString()));
	} else if (token == "\\color") {
		lex.next();
		font.setLyXColor(lex.getString());
	} else if (token == "\\align") {
		int tmpret = lex.findToken(string_align);
		if (tmpret == -1) ++tmpret;
		if (tmpret != LYX_LAYOUT_DEFAULT) { // tmpret != 99 ???
			int const tmpret2 = int(pow(2.0, tmpret));
			//lyxerr << "Tmpret2 = " << tmpret2 << endl;
			par->params().align(LyXAlignment(tmpret2));
		}
	} else if (token == "\\added_space_top") {
		lex.nextToken();
		par->params().spaceTop(VSpace(lex.getString()));
	} else if (token == "\\added_space_bottom") {
		lex.nextToken();
		par->params().spaceBottom(VSpace(lex.getString()));
#ifndef NO_COMPABILITY
#ifndef NO_PEXTRA_REALLY
	} else if (token == "\\pextra_type") {
		lex.nextToken();
		par->params().pextraType(lex.getInteger());
	} else if (token == "\\pextra_width") {
		lex.nextToken();
		par->params().pextraWidth(lex.getString());
	} else if (token == "\\pextra_widthp") {
		lex.nextToken();
		par->params().pextraWidthp(lex.getString());
	} else if (token == "\\pextra_alignment") {
		lex.nextToken();
		par->params().pextraAlignment(lex.getInteger());
	} else if (token == "\\pextra_hfill") {
		lex.nextToken();
		par->params().pextraHfill(lex.getInteger());
	} else if (token == "\\pextra_start_minipage") {
		lex.nextToken();
		par->params().pextraStartMinipage(lex.getInteger());
#endif
#endif
	} else if (token == "\\labelwidthstring") {
		lex.eatLine();
		par->params().labelWidthString(lex.getString());
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
#ifndef NO_COMPABILITY
		insertErtContents(par, pos, font, false);
		ert_stack.push(ert_comp);
		ert_comp = ErtComp();
#endif
		readInset(lex, par, pos, font);
#ifndef NO_COMPABILITY
		ert_comp = ert_stack.top();
		ert_stack.pop();
		insertErtContents(par, pos, font);
#endif
	} else if (token == "\\SpecialChar") {
		LyXLayout const & layout =
			textclasslist.Style(params.textclass, 
					    par->getLayout());

		// Insets don't make sense in a free-spacing context! ---Kayvan
		if (layout.free_spacing) {
			if (lex.isOK()) {
				lex.next();
				string next_token = lex.getString();
				if (next_token == "\\-") {
					par->insertChar(pos, '-', font);
				} else if (next_token == "\\protected_separator"
					|| next_token == "~") {
					par->insertChar(pos, ' ', font);
				} else {
					lex.printError("Token `$$Token' "
						       "is in free space "
						       "paragraph layout!");
					--pos;
				}
			}
		} else {
			Inset * inset = new InsetSpecialChar;
			inset->read(this, lex);
			par->insertInset(pos, inset, font);
		}
		++pos;
	} else if (token == "\\newline") {
#ifndef NO_COMPABILITY
		if (!ert_comp.in_tabular && ert_comp.active) {
			ert_comp.contents += char(Paragraph::META_NEWLINE);
		} else {
			// Since we cannot know it this is only a regular
			// newline or a tabular cell delimter we have to
			// handle the ERT here.
			insertErtContents(par, pos, font, false);

			par->insertChar(pos, Paragraph::META_NEWLINE, font);
			++pos;
		}
#else
		par->insertChar(pos, Paragraph::META_NEWLINE, font);
		++pos;
#endif
	} else if (token == "\\LyXTable") {
#ifndef NO_COMPABILITY
		ert_comp.in_tabular = true;
#endif
		Inset * inset = new InsetTabular(*this);
		inset->read(this, lex);
		par->insertInset(pos, inset, font);
		++pos;
	} else if (token == "\\hfill") {
		par->insertChar(pos, Paragraph::META_HFILL, font);
		++pos;
	} else if (token == "\\protected_separator") { // obsolete
		// This is a backward compability thingie. (Lgb)
		// Remove it later some time...introduced with fileformat
		// 2.16. (Lgb)
		LyXLayout const & layout =
			textclasslist.Style(params.textclass, 
					    par->getLayout());

		if (layout.free_spacing) {
			par->insertChar(pos, ' ', font);
		} else {
			Inset * inset = new InsetSpecialChar(InsetSpecialChar::PROTECTED_SEPARATOR);
			par->insertInset(pos, inset, font);
		}
		++pos;
	} else if (token == "\\bibitem") {  // ale970302
		if (!par->bibkey) {
			InsetCommandParams p("bibitem", "dummy");
			par->bibkey = new InsetBibKey(p);
		}
		par->bibkey->read(this, lex);		        
	} else if (token == "\\backslash") {
#ifndef NO_COMPABILITY
		if (ert_comp.active) {
			ert_comp.contents += "\\";
		} else {
#endif
		par->insertChar(pos, '\\', font);
		++pos;
#ifndef NO_COMPABILITY
		}
#endif
	} else if (token == "\\the_end") {
#ifndef NO_COMPABILITY
		// If we still have some ert active here we have to insert
		// it so we don't loose it. (Lgb)
		insertErtContents(par, pos, font);
#endif
		the_end_read = true;
#ifndef NO_COMPABILITY
#ifndef NO_PEXTRA_REALLY
		minipar = parBeforeMinipage = 0;
#endif
#endif
	} else {
#ifndef NO_COMPABILITY
		if (ert_comp.active) {
			ert_comp.contents += token;
		} else {
#endif
		// This should be insurance for the future: (Asger)
		lex.printError("Unknown token `$$Token'. "
			       "Inserting as text.");
		string::const_iterator cit = token.begin();
		string::const_iterator end = token.end();
		for (; cit != end; ++cit) {
			par->insertChar(pos, (*cit), font);
			++pos;
		}
#ifndef NO_COMPABILITY
		}
#endif
	}

#ifndef NO_COMPABILITY
#ifndef NO_PEXTRA_REALLY
	// I wonder if we could use this blanket fix for all the
	// checkminipage cases...
	if (par && par->size()) {
		// It is possible that this will check to often,
		// but that should not be an correctness issue.
		// Only a speed issue.
		checkminipage = true;
	}
	
	// now check if we have a minipage paragraph as at this
	// point we already read all the necessary data!
	// this cannot be done in layout because there we did
	// not read yet the paragraph PEXTRA-params (Jug)
	//
	// BEGIN pextra_minipage compability
	// This should be removed in 1.3.x (Lgb)
	
	// This compability code is not perfect. In a couple
	// of rand cases it fails. When the minipage par is
	// the first par in the document, and when there are
	// none or only one regular paragraphs after the
	// minipage. Currently I am not investing any effort
	// in fixing those cases.

	//lyxerr << "Call depth: " << call_depth << endl;
	if (checkminipage && (call_depth == 1)) {
	checkminipage = false;
	if (minipar && (minipar != par) &&
	    (par->params().pextraType()==Paragraph::PEXTRA_MINIPAGE))
	{
		lyxerr << "minipages in a row" << endl;
		if (par->params().pextraStartMinipage()) {
			lyxerr << "start new minipage" << endl;
			// minipages in a row
			par->previous()->next(0);
			par->previous(0);
				
			Paragraph * tmp = minipar;
			while (tmp) {
				tmp->params().pextraType(0);
				tmp->params().pextraWidth(string());
				tmp->params().pextraWidthp(string());
				tmp->params().pextraAlignment(0);
				tmp->params().pextraHfill(false);
				tmp->params().pextraStartMinipage(false);
				tmp = tmp->next();
			}
			// create a new paragraph to insert the
			// minipages in the following case
			if (par->params().pextraStartMinipage() &&
			    !par->params().pextraHfill())
			{
				Paragraph * p = new Paragraph;
				p->layout = 0;
				p->previous(parBeforeMinipage);
				parBeforeMinipage->next(p);
				p->next(0);
				p->params().depth(parBeforeMinipage->params().depth());
				parBeforeMinipage = p;
			}
			InsetMinipage * mini = new InsetMinipage;
			mini->pos(static_cast<InsetMinipage::Position>(par->params().pextraAlignment()));
			mini->width(par->params().pextraWidth());
			if (!par->params().pextraWidthp().empty()) {
			    lyxerr << "WP:" << mini->width() << endl;
			    mini->width(tostr(par->params().pextraWidthp())+"%");
			}
			mini->inset.paragraph(par);
			// Insert the minipage last in the
			// previous paragraph.
			if (par->params().pextraHfill()) {
				parBeforeMinipage->insertChar
					(parBeforeMinipage->size(), Paragraph::META_HFILL);
			}
			parBeforeMinipage->insertInset
				(parBeforeMinipage->size(), mini);
				
			minipar = par;
		} else {
			lyxerr << "new minipage par" << endl;
			//nothing to do just continue reading
		}
			
	} else if (minipar && (minipar != par)) {
		lyxerr << "last minipage par read" << endl;
		// The last paragraph read was not part of a
		// minipage but the par linked list is...
		// So we need to remove the last par from the
		// rest
		if (par->previous())
			par->previous()->next(0);
		par->previous(parBeforeMinipage);
		parBeforeMinipage->next(par);
		Paragraph * tmp = minipar;
		while (tmp) {
			tmp->params().pextraType(0);
			tmp->params().pextraWidth(string());
			tmp->params().pextraWidthp(string());
			tmp->params().pextraAlignment(0);
			tmp->params().pextraHfill(false);
			tmp->params().pextraStartMinipage(false);
			tmp = tmp->next();
		}
		depth = parBeforeMinipage->params().depth();
		minipar = parBeforeMinipage = 0;
	} else if (!minipar &&
		   (par->params().pextraType() == Paragraph::PEXTRA_MINIPAGE))
	{
		// par is the first paragraph in a minipage
		lyxerr << "begin minipage" << endl;
		// To minimize problems for
		// the users we will insert
		// the first minipage in
		// a sequence of minipages
		// in its own paragraph.
		Paragraph * p = new Paragraph;
		p->layout = 0;
		p->previous(par->previous());
		p->next(0);
		p->params().depth(depth);
		par->params().depth(0);
		depth = 0;
		if (par->previous())
			par->previous()->next(p);
		par->previous(0);
		parBeforeMinipage = p;
		minipar = par;
		if (!first_par || (first_par == par))
			first_par = p;

		InsetMinipage * mini = new InsetMinipage;
		mini->pos(static_cast<InsetMinipage::Position>(minipar->params().pextraAlignment()));
		mini->width(minipar->params().pextraWidth());
		if (!par->params().pextraWidthp().empty()) {
		    lyxerr << "WP:" << mini->width() << endl;
		    mini->width(tostr(par->params().pextraWidthp())+"%");
		}
		mini->inset.paragraph(minipar);
			
		// Insert the minipage last in the
		// previous paragraph.
		if (minipar->params().pextraHfill()) {
			parBeforeMinipage->insertChar
				(parBeforeMinipage->size(),Paragraph::META_HFILL);
		}
		parBeforeMinipage->insertInset
			(parBeforeMinipage->size(), mini);
	}
	}
	// End of pextra_minipage compability
	--call_depth;
#endif
#endif
	return the_end_read;
}

// needed to insert the selection
void Buffer::insertStringAsLines(Paragraph *& par, Paragraph::size_type & pos,
                                 LyXFont const & fn,string const & str) const
{
	LyXLayout const & layout = textclasslist.Style(params.textclass, 
	                                               par->getLayout());
	LyXFont font = fn;
	
	(void)par->checkInsertChar(font);
	// insert the string, don't insert doublespace
	bool space_inserted = true;
	for(string::const_iterator cit = str.begin(); 
	    cit != str.end(); ++cit) {
		if (*cit == '\n') {
			if (par->size() || layout.keepempty) { 
				par->breakParagraph(params, pos, 
				                    layout.isEnvironment());
				par = par->next();
				pos = 0;
				space_inserted = true;
			} else {
				continue;
			}
			// do not insert consecutive spaces if !free_spacing
		} else if ((*cit == ' ' || *cit == '\t') &&
		           space_inserted && !layout.free_spacing)
		{
			continue;
		} else if (*cit == '\t') {
			if (!layout.free_spacing) {
				// tabs are like spaces here
				par->insertChar(pos, ' ', font);
				++pos;
				space_inserted = true;
			} else {
				const Paragraph::size_type nb = 8 - pos % 8;
				for (Paragraph::size_type a = 0; 
				     a < nb ; ++a) {
					par->insertChar(pos, ' ', font);
					++pos;
				}
				space_inserted = true;
			}
		} else if (!IsPrintable(*cit)) {
			// Ignore unprintables
			continue;
		} else {
			// just insert the character
			par->insertChar(pos, *cit, font);
			++pos;
			space_inserted = (*cit == ' ');
		}

	}	
}


void Buffer::readInset(LyXLex & lex, Paragraph *& par,
		       int & pos, LyXFont & font)
{
	// consistency check
	if (lex.getString() != "\\begin_inset") {
		lyxerr << "Buffer::readInset: Consistency check failed."
		       << endl;
	}
	
	Inset * inset = 0;

	lex.next();
	string const tmptok = lex.getString();
	last_inset_read = tmptok;

	// test the different insets
	if (tmptok == "LatexCommand") {
		InsetCommandParams inscmd;
		inscmd.read(lex);

		string const cmdName = inscmd.getCmdName();
		
		// This strange command allows LyX to recognize "natbib" style
		// citations: citet, citep, Citet etc.
		if (compare_no_case(cmdName, "cite", 4) == 0) {
			inset = new InsetCitation(inscmd);
		} else if (cmdName == "bibitem") {
			lex.printError("Wrong place for bibitem");
			inset = new InsetBibKey(inscmd);
		} else if (cmdName == "BibTeX") {
			inset = new InsetBibtex(inscmd);
		} else if (cmdName == "index") {
			inset = new InsetIndex(inscmd);
		} else if (cmdName == "include") {
			inset = new InsetInclude(inscmd, *this);
		} else if (cmdName == "label") {
			inset = new InsetLabel(inscmd);
		} else if (cmdName == "url"
			   || cmdName == "htmlurl") {
			inset = new InsetUrl(inscmd);
		} else if (cmdName == "ref"
			   || cmdName == "pageref"
			   || cmdName == "vref"
			   || cmdName == "vpageref"
			   || cmdName == "prettyref") {
			if (!inscmd.getOptions().empty()
			    || !inscmd.getContents().empty()) {
				inset = new InsetRef(inscmd, *this);
			}
		} else if (cmdName == "tableofcontents") {
			inset = new InsetTOC(inscmd);
		} else if (cmdName == "listofalgorithms") {
			inset = new InsetFloatList("algorithm");
		} else if (cmdName == "listoffigures") {
			inset = new InsetFloatList("figure");
		} else if (cmdName == "listoftables") {
			inset = new InsetFloatList("table");
		} else if (cmdName == "printindex") {
			inset = new InsetPrintIndex(inscmd);
		} else if (cmdName == "lyxparent") {
			inset = new InsetParent(inscmd, *this);
		}
	} else {
		bool alreadyread = false;
		if (tmptok == "Quotes") {
			inset = new InsetQuotes;
		} else if (tmptok == "External") {
			inset = new InsetExternal;
		} else if (tmptok == "FormulaMacro") {
			inset = new InsetFormulaMacro;
		} else if (tmptok == "Formula") {
			inset = new InsetFormula;
		} else if (tmptok == "Figure") { // Backward compatibility
			inset = new InsetFig(100, 100, *this);
			//inset = new InsetGraphics;
		} else if (tmptok == "Graphics") {
			inset = new InsetGraphics;
		} else if (tmptok == "Info") {// backwards compatibility
			inset = new InsetNote(this,
					      lex.getLongString("\\end_inset"),
					      true);
			alreadyread = true;
		} else if (tmptok == "Note") {
			inset = new InsetNote;
		} else if (tmptok == "Include") {
			InsetCommandParams p( "Include" );
			inset = new InsetInclude(p, *this);
		} else if (tmptok == "ERT") {
			inset = new InsetERT;
		} else if (tmptok == "Tabular") {
			inset = new InsetTabular(*this);
		} else if (tmptok == "Text") {
			inset = new InsetText;
		} else if (tmptok == "Foot") {
			inset = new InsetFoot;
		} else if (tmptok == "Marginal") {
			inset = new InsetMarginal;
		} else if (tmptok == "Minipage") {
			inset = new InsetMinipage;
		} else if (tmptok == "Float") {
			lex.next();
			string tmptok = lex.getString();
			inset = new InsetFloat(tmptok);
#if 0
		} else if (tmptok == "List") {
			inset = new InsetList;
		} else if (tmptok == "Theorem") {
			inset = new InsetList;
#endif
		} else if (tmptok == "Caption") {
			inset = new InsetCaption;
		} else if (tmptok == "FloatList") {
			inset = new InsetFloatList;
		}
		
		if (inset && !alreadyread) inset->read(this, lex);
	}
	
	if (inset) {
		par->insertInset(pos, inset, font);
		++pos;
	}
}


bool Buffer::readFile(LyXLex & lex, Paragraph * par)
{
	if (lex.isOK()) {
		lex.next();
		string const token(lex.getString());
		if (token == "\\lyxformat") { // the first token _must_ be...
			lex.eatLine();
			string tmp_format = lex.getString();
			//lyxerr << "LyX Format: `" << tmp_format << "'" << endl;
			// if present remove ".," from string.
			string::size_type dot = tmp_format.find_first_of(".,");
			//lyxerr << "           dot found at " << dot << endl;
			if (dot != string::npos)
				tmp_format.erase(dot, 1);
			file_format = strToInt(tmp_format);
			if (file_format == LYX_FORMAT) {
				// current format
			} else if (file_format > LYX_FORMAT) {
				// future format
				WriteAlert(_("Warning!"),
					   _("LyX file format is newer that what"),
					   _("is supported in this LyX version. Expect some problems."));
				
			} else if (file_format < LYX_FORMAT) {
				// old formats
				if (file_format < 200) {
					WriteAlert(_("ERROR!"),
						   _("Old LyX file format found. "
						     "Use LyX 0.10.x to read this!"));
					return false;
				}
			}
			bool the_end = readLyXformat2(lex, par);
			setPaperStuff();
			// the_end was added in 213
			if (file_format < 213)
				the_end = true;

			if (!the_end)
				WriteAlert(_("Warning!"),
					   _("Reading of document is not complete"),
					   _("Maybe the document is truncated"));
			return true;
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
				    subst(os::slashify_path(s),'/','!'));

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

		// Should probably have some more error checking here.
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
					"backup copy. Beware." << endl;
			}
		}
	}
	
	if (writeFile(fileName(), false)) {
		markLyxClean();
		removeAutosaveFile(fileName());
	} else {
		// Saving failed, so backup is not backup
		if (lyxrc.make_backup) {
			lyx::rename(s, fileName());
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

#ifdef HAVE_LOCALE
	// Use the standard "C" locale for file output.
	ofs.imbue(std::locale::classic());
#endif

	// The top of the file should not be written by params.

	// write out a comment in the top of the file
	ofs << '#' << lyx_docversion 
	    << " created this file. For more info see http://www.lyx.org/\n"
	    << "\\lyxformat " << LYX_FORMAT << "\n";

	// now write out the buffer paramters.
	params.writeFile(ofs);

	Paragraph::depth_type depth = 0;

	// this will write out all the paragraphs
	// using recursive descent.
	paragraph->writeFile(this, ofs, params, depth);

	// Write marker that shows file is complete
	ofs << "\n\\the_end" << endl;

	ofs.close();

	// how to check if close went ok?
	// Following is an attempt... (BE 20001011)
	
	// good() returns false if any error occured, including some
	//        formatting error.
	// bad()  returns true if something bad happened in the buffer,
	//        which should include file system full errors.

	bool status = true;
	if (!ofs.good()) {
		status = false;
#if 0
		if (ofs.bad()) {
			lyxerr << "Buffer::writeFile: BAD ERROR!" << endl;
		} else {
			lyxerr << "Buffer::writeFile: NOT SO BAD ERROR!"
			       << endl;
		}
#endif
	}
	
	return status;
}


string const Buffer::asciiParagraph(Paragraph const * par,
				    unsigned int linelen) const
{
	ostringstream buffer;
	Paragraph::depth_type depth = 0;
	int ltype = 0;
	Paragraph::depth_type ltype_depth = 0;
	string::size_type currlinelen = 0;
	bool ref_printed = false;

	int noparbreak = 0;
	int islatex = 0;
	if (!par->previous()) {
		// begins or ends a deeper area ?
		if (depth != par->params().depth()) {
			if (par->params().depth() > depth) {
				while (par->params().depth() > depth) {
					++depth;
				}
			} else {
				while (par->params().depth() < depth) {
					--depth;
				}
			}
		}
		
		// First write the layout
		string const tmp = textclasslist.NameOfLayout(params.textclass, par->layout);
		if (tmp == "Itemize") {
			ltype = 1;
			ltype_depth = depth + 1;
		} else if (tmp == "Enumerate") {
			ltype = 2;
			ltype_depth = depth + 1;
		} else if (contains(tmp, "ection")) {
			ltype = 3;
			ltype_depth = depth + 1;
		} else if (contains(tmp, "aragraph")) {
			ltype = 4;
			ltype_depth = depth + 1;
		} else if (tmp == "Description") {
			ltype = 5;
			ltype_depth = depth + 1;
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
		lyxerr << "Should this ever happen?" << endl;
	}

	for (Paragraph::size_type i = 0; i < par->size(); ++i) {
		if (!i && !noparbreak) {
			if (linelen > 0)
				buffer << "\n\n";
			for (Paragraph::depth_type j = 0; j < depth; ++j)
				buffer << "  ";
			currlinelen = depth * 2;
			switch (ltype) {
			case 0: // Standard
			case 4: // (Sub)Paragraph
			case 5: // Description
				break;
			case 6: // Abstract
				if (linelen > 0)
					buffer << "Abstract\n\n";
				else
					buffer << "Abstract: ";
				break;
			case 7: // Bibliography
				if (!ref_printed) {
					if (linelen > 0)
						buffer << "References\n\n";
					else
						buffer << "References: ";
					ref_printed = true;
				}
				break;
			default:
				buffer << par->params().labelString() << " ";
				break;
			}
			if (ltype_depth > depth) {
				for (Paragraph::depth_type j = ltype_depth - 1; 
				     j > depth; --j)
					buffer << "  ";
				currlinelen += (ltype_depth-depth)*2;
			}
		}
		
		char c = par->getUChar(params, i);
		if (islatex)
			continue;
		switch (c) {
		case Paragraph::META_INSET:
		{
			Inset const * inset = par->getInset(i);
			if (inset) {
				if (!inset->ascii(this, buffer)) {
					string dummy;
					string const s =
						rsplit(buffer.str().c_str(),
						       dummy, '\n');
					currlinelen += s.length();
				} else {
					// to be sure it breaks paragraph
					currlinelen += linelen;
				}
			}
		}
		break;
		
		case Paragraph::META_NEWLINE:
			if (linelen > 0) {
				buffer << "\n";
				for (Paragraph::depth_type j = 0; 
				     j < depth; ++j)
					buffer << "  ";
			}
			currlinelen = depth * 2;
			if (ltype_depth > depth) {
				for (Paragraph::depth_type j = ltype_depth;
				     j > depth; --j)
					buffer << "  ";
				currlinelen += (ltype_depth - depth) * 2;
			}
			break;
			
		case Paragraph::META_HFILL: 
			buffer << "\t";
			break;

		default:
			if ((linelen > 0) && (currlinelen > (linelen - 10)) &&
			    (c == ' ') && ((i + 2) < par->size()))
			{
				buffer << "\n";
				for (Paragraph::depth_type j = 0; 
				     j < depth; ++j)
					buffer << "  ";
				currlinelen = depth * 2;
				if (ltype_depth > depth) {
					for (Paragraph::depth_type j = ltype_depth;
					    j > depth; --j)
						buffer << "  ";
					currlinelen += (ltype_depth-depth)*2;
				}
			} else if (c != '\0') {
				buffer << c;
				++currlinelen;
			} else
				lyxerr[Debug::INFO] << "writeAsciiFile: NULL char in structure." << endl;
			break;
		}
	}
	return buffer.str().c_str();
}


void Buffer::writeFileAscii(string const & fname, int linelen) 
{
	ofstream ofs(fname.c_str());
	if (!ofs) {
		WriteFSAlert(_("Error: Cannot write file:"), fname);
		return;
	}
	writeFileAscii(ofs, linelen);
}


void Buffer::writeFileAscii(ostream & ofs, int linelen) 
{
	Paragraph * par = paragraph;
	while (par) {
		ofs << asciiParagraph(par, linelen);
		par = par->next();
	}
	ofs << "\n";
}

bool use_babel;

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
		ofs << "%% " << lyx_docversion << " created this file.  "
			"For more info, see http://www.lyx.org/.\n"
			"%% Do not edit unless you really know what "
			"you are doing.\n";
		texrow.newline();
		texrow.newline();
	}
	lyxerr[Debug::INFO] << "lyx header finished" << endl;
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
			    << os::external_path(original_path) << "/}}\n"
			    << "\\makeatother\n";
			texrow.newline();
			texrow.newline();
			texrow.newline();
		}
		
		ofs << "\\documentclass";
		
		ostringstream options; // the document class options.
		
		if (tokenPos(tclass.opt_fontsize(),
			     '|', params.fontsize) >= 0) {
			// only write if existing in list (and not default)
			options << params.fontsize << "pt,";
		}
		
		
		if (!params.use_geometry &&
		    (params.paperpackage == BufferParams::PACKAGE_NONE)) {
			switch (params.papersize) {
			case BufferParams::PAPER_A4PAPER:
				options << "a4paper,";
				break;
			case BufferParams::PAPER_USLETTER:
				options << "letterpaper,";
				break;
			case BufferParams::PAPER_A5PAPER:
				options << "a5paper,";
				break;
			case BufferParams::PAPER_B5PAPER:
				options << "b5paper,";
				break;
			case BufferParams::PAPER_EXECUTIVEPAPER:
				options << "executivepaper,";
				break;
			case BufferParams::PAPER_LEGALPAPER:
				options << "legalpaper,";
				break;
			}
		}

		// if needed
		if (params.sides != tclass.sides()) {
			switch (params.sides) {
			case LyXTextClass::OneSide:
				options << "oneside,";
				break;
			case LyXTextClass::TwoSides:
				options << "twoside,";
				break;
			}
		}

		// if needed
		if (params.columns != tclass.columns()) {
			if (params.columns == 2)
				options << "twocolumn,";
			else
				options << "onecolumn,";
		}

		if (!params.use_geometry 
		    && params.orientation == BufferParams::ORIENTATION_LANDSCAPE)
			options << "landscape,";
		
		// language should be a parameter to \documentclass
		use_babel = false;
		ostringstream language_options;
		if (params.language->babel() == "hebrew"
		    && default_language->babel() != "hebrew")
			 // This seems necessary
			features.UsedLanguages.insert(default_language);

		if (lyxrc.language_use_babel ||
		    params.language->lang() != lyxrc.default_language ||
		    !features.UsedLanguages.empty()) {
			use_babel = true;
			for (LaTeXFeatures::LanguageList::const_iterator cit =
				     features.UsedLanguages.begin();
			     cit != features.UsedLanguages.end(); ++cit)
				language_options << (*cit)->babel() << ',';
			language_options << params.language->babel();
			if (lyxrc.language_global_options)
				options << language_options.str() << ',';
		}

		// the user-defined options
		if (!params.options.empty()) {
			options << params.options << ',';
		}

		string strOptions(options.str().c_str());
		if (!strOptions.empty()){
			strOptions = strip(strOptions, ',');
			ofs << '[' << strOptions << ']';
		}
		
		ofs << '{'
		    << textclasslist.LatexnameOfClass(params.textclass)
		    << "}\n";
		texrow.newline();
		// end of \documentclass defs
		
		// font selection must be done before loading fontenc.sty
		// The ae package is not needed when using OT1 font encoding.
		if (params.fonts != "default" &&
		    (params.fonts != "ae" || lyxrc.fontenc != "default")) {
			ofs << "\\usepackage{" << params.fonts << "}\n";
			texrow.newline();
			if (params.fonts == "ae") {
				ofs << "\\usepackage{aecompl}\n";
				texrow.newline();
			}
		}
		// this one is not per buffer
		if (lyxrc.fontenc != "default") {
			ofs << "\\usepackage[" << lyxrc.fontenc
			    << "]{fontenc}\n";
			texrow.newline();
		}

		if (params.inputenc == "auto") {
			string const doc_encoding =
				params.language->encoding()->LatexName();

			// Create a list with all the input encodings used 
			// in the document
			set<string> encodings;
			for (LaTeXFeatures::LanguageList::const_iterator it =
				     features.UsedLanguages.begin();
			     it != features.UsedLanguages.end(); ++it)
				if ((*it)->encoding()->LatexName() != doc_encoding)
					encodings.insert((*it)->encoding()->LatexName());

			ofs << "\\usepackage[";
			std::copy(encodings.begin(), encodings.end(),
				  std::ostream_iterator<string>(ofs, ","));
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
		if (features.amsstyle
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

		// We try to load babel late, in case it interferes
		// with other packages.
		if (use_babel) {
			string tmp = lyxrc.language_package;
			if (!lyxrc.language_global_options
			    && tmp == "\\usepackage{babel}")
				tmp = string("\\usepackage[") +
					language_options.str().c_str() +
					"]{babel}";
			ofs << tmp << "\n";
			texrow.newline();
		}

		// make the body.
		ofs << "\\begin{document}\n";
		texrow.newline();
	} // only_body
	lyxerr[Debug::INFO] << "preamble finished, now the body." << endl;

	if (!lyxrc.language_auto_begin) {
		ofs << subst(lyxrc.language_command_begin, "$$lang",
			     params.language->babel())
		    << endl;
		texrow.newline();
	}
	
	latexParagraphs(ofs, paragraph, 0, texrow);

	// add this just in case after all the paragraphs
	ofs << endl;
	texrow.newline();

	if (!lyxrc.language_auto_end) {
		ofs << subst(lyxrc.language_command_end, "$$lang",
			     params.language->babel())
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
	
	lyxerr[Debug::INFO] << "Finished making latex file." << endl;
}


//
// LaTeX all paragraphs from par to endpar, if endpar == 0 then to the end
//
void Buffer::latexParagraphs(ostream & ofs, Paragraph * par,
			     Paragraph * endpar, TexRow & texrow) const
{
	bool was_title = false;
	bool already_title = false;

	// if only_body
	while (par != endpar) {
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
		
		if (layout.isEnvironment()) {
			par = par->TeXEnvironment(this, params, ofs, texrow);
		} else {
			par = par->TeXOnePar(this, params, ofs, texrow, false);
		}
	}
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


void Buffer::sgmlOpenTag(ostream & os, Paragraph::depth_type depth,
			 string const & latexname) const
{
	if (!latexname.empty() && latexname != "!-- --")
		os << "<!-- " << depth << " -->" << "<" << latexname << ">";
	//os << string(depth, ' ') << "<" << latexname << ">\n";
}


void Buffer::sgmlCloseTag(ostream & os, Paragraph::depth_type depth,
			  string const & latexname) const
{
	if (!latexname.empty() && latexname != "!-- --")
		os << "<!-- " << depth << " -->" << "</" << latexname << ">\n";
	//os << string(depth, ' ') << "</" << latexname << ">\n";
}


void Buffer::makeLinuxDocFile(string const & fname, bool nice, bool body_only)
{
	ofstream ofs(fname.c_str());

	if (!ofs) {
		WriteAlert(_("LYX_ERROR:"), _("Cannot write file"), fname);
		return;
	}

	niceFile = nice; // this will be used by included files.

        LyXTextClass const & tclass =
		textclasslist.TextClass(params.textclass);

	LaTeXFeatures features(params, tclass.numLayouts());
	validate(features);

	texrow.reset();

	string top_element = textclasslist.LatexnameOfClass(params.textclass);

	if (!body_only) {
		ofs << "<!doctype linuxdoc system";

		string preamble = params.preamble;
		preamble += features.getIncludedFiles(fname);
		preamble += features.getLyXSGMLEntities();

		if (!preamble.empty()) {
			ofs << " [ " << preamble << " ]";
		}
		ofs << ">\n\n";

		if (params.options.empty())
			sgmlOpenTag(ofs, 0, top_element);
		else {
			string top = top_element;
			top += " ";
			top += params.options;
			sgmlOpenTag(ofs, 0, top);
		}
	}

	ofs << "<!-- "  << lyx_docversion
	    << " created this file. For more info see http://www.lyx.org/"
	    << " -->\n";

	Paragraph::depth_type depth = 0; // paragraph depth
	Paragraph * par = paragraph;
        string item_name;
	vector<string> environment_stack(5);

	while (par) {
		LyXLayout const & style =
			textclasslist.Style(params.textclass,
					    par->layout);

		// treat <toc> as a special case for compatibility with old code
		if (par->getChar(0) == Paragraph::META_INSET) {
		        Inset * inset = par->getInset(0);
			Inset::Code lyx_code = inset->lyxCode();
			if (lyx_code == Inset::TOC_CODE){
				string const temp = "toc";
				sgmlOpenTag(ofs, depth, temp);

				par = par->next();
				continue;
			}
		}

		// environment tag closing
		for (; depth > par->params().depth(); --depth) {
			sgmlCloseTag(ofs, depth, environment_stack[depth]);
			environment_stack[depth].erase();
		}

		// write opening SGML tags
		switch (style.latextype) {
		case LATEX_PARAGRAPH:
			if (depth == par->params().depth() 
			   && !environment_stack[depth].empty()) {
				sgmlCloseTag(ofs, depth, environment_stack[depth]);
				environment_stack[depth].erase();
				if (depth) 
					--depth;
				else
				        ofs << "</p>";
			}
			sgmlOpenTag(ofs, depth, style.latexname());
			break;

		case LATEX_COMMAND:
			if (depth!= 0)
				linuxDocError(par, 0,
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
			if (depth == par->params().depth() 
			    && environment_stack[depth] != style.latexname()) {
				sgmlCloseTag(ofs, depth,
					     environment_stack[depth]);
				environment_stack[depth].erase();
			}
			if (depth < par->params().depth()) {
			       depth = par->params().depth();
			       environment_stack[depth].erase();
			}
			if (environment_stack[depth] != style.latexname()) {
				if (depth == 0) {
					sgmlOpenTag(ofs, depth, "p");
				}
				sgmlOpenTag(ofs, depth, style.latexname());

				if (environment_stack.size() == depth + 1)
					environment_stack.push_back("!-- --");
				environment_stack[depth] = style.latexname();
			}

			if (style.latexparam() == "CDATA")
				ofs << "<![CDATA[";

			if (style.latextype == LATEX_ENVIRONMENT) break;

			if (style.labeltype == LABEL_MANUAL)
				item_name = "tag";
			else
				item_name = "item";

			sgmlOpenTag(ofs, depth + 1, item_name);
			break;
		default:
			sgmlOpenTag(ofs, depth, style.latexname());
			break;
		}

		simpleLinuxDocOnePar(ofs, par, depth);

		par = par->next();

		ofs << "\n";
		// write closing SGML tags
		switch (style.latextype) {
		case LATEX_COMMAND:
			break;
		case LATEX_ENVIRONMENT:
		case LATEX_ITEM_ENVIRONMENT:
			if (style.latexparam() == "CDATA")
				ofs << "]]>";
			break;
		default:
			sgmlCloseTag(ofs, depth, style.latexname());
			break;
		}
	}
   
	// Close open tags
	for (int i=depth; i >= 0; --i)
	        sgmlCloseTag(ofs, depth, environment_stack[i]);

	if (!body_only) {
		ofs << "\n\n";
		sgmlCloseTag(ofs, 0, top_element);
	}

	ofs.close();
	// How to check for successful close
}


// checks, if newcol chars should be put into this line
// writes newline, if necessary.
namespace {

void linux_doc_line_break(ostream & os, string::size_type & colcount,
			  string::size_type newcol)
{
	colcount += newcol;
	if (colcount > lyxrc.ascii_linelen) {
		os << "\n";
		colcount = newcol; // assume write after this call
	}
}

enum PAR_TAG {
	NONE=0,
	TT = 1,
	SF = 2,
	BF = 4,
	IT = 8,
	SL = 16,
	EM = 32
};


string tag_name(PAR_TAG const & pt) {
	switch (pt) {
	case NONE: return "!-- --";
	case TT: return "tt";
	case SF: return "sf";
	case BF: return "bf";
	case IT: return "it";
	case SL: return "sl";
	case EM: return "em";
	}
	return "";
}


inline
void operator|=(PAR_TAG & p1, PAR_TAG const & p2)
{
        p1 = static_cast<PAR_TAG>(p1 | p2);
}


inline
void reset(PAR_TAG & p1, PAR_TAG const & p2)
{
	p1 = static_cast<PAR_TAG>( p1 & ~p2);
}

} // namespace anon


// Handle internal paragraph parsing -- layout already processed.
void Buffer::simpleLinuxDocOnePar(ostream & os,
				  Paragraph * par, 
				  Paragraph::depth_type /*depth*/)
{
	LyXLayout const & style = textclasslist.Style(params.textclass,
						      par->getLayout());
        string::size_type char_line_count = 5;     // Heuristic choice ;-) 

	// gets paragraph main font
	LyXFont font_old;
	bool desc_on;
	if (style.labeltype == LABEL_MANUAL) {
		font_old = style.labelfont;
		desc_on = true;
	} else {
		font_old = style.font;
		desc_on = false;
	}

	LyXFont::FONT_FAMILY family_type = LyXFont::ROMAN_FAMILY;
	LyXFont::FONT_SERIES series_type = LyXFont::MEDIUM_SERIES;
	LyXFont::FONT_SHAPE  shape_type  = LyXFont::UP_SHAPE;
	bool is_em = false;

	stack<PAR_TAG> tag_state;
	// parsing main loop
	for (Paragraph::size_type i = 0; i < par->size(); ++i) {

		PAR_TAG tag_close = NONE;
		list < PAR_TAG > tag_open;

		LyXFont const font = par->getFont(params, i);

		if (font_old.family() != font.family()) {
			switch (family_type) {
			case LyXFont::SANS_FAMILY:
				tag_close |= SF;
				break;
			case LyXFont::TYPEWRITER_FAMILY:
				tag_close |= TT;
				break;
			default:
				break;
			}

			family_type = font.family();

			switch (family_type) {
			case LyXFont::SANS_FAMILY:
				tag_open.push_back(SF);
				break;
			case LyXFont::TYPEWRITER_FAMILY:
				tag_open.push_back(TT);
				break;
			default:
				break;
			}
		}

		if (font_old.series() != font.series()) {
			switch (series_type) {
			case LyXFont::BOLD_SERIES:
				tag_close |= BF;
				break;
			default:
				break;
			}

			series_type = font.series();

			switch (series_type) {
			case LyXFont::BOLD_SERIES:
				tag_open.push_back(BF);
				break;
			default:
				break;
			}

		}

		if (font_old.shape() != font.shape()) {
			switch (shape_type) {
			case LyXFont::ITALIC_SHAPE:
				tag_close |= IT;
				break;
			case LyXFont::SLANTED_SHAPE:
				tag_close |= SL;
				break;
			default:
				break;
			}

			shape_type = font.shape();

			switch (shape_type) {
			case LyXFont::ITALIC_SHAPE:
				tag_open.push_back(IT);
				break;
			case LyXFont::SLANTED_SHAPE:
				tag_open.push_back(SL);
				break;
			default:
				break;
			}
		}
		// handle <em> tag
		if (font_old.emph() != font.emph()) {
			if (font.emph() == LyXFont::ON) {
				tag_open.push_back(EM);
				is_em = true;
			}
			else if (is_em) {
				tag_close |= EM;
				is_em = false;
			}
		}

		list < PAR_TAG > temp;
		while(!tag_state.empty() && tag_close ) {
			PAR_TAG k =  tag_state.top();
			tag_state.pop();
			os << "</" << tag_name(k) << ">";
			if (tag_close & k)
				reset(tag_close,k);
			else
				temp.push_back(k);
		}

		for(list< PAR_TAG >::const_iterator j = temp.begin();
		    j != temp.end(); ++j) {
			tag_state.push(*j);
			os << "<" << tag_name(*j) << ">";
		}

		for(list< PAR_TAG >::const_iterator j = tag_open.begin();
		    j != tag_open.end(); ++j) {
			tag_state.push(*j);
			os << "<" << tag_name(*j) << ">";
		}

		char c = par->getChar(i);

		if (c == Paragraph::META_INSET) {
			Inset * inset = par->getInset(i);
			inset->linuxdoc(this, os);
			font_old = font;
			continue;
		}

		if (style.latexparam() == "CDATA") {
			// "TeX"-Mode on == > SGML-Mode on.
			if (c != '\0')
				os << c;
			++char_line_count;
		} else {
			string sgml_string;
			if (par->linuxDocConvertChar(c, sgml_string)
			    && !style.free_spacing) { 
				// in freespacing mode, spaces are
				// non-breaking characters
				if (desc_on) {// if char is ' ' then...

					++char_line_count;
					linux_doc_line_break(os, char_line_count, 6);
					os << "</tag>";
					desc_on = false;
				} else  {
					linux_doc_line_break(os, char_line_count, 1);
					os << c;
				}
			} else {
				os << sgml_string;
				char_line_count += sgml_string.length();
			}
		}
		font_old = font;
	}

	while (!tag_state.empty()) {
		os << "</" << tag_name(tag_state.top()) << ">";
		tag_state.pop();
	}

	// resets description flag correctly
	if (desc_on) {
		// <tag> not closed...
		linux_doc_line_break(os, char_line_count, 6);
		os << "</tag>";
	}
}


// Print an error message.
void Buffer::linuxDocError(Paragraph * par, int pos,
			   string const & message) 
{
	// insert an error marker in text
	InsetError * new_inset = new InsetError(message);
	par->insertInset(pos, new_inset);
}


void Buffer::makeDocBookFile(string const & fname, bool nice, bool only_body)
{
	ofstream ofs(fname.c_str());
	if (!ofs) {
		WriteAlert(_("LYX_ERROR:"), _("Cannot write file"), fname);
		return;
	}

	Paragraph * par = paragraph;

	niceFile = nice; // this will be used by Insetincludes.

        LyXTextClass const & tclass =
		textclasslist.TextClass(params.textclass);

	LaTeXFeatures features(params, tclass.numLayouts());
	validate(features);
   
	texrow.reset();

	string top_element = textclasslist.LatexnameOfClass(params.textclass);

	if (!only_body) {
		ofs << "<!DOCTYPE " << top_element
		    << "  PUBLIC \"-//OASIS//DTD DocBook V4.1//EN\"";

		string preamble = params.preamble;
		preamble += features.getIncludedFiles(fname);
		preamble += features.getLyXSGMLEntities();

		if (!preamble.empty()) {
			ofs << "\n [ " << preamble << " ]";
		}
		ofs << ">\n\n";
	}

	string top = top_element;	
	top += " lang=\"";
	top += params.language->code();
	top += "\"";

	if (!params.options.empty()) {
		top += " ";
		top += params.options;
	}
	sgmlOpenTag(ofs, 0, top);

	ofs << "<!-- DocBook file was created by " << lyx_docversion
	    << "\n  See http://www.lyx.org/ for more information -->\n";

	vector<string> environment_stack(10);
	vector<string> environment_inner(10);
	vector<string> command_stack(10);

	bool command_flag = false;
	Paragraph::depth_type command_depth = 0;
	Paragraph::depth_type command_base = 0;
	Paragraph::depth_type cmd_depth = 0;
	Paragraph::depth_type depth = 0; // paragraph depth

        string item_name;
	string command_name;

	while (par) {
		string sgmlparam;
		string c_depth;
		string c_params;
		int desc_on = 0; // description mode

		LyXLayout const & style =
			textclasslist.Style(params.textclass,
					    par->layout);

		// environment tag closing
		for (; depth > par->params().depth(); --depth) {
			if (environment_inner[depth] != "!-- --") {
				item_name = "listitem";
				sgmlCloseTag(ofs, command_depth + depth,
					     item_name);
				if (environment_inner[depth] == "varlistentry")
					sgmlCloseTag(ofs, depth+command_depth,
						     environment_inner[depth]);
			}
			sgmlCloseTag(ofs, depth + command_depth,
				     environment_stack[depth]);
			environment_stack[depth].erase();
			environment_inner[depth].erase();
		}

		if (depth == par->params().depth()
		   && environment_stack[depth] != style.latexname()
		   && !environment_stack[depth].empty()) {
			if (environment_inner[depth] != "!-- --") {
				item_name= "listitem";
				sgmlCloseTag(ofs, command_depth+depth,
					     item_name);
				if (environment_inner[depth] == "varlistentry")
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
		switch (style.latextype) {
		case LATEX_PARAGRAPH:
			sgmlOpenTag(ofs, depth + command_depth,
				    style.latexname());
			break;

		case LATEX_COMMAND:
			if (depth != 0)
				linuxDocError(par, 0,
					      _("Error : Wrong depth for "
						"LatexType Command.\n"));
			
			command_name = style.latexname();
			
			sgmlparam = style.latexparam();
			c_params = split(sgmlparam, c_depth,'|');
			
			cmd_depth = lyx::atoi(c_depth);
			
			if (command_flag) {
				if (cmd_depth < command_base) {
					for (Paragraph::depth_type j = command_depth; j >= command_base; --j)
						sgmlCloseTag(ofs, j, command_stack[j]);
					command_depth = command_base = cmd_depth;
				} else if (cmd_depth <= command_depth) {
					for (int j = command_depth; j >= int(cmd_depth); --j)
						sgmlCloseTag(ofs, j, command_stack[j]);
					command_depth = cmd_depth;
				} else
					command_depth = cmd_depth;
			} else {
				command_depth = command_base = cmd_depth;
				command_flag = true;
			}
			if (command_stack.size() == command_depth + 1)
				command_stack.push_back(string());
			command_stack[command_depth] = command_name;

			// treat label as a special case for
			// more WYSIWYM handling.
			if (par->getChar(0) == Paragraph::META_INSET) {
			        Inset * inset = par->getInset(0);
				Inset::Code lyx_code = inset->lyxCode();
				if (lyx_code == Inset::LABEL_CODE){
					command_name += " id=\"";
					command_name += (static_cast<InsetCommand *>(inset))->getContents();
					command_name += "\"";
					desc_on = 3;
				}
			}

			sgmlOpenTag(ofs, depth + command_depth, command_name);
			if (c_params.empty())
				item_name = "title";
			else
				item_name = c_params;
			sgmlOpenTag(ofs, depth + 1 + command_depth, item_name);
			break;

		case LATEX_ENVIRONMENT:
		case LATEX_ITEM_ENVIRONMENT:
			if (depth < par->params().depth()) {
				depth = par->params().depth();
				environment_stack[depth].erase();
			}

			if (environment_stack[depth] != style.latexname()) {
				if(environment_stack.size() == depth + 1) {
					environment_stack.push_back("!-- --");
					environment_inner.push_back("!-- --");
				}
				environment_stack[depth] = style.latexname();
				environment_inner[depth] = "!-- --";
				sgmlOpenTag(ofs, depth + command_depth,
					    environment_stack[depth]);
			} else {
				if (environment_inner[depth] != "!-- --") {
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
			
			if (style.latextype == LATEX_ENVIRONMENT) {
				if (!style.latexparam().empty()) {
					if(style.latexparam() == "CDATA")
						ofs << "<![CDATA[";
					else
						sgmlOpenTag(ofs, depth + command_depth,
							    style.latexparam());
				}
				break;
			}

			desc_on = (style.labeltype == LABEL_MANUAL);

			if (desc_on)
				environment_inner[depth]= "varlistentry";
			else
				environment_inner[depth]= "listitem";

			sgmlOpenTag(ofs, depth + 1 + command_depth,
				    environment_inner[depth]);

			if (desc_on) {
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

		string extra_par;
		simpleDocBookOnePar(ofs, extra_par, par, desc_on,
				    depth + 1 + command_depth);
		par = par->next();

		string end_tag;
		// write closing SGML tags
		switch (style.latextype) {
		case LATEX_COMMAND:
			if (c_params.empty())
				end_tag = "title";
			else
				end_tag = c_params;
			sgmlCloseTag(ofs, depth + command_depth, end_tag);
			break;
		case LATEX_ENVIRONMENT:
			if (!style.latexparam().empty()) {
				if(style.latexparam() == "CDATA")
					ofs << "]]>";
				else
					sgmlCloseTag(ofs, depth + command_depth,
						     style.latexparam());
			}
			break;
		case LATEX_ITEM_ENVIRONMENT:
			if (desc_on == 1) break;
			end_tag= "para";
			sgmlCloseTag(ofs, depth + 1 + command_depth, end_tag);
			break;
		case LATEX_PARAGRAPH:
			sgmlCloseTag(ofs, depth + command_depth, style.latexname());
			break;
		default:
			sgmlCloseTag(ofs, depth + command_depth, style.latexname());
			break;
		}
	}

	// Close open tags
	for (int d = depth; d >= 0; --d) {
		if (!environment_stack[depth].empty()) {
			if (environment_inner[depth] != "!-- --") {
				item_name = "listitem";
				sgmlCloseTag(ofs, command_depth + depth,
					     item_name);
                               if (environment_inner[depth] == "varlistentry")
				       sgmlCloseTag(ofs, depth + command_depth,
						    environment_inner[depth]);
			}
			
			sgmlCloseTag(ofs, depth + command_depth,
				     environment_stack[depth]);
		}
	}
	
	for (int j = command_depth; j >= 0 ; --j)
		if (!command_stack[j].empty())
			sgmlCloseTag(ofs, j, command_stack[j]);

	ofs << "\n\n";
	sgmlCloseTag(ofs, 0, top_element);

	ofs.close();
	// How to check for successful close
}


void Buffer::simpleDocBookOnePar(ostream & os, string & extra,
				 Paragraph * par, int & desc_on,
				 Paragraph::depth_type depth) const
{
	bool emph_flag = false;

	LyXLayout const & style = textclasslist.Style(params.textclass,
						      par->getLayout());

	LyXFont font_old = style.labeltype == LABEL_MANUAL ? style.labelfont : style.font;

	int char_line_count = depth;
	//if (!style.free_spacing)
	//	os << string(depth,' ');

	// parsing main loop
	for (Paragraph::size_type i = 0;
	     i < par->size(); ++i) {
		LyXFont font = par->getFont(params, i);

		// handle <emphasis> tag
		if (font_old.emph() != font.emph()) {
			if (font.emph() == LyXFont::ON) {
				os << "<emphasis>";
				emph_flag = true;
			}else if(i) {
				os << "</emphasis>";
				emph_flag = false;
			}
		}
      
		char c = par->getChar(i);

		if (c == Paragraph::META_INSET) {
			Inset * inset = par->getInset(i);
			ostringstream ost;
			inset->docbook(this, ost);
			string tmp_out = ost.str().c_str();

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
			if (desc_on!= 3 || i!= 0) {
				if (!tmp_out.empty() && tmp_out[0] == '@') {
					if (desc_on == 4)
						extra += frontStrip(tmp_out, '@');
					else
						os << frontStrip(tmp_out, '@');
				}
				else
					os << tmp_out;
			}
		} else {
			string sgml_string;
			par->linuxDocConvertChar(c, sgml_string);

			if (style.pass_thru) {
				os << c;
			} else if(style.free_spacing || c != ' ') {
					os << sgml_string;
			} else if (desc_on ==1) {
				++char_line_count;
				os << "\n</term><listitem><para>";
				desc_on = 2;
			} else {
				os << ' ';
			}
		}
		font_old = font;
	}

	if (emph_flag) {
		os << "</emphasis>";
	}
	
	// resets description flag correctly
	if (desc_on == 1) {
		// <term> not closed...
		os << "</term>";
	}
	if(style.free_spacing) os << '\n';
}


// This should be enabled when the Chktex class is implemented. (Asger)
// chktex should be run with these flags disabled: 3, 22, 25, 30, 38(?)
// Other flags: -wall -v0 -x
int Buffer::runChktex()
{
	if (!users->text) return 0;

	users->owner()->prohibitInput();

	// get LaTeX-Filename
	string const name = getLatexName();
	string path = OnlyPath(filename);

	string const org_path = path;
	if (lyxrc.use_tempdir || !IsDirWriteable(path)) {
		path = tmppath;	 
	}

	Path p(path); // path to LaTeX file
	users->owner()->message(_("Running chktex..."));

	// Remove all error insets
	bool const removedErrorInsets = users->removeAutoInsets();

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
	}
	users->owner()->allowInput();

	return res;
}


void Buffer::validate(LaTeXFeatures & features) const
{
	Paragraph * par = paragraph;
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
		par = par->next();
	}

	// the bullet shapes are buffer level not paragraph level
	// so they are tested here
	for (int i = 0; i < 4; ++i) {
		if (params.user_defined_bullets[i] != ITEMIZE_DEFAULTS[i]) {
			int const font = params.user_defined_bullets[i].getFont();
			if (font == 0) {
				int const c = params
					.user_defined_bullets[i]
					.getCharacter();
				if (c == 16
				   || c == 17
				   || c == 25
				   || c == 26
				   || c == 31) {
					features.latexsym = true;
				}
			} else if (font == 1) {
				features.amssymb = true;
			} else if ((font >= 2 && font <= 5)) {
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
	char const c1 = params.paperpackage;
	if (c1 == BufferParams::PACKAGE_NONE) {
		char const c2 = params.papersize2;
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
string const Buffer::getIncludeonlyList(char delim)
{
	string lst;
	for (inset_iterator it = inset_iterator_begin();
	    it != inset_iterator_end(); ++it) {
		if ((*it)->lyxCode() == Inset::INCLUDE_CODE) {
			InsetInclude * insetinc = 
				static_cast<InsetInclude *>(*it);
			if (insetinc->isIncludeOnly()) {
				if (!lst.empty())
					lst += delim;
				lst += insetinc->getRelFileBaseName();
			}
		}
	}
	lyxerr[Debug::INFO] << "Includeonly(" << lst << ')' << endl;
	return lst;
}


vector<string> const Buffer::getLabelList()
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
		vector<string> const l = (*it)->getLabelList();
		label_list.insert(label_list.end(), l.begin(), l.end());
	}
	return label_list;
}


Buffer::Lists const Buffer::getLists() const
{
	Lists l;
	Paragraph * par = paragraph;

#if 1
	std::pair<bool, LyXTextClassList::size_type> const tmp =
		textclasslist.NumberOfLayout(params.textclass, "Caption");
	bool const found = tmp.first;
	LyXTextClassList::size_type const cap = tmp.second;
	
#else
	// This is the prefered way to to this, but boost::tie can break
	// some compilers
	bool found;
	LyXTextClassList::size_type cap;
	boost::tie(found, cap) = textclasslist
		.NumberOfLayout(params.textclass, "Caption");
#endif

	while (par) {
		char const labeltype =
			textclasslist.Style(params.textclass, 
					    par->getLayout()).labeltype;
		
		if (labeltype >= LABEL_COUNTER_CHAPTER
		    && labeltype <= LABEL_COUNTER_CHAPTER + params.tocdepth) {
				// insert this into the table of contents
			SingleList & item = l["TOC"];
			int depth = max(0,
					labeltype - 
					textclasslist.TextClass(params.textclass).maxcounter());
			item.push_back(TocItem(par, depth, par->asString(this, true)));
		}
		// For each paragrph, traverse its insets and look for
		// FLOAT_CODE
		
		if (found) {
			Paragraph::inset_iterator it =
				par->inset_iterator_begin();
			Paragraph::inset_iterator end =
				par->inset_iterator_end();
			
			for (; it != end; ++it) {
				if ((*it)->lyxCode() == Inset::FLOAT_CODE) {
					InsetFloat * il =
						static_cast<InsetFloat*>(*it);
					
					string const type = il->type();
					
					// Now find the caption in the float...
					// We now tranverse the paragraphs of
					// the inset...
					Paragraph * tmp = il->inset.paragraph();
					while (tmp) {
						if (tmp->layout == cap) {
							SingleList & item = l[type];
							string const str =
								tostr(item.size()+1) + ". " + tmp->asString(this, false);
							item.push_back(TocItem(tmp, 0 , str));
						}
						tmp = tmp->next();
					}
				}
			}
		} else {
			lyxerr << "caption not found" << endl;
		}
		
		par = par->next();
	}
	return l;
}


// This is also a buffer property (ale)
vector<pair<string, string> > const Buffer::getBibkeyList()
{
	/// if this is a child document and the parent is already loaded
	/// Use the parent's list instead  [ale990412]
        if (!params.parentname.empty() && bufferlist.exists(params.parentname)) {
		Buffer * tmp = bufferlist.getBuffer(params.parentname);
		if (tmp)
			return tmp->getBibkeyList();
	}

	vector<pair<string, string> > keys;
	Paragraph * par = paragraph;
	while (par) {
		if (par->bibkey)
			keys.push_back(pair<string, string>(par->bibkey->getContents(),
							   par->asString(this, false)));
		par = par->next();
	}

	// Might be either using bibtex or a child has bibliography
	if (keys.empty()) {
		for (inset_iterator it = inset_iterator_begin();
			it != inset_iterator_end(); ++it) {
			// Search for Bibtex or Include inset
			if ((*it)->lyxCode() == Inset::BIBTEX_CODE) {
				vector<pair<string,string> > tmp =
					static_cast<InsetBibtex*>(*it)->getKeys(this);
				keys.insert(keys.end(), tmp.begin(), tmp.end());
			} else if ((*it)->lyxCode() == Inset::INCLUDE_CODE) {
				vector<pair<string,string> > const tmp =
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
		DEPCLEAN * item = dep_clean;
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


bool Buffer::dispatch(string const & command)
{
	// Split command string into command and argument
	string cmd;
	string line = frontStrip(command);
	string const arg = strip(frontStrip(split(line, cmd, ' ')));

	return dispatch(lyxaction.LookupFunc(cmd), arg);
}


bool Buffer::dispatch(int action, string const & argument)
{
	bool dispatched = true;
	switch (action) {
		case LFUN_EXPORT: 
			Exporter::Export(this, argument, false);
			break;

		default:
			dispatched = false;
	}
	return dispatched;
}


void Buffer::resizeInsets(BufferView * bv)
{
	/// then remove all LyXText in text-insets
	Paragraph * par = paragraph;
	for (; par; par = par->next()) {
	    par->resizeInsetsLyXText(bv);
	}
}


void Buffer::redraw()
{
	users->redraw(); 
	users->fitCursor(); 
}


void Buffer::changeLanguage(Language const * from, Language const * to)
{

	ParIterator end = par_iterator_end();
	for (ParIterator it = par_iterator_begin(); it != end; ++it)
		(*it)->changeLanguage(params, from, to);
}


bool Buffer::isMultiLingual()
{
	ParIterator end = par_iterator_end();
	for (ParIterator it = par_iterator_begin(); it != end; ++it)
		if ((*it)->isMultiLingual(params))
			return true;

	return false;
}


Buffer::inset_iterator::inset_iterator(Paragraph * paragraph,
				       Paragraph::size_type pos)
	: par(paragraph)
{
	it = par->InsetIterator(pos);
	if (it == par->inset_iterator_end()) {
		par = par->next();
		setParagraph();
	}
}


void Buffer::inset_iterator::setParagraph()
{
	while (par) {
		it = par->inset_iterator_begin();
		if (it != par->inset_iterator_end())
			return;
		par = par->next();
	}
	//it = 0;
	// We maintain an invariant that whenever par = 0 then it = 0
}


Inset * Buffer::getInsetFromID(int id_arg) const
{
	for (inset_iterator it = inset_const_iterator_begin();
		 it != inset_const_iterator_end(); ++it)
	{
		if ((*it)->id() == id_arg)
			return *it;
		Inset * in = (*it)->getInsetFromID(id_arg);
		if (in)
			return in;
	}
	return 0;
}


Paragraph * Buffer::getParFromID(int id) const
{
	if (id < 0) return 0;
	Paragraph * par = paragraph;
	while (par) {
		if (par->id() == id) {
			return par;
		}
		Paragraph * tmp = par->getParFromID(id);
		if (tmp) {
			return tmp;
		}
		par = par->next();
	}
	return 0;
}


ParIterator Buffer::par_iterator_begin()
{
        return ParIterator(paragraph);
}


ParIterator Buffer::par_iterator_end()
{
        return ParIterator();
}
