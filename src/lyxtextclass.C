/**
 * \file lyxtextclass.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 * \author John Levon
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "lyxtextclass.h"
#include "debug.h"
#include "lyxlex.h"
#include "counters.h"
#include "Floating.h"
#include "FloatList.h"

#include "support/lstrings.h"
#include "support/filetools.h"

using lyx::support::LibFileSearch;
using lyx::support::MakeDisplayPath;
using lyx::support::rtrim;
using lyx::support::subst;

using std::endl;
using std::find_if;
using std::remove_if;
using std::string;
using std::ostream;


namespace { // anon

struct compare_name {

	compare_name(string const & name)
		: name_(name)
	{}

	bool operator()(boost::shared_ptr<LyXLayout> const & c)
	{
		return c->name() == name_;
	}

	string name_;

};

} // anon


LyXTextClass::LyXTextClass(string const & fn, string const & cln,
			   string const & desc, bool texClassAvail )
	: name_(fn), latexname_(cln), description_(desc),
	  floatlist_(new FloatList), ctrs_(new Counters), texClassAvail_(texClassAvail)
{
	outputType_ = LATEX;
	columns_ = 1;
	sides_ = OneSide;
	secnumdepth_ = 3;
	tocdepth_ = 3;
	pagestyle_ = "default";
	defaultfont_ = LyXFont(LyXFont::ALL_SANE);
	opt_fontsize_ = "10|11|12";
	opt_pagestyle_ = "empty|plain|headings|fancy";
	provides_ = nothing;
	titletype_ = TITLE_COMMAND_AFTER;
	titlename_ = "maketitle";
	loaded = false;
}


bool LyXTextClass::isTeXClassAvailable() const
{
	return texClassAvail_;
}


bool LyXTextClass::do_readStyle(LyXLex & lexrc, LyXLayout & lay)
{
	lyxerr[Debug::TCLASS] << "Reading style " << lay.name() << endl;
	if (!lay.Read(lexrc, *this)) {
		// Resolve fonts
		lay.resfont = lay.font;
		lay.resfont.realize(defaultfont());
		lay.reslabelfont = lay.labelfont;
		lay.reslabelfont.realize(defaultfont());
		return false; // no errors
	}
	lyxerr << "Error parsing style `" << lay.name() << '\'' << endl;
	return true;
}


enum TextClassTags {
	TC_OUTPUTTYPE = 1,
	TC_INPUT,
	TC_STYLE,
	TC_DEFAULTSTYLE,
	TC_CHARSTYLE,
	TC_ENVIRONMENT,
	TC_NOSTYLE,
	TC_COLUMNS,
	TC_SIDES,
	TC_PAGESTYLE,
	TC_DEFAULTFONT,
	TC_SECNUMDEPTH,
	TC_TOCDEPTH,
	TC_CLASSOPTIONS,
	TC_PREAMBLE,
	TC_PROVIDESAMSMATH,
	TC_PROVIDESNATBIB,
	TC_PROVIDESMAKEIDX,
	TC_PROVIDESURL,
	TC_LEFTMARGIN,
	TC_RIGHTMARGIN,
	TC_FLOAT,
	TC_COUNTER,
	TC_NOFLOAT,
	TC_TITLELATEXNAME,
	TC_TITLELATEXTYPE
};

// Reads a textclass structure from file.
bool LyXTextClass::Read(string const & filename, bool merge)
{
	keyword_item textClassTags[] = {
		{ "charstyle",       TC_CHARSTYLE },
		{ "classoptions",    TC_CLASSOPTIONS },
		{ "columns",         TC_COLUMNS },
		{ "counter",         TC_COUNTER },
		{ "defaultfont",     TC_DEFAULTFONT },
		{ "defaultstyle",    TC_DEFAULTSTYLE },
		{ "environment",     TC_ENVIRONMENT },
		{ "float",           TC_FLOAT },
		{ "input",           TC_INPUT },
		{ "leftmargin",      TC_LEFTMARGIN },
		{ "nofloat",         TC_NOFLOAT },
		{ "nostyle",         TC_NOSTYLE },
		{ "outputtype",      TC_OUTPUTTYPE },
		{ "pagestyle",       TC_PAGESTYLE },
		{ "preamble",        TC_PREAMBLE },
		{ "providesamsmath", TC_PROVIDESAMSMATH },
		{ "providesmakeidx", TC_PROVIDESMAKEIDX },
		{ "providesnatbib",  TC_PROVIDESNATBIB },
		{ "providesurl",     TC_PROVIDESURL },
		{ "rightmargin",     TC_RIGHTMARGIN },
		{ "secnumdepth",     TC_SECNUMDEPTH },
		{ "sides",           TC_SIDES },
		{ "style",           TC_STYLE },
		{ "titlelatexname",  TC_TITLELATEXNAME },
		{ "titlelatextype",  TC_TITLELATEXTYPE },
		{ "tocdepth",        TC_TOCDEPTH }
	};

	if (!merge)
		lyxerr[Debug::TCLASS] << "Reading textclass "
				      << MakeDisplayPath(filename)
				      << endl;
	else
		lyxerr[Debug::TCLASS] << "Reading input file "
				     << MakeDisplayPath(filename)
				     << endl;

	LyXLex lexrc(textClassTags,
		sizeof(textClassTags) / sizeof(textClassTags[0]));
	bool error = false;

	lexrc.setFile(filename);
	if (!lexrc.isOK()) error = true;

	// parsing
	while (lexrc.isOK() && !error) {
		int le = lexrc.lex();

		switch (le) {
		case LyXLex::LEX_FEOF:
			continue;

		case LyXLex::LEX_UNDEF:
			lexrc.printError("Unknown TextClass tag `$$Token'");
			error = true;
			continue;

		default:
			break;
		}

		switch (static_cast<TextClassTags>(le)) {

		case TC_OUTPUTTYPE:   // output type definition
			readOutputType(lexrc);
			break;

		case TC_INPUT: // Include file
			if (lexrc.next()) {
				string tmp = LibFileSearch("layouts",
							    lexrc.getString(),
							    "layout");

				if (Read(tmp, true)) {
					lexrc.printError("Error reading input"
							 "file: "+tmp);
					error = true;
				}
			}
			break;

		case TC_DEFAULTSTYLE:
			if (lexrc.next()) {
				string const name = subst(lexrc.getString(),
							  '_', ' ');
				defaultlayout_ = name;
			}
			break;

		case TC_ENVIRONMENT:
		case TC_STYLE:
			if (lexrc.next()) {
				string const name = subst(lexrc.getString(),
						    '_', ' ');
				if (hasLayout(name)) {
					LyXLayout * lay = operator[](name).get();
					error = do_readStyle(lexrc, *lay);
				} else {
					LyXLayout lay;
					lay.setName(name);
					if (le == TC_ENVIRONMENT)
						lay.is_environment = true;
					if (!(error = do_readStyle(lexrc, lay)))
						layoutlist_.push_back(
							boost::shared_ptr<LyXLayout>(new LyXLayout(lay))
							);

					if (defaultlayout_.empty()) {
						// We do not have a default
						// layout yet, so we choose
						// the first layout we
						// encounter.
						defaultlayout_ = name;
					}
				}
			}
			else {
				lexrc.printError("No name given for style: `$$Token'.");
				error = true;
			}
			break;

		case TC_NOSTYLE:
			if (lexrc.next()) {
				string const style = subst(lexrc.getString(),
						     '_', ' ');
				if (!delete_layout(style))
					lyxerr << "Cannot delete style `"
					       << style << '\'' << endl;
//					lexrc.printError("Cannot delete style"
//							 " `$$Token'");
			}
			break;

		case TC_COLUMNS:
			if (lexrc.next())
				columns_ = lexrc.getInteger();
			break;

		case TC_SIDES:
			if (lexrc.next()) {
				switch (lexrc.getInteger()) {
				case 1: sides_ = OneSide; break;
				case 2: sides_ = TwoSides; break;
				default:
					lyxerr << "Impossible number of page"
						" sides, setting to one."
					       << endl;
					sides_ = OneSide;
					break;
				}
			}
			break;

		case TC_PAGESTYLE:
			lexrc.next();
			pagestyle_ = rtrim(lexrc.getString());
			break;

		case TC_DEFAULTFONT:
			defaultfont_.lyxRead(lexrc);
			if (!defaultfont_.resolved()) {
				lexrc.printError("Warning: defaultfont should "
						 "be fully instantiated!");
				defaultfont_.realize(LyXFont(LyXFont::ALL_SANE));
			}
			break;

		case TC_SECNUMDEPTH:
			lexrc.next();
			secnumdepth_ = lexrc.getInteger();
			break;

		case TC_TOCDEPTH:
			lexrc.next();
			tocdepth_ = lexrc.getInteger();
			break;

			// First step to support options
		case TC_CLASSOPTIONS:
			readClassOptions(lexrc);
			break;

		case TC_PREAMBLE:
			preamble_ = lexrc.getLongString("EndPreamble");
			break;

		case TC_PROVIDESAMSMATH:
			if (lexrc.next() && lexrc.getInteger())
				provides_ |= amsmath;
			break;

		case TC_PROVIDESNATBIB:
			if (lexrc.next() && lexrc.getInteger())
				provides_ |= natbib;
			break;

		case TC_PROVIDESMAKEIDX:
			if (lexrc.next() && lexrc.getInteger())
				provides_ |= makeidx;
			break;

		case TC_PROVIDESURL:
			if (lexrc.next() && lexrc.getInteger())
				provides_ |= url;
			break;

		case TC_LEFTMARGIN:	// left margin type
			if (lexrc.next())
				leftmargin_ = lexrc.getString();
			break;

		case TC_RIGHTMARGIN:	// right margin type
			if (lexrc.next())
				rightmargin_ = lexrc.getString();
			break;
		case TC_CHARSTYLE:
			if (lexrc.next()) {
				string const name = subst(lexrc.getString(), '_', ' ');
				readCharStyle(lexrc, name);
			}
			break;
		case TC_FLOAT:
			readFloat(lexrc);
			break;
		case TC_COUNTER:
			readCounter(lexrc);
			break;
		case TC_TITLELATEXTYPE:
			readTitleType(lexrc);
			break;
		case TC_TITLELATEXNAME:
			if (lexrc.next())
				titlename_ = lexrc.getString();
			break;
		case TC_NOFLOAT:
			if (lexrc.next()) {
				string const nofloat = lexrc.getString();
				floatlist_->erase(nofloat);
			}
			break;
		}
	}

	if (!merge) { // we are at top level here.
		lyxerr[Debug::TCLASS] << "Finished reading textclass "
				      << MakeDisplayPath(filename)
				      << endl;
		if (defaultlayout_.empty()) {
			lyxerr << "Error: Textclass '" << name_
			       << "' is missing a defaultstyle." << endl;
			error = true;
		}
	} else
		lyxerr[Debug::TCLASS] << "Finished reading input file "
				      << MakeDisplayPath(filename)
				      << endl;

	return error;
}


void LyXTextClass::readTitleType(LyXLex & lexrc)
{
	keyword_item titleTypeTags[] = {
		{ "commandafter", TITLE_COMMAND_AFTER },
		{ "environment", TITLE_ENVIRONMENT }
	};

	pushpophelper pph(lexrc, titleTypeTags, TITLE_ENVIRONMENT);

	int le = lexrc.lex();
	switch (le) {
	case LyXLex::LEX_UNDEF:
		lexrc.printError("Unknown output type `$$Token'");
		return;
	case TITLE_COMMAND_AFTER:
	case TITLE_ENVIRONMENT:
		titletype_ = static_cast<LYX_TITLE_LATEX_TYPES>(le);
		break;
	default:
		lyxerr << "Unhandled value " << le
		       << " in LyXTextClass::readTitleType." << endl;

		break;
	}
}


void LyXTextClass::readOutputType(LyXLex & lexrc)
{
	keyword_item outputTypeTags[] = {
		{ "docbook", DOCBOOK },
		{ "latex", LATEX },
		{ "linuxdoc", LINUXDOC },
		{ "literate", LITERATE }
	};

	pushpophelper pph(lexrc, outputTypeTags, LITERATE);

	int le = lexrc.lex();
	switch (le) {
	case LyXLex::LEX_UNDEF:
		lexrc.printError("Unknown output type `$$Token'");
		return;
	case LATEX:
	case LINUXDOC:
	case DOCBOOK:
	case LITERATE:
		outputType_ = static_cast<OutputType>(le);
		break;
	default:
		lyxerr << "Unhandled value " << le
		       << " in LyXTextClass::readOutputType." << endl;

		break;
	}
}


enum MaxCounterTags {
	MC_COUNTER_CHAPTER = 1,
	MC_COUNTER_SECTION,
	MC_COUNTER_SUBSECTION,
	MC_COUNTER_SUBSUBSECTION,
	MC_COUNTER_PARAGRAPH,
	MC_COUNTER_SUBPARAGRAPH,
	MC_COUNTER_ENUMI,
	MC_COUNTER_ENUMII,
	MC_COUNTER_ENUMIII,
	MC_COUNTER_ENUMIV
};


enum ClassOptionsTags {
	CO_FONTSIZE = 1,
	CO_PAGESTYLE,
	CO_OTHER,
	CO_HEADER,
	CO_END
};


void LyXTextClass::readClassOptions(LyXLex & lexrc)
{
	keyword_item classOptionsTags[] = {
		{"end", CO_END },
		{"fontsize", CO_FONTSIZE },
		{"header", CO_HEADER },
		{"other", CO_OTHER },
		{"pagestyle", CO_PAGESTYLE }
	};

	lexrc.pushTable(classOptionsTags, CO_END);
	bool getout = false;
	while (!getout && lexrc.isOK()) {
		int le = lexrc.lex();
		switch (le) {
		case LyXLex::LEX_UNDEF:
			lexrc.printError("Unknown ClassOption tag `$$Token'");
			continue;
		default: break;
		}
		switch (static_cast<ClassOptionsTags>(le)) {
		case CO_FONTSIZE:
			lexrc.next();
			opt_fontsize_ = rtrim(lexrc.getString());
			break;
		case CO_PAGESTYLE:
			lexrc.next();
			opt_pagestyle_ = rtrim(lexrc.getString());
			break;
		case CO_OTHER:
			lexrc.next();
			options_ = lexrc.getString();
			break;
		case CO_HEADER:
			lexrc.next();
			class_header_ = subst(lexrc.getString(), "&quot;", "\"");
			break;
		case CO_END:
			getout = true;
			break;
		}
	}
	lexrc.popTable();
}

enum CharStyleTags {
	CS_FONT = 1,
	CS_LABELFONT,
	CS_LATEXTYPE,
	CS_LATEXNAME,
	CS_LATEXPARAM,
	CS_PREAMBLE,
	CS_END
};


void LyXTextClass::readCharStyle(LyXLex & lexrc, string const & name)
{
	keyword_item elementTags[] = {
		{ "end", CS_END },
		{ "font", CS_FONT },
		{ "labelfont", CS_LABELFONT },
		{ "latexname", CS_LATEXNAME },
		{ "latexparam", CS_LATEXPARAM },
		{ "latextype", CS_LATEXTYPE },
		{ "preamble", CS_PREAMBLE}
	};

	lexrc.pushTable(elementTags, CS_END);

	string latextype;
	string latexname;
	string latexparam;
	LyXFont font(LyXFont::ALL_INHERIT);
	LyXFont labelfont(LyXFont::ALL_INHERIT);
	string preamble;
	
	bool getout = false;
	while (!getout && lexrc.isOK()) {
		int le = lexrc.lex();
		switch (le) {
		case LyXLex::LEX_UNDEF:
			lexrc.printError("Unknown ClassOption tag `$$Token'");
			continue;
		default: break;
		}
		switch (static_cast<CharStyleTags>(le)) {
		case CS_LATEXTYPE:
			lexrc.next();
			latextype = lexrc.getString();
			break;
		case CS_LATEXNAME:
			lexrc.next();
			latexname = lexrc.getString();
			break;
		case CS_LATEXPARAM:
			lexrc.next();
			latexparam = subst(lexrc.getString(), "&quot;", "\"");
			break;
		case CS_LABELFONT:
			labelfont.lyxRead(lexrc);
			break;
		case CS_FONT:
			font.lyxRead(lexrc);
			labelfont = font;
			break;
		case CS_PREAMBLE:
			preamble = lexrc.getLongString("EndPreamble");
			break;
		case CS_END:
			getout = true;
			break;
		}
	}

	//
	// Here add element to list if getout == true
	if (getout) {
		CharStyle cs;
		cs.name = name;
		cs.latextype = latextype;
		cs.latexname = latexname;
		cs.latexparam = latexparam;
		cs.font = font;
		cs.labelfont = labelfont;
		cs.preamble = preamble;
		charstyles().push_back(cs);
	}

	lexrc.popTable();
}


enum FloatTags {
	FT_TYPE = 1,
	FT_NAME,
	FT_PLACEMENT,
	FT_EXT,
	FT_WITHIN,
	FT_STYLE,
	FT_LISTNAME,
	FT_BUILTIN,
	FT_END
};


void LyXTextClass::readFloat(LyXLex & lexrc)
{
	keyword_item floatTags[] = {
		{ "end", FT_END },
		{ "extension", FT_EXT },
		{ "guiname", FT_NAME },
		{ "latexbuiltin", FT_BUILTIN },
		{ "listname", FT_LISTNAME },
		{ "numberwithin", FT_WITHIN },
		{ "placement", FT_PLACEMENT },
		{ "style", FT_STYLE },
		{ "type", FT_TYPE }
	};

	lexrc.pushTable(floatTags, FT_END);

	string type;
	string placement;
	string ext;
	string within;
	string style;
	string name;
	string listname;
	bool builtin = false;

	bool getout = false;
	while (!getout && lexrc.isOK()) {
		int le = lexrc.lex();
		switch (le) {
		case LyXLex::LEX_UNDEF:
			lexrc.printError("Unknown ClassOption tag `$$Token'");
			continue;
		default: break;
		}
		switch (static_cast<FloatTags>(le)) {
		case FT_TYPE:
			lexrc.next();
			type = lexrc.getString();
			// Here we could check if this type is already defined
			// and modify it with the rest of the vars instead.
			break;
		case FT_NAME:
			lexrc.next();
			name = lexrc.getString();
			break;
		case FT_PLACEMENT:
			lexrc.next();
			placement = lexrc.getString();
			break;
		case FT_EXT:
			lexrc.next();
			ext = lexrc.getString();
			break;
		case FT_WITHIN:
			lexrc.next();
			within = lexrc.getString();
			if (within == "none")
				within.erase();
			break;
		case FT_STYLE:
			lexrc.next();
			style = lexrc.getString();
			break;
		case FT_LISTNAME:
			lexrc.next();
			listname = lexrc.getString();
			break;
		case FT_BUILTIN:
			lexrc.next();
			builtin = lexrc.getBool();
			break;
		case FT_END:
			getout = true;
			break;
		}
	}

	// Here if have a full float if getout == true
	if (getout) {
		Floating newfloat(type, placement, ext, within,
				  style, name, listname, builtin);
		floatlist_->newFloat(newfloat);
	}

	lexrc.popTable();
}


enum CounterTags {
	CT_NAME = 1,
	CT_WITHIN,
	CT_END
};

void LyXTextClass::readCounter(LyXLex & lexrc)
{
	keyword_item counterTags[] = {
		{ "end", CT_END },
		{ "name", CT_NAME },
		{ "within", CT_WITHIN }
	};

	lexrc.pushTable(counterTags, CT_END);

	string name;
	string within;

	bool getout = false;
	while (!getout && lexrc.isOK()) {
		int le = lexrc.lex();
		switch (le) {
		case LyXLex::LEX_UNDEF:
			lexrc.printError("Unknown ClassOption tag `$$Token'");
			continue;
		default: break;
		}
		switch (static_cast<CounterTags>(le)) {
		case CT_NAME:
			lexrc.next();
			name = lexrc.getString();
			break;
		case CT_WITHIN:
			lexrc.next();
			within = lexrc.getString();
			if (within == "none")
				within.erase();
			break;
		case CT_END:
			getout = true;
			break;
		}
	}

	// Here if have a full counter if getout == true
	if (getout) {
		if (within.empty()) {
			ctrs_->newCounter(name);
		} else {
			ctrs_->newCounter(name, within);
		}
	}

	lexrc.popTable();
}


LyXFont const & LyXTextClass::defaultfont() const
{
	return defaultfont_;
}


string const & LyXTextClass::leftmargin() const
{
	return leftmargin_;
}


string const & LyXTextClass::rightmargin() const
{
	return rightmargin_;
}


bool LyXTextClass::hasLayout(string const & n) const
{
	string const name = (n.empty() ? defaultLayoutName() : n);

	return find_if(layoutlist_.begin(), layoutlist_.end(),
		       compare_name(name))
		!= layoutlist_.end();
}



LyXLayout_ptr const & LyXTextClass::operator[](string const & name) const
{
	BOOST_ASSERT(!name.empty());

	LayoutList::const_iterator cit =
		find_if(layoutlist_.begin(),
			layoutlist_.end(),
			compare_name(name));

	if (cit == layoutlist_.end()) {
		lyxerr << "We failed to find the layout '" << name
		       << "' in the layout list. You MUST investigate!"
		       << endl;
		for (LayoutList::const_iterator it = layoutlist_.begin();
		         it != layoutlist_.end(); ++it)
			lyxerr  << " " << it->get()->name() << endl;

		// we require the name to exist
		BOOST_ASSERT(false);
	}

	return (*cit);
}



bool LyXTextClass::delete_layout(string const & name)
{
	if (name == defaultLayoutName())
		return false;

	LayoutList::iterator it =
		remove_if(layoutlist_.begin(), layoutlist_.end(),
			  compare_name(name));

	LayoutList::iterator end = layoutlist_.end();
	bool const ret = (it != end);
	layoutlist_.erase(it, end);
	return ret;
}


// Load textclass info if not loaded yet
bool LyXTextClass::load() const
{
	if (loaded)
		return true;

	// Read style-file
	string const real_file = LibFileSearch("layouts", name_, "layout");

	if (const_cast<LyXTextClass*>(this)->Read(real_file)) {
		lyxerr << "Error reading `"
		       << MakeDisplayPath(real_file)
		       << "'\n(Check `" << name_
		       << "')\nCheck your installation and "
			"try Options/Reconfigure..." << endl;
		loaded = false;
	}
	loaded = true;
	return loaded;
}


FloatList & LyXTextClass::floats()
{
	return *floatlist_.get();
}


FloatList const & LyXTextClass::floats() const
{
	return *floatlist_.get();
}


Counters & LyXTextClass::counters() const
{
	return *ctrs_.get();
}


CharStyles::iterator LyXTextClass::charstyle(string const & s) const
{
	CharStyles::iterator cs = charstyles().begin();
	CharStyles::iterator csend = charstyles().end();
	for (; cs != csend; ++cs) {
		if (cs->name == s)
			return cs;
	}
	return csend;
}


string const & LyXTextClass::defaultLayoutName() const
{
	// This really should come from the actual layout... (Lgb)
	return defaultlayout_;
}


LyXLayout_ptr const & LyXTextClass::defaultLayout() const
{
	return operator[](defaultLayoutName());
}


string const & LyXTextClass::name() const
{
	return name_;
}


string const & LyXTextClass::latexname() const
{
	const_cast<LyXTextClass*>(this)->load();
	return latexname_;
}


string const & LyXTextClass::description() const
{
	return description_;
}


string const & LyXTextClass::opt_fontsize() const
{
	return opt_fontsize_;
}


string const & LyXTextClass::opt_pagestyle() const
{
	return opt_pagestyle_;
}


string const & LyXTextClass::options() const
{
	return options_;
}


string const & LyXTextClass::class_header() const
{
	return class_header_;
}


string const & LyXTextClass::pagestyle() const
{
	return pagestyle_;
}


string const & LyXTextClass::preamble() const
{
	return preamble_;
}


LyXTextClass::PageSides LyXTextClass::sides() const
{
	return sides_;
}


int LyXTextClass::secnumdepth() const
{
	return secnumdepth_;
}


int LyXTextClass::tocdepth() const
{
	return tocdepth_;
}


OutputType LyXTextClass::outputType() const
{
	return outputType_;
}


bool LyXTextClass::provides(LyXTextClass::Provides p) const
{
	return provides_ & p;
}


unsigned int LyXTextClass::columns() const
{
	return columns_;
}


LYX_TITLE_LATEX_TYPES LyXTextClass::titletype() const
{
	return titletype_;
}


string const & LyXTextClass::titlename() const
{
	return titlename_;
}


int LyXTextClass::size() const
{
	return layoutlist_.size();
}


ostream & operator<<(ostream & os, LyXTextClass::PageSides p)
{
	switch (p) {
	case LyXTextClass::OneSide:
		os << '1';
		break;
	case LyXTextClass::TwoSides:
		os << '2';
		break;
	}
	return os;
}
