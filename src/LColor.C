/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *	    Copyright 1998-2001 The LyX Team
 *
 *======================================================*/

#include <config.h>

#include "debug.h"
#include "LColor.h"
#include "support/LAssert.h"
#include "gettext.h"
#include "support/lstrings.h"

#include <map>

using namespace lyx::support;

using std::endl;


namespace {

struct ColorEntry {
	LColor::color lcolor;
	char const * guiname;
	char const * latexname;
	char const * x11name;
	char const * lyxname;
};

}


struct LColor::Pimpl {
	///
	struct information {
		/// the name as it appears in the GUI
		string guiname;
		/// the name used in LaTeX
		string latexname;
		/// the name for X11
		string x11name;
		/// the name for LyX
		string lyxname;
	};

	/// initialise a color entry
	void fill(ColorEntry const & entry)
	{
		information & in = infotab[entry.lcolor];
		in.guiname   = entry.guiname;
		in.latexname = entry.latexname;
		in.x11name   = entry.x11name;
		in.lyxname   = entry.lyxname;
	}

	///
	typedef std::map<LColor::color, information> InfoTab;
	/// the table of color information
	InfoTab infotab;
};


LColor::LColor()
	: pimpl_(new Pimpl)
{
	//  LColor::color, gui, latex, x11, lyx
	static ColorEntry const items[] = {
	{ none, N_("none"), "none", "black", "none" },
	{ black, N_("black"), "black", "black", "black" },
	{ white, N_("white"), "white", "white", "white" },
	{ red, N_("red"), "red", "red", "red" },
	{ green, N_("green"), "green", "green", "green" },
	{ blue, N_("blue"), "blue", "blue", "blue" },
	{ cyan, N_("cyan"), "cyan", "cyan", "cyan" },
	{ magenta, N_("magenta"), "magenta", "magenta", "magenta" },
	{ yellow, N_("yellow"), "yellow", "yellow", "yellow" },
	{ cursor, N_("cursor"), "cursor", "black", "cursor" },
	{ background, N_("background"), "background", "linen", "background" },
	{ foreground, N_("text"), "foreground", "black", "foreground" },
	{ selection, N_("selection"), "selection", "LightBlue", "selection" },
	{ latex, N_("LaTeX text"), "latex", "DarkRed", "latex" },
	{ preview, N_("previewed snippet"), "preview", "black", "preview" },
	{ note, N_("note"), "note", "yellow", "note" },
	{ notebg, N_("note background"), "notebg", "yellow", "notebg" },
	{ comment, N_("comment"), "comment", "magenta", "comment" },
	{ commentbg, N_("comment background"), "commentbg", "linen", "commentbg" },
	{ greyedout, N_("greyedout inset"), "greyedout", "red", "greyedout" },
	{ greyedoutbg, N_("greyedout inset background"), "greyedoutbg", "linen", "greyedoutbg" },
	{ depthbar, N_("depth bar"), "depthbar", "IndianRed", "depthbar" },
	{ language, N_("language"), "language", "Blue", "language" },
	{ command, N_("command inset"), "command", "black", "command" },
	{ commandbg, N_("command inset background"), "commandbg", "azure", "commandbg" },
	{ commandframe, N_("command inset frame"), "commandframe", "black", "commandframe" },
	{ special, N_("special character"), "special", "RoyalBlue", "special" },
	{ math, N_("math"), "math", "DarkBlue", "math" },
	{ mathbg, N_("math background"), "mathbg", "linen", "mathbg" },
	{ graphicsbg, N_("graphics background"), "graphicsbg", "linen", "graphicsbg" },
	{ mathmacrobg, N_("Math macro background"), "mathmacrobg", "linen", "mathmacrobg" },
	{ mathframe, N_("math frame"), "mathframe", "Magenta", "mathframe" },
	{ mathline, N_("math line"), "mathline", "Blue", "mathline" },
	{ captionframe, N_("caption frame"), "captionframe", "DarkRed", "captionframe" },
	{ collapsable, N_("collapsable inset text"), "collapsable", "DarkRed", "collapsable" },
	{ collapsableframe, N_("collapsable inset frame"), "collapsableframe", "IndianRed", "collapsableframe" },
	{ insetbg, N_("inset background"), "insetbg", "grey80", "insetbg" },
	{ insetframe, N_("inset frame"), "insetframe", "IndianRed", "insetframe" },
	{ error, N_("LaTeX error"), "error", "Red", "error" },
	{ eolmarker, N_("end-of-line marker"), "eolmarker", "Brown", "eolmarker" },
	{ appendix, N_("appendix marker"), "appendix", "Brown", "appendix" },
	{ changebar, N_("change bar"), "changebar", "Blue", "changebar" },
	{ strikeout, N_("Deleted text"), "strikeout", "Red", "strikeout" },
	{ newtext, N_("Added text"), "newtext", "Blue", "newtext" },
	{ added_space, N_("added space markers"), "added_space", "Brown", "added_space" },
	{ topline, N_("top/bottom line"), "topline", "Brown", "topline" },
	{ tabularline, N_("table line"), "tabularline", "black",
	     "tabularline" },
	{ tabularonoffline, N_("table on/off line"), "tabularonoffline",
	     "LightSteelBlue", "tabularonoffline" },
	{ bottomarea, N_("bottom area"), "bottomarea", "grey40", "bottomarea" },
	{ pagebreak, N_("page break"), "pagebreak", "RoyalBlue", "pagebreak" },
	{ top, N_("top of button"), "top", "grey90", "top" },
	{ bottom, N_("bottom of button"), "bottom", "grey60", "bottom" },
	{ left, N_("left of button"), "left", "grey90", "left" },
	{ right, N_("right of button"), "right", "grey60", "right" },
	{ buttonbg, N_("button background"), "buttonbg", "grey80", "buttonbg" },
	{ inherit, N_("inherit"), "inherit", "black", "inherit" },
	{ ignore, N_("ignore"), "ignore", "black", "ignore" },
	{ ignore, 0, 0, 0, 0 }
	};

	for (int i = 0; items[i].guiname; ++i)
		pimpl_->fill(items[i]);
}


