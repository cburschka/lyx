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

#ifdef __GNUG__
#pragma implementation
#endif

#include "buffer.h"
#include "bufferlist.h"
#include "counters.h"
#include "LyXAction.h"
#include "lyxrc.h"
#include "lyxlex.h"
#include "tex-strings.h"
#include "layout.h"
#include "bufferview_funcs.h"
#include "lyxfont.h"
#include "version.h"
#include "LaTeX.h"
#include "Chktex.h"
#include "debug.h"
#include "LaTeXFeatures.h"
#include "lyxtext.h"
#include "gettext.h"
#include "language.h"
#include "encoding.h"
#include "exporter.h"
#include "Lsstream.h"
#include "converter.h"
#include "BufferView.h"
#include "ParagraphParameters.h"
#include "iterators.h"
#include "lyxtextclasslist.h"
#include "sgml.h"
#include "paragraph_funcs.h"

#include "frontends/LyXView.h"

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
#include "insets/insettext.h"
#include "insets/insetert.h"
#include "insets/insetgraphics.h"
#include "insets/insetfoot.h"
#include "insets/insetmarginal.h"
#include "insets/insetoptarg.h"
#include "insets/insetminipage.h"
#include "insets/insetfloat.h"
#include "insets/insettabular.h"
#if 0
#include "insets/insettheorem.h"
#include "insets/insetlist.h"
#endif
#include "insets/insetcaption.h"
#include "insets/insetfloatlist.h"

#include "frontends/Dialogs.h"
#include "frontends/Alert.h"

#include "graphics/Previews.h"

#include "support/textutils.h"
#include "support/filetools.h"
#include "support/path.h"
#include "support/os.h"
#include "support/lyxlib.h"
#include "support/FileInfo.h"
#include "support/lyxmanip.h"
#include "support/lyxalgo.h" // for lyx::count

#include <boost/bind.hpp>
#include <boost/tuple/tuple.hpp>

#include <fstream>
#include <iomanip>
#include <map>
#include <stack>
#include <list>
#include <algorithm>

#include <cstdlib>
#include <cmath>
#include <unistd.h>
#include <sys/types.h>
#include <utime.h>

#ifdef HAVE_LOCALE
#include <locale>
#endif

#ifndef CXX_GLOBAL_CSTD
using std::pow;
#endif

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
using std::for_each;

using lyx::pos_type;
using lyx::textclass_type;

// all these externs should eventually be removed.
extern BufferList bufferlist;

namespace {

const int LYX_FORMAT = 220;

} // namespace anon

Buffer::Buffer(string const & file, bool ronly)
	: niceFile(true), lyx_clean(true), bak_clean(true),
	  unnamed(false), dep_clean(0), read_only(ronly),
	  filename_(file), users(0), ctrs(new Counters)
{
	lyxerr[Debug::INFO] << "Buffer::Buffer()" << endl;
//	filename = file;
	filepath_ = OnlyPath(file);
//	lyx_clean = true;
//	bak_clean = true;
//	dep_clean = 0;
//	read_only = ronly;
//	unnamed = false;
//	users = 0;
	lyxvc.buffer(this);
	if (read_only || lyxrc.use_tempdir) {
		tmppath = CreateBufferTmpDir();
	} else {
		tmppath.erase();
	}
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

	paragraphs.clear();

	// Remove any previewed LaTeX snippets assocoated with this buffer.
	grfx::Previews::get().removeLoader(this);
}


string const Buffer::getLatexName(bool no_path) const
{
	string const name = ChangeExtension(MakeLatexName(fileName()), ".tex");
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
		users->owner()->getDialogs().updateBufferDependent(false);
	}
}


/// Update window titles of all users
// Should work on a list
void Buffer::updateTitles() const
{
	if (users)
		users->owner()->updateWindowTitle();
}


/// Reset autosave timer of all users
// Should work on a list
void Buffer::resetAutosaveTimers() const
{
	if (users)
		users->owner()->resetAutosaveTimer();
}


void Buffer::setFileName(string const & newfile)
{
	filename_ = MakeAbsPath(newfile);
	filepath_ = OnlyPath(filename_);
	setReadonly(IsFileWriteable(filename_) == 0);
	updateTitles();
}


// We'll remove this later. (Lgb)
namespace {

string last_inset_read;

#ifdef WITH_WARNINGS
#warning And _why_ is this here? (Lgb)
#endif
int unknown_layouts;
int unknown_tokens;

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
	unknown_tokens = 0;

	int pos = 0;
	Paragraph::depth_type depth = 0;
	bool the_end_read = false;

	Paragraph * first_par = 0;
	LyXFont font(LyXFont::ALL_INHERIT, params.language);

#if 0
	if (file_format < 216 && params.language->lang() == "hebrew")
		font.setLanguage(default_language);
#endif

	if (!par) {
		par = new Paragraph;
		par->layout(params.getLyXTextClass().defaultLayout());
	} else {
		// We are inserting into an existing document
		users->text->breakParagraph(users);
		first_par = users->text->ownerParagraph();
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

	paragraphs.set(first_par);

	if (unknown_layouts > 0) {
		string s = _("Couldn't set the layout for ");
		if (unknown_layouts == 1) {
			s += _("one paragraph");
		} else {
			s += tostr(unknown_layouts);
			s += _(" paragraphs");
		}
		Alert::alert(_("Textclass Loading Error!"), s,
			   _("When reading " + fileName()));
	}

	if (unknown_tokens > 0) {
		string s = _("Encountered ");
		if (unknown_tokens == 1) {
			s += _("one unknown token");
		} else {
			s += tostr(unknown_tokens);
			s += _(" unknown tokens");
		}
		Alert::alert(_("Textclass Loading Error!"), s,
			   _("When reading " + fileName()));
	}

	return the_end_read;
}


bool
Buffer::parseSingleLyXformat2Token(LyXLex & lex, Paragraph *& par,
				   Paragraph *& first_par,
				   string const & token, int & pos,
				   Paragraph::depth_type & depth,
				   LyXFont & font
	)
{
	bool the_end_read = false;

	// The order of the tags tested may seem unnatural, but this
	// has been done in order to reduce the number of string
	// comparisons needed to recognize a given token. This leads
	// on large documents like UserGuide to a reduction of a
	// factor 5! (JMarc)
	if (token[0] != '\\') {
		for (string::const_iterator cit = token.begin();
		     cit != token.end(); ++cit) {
			par->insertChar(pos, (*cit), font);
			++pos;
		}
	} else if (token == "\\layout") {
		// reset the font as we start a new layout and if the font is
		// not ALL_INHERIT,document_language then it will be set to the
		// right values after this tag (Jug 20020420)
		font = LyXFont(LyXFont::ALL_INHERIT, params.language);

#if 0
		if (file_format < 216 && params.language->lang() == "hebrew")
			font.setLanguage(default_language);
#endif

		lex.eatLine();
		string layoutname = lex.getString();

		LyXTextClass const & tclass = params.getLyXTextClass();

		if (layoutname.empty()) {
			layoutname = tclass.defaultLayoutName();
		}
		bool hasLayout = tclass.hasLayout(layoutname);
		if (!hasLayout) {
			lyxerr << "Layout '" << layoutname << "' does not"
			       << " exist in textclass '" << tclass.name()
			       << "'." << endl;
			lyxerr << "Trying to use default layout instead."
			       << endl;
			layoutname = tclass.defaultLayoutName();
		}

#ifdef USE_CAPTION
		// The is the compability reading of layout caption.
		// It can be removed in LyX version 1.3.0. (Lgb)
		if (compare_ascii_no_case(layoutname, "caption") == 0) {
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
				par->layout(params.getLyXTextClass().defaultLayout());
			}
			pos = 0;
			par->layout(params.getLyXTextClass()[layoutname]);
			// Test whether the layout is obsolete.
			LyXLayout_ptr const & layout = par->layout();
			if (!layout->obsoleted_by().empty())
				par->layout(params.getLyXTextClass()[layout->obsoleted_by()]);
			par->params().depth(depth);
#if USE_CAPTION
		}
#endif

	} else if (token == "\\end_inset") {
		lyxerr << "Solitary \\end_inset. Missing \\begin_inset?.\n"
		       << "Last inset read was: " << last_inset_read
		       << endl;
		// Simply ignore this. The insets do not have
		// to read this.
		// But insets should read it, it is a part of
		// the inset isn't it? Lgb.
	} else if (token == "\\begin_inset") {
		readInset(lex, par, pos, font);
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
	} else if (token == "\\SpecialChar") {
		LyXLayout_ptr const & layout = par->layout();

		// Insets don't make sense in a free-spacing context! ---Kayvan
		if (layout->free_spacing || par->isFreeSpacing()) {
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
	} else if (token == "\\i") {
		Inset * inset = new InsetLatexAccent;
		inset->read(this, lex);
		par->insertInset(pos, inset, font);
		++pos;
	} else if (token == "\\backslash") {
		par->insertChar(pos, '\\', font);
		++pos;
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
		pair<bool, textclass_type> pp =
			textclasslist.NumberOfClass(lex.getString());
		if (pp.first) {
			params.textclass = pp.second;
		} else {
			Alert::alert(string(_("Textclass error")),
				string(_("The document uses an unknown textclass \"")) +
				lex.getString() + string("\"."),
				string(_("LyX will not be able to produce output correctly.")));
			params.textclass = 0;
		}
		if (!params.getLyXTextClass().load()) {
			// if the textclass wasn't loaded properly
			// we need to either substitute another
			// or stop loading the file.
			// I can substitute but I don't see how I can
			// stop loading... ideas??  ARRae980418
			Alert::alert(_("Textclass Loading Error!"),
				   string(_("Can't load textclass ")) +
				   params.getLyXTextClass().name(),
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
	} else if (token == "\\leftindent") {
		lex.nextToken();
		LyXLength value(lex.getString());
		par->params().leftIndent(value);
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
		if (tmpret == -1)
			++tmpret;
		params.paragraph_separation =
			static_cast<BufferParams::PARSEP>(tmpret);
	} else if (token == "\\defskip") {
		lex.nextToken();
		params.defskip = VSpace(lex.getString());
	} else if (token == "\\quotes_language") {
		int tmpret = lex.findToken(string_quotes_language);
		if (tmpret == -1)
			++tmpret;
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
		if (tmpret == -1)
			++tmpret;
		params.orientation =
			static_cast<BufferParams::PAPER_ORIENTATION>(tmpret);
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
		params.fontsize = rtrim(lex.getString());
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
		params.pagestyle = rtrim(lex.getString());
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
		// The bullet class should be able to read this.
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
		string const tmp = rtrim(lex.getString());
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
		string const tmp = rtrim(lex.getString());
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
	} else if (token == "\\align") {
		int tmpret = lex.findToken(string_align);
		if (tmpret == -1) ++tmpret;
		int const tmpret2 = int(pow(2.0, tmpret));
		par->params().align(LyXAlignment(tmpret2));
	} else if (token == "\\added_space_top") {
		lex.nextToken();
		VSpace value = VSpace(lex.getString());
		// only add the length when value > 0 or
		// with option keep
		if ((value.length().len().value() != 0) ||
		    value.keep() ||
		    (value.kind() != VSpace::LENGTH))
			par->params().spaceTop(value);
	} else if (token == "\\added_space_bottom") {
		lex.nextToken();
		VSpace value = VSpace(lex.getString());
		// only add the length when value > 0 or
		// with option keep
		if ((value.length().len().value() != 0) ||
		   value.keep() ||
		    (value.kind() != VSpace::LENGTH))
			par->params().spaceBottom(value);
	} else if (token == "\\labelwidthstring") {
		lex.eatLine();
		par->params().labelWidthString(lex.getString());
		// do not delete this token, it is still needed!
	} else if (token == "\\newline") {
		par->insertChar(pos, Paragraph::META_NEWLINE, font);
		++pos;
	} else if (token == "\\LyXTable") {
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
		LyXLayout_ptr const & layout = par->layout();

		if (layout->free_spacing || par->isFreeSpacing()) {
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
	} else if (token == "\\the_end") {
		the_end_read = true;
	} else {
		// This should be insurance for the future: (Asger)
		++unknown_tokens;
		lex.eatLine();
		string const s = _("Unknown token: ") + token
			+ " " + lex.text()  + "\n";
		// we can do this here this way because we're actually reading
		// the buffer and don't care about LyXText right now.
		InsetError * new_inset = new InsetError(s);
		par->insertInset(pos, new_inset, LyXFont(LyXFont::ALL_INHERIT,
				 params.language));

	}

	return the_end_read;
}

// needed to insert the selection
void Buffer::insertStringAsLines(Paragraph *& par, pos_type & pos,
				 LyXFont const & fn,string const & str) const
{
	LyXLayout_ptr const & layout = par->layout();

	LyXFont font = fn;

	par->checkInsertChar(font);
	// insert the string, don't insert doublespace
	bool space_inserted = true;
	bool autobreakrows = !par->inInset() ||
		static_cast<InsetText *>(par->inInset())->getAutoBreakRows();
	for(string::const_iterator cit = str.begin();
	    cit != str.end(); ++cit) {
		if (*cit == '\n') {
			if (autobreakrows && (!par->empty() || layout->keepempty)) {
				breakParagraph(params, par, pos,
					       layout->isEnvironment());
				par = par->next();
				pos = 0;
				space_inserted = true;
			} else {
				continue;
			}
			// do not insert consecutive spaces if !free_spacing
		} else if ((*cit == ' ' || *cit == '\t') &&
			   space_inserted && !layout->free_spacing &&
				   !par->isFreeSpacing())
		{
			continue;
		} else if (*cit == '\t') {
			if (!layout->free_spacing && !par->isFreeSpacing()) {
				// tabs are like spaces here
				par->insertChar(pos, ' ', font);
				++pos;
				space_inserted = true;
			} else {
				const pos_type nb = 8 - pos % 8;
				for (pos_type a = 0; a < nb ; ++a) {
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
		if (compare_ascii_no_case(cmdName.substr(0,4), "cite") == 0) {
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
//			inset = new InsetFig(100, 100, *this);
			inset = new InsetGraphics;
		} else if (tmptok == "Graphics") {
			inset = new InsetGraphics;
		} else if (tmptok == "Info") {// backwards compatibility
			inset = new InsetNote(this,
					      lex.getLongString("\\end_inset"),
					      true);
			alreadyread = true;
		} else if (tmptok == "Note") {
			inset = new InsetNote(params);
		} else if (tmptok == "Include") {
			InsetCommandParams p("Include");
			inset = new InsetInclude(p, *this);
		} else if (tmptok == "ERT") {
			inset = new InsetERT(params);
		} else if (tmptok == "Tabular") {
			inset = new InsetTabular(*this);
		} else if (tmptok == "Text") {
			inset = new InsetText(params);
		} else if (tmptok == "Foot") {
			inset = new InsetFoot(params);
		} else if (tmptok == "Marginal") {
			inset = new InsetMarginal(params);
		} else if (tmptok == "OptArg") {
			inset = new InsetOptArg(params);
		} else if (tmptok == "Minipage") {
			inset = new InsetMinipage(params);
		} else if (tmptok == "Float") {
			lex.next();
			string tmptok = lex.getString();
			inset = new InsetFloat(params, tmptok);
#if 0
		} else if (tmptok == "List") {
			inset = new InsetList;
		} else if (tmptok == "Theorem") {
			inset = new InsetList;
#endif
		} else if (tmptok == "Caption") {
			inset = new InsetCaption(params);
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
				Alert::alert(_("Warning!"),
					   _("LyX file format is newer that what"),
					   _("is supported in this LyX version. Expect some problems."));

			} else if (file_format < LYX_FORMAT) {
				// old formats
				if (file_format < 200) {
					Alert::alert(_("ERROR!"),
						   _("Old LyX file format found. "
						     "Use LyX 0.10.x to read this!"));
					return false;
				} else {
					string const command = "lyx2lyx "
						+ QuoteName(filename_);
					cmd_ret const ret = RunCommand(command);
					if (ret.first) {
						Alert::alert(_("ERROR!"),
						     _("An error occured while "
						       "running the conversion script."));
						return false;
					}
					istringstream is(ret.second);
					LyXLex tmplex(0, 0);
					tmplex.setStream(is);
					return readFile(tmplex);
				}
			}
			bool the_end = readLyXformat2(lex, par);
			params.setPaperStuff();

#if 0
			// the_end was added in 213
			if (file_format < 213)
				the_end = true;
#endif

			if (!the_end) {
				Alert::alert(_("Warning!"),
					   _("Reading of document is not complete"),
					   _("Maybe the document is truncated"));
			}
			return true;
		} else { // "\\lyxformat" not found
			Alert::alert(_("ERROR!"), _("Not a LyX file!"));
		}
	} else
		Alert::alert(_("ERROR!"), _("Unable to read file!"));
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

	if (writeFile(fileName())) {
		markClean();
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


bool Buffer::writeFile(string const & fname) const
{
	if (read_only && (fname == fileName())) {
		return false;
	}

	FileInfo finfo(fname);
	if (finfo.exist() && !finfo.writable()) {
		return false;
	}

	ofstream ofs(fname.c_str());
	if (!ofs) {
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
	ParagraphList::iterator pit = paragraphs.begin();
	ParagraphList::iterator pend = paragraphs.end();
	for (; pit != pend; ++pit)
		pit->write(this, ofs, params, depth);

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


namespace {

pair<int, string> const addDepth(int depth, int ldepth)
{
	int d = depth * 2;
	if (ldepth > depth)
		d += (ldepth - depth) * 2;
	return make_pair(d, string(d, ' '));
}

}


string const Buffer::asciiParagraph(Paragraph const & par,
				    unsigned int linelen,
				    bool noparbreak) const
{
	ostringstream buffer;
	Paragraph::depth_type depth = 0;
	int ltype = 0;
	Paragraph::depth_type ltype_depth = 0;
	bool ref_printed = false;
//	if (!par->previous()) {
#if 0
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
#else
	depth = par.params().depth();
#endif

	// First write the layout
	string const & tmp = par.layout()->name();
	if (compare_no_case(tmp, "itemize") == 0) {
		ltype = 1;
		ltype_depth = depth + 1;
	} else if (compare_ascii_no_case(tmp, "enumerate") == 0) {
		ltype = 2;
		ltype_depth = depth + 1;
	} else if (contains(ascii_lowercase(tmp), "ection")) {
		ltype = 3;
		ltype_depth = depth + 1;
	} else if (contains(ascii_lowercase(tmp), "aragraph")) {
		ltype = 4;
		ltype_depth = depth + 1;
	} else if (compare_ascii_no_case(tmp, "description") == 0) {
		ltype = 5;
		ltype_depth = depth + 1;
	} else if (compare_ascii_no_case(tmp, "abstract") == 0) {
		ltype = 6;
		ltype_depth = 0;
	} else if (compare_ascii_no_case(tmp, "bibliography") == 0) {
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
//	} else {
//		lyxerr << "Should this ever happen?" << endl;
//	}

	// linelen <= 0 is special and means we don't have paragraph breaks

	string::size_type currlinelen = 0;

	if (!noparbreak) {
		if (linelen > 0)
			buffer << "\n\n";

		buffer << string(depth * 2, ' ');
		currlinelen += depth * 2;

		//--
		// we should probably change to the paragraph language in the
		// gettext here (if possible) so that strings are outputted in
		// the correct language! (20012712 Jug)
		//--
		switch (ltype) {
		case 0: // Standard
		case 4: // (Sub)Paragraph
		case 5: // Description
			break;
		case 6: // Abstract
			if (linelen > 0) {
				buffer << _("Abstract") << "\n\n";
				currlinelen = 0;
			} else {
				string const abst = _("Abstract: ");
				buffer << abst;
				currlinelen += abst.length();
			}
			break;
		case 7: // Bibliography
			if (!ref_printed) {
				if (linelen > 0) {
					buffer << _("References") << "\n\n";
					currlinelen = 0;
				} else {
					string const refs = _("References: ");
					buffer << refs;
					currlinelen += refs.length();
				}

				ref_printed = true;
			}
			break;
		default:
		{
			string const parlab = par.params().labelString();
			buffer << parlab << " ";
			currlinelen += parlab.length() + 1;
		}
		break;

		}
	}

	if (!currlinelen) {
		pair<int, string> p = addDepth(depth, ltype_depth);
		buffer << p.second;
		currlinelen += p.first;
	}

	// this is to change the linebreak to do it by word a bit more
	// intelligent hopefully! (only in the case where we have a
	// max linelenght!) (Jug)

	string word;

	for (pos_type i = 0; i < par.size(); ++i) {
		char c = par.getUChar(params, i);
		switch (c) {
		case Paragraph::META_INSET:
		{
			Inset const * inset = par.getInset(i);
			if (inset) {
				if (linelen > 0) {
					buffer << word;
					currlinelen += word.length();
					word.erase();
				}
				if (inset->ascii(this, buffer, linelen)) {
					// to be sure it breaks paragraph
					currlinelen += linelen;
				}
			}
		}
		break;

		case Paragraph::META_NEWLINE:
			if (linelen > 0) {
				buffer << word << "\n";
				word.erase();

				pair<int, string> p = addDepth(depth,
							       ltype_depth);
				buffer << p.second;
				currlinelen = p.first;
			}
			break;

		case Paragraph::META_HFILL:
			buffer << word << "\t";
			currlinelen += word.length() + 1;
			word.erase();
			break;

		default:
			if (c == ' ') {
				if (linelen > 0 &&
				    currlinelen + word.length() > linelen - 10) {
					buffer << "\n";
					pair<int, string> p =
						addDepth(depth, ltype_depth);
					buffer << p.second;
					currlinelen = p.first;
				}

				buffer << word << ' ';
				currlinelen += word.length() + 1;
				word.erase();

			} else {
				if (c != '\0') {
					word += c;
				} else {
					lyxerr[Debug::INFO] <<
						"writeAsciiFile: NULL char in structure." << endl;
				}
				if ((linelen > 0) &&
					(currlinelen + word.length()) > linelen)
				{
					buffer << "\n";

					pair<int, string> p =
						addDepth(depth, ltype_depth);
					buffer << p.second;
					currlinelen = p.first;
				}
			}
			break;
		}
	}
	buffer << word;
	return buffer.str().c_str();
}


void Buffer::writeFileAscii(string const & fname, int linelen)
{
	ofstream ofs(fname.c_str());
	if (!ofs) {
		Alert::err_alert(_("Error: Cannot write file:"), fname);
		return;
	}
	writeFileAscii(ofs, linelen);
}


void Buffer::writeFileAscii(ostream & os, int linelen)
{
	ParagraphList::iterator beg = paragraphs.begin();
	ParagraphList::iterator end = paragraphs.end();
	ParagraphList::iterator it = beg;
	for (; it != end; ++it) {
		os << asciiParagraph(*it, linelen, it == beg);
	}
	os << "\n";
}


bool use_babel;


void Buffer::makeLaTeXFile(string const & fname,
			   string const & original_path,
			   bool nice, bool only_body, bool only_preamble)
{
	lyxerr[Debug::LATEX] << "makeLaTeXFile..." << endl;

	ofstream ofs(fname.c_str());
	if (!ofs) {
		Alert::err_alert(_("Error: Cannot open file: "), fname);
		return;
	}

	makeLaTeXFile(ofs, original_path, nice, only_body, only_preamble);

	ofs.close();
	if (ofs.fail()) {
		lyxerr << "File was not closed properly." << endl;
	}
}


void Buffer::makeLaTeXFile(ostream & os,
			   string const & original_path,
			   bool nice, bool only_body, bool only_preamble)
{
	niceFile = nice; // this will be used by Insetincludes.

	// validate the buffer.
	lyxerr[Debug::LATEX] << "  Validating buffer..." << endl;
	LaTeXFeatures features(params);
	validate(features);
	lyxerr[Debug::LATEX] << "  Buffer validation done." << endl;

	texrow.reset();
	// The starting paragraph of the coming rows is the
	// first paragraph of the document. (Asger)
	texrow.start(&*(paragraphs.begin()), 0);

	if (!only_body && nice) {
		os << "%% " << lyx_docversion << " created this file.  "
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
	// input@path is set when the actual parameter
	// original_path is set. This is done for usual tex-file, but not
	// for nice-latex-file. (Matthias 250696)
	if (!only_body) {
		if (!nice) {
			// code for usual, NOT nice-latex-file
			os << "\\batchmode\n"; // changed
			// from \nonstopmode
			texrow.newline();
		}
		if (!original_path.empty()) {
			string inputpath = os::external_path(original_path);
			subst(inputpath, "~", "\\string~");
			os << "\\makeatletter\n"
			    << "\\def\\input@path{{"
			    << inputpath << "/}}\n"
			    << "\\makeatother\n";
			texrow.newline();
			texrow.newline();
			texrow.newline();
		}

		os << "\\documentclass";

		LyXTextClass const & tclass = params.getLyXTextClass();

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
			features.useLanguage(default_language);

		if (lyxrc.language_use_babel ||
		    params.language->lang() != lyxrc.default_language ||
		    features.hasLanguages()) {
			use_babel = true;
			language_options << features.getLanguages();
			language_options << params.language->babel();
			if (lyxrc.language_global_options)
				options << language_options.str() << ',';
		}

		// the user-defined options
		if (!params.options.empty()) {
			options << params.options << ',';
		}

		string strOptions(options.str().c_str());
		if (!strOptions.empty()) {
			strOptions = rtrim(strOptions, ",");
			os << '[' << strOptions << ']';
		}

		os << '{' << tclass.latexname() << "}\n";
		texrow.newline();
		// end of \documentclass defs

		// font selection must be done before loading fontenc.sty
		// The ae package is not needed when using OT1 font encoding.
		if (params.fonts != "default" &&
		    (params.fonts != "ae" || lyxrc.fontenc != "default")) {
			os << "\\usepackage{" << params.fonts << "}\n";
			texrow.newline();
			if (params.fonts == "ae") {
				os << "\\usepackage{aecompl}\n";
				texrow.newline();
			}
		}
		// this one is not per buffer
		if (lyxrc.fontenc != "default") {
			os << "\\usepackage[" << lyxrc.fontenc
			    << "]{fontenc}\n";
			texrow.newline();
		}

		if (params.inputenc == "auto") {
			string const doc_encoding =
				params.language->encoding()->LatexName();

			// Create a list with all the input encodings used
			// in the document
			set<string> encodings = features.getEncodingSet(doc_encoding);

			os << "\\usepackage[";
			std::copy(encodings.begin(), encodings.end(),
				  std::ostream_iterator<string>(os, ","));
			os << doc_encoding << "]{inputenc}\n";
			texrow.newline();
		} else if (params.inputenc != "default") {
			os << "\\usepackage[" << params.inputenc
			    << "]{inputenc}\n";
			texrow.newline();
		}

		// At the very beginning the text parameters.
		if (params.paperpackage != BufferParams::PACKAGE_NONE) {
			switch (params.paperpackage) {
			case BufferParams::PACKAGE_A4:
				os << "\\usepackage{a4}\n";
				texrow.newline();
				break;
			case BufferParams::PACKAGE_A4WIDE:
				os << "\\usepackage{a4wide}\n";
				texrow.newline();
				break;
			case BufferParams::PACKAGE_WIDEMARGINSA4:
				os << "\\usepackage[widemargins]{a4}\n";
				texrow.newline();
				break;
			}
		}
		if (params.use_geometry) {
			os << "\\usepackage{geometry}\n";
			texrow.newline();
			os << "\\geometry{verbose";
			if (params.orientation == BufferParams::ORIENTATION_LANDSCAPE)
				os << ",landscape";
			switch (params.papersize2) {
			case BufferParams::VM_PAPER_CUSTOM:
				if (!params.paperwidth.empty())
					os << ",paperwidth="
					    << params.paperwidth;
				if (!params.paperheight.empty())
					os << ",paperheight="
					    << params.paperheight;
				break;
			case BufferParams::VM_PAPER_USLETTER:
				os << ",letterpaper";
				break;
			case BufferParams::VM_PAPER_USLEGAL:
				os << ",legalpaper";
				break;
			case BufferParams::VM_PAPER_USEXECUTIVE:
				os << ",executivepaper";
				break;
			case BufferParams::VM_PAPER_A3:
				os << ",a3paper";
				break;
			case BufferParams::VM_PAPER_A4:
				os << ",a4paper";
				break;
			case BufferParams::VM_PAPER_A5:
				os << ",a5paper";
				break;
			case BufferParams::VM_PAPER_B3:
				os << ",b3paper";
				break;
			case BufferParams::VM_PAPER_B4:
				os << ",b4paper";
				break;
			case BufferParams::VM_PAPER_B5:
				os << ",b5paper";
				break;
			default:
				// default papersize ie BufferParams::VM_PAPER_DEFAULT
				switch (lyxrc.default_papersize) {
				case BufferParams::PAPER_DEFAULT: // keep compiler happy
				case BufferParams::PAPER_USLETTER:
					os << ",letterpaper";
					break;
				case BufferParams::PAPER_LEGALPAPER:
					os << ",legalpaper";
					break;
				case BufferParams::PAPER_EXECUTIVEPAPER:
					os << ",executivepaper";
					break;
				case BufferParams::PAPER_A3PAPER:
					os << ",a3paper";
					break;
				case BufferParams::PAPER_A4PAPER:
					os << ",a4paper";
					break;
				case BufferParams::PAPER_A5PAPER:
					os << ",a5paper";
					break;
				case BufferParams::PAPER_B5PAPER:
					os << ",b5paper";
					break;
				}
			}
			if (!params.topmargin.empty())
				os << ",tmargin=" << params.topmargin;
			if (!params.bottommargin.empty())
				os << ",bmargin=" << params.bottommargin;
			if (!params.leftmargin.empty())
				os << ",lmargin=" << params.leftmargin;
			if (!params.rightmargin.empty())
				os << ",rmargin=" << params.rightmargin;
			if (!params.headheight.empty())
				os << ",headheight=" << params.headheight;
			if (!params.headsep.empty())
				os << ",headsep=" << params.headsep;
			if (!params.footskip.empty())
				os << ",footskip=" << params.footskip;
			os << "}\n";
			texrow.newline();
		}

		if (tokenPos(tclass.opt_pagestyle(),
			     '|', params.pagestyle) >= 0) {
			if (params.pagestyle == "fancy") {
				os << "\\usepackage{fancyhdr}\n";
				texrow.newline();
			}
			os << "\\pagestyle{" << params.pagestyle << "}\n";
			texrow.newline();
		}

		if (params.secnumdepth != tclass.secnumdepth()) {
			os << "\\setcounter{secnumdepth}{"
			    << params.secnumdepth
			    << "}\n";
			texrow.newline();
		}
		if (params.tocdepth != tclass.tocdepth()) {
			os << "\\setcounter{tocdepth}{"
			    << params.tocdepth
			    << "}\n";
			texrow.newline();
		}

		if (params.paragraph_separation) {
			switch (params.defskip.kind()) {
			case VSpace::SMALLSKIP:
				os << "\\setlength\\parskip{\\smallskipamount}\n";
				break;
			case VSpace::MEDSKIP:
				os << "\\setlength\\parskip{\\medskipamount}\n";
				break;
			case VSpace::BIGSKIP:
				os << "\\setlength\\parskip{\\bigskipamount}\n";
				break;
			case VSpace::LENGTH:
				os << "\\setlength\\parskip{"
				    << params.defskip.length().asLatexString()
				    << "}\n";
				break;
			default: // should never happen // Then delete it.
				os << "\\setlength\\parskip{\\medskipamount}\n";
				break;
			}
			texrow.newline();

			os << "\\setlength\\parindent{0pt}\n";
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

		int const nlines =
			int(lyx::count(preamble.begin(), preamble.end(), '\n'));
		for (int j = 0; j != nlines; ++j) {
			texrow.newline();
		}

		// We try to load babel late, in case it interferes
		// with other packages.
		if (use_babel) {
			string tmp = lyxrc.language_package;
			if (!lyxrc.language_global_options
			    && tmp == "\\usepackage{babel}")
				tmp = string("\\usepackage[") +
					language_options.str().c_str() +
					"]{babel}";
			preamble += tmp + "\n";
			preamble += features.getBabelOptions();
		}

		preamble += "\\makeatother\n";

		os << preamble;

		if (only_preamble)
			return;

		// make the body.
		os << "\\begin{document}\n";
		texrow.newline();
	} // only_body
	lyxerr[Debug::INFO] << "preamble finished, now the body." << endl;

	if (!lyxrc.language_auto_begin) {
		os << subst(lyxrc.language_command_begin, "$$lang",
			     params.language->babel())
		    << endl;
		texrow.newline();
	}

	latexParagraphs(os, &*(paragraphs.begin()), 0, texrow);

	// add this just in case after all the paragraphs
	os << endl;
	texrow.newline();

	if (!lyxrc.language_auto_end) {
		os << subst(lyxrc.language_command_end, "$$lang",
			     params.language->babel())
		    << endl;
		texrow.newline();
	}

	if (!only_body) {
		os << "\\end{document}\n";
		texrow.newline();

		lyxerr[Debug::LATEX] << "makeLaTeXFile...done" << endl;
	} else {
		lyxerr[Debug::LATEX] << "LaTeXFile for inclusion made."
				     << endl;
	}

	// Just to be sure. (Asger)
	texrow.newline();

	lyxerr[Debug::INFO] << "Finished making latex file." << endl;
	lyxerr[Debug::INFO] << "Row count was " << texrow.rows()-1 << "." << endl;

	// we want this to be true outside previews (for insetexternal)
	niceFile = true;
}


//
// LaTeX all paragraphs from par to endpar, if endpar == 0 then to the end
//
void Buffer::latexParagraphs(ostream & ofs, Paragraph * par,
			     Paragraph * endpar, TexRow & texrow,
			     bool moving_arg) const
{
	bool was_title = false;
	bool already_title = false;

	// if only_body
	while (par != endpar) {
		Inset * in = par->inInset();
		// well we have to check if we are in an inset with unlimited
		// length (all in one row) if that is true then we don't allow
		// any special options in the paragraph and also we don't allow
		// any environment other then "Standard" to be valid!
		if ((in == 0) || !in->forceDefaultParagraphs(in)) {
			LyXLayout_ptr const & layout = par->layout();

			if (layout->intitle) {
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

			if (layout->isEnvironment() ||
				!par->params().leftIndent().zero())
			{
				par = par->TeXEnvironment(this, params, ofs, texrow);
			} else {
				par = par->TeXOnePar(this, params, ofs, texrow, moving_arg);
			}
		} else {
			par = par->TeXOnePar(this, params, ofs, texrow, moving_arg);
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
	return params.getLyXTextClass().outputType() == LATEX;
}


bool Buffer::isLinuxDoc() const
{
	return params.getLyXTextClass().outputType() == LINUXDOC;
}


bool Buffer::isLiterate() const
{
	return params.getLyXTextClass().outputType() == LITERATE;
}


bool Buffer::isDocBook() const
{
	return params.getLyXTextClass().outputType() == DOCBOOK;
}


bool Buffer::isSGML() const
{
	LyXTextClass const & tclass = params.getLyXTextClass();

	return tclass.outputType() == LINUXDOC ||
	       tclass.outputType() == DOCBOOK;
}


int Buffer::sgmlOpenTag(ostream & os, Paragraph::depth_type depth, bool mixcont,
			 string const & latexname) const
{
	if (!latexname.empty() && latexname != "!-- --") {
		if (!mixcont)
			os << string(" ",depth);
		os << "<" << latexname << ">";
	}

	if (!mixcont)
		os << endl;

	return mixcont?0:1;
}


int Buffer::sgmlCloseTag(ostream & os, Paragraph::depth_type depth, bool mixcont,
			  string const & latexname) const
{
	if (!latexname.empty() && latexname != "!-- --") {
		if (!mixcont)
			os << endl << string(" ",depth);
		os << "</" << latexname << ">";
	}

	if (!mixcont)
		os << endl;

	return mixcont?0:1;
}


void Buffer::makeLinuxDocFile(string const & fname, bool nice, bool body_only)
{
	ofstream ofs(fname.c_str());

	if (!ofs) {
		Alert::alert(_("LYX_ERROR:"), _("Cannot write file"), fname);
		return;
	}

	niceFile = nice; // this will be used by included files.

	LaTeXFeatures features(params);

	validate(features);

	texrow.reset();

	LyXTextClass const & tclass = params.getLyXTextClass();

	string top_element = tclass.latexname();

	if (!body_only) {
		ofs << "<!doctype linuxdoc system";

		string preamble = params.preamble;
		const string name = nice ? ChangeExtension(filename_, ".sgml")
			 : fname;
		preamble += features.getIncludedFiles(name);
		preamble += features.getLyXSGMLEntities();

		if (!preamble.empty()) {
			ofs << " [ " << preamble << " ]";
		}
		ofs << ">\n\n";

		if (params.options.empty())
			sgmlOpenTag(ofs, 0, false, top_element);
		else {
			string top = top_element;
			top += " ";
			top += params.options;
			sgmlOpenTag(ofs, 0, false, top);
		}
	}

	ofs << "<!-- "  << lyx_docversion
	    << " created this file. For more info see http://www.lyx.org/"
	    << " -->\n";

	Paragraph::depth_type depth = 0; // paragraph depth
	Paragraph * par = &*(paragraphs.begin());
	string item_name;
	vector<string> environment_stack(5);

	while (par) {
		LyXLayout_ptr const & style = par->layout();
		// treat <toc> as a special case for compatibility with old code
		if (par->isInset(0)) {
			Inset * inset = par->getInset(0);
			Inset::Code lyx_code = inset->lyxCode();
			if (lyx_code == Inset::TOC_CODE) {
				string const temp = "toc";
				sgmlOpenTag(ofs, depth, false, temp);

				par = par->next();
				continue;
			}
		}

		// environment tag closing
		for (; depth > par->params().depth(); --depth) {
			sgmlCloseTag(ofs, depth, false, environment_stack[depth]);
			environment_stack[depth].erase();
		}

		// write opening SGML tags
		switch (style->latextype) {
		case LATEX_PARAGRAPH:
			if (depth == par->params().depth()
			   && !environment_stack[depth].empty()) {
				sgmlCloseTag(ofs, depth, false, environment_stack[depth]);
				environment_stack[depth].erase();
				if (depth)
					--depth;
				else
					ofs << "</p>";
			}
			sgmlOpenTag(ofs, depth, false, style->latexname());
			break;

		case LATEX_COMMAND:
			if (depth!= 0)
				sgmlError(par, 0,
					  _("Error : Wrong depth for"
					    " LatexType Command.\n"));

			if (!environment_stack[depth].empty()) {
				sgmlCloseTag(ofs, depth, false, environment_stack[depth]);
				ofs << "</p>";
			}

			environment_stack[depth].erase();
			sgmlOpenTag(ofs, depth, false, style->latexname());
			break;

		case LATEX_ENVIRONMENT:
		case LATEX_ITEM_ENVIRONMENT:
		{
			string const & latexname = style->latexname();

			if (depth == par->params().depth()
			    && environment_stack[depth] != latexname) {
				sgmlCloseTag(ofs, depth, false,
					     environment_stack[depth]);
				environment_stack[depth].erase();
			}
			if (depth < par->params().depth()) {
			       depth = par->params().depth();
			       environment_stack[depth].erase();
			}
			if (environment_stack[depth] != latexname) {
				if (depth == 0) {
					sgmlOpenTag(ofs, depth, false, "p");
				}
				sgmlOpenTag(ofs, depth, false, latexname);

				if (environment_stack.size() == depth + 1)
					environment_stack.push_back("!-- --");
				environment_stack[depth] = latexname;
			}

			if (style->latexparam() == "CDATA")
				ofs << "<![CDATA[";

			if (style->latextype == LATEX_ENVIRONMENT) break;

			if (style->labeltype == LABEL_MANUAL)
				item_name = "tag";
			else
				item_name = "item";

			sgmlOpenTag(ofs, depth + 1, false, item_name);
		}
		break;

		default:
			sgmlOpenTag(ofs, depth, false, style->latexname());
			break;
		}

		simpleLinuxDocOnePar(ofs, par, depth);

		par = par->next();

		ofs << "\n";
		// write closing SGML tags
		switch (style->latextype) {
		case LATEX_COMMAND:
			break;
		case LATEX_ENVIRONMENT:
		case LATEX_ITEM_ENVIRONMENT:
			if (style->latexparam() == "CDATA")
				ofs << "]]>";
			break;
		default:
			sgmlCloseTag(ofs, depth, false, style->latexname());
			break;
		}
	}

	// Close open tags
	for (int i = depth; i >= 0; --i)
		sgmlCloseTag(ofs, depth, false, environment_stack[i]);

	if (!body_only) {
		ofs << "\n\n";
		sgmlCloseTag(ofs, 0, false, top_element);
	}

	ofs.close();
	// How to check for successful close

	// we want this to be true outside previews (for insetexternal)
	niceFile = true;
}


// checks, if newcol chars should be put into this line
// writes newline, if necessary.
namespace {

void sgmlLineBreak(ostream & os, string::size_type & colcount,
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
	p1 = static_cast<PAR_TAG>(p1 & ~p2);
}

} // anon


// Handle internal paragraph parsing -- layout already processed.
void Buffer::simpleLinuxDocOnePar(ostream & os,
	Paragraph * par,
	Paragraph::depth_type /*depth*/)
{
	LyXLayout_ptr const & style = par->layout();

	string::size_type char_line_count = 5;     // Heuristic choice ;-)

	// gets paragraph main font
	LyXFont font_old;
	bool desc_on;
	if (style->labeltype == LABEL_MANUAL) {
		font_old = style->labelfont;
		desc_on = true;
	} else {
		font_old = style->font;
		desc_on = false;
	}

	LyXFont::FONT_FAMILY family_type = LyXFont::ROMAN_FAMILY;
	LyXFont::FONT_SERIES series_type = LyXFont::MEDIUM_SERIES;
	LyXFont::FONT_SHAPE  shape_type  = LyXFont::UP_SHAPE;
	bool is_em = false;

	stack<PAR_TAG> tag_state;
	// parsing main loop
	for (pos_type i = 0; i < par->size(); ++i) {

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
		while (!tag_state.empty() && tag_close) {
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

		if (style->latexparam() == "CDATA") {
			// "TeX"-Mode on == > SGML-Mode on.
			if (c != '\0')
				os << c;
			++char_line_count;
		} else {
			bool ws;
			string str;
			boost::tie(ws, str) = sgml::escapeChar(c);
			if (ws && !style->free_spacing && !par->isFreeSpacing()) {
				// in freespacing mode, spaces are
				// non-breaking characters
				if (desc_on) {// if char is ' ' then...

					++char_line_count;
					sgmlLineBreak(os, char_line_count, 6);
					os << "</tag>";
					desc_on = false;
				} else  {
					sgmlLineBreak(os, char_line_count, 1);
					os << c;
				}
			} else {
				os << str;
				char_line_count += str.length();
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
		sgmlLineBreak(os, char_line_count, 6);
		os << "</tag>";
	}
}


// Print an error message.
void Buffer::sgmlError(Paragraph * /*par*/, int /*pos*/,
	string const & /*message*/) const
{
#ifdef WITH_WARNINGS
#warning This is wrong we cannot insert an inset like this!!!
	// I guess this was Jose' so I explain you more or less why this
	// is wrong. This way you insert something in the paragraph and
	// don't tell it to LyXText (row rebreaking and undo handling!!!)
	// I deactivate this code, have a look at BufferView::insertErrors
	// how you should do this correctly! (Jug 20020315)
#endif
#if 0
	// insert an error marker in text
	InsetError * new_inset = new InsetError(message);
	par->insertInset(pos, new_inset, LyXFont(LyXFont::ALL_INHERIT,
			 params.language));
#endif
}


void Buffer::makeDocBookFile(string const & fname, bool nice, bool only_body)
{
	ofstream ofs(fname.c_str());
	if (!ofs) {
		Alert::alert(_("LYX_ERROR:"), _("Cannot write file"), fname);
		return;
	}

	Paragraph * par = &*(paragraphs.begin());

	niceFile = nice; // this will be used by Insetincludes.

	LaTeXFeatures features(params);
	validate(features);

	texrow.reset();

	LyXTextClass const & tclass = params.getLyXTextClass();
	string top_element = tclass.latexname();

	if (!only_body) {
		ofs << "<!DOCTYPE " << top_element
		    << "  PUBLIC \"-//OASIS//DTD DocBook V4.1//EN\"";

		string preamble = params.preamble;
		const string name = nice ? ChangeExtension(filename_, ".sgml")
			 : fname;
		preamble += features.getIncludedFiles(name);
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
	sgmlOpenTag(ofs, 0, false, top);

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

		LyXLayout_ptr const & style = par->layout();

		// environment tag closing
		for (; depth > par->params().depth(); --depth) {
			if (environment_inner[depth] != "!-- --") {
				item_name = "listitem";
				sgmlCloseTag(ofs, command_depth + depth, false, item_name);
				if (environment_inner[depth] == "varlistentry")
					sgmlCloseTag(ofs, depth+command_depth, false, environment_inner[depth]);
			}
			sgmlCloseTag(ofs, depth + command_depth, false, environment_stack[depth]);
			environment_stack[depth].erase();
			environment_inner[depth].erase();
		}

		if (depth == par->params().depth()
		   && environment_stack[depth] != style->latexname()
		   && !environment_stack[depth].empty()) {
			if (environment_inner[depth] != "!-- --") {
				item_name= "listitem";
				sgmlCloseTag(ofs, command_depth+depth, false, item_name);
				if (environment_inner[depth] == "varlistentry")
					sgmlCloseTag(ofs, depth + command_depth, false, environment_inner[depth]);
			}

			sgmlCloseTag(ofs, depth + command_depth, false, environment_stack[depth]);

			environment_stack[depth].erase();
			environment_inner[depth].erase();
		}

		// Write opening SGML tags.
		switch (style->latextype) {
		case LATEX_PARAGRAPH:
			sgmlOpenTag(ofs, depth + command_depth,
				    false, style->latexname());
			break;

		case LATEX_COMMAND:
			if (depth != 0)
				sgmlError(par, 0,
					  _("Error : Wrong depth for "
					    "LatexType Command.\n"));

			command_name = style->latexname();

			sgmlparam = style->latexparam();
			c_params = split(sgmlparam, c_depth,'|');

			cmd_depth = lyx::atoi(c_depth);

			if (command_flag) {
				if (cmd_depth < command_base) {
					for (Paragraph::depth_type j = command_depth;
					     j >= command_base; --j) {
						sgmlCloseTag(ofs, j, false, command_stack[j]);
						ofs << endl;
					}
					command_depth = command_base = cmd_depth;
				} else if (cmd_depth <= command_depth) {
					for (int j = command_depth;
					     j >= int(cmd_depth); --j) {
						sgmlCloseTag(ofs, j, false, command_stack[j]);
						ofs << endl;
					}
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
			// This is a hack while paragraphs can't have
			// attributes, like id in this case.
			if (par->isInset(0)) {
				Inset * inset = par->getInset(0);
				Inset::Code lyx_code = inset->lyxCode();
				if (lyx_code == Inset::LABEL_CODE) {
					command_name += " id=\"";
					command_name += (static_cast<InsetCommand *>(inset))->getContents();
					command_name += "\"";
					desc_on = 3;
				}
			}

			sgmlOpenTag(ofs, depth + command_depth, false, command_name);

			item_name = c_params.empty()?"title":c_params;
			sgmlOpenTag(ofs, depth + 1 + command_depth, false, item_name);
			break;

		case LATEX_ENVIRONMENT:
		case LATEX_ITEM_ENVIRONMENT:
			if (depth < par->params().depth()) {
				depth = par->params().depth();
				environment_stack[depth].erase();
			}

			if (environment_stack[depth] != style->latexname()) {
				if (environment_stack.size() == depth + 1) {
					environment_stack.push_back("!-- --");
					environment_inner.push_back("!-- --");
				}
				environment_stack[depth] = style->latexname();
				environment_inner[depth] = "!-- --";
				sgmlOpenTag(ofs, depth + command_depth, false, environment_stack[depth]);
			} else {
				if (environment_inner[depth] != "!-- --") {
					item_name= "listitem";
					sgmlCloseTag(ofs, command_depth + depth, false, item_name);
					if (environment_inner[depth] == "varlistentry")
						sgmlCloseTag(ofs, depth + command_depth, false, environment_inner[depth]);
				}
			}

			if (style->latextype == LATEX_ENVIRONMENT) {
				if (!style->latexparam().empty()) {
					if (style->latexparam() == "CDATA")
						ofs << "<![CDATA[";
					else
						sgmlOpenTag(ofs, depth + command_depth, false, style->latexparam());
				}
				break;
			}

			desc_on = (style->labeltype == LABEL_MANUAL);

			environment_inner[depth] = desc_on ? "varlistentry" : "listitem";
			sgmlOpenTag(ofs, depth + 1 + command_depth,
				    false, environment_inner[depth]);

			item_name = desc_on ? "term" : "para";
			sgmlOpenTag(ofs, depth + 1 + command_depth,
				    false, item_name);
			break;
		default:
			sgmlOpenTag(ofs, depth + command_depth,
				    false, style->latexname());
			break;
		}

		simpleDocBookOnePar(ofs, par, desc_on,
				    depth + 1 + command_depth);
		par = par->next();

		string end_tag;
		// write closing SGML tags
		switch (style->latextype) {
		case LATEX_COMMAND:
			end_tag = c_params.empty() ? "title" : c_params;
			sgmlCloseTag(ofs, depth + command_depth,
				     false, end_tag);
			break;
		case LATEX_ENVIRONMENT:
			if (!style->latexparam().empty()) {
				if (style->latexparam() == "CDATA")
					ofs << "]]>";
				else
					sgmlCloseTag(ofs, depth + command_depth, false, style->latexparam());
			}
			break;
		case LATEX_ITEM_ENVIRONMENT:
			if (desc_on == 1) break;
			end_tag= "para";
			sgmlCloseTag(ofs, depth + 1 + command_depth, false, end_tag);
			break;
		case LATEX_PARAGRAPH:
			sgmlCloseTag(ofs, depth + command_depth, false, style->latexname());
			break;
		default:
			sgmlCloseTag(ofs, depth + command_depth, false, style->latexname());
			break;
		}
	}

	// Close open tags
	for (int d = depth; d >= 0; --d) {
		if (!environment_stack[depth].empty()) {
			if (environment_inner[depth] != "!-- --") {
				item_name = "listitem";
				sgmlCloseTag(ofs, command_depth + depth, false, item_name);
			       if (environment_inner[depth] == "varlistentry")
				       sgmlCloseTag(ofs, depth + command_depth, false, environment_inner[depth]);
			}

			sgmlCloseTag(ofs, depth + command_depth, false, environment_stack[depth]);
		}
	}

	for (int j = command_depth; j >= 0 ; --j)
		if (!command_stack[j].empty()) {
			sgmlCloseTag(ofs, j, false, command_stack[j]);
			ofs << endl;
		}

	ofs << "\n\n";
	sgmlCloseTag(ofs, 0, false, top_element);

	ofs.close();
	// How to check for successful close

	// we want this to be true outside previews (for insetexternal)
	niceFile = true;
}


void Buffer::simpleDocBookOnePar(ostream & os,
				 Paragraph * par, int & desc_on,
				 Paragraph::depth_type depth) const
{
	bool emph_flag = false;

	LyXLayout_ptr const & style = par->layout();

	LyXFont font_old = (style->labeltype == LABEL_MANUAL ? style->labelfont : style->font);

	int char_line_count = depth;
	//if (!style.free_spacing)
	//	os << string(depth,' ');

	// parsing main loop
	for (pos_type i = 0; i < par->size(); ++i) {
		LyXFont font = par->getFont(params, i);

		// handle <emphasis> tag
		if (font_old.emph() != font.emph()) {
			if (font.emph() == LyXFont::ON) {
				if (style->latexparam() == "CDATA")
					os << "]]>";
				os << "<emphasis>";
				if (style->latexparam() == "CDATA")
					os << "<![CDATA[";
				emph_flag = true;
			} else if (i) {
				if (style->latexparam() == "CDATA")
					os << "]]>";
				os << "</emphasis>";
				if (style->latexparam() == "CDATA")
					os << "<![CDATA[";
				emph_flag = false;
			}
		}


		if (par->isInset(i)) {
			Inset * inset = par->getInset(i);
			// don't print the inset in position 0 if desc_on == 3 (label)
			if (i || desc_on != 3) {
				if (style->latexparam() == "CDATA")
					os << "]]>";
				inset->docbook(this, os, false);
				if (style->latexparam() == "CDATA")
					os << "<![CDATA[";
			}
		} else {
			char c = par->getChar(i);
			bool ws;
			string str;
			boost::tie(ws, str) = sgml::escapeChar(c);

			if (style->pass_thru) {
				os << c;
			} else if (style->free_spacing || par->isFreeSpacing() || c != ' ') {
					os << str;
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
		if (style->latexparam() == "CDATA")
			os << "]]>";
		os << "</emphasis>";
		if (style->latexparam() == "CDATA")
			os << "<![CDATA[";
	}

	// resets description flag correctly
	if (desc_on == 1) {
		// <term> not closed...
		os << "</term>\n<listitem><para>&nbsp;</para>";
	}
	if (style->free_spacing)
		os << '\n';
}


// chktex should be run with these flags disabled: 3, 22, 25, 30, 38(?)
// Other flags: -wall -v0 -x
int Buffer::runChktex()
{
	if (!users->text) return 0;

	users->owner()->prohibitInput();

	// get LaTeX-Filename
	string const name = getLatexName();
	string path = filePath();

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
	Chktex chktex(lyxrc.chktex_command, name, filePath());
	int res = chktex.run(terr); // run chktex

	if (res == -1) {
		Alert::alert(_("chktex did not work!"),
			   _("Could not run with file:"), name);
	} else if (res > 0) {
		// Insert all errors as errors boxes
		users->insertErrors(terr);
	}

	// if we removed error insets before we ran chktex or if we inserted
	// error insets after we ran chktex, this must be run:
	if (removedErrorInsets || res) {
#warning repaint needed here, or do you mean update() ?
		users->repaint();
		users->fitCursor();
	}
	users->owner()->allowInput();

	return res;
}


void Buffer::validate(LaTeXFeatures & features) const
{
	LyXTextClass const & tclass = params.getLyXTextClass();

	// AMS Style is at document level
	if (params.use_amsmath || tclass.provides(LyXTextClass::amsmath))
		features.require("amsmath");

	for_each(paragraphs.begin(), paragraphs.end(),
		 boost::bind(&Paragraph::validate, _1, boost::ref(features)));

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
					features.require("latexsym");
				}
			} else if (font == 1) {
				features.require("amssymb");
			} else if ((font >= 2 && font <= 5)) {
				features.require("pifont");
			}
		}
	}

	if (lyxerr.debugging(Debug::LATEX)) {
		features.showStruct();
	}
}


// This function should be in Buffer because it's a buffer's property (ale)
string const Buffer::getIncludeonlyList(char delim)
{
	string lst;
	for (inset_iterator it = inset_iterator_begin();
	    it != inset_iterator_end(); ++it) {
		if (it->lyxCode() == Inset::INCLUDE_CODE) {
			InsetInclude & inc = static_cast<InsetInclude &>(*it);
			if (inc.isIncludeOnly()) {
				if (!lst.empty())
					lst += delim;
				lst += inc.getRelFileBaseName();
			}
		}
	}
	lyxerr[Debug::INFO] << "Includeonly(" << lst << ')' << endl;
	return lst;
}


vector<string> const Buffer::getLabelList() const
{
	/// if this is a child document and the parent is already loaded
	/// Use the parent's list instead  [ale990407]
	if (!params.parentname.empty()
	    && bufferlist.exists(params.parentname)) {
		Buffer const * tmp = bufferlist.getBuffer(params.parentname);
		if (tmp)
			return tmp->getLabelList();
	}

	vector<string> label_list;
	for (inset_iterator it = inset_const_iterator_begin();
	     it != inset_const_iterator_end(); ++it) {
		vector<string> const l = it->getLabelList();
		label_list.insert(label_list.end(), l.begin(), l.end());
	}
	return label_list;
}


// This is also a buffer property (ale)
vector<pair<string, string> > const Buffer::getBibkeyList() const
{
	typedef pair<string, string> StringPair;
	/// if this is a child document and the parent is already loaded
	/// Use the parent's list instead  [ale990412]
	if (!params.parentname.empty() && bufferlist.exists(params.parentname)) {
		Buffer const * tmp = bufferlist.getBuffer(params.parentname);
		if (tmp)
			return tmp->getBibkeyList();
	}

	vector<StringPair> keys;
	ParagraphList::iterator pit = paragraphs.begin();
	ParagraphList::iterator pend = paragraphs.end();
	for (; pit != pend; ++pit) {
		if (pit->bibkey) {
			string const key = pit->bibkey->getContents();
			string const opt = pit->bibkey->getOptions();
			string const ref = pit->asString(this, false);
			string const info = opt + "TheBibliographyRef" + ref;

			keys.push_back(StringPair(key, info));
		}
	}

	// Might be either using bibtex or a child has bibliography
	if (keys.empty()) {
		for (inset_iterator it = inset_const_iterator_begin();
			it != inset_const_iterator_end(); ++it) {
			// Search for Bibtex or Include inset
			if (it->lyxCode() == Inset::BIBTEX_CODE) {
				vector<StringPair> tmp =
					static_cast<InsetBibtex &>(*it).getKeys(this);
				keys.insert(keys.end(), tmp.begin(), tmp.end());
			} else if (it->lyxCode() == Inset::INCLUDE_CODE) {
				vector<StringPair> const tmp =
					static_cast<InsetInclude &>(*it).getKeys();
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


bool Buffer::dispatch(string const & command, bool * result)
{
	// Split command string into command and argument
	string cmd;
	string line = ltrim(command);
	string const arg = trim(split(line, cmd, ' '));

	return dispatch(lyxaction.LookupFunc(cmd), arg, result);
}


bool Buffer::dispatch(int action, string const & argument, bool * result)
{
	bool dispatched = true;

	switch (action) {
		case LFUN_EXPORT: {
			bool const tmp = Exporter::Export(this, argument, false);
			if (result)
				*result = tmp;
			break;
		}

		default:
			dispatched = false;
	}
	return dispatched;
}


void Buffer::resizeInsets(BufferView * bv)
{
	/// then remove all LyXText in text-insets
	for_each(paragraphs.begin(), paragraphs.end(),
		 boost::bind(&Paragraph::resizeInsetsLyXText, _1, bv));
}


void Buffer::redraw()
{
#warning repaint needed here, or do you mean update() ?
	users->repaint();
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


Counters & Buffer::counters() const
{
	return *ctrs.get();
}


void Buffer::inset_iterator::setParagraph()
{
	while (pit != pend) {
		it = pit->insetlist.begin();
		if (it != pit->insetlist.end())
			return;
		++pit;
	}
}


Inset * Buffer::getInsetFromID(int id_arg) const
{
	for (inset_iterator it = inset_const_iterator_begin();
		 it != inset_const_iterator_end(); ++it)
	{
		if (it->id() == id_arg)
			return &(*it);
		Inset * in = it->getInsetFromID(id_arg);
		if (in)
			return in;
	}
	return 0;
}


Paragraph * Buffer::getParFromID(int id) const
{
	if (id < 0)
		return 0;

	ParagraphList::iterator it = paragraphs.begin();
	ParagraphList::iterator end = paragraphs.end();
	for (; it != end; ++it) {
		if (it->id() == id) {
			return &*it;
		}
		Paragraph * tmp = it->getParFromID(id);
		if (tmp) {
			return tmp;
		}
	}
	return 0;
}


ParIterator Buffer::par_iterator_begin()
{
	return ParIterator(&*(paragraphs.begin()));
}


ParIterator Buffer::par_iterator_end()
{
	return ParIterator();
}


void Buffer::addUser(BufferView * u)
{
	users = u;
}


void Buffer::delUser(BufferView *)
{
	users = 0;
}


Language const * Buffer::getLanguage() const
{
	return params.language;
}


bool Buffer::isClean() const
{
	return lyx_clean;
}


bool Buffer::isBakClean() const
{
	return bak_clean;
}


void Buffer::markClean() const
{
	if (!lyx_clean) {
		lyx_clean = true;
		updateTitles();
	}
	// if the .lyx file has been saved, we don't need an
	// autosave
	bak_clean = true;
}


void Buffer::markBakClean()
{
	bak_clean = true;
}


void Buffer::setUnnamed(bool flag)
{
	unnamed = flag;
}


bool Buffer::isUnnamed()
{
	return unnamed;
}


void Buffer::markDirty()
{
	if (lyx_clean) {
		lyx_clean = false;
		updateTitles();
	}
	bak_clean = false;
	DEPCLEAN * tmp = dep_clean;
	while (tmp) {
		tmp->clean = false;
		tmp = tmp->next;
	}
}


string const & Buffer::fileName() const
{
	return filename_;
}


string const & Buffer::filePath() const
{
	return filepath_;
}


bool Buffer::isReadonly() const
{
	return read_only;
}


BufferView * Buffer::getUser() const
{
	return users;
}


void Buffer::setParentName(string const & name)
{
	params.parentname = name;
}


Buffer::inset_iterator::inset_iterator()
	: pit(0), pend(0)
{}


Buffer::inset_iterator::inset_iterator(base_type p, base_type e)
	: pit(p), pend(e)
{
	setParagraph();
}


Buffer::inset_iterator & Buffer::inset_iterator::operator++()
{
	if (pit != pend) {
		++it;
		if (it == pit->insetlist.end()) {
			++pit;
			setParagraph();
		}
	}
	return *this;
}


Buffer::inset_iterator Buffer::inset_iterator::operator++(int)
{
	inset_iterator tmp = *this;
	++*this;
	return tmp;
}


Buffer::inset_iterator::reference Buffer::inset_iterator::operator*()
{
	return *it.getInset();
}


Buffer::inset_iterator::pointer Buffer::inset_iterator::operator->()
{
	return it.getInset();
}


Paragraph * Buffer::inset_iterator::getPar()
{
	return &(*pit);
}


lyx::pos_type Buffer::inset_iterator::getPos() const
{
	return it.getPos();
}


bool operator==(Buffer::inset_iterator const & iter1,
		Buffer::inset_iterator const & iter2)
{
	return iter1.pit == iter2.pit
		&& (iter1.pit == iter1.pend || iter1.it == iter2.it);
}


bool operator!=(Buffer::inset_iterator const & iter1,
		Buffer::inset_iterator const & iter2)
{
	return !(iter1 == iter2);
}