LColor::LColor(LColor const & c)
	: pimpl_(new Pimpl(*c.pimpl_))
{}


LColor::~LColor()
{
	delete pimpl_;
}


void LColor::operator=(LColor const & c)
{
	LColor tmp(c);
	std::swap(pimpl_, tmp.pimpl_);
}



string const LColor::getGUIName(LColor::color c) const
{
	Pimpl::InfoTab::const_iterator ici = pimpl_->infotab.find(c);
	if (ici != pimpl_->infotab.end())
		return _(ici->second.guiname);
	return "none";
}


string const LColor::getX11Name(LColor::color c) const
{
	Pimpl::InfoTab::const_iterator ici = pimpl_->infotab.find(c);
	if (ici != pimpl_->infotab.end())
		return ici->second.x11name;

	lyxerr << "LyX internal error: Missing color"
		" entry in LColor.C for " << int(c) << endl;
	lyxerr << "Using black." << endl;
	return "black";
}


string const LColor::getLaTeXName(LColor::color c) const
{
	Pimpl::InfoTab::const_iterator ici = pimpl_->infotab.find(c);
	if (ici != pimpl_->infotab.end())
		return ici->second.latexname;
	return "black";
}


string const LColor::getLyXName(LColor::color c) const
{
	Pimpl::InfoTab::const_iterator ici = pimpl_->infotab.find(c);
	if (ici != pimpl_->infotab.end())
		return ici->second.lyxname;
	return "black";
}


void LColor::setColor(LColor::color col, string const & x11name)
{
	Pimpl::InfoTab::iterator iti = pimpl_->infotab.find(col);
	if (iti != pimpl_->infotab.end()) {
		iti->second.x11name = x11name;
		return;
	}
	lyxerr << "LyX internal error: color and such." << endl;
	Assert(false);
}


bool LColor::setColor(string const & lyxname, string const & x11name)
{
	color col = getFromLyXName(lyxname);

	// "inherit" is returned for colors not in the database
	// (and anyway should not be redefined)
	if (col == inherit || col == ignore) {
		lyxerr << "Color " << lyxname << " is undefined or may not be"
			" redefined" << endl;
		return false;
	}
	setColor (col, x11name);
	return true;
}


LColor::color LColor::getFromGUIName(string const & guiname) const
{
	Pimpl::InfoTab::const_iterator ici = pimpl_->infotab.begin();
	Pimpl::InfoTab::const_iterator end = pimpl_->infotab.end();
	for (; ici != end; ++ici) {
		if (!compare_ascii_no_case(_(ici->second.guiname), guiname))
			return ici->first;
	}
	return LColor::inherit;
}


LColor::color LColor::getFromLyXName(string const & lyxname) const
{
	Pimpl::InfoTab::const_iterator ici = pimpl_->infotab.begin();
	Pimpl::InfoTab::const_iterator end = pimpl_->infotab.end();
	for (; ici != end; ++ici) {
		if (!compare_ascii_no_case(ici->second.lyxname, lyxname))
			return ici->first;
	}
	return LColor::inherit;
}


// The evil global LColor instance
LColor lcolor;
// An equally evil global system LColor instance
LColor system_lcolor;
