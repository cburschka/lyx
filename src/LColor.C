/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *	    Copyright 1998-2001 The LyX Team
 *
 *======================================================*/

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "debug.h"
#include "LColor.h"
#include "support/LAssert.h"
#include "gettext.h"
#include "support/lstrings.h"

using std::endl;

void LColor::fill(LColor::color col, string const & gui,
		  string const & latex, string const & x11,
		  string const & lyx) {
	information in;
	in.guiname = gui;
	in.latexname = latex;
	in.x11name = x11;
	in.lyxname = lyx;

	infotab[col] = in;
}

struct ColorEntry {
	LColor::color lcolor;
	char const * guiname;
	char const * latexname;
	char const * x11name;
	char const * lyxname;
};


LColor::LColor()
{
	//  LColor::color, gui, latex, x11, lyx
	ColorEntry items[] = {
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
	{ latex, N_("latex text"), "latex", "DarkRed", "latex" },
	{ preview, N_("previewed snippet"), "preview", "black", "preview" },
	{ note, N_("note"), "note", "yellow", "note" },
	{ notebg, N_("note background"), "notebg", "yellow", "notebg" },
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
	{ mathcursor, N_("math cursor"), "mathcursor", "black", "mathcursor" },
	{ mathline, N_("math line"), "mathline", "Blue", "mathline" },
	{ captionframe, N_("caption frame"), "captionframe", "DarkRed", "captionframe" },
	{ collapsable, N_("collapsable inset text"), "collapsable", "DarkRed", "collapsable" },
	{ collapsableframe, N_("collapsable inset frame"), "collapsableframe", "IndianRed", "collapsableframe" },
	{ insetbg, N_("inset background"), "insetbg", "grey80", "insetbg" },
	{ insetframe, N_("inset frame"), "insetframe", "IndianRed", "insetframe" },
	{ error, N_("LaTeX error"), "error", "Red", "error" },
	{ eolmarker, N_("end-of-line marker"), "eolmarker", "Brown", "eolmarker" },
	{ appendixline, N_("appendix line"), "appendixline", "Brown", "appendixline" },
	{ added_space, N_("added space markers"), "added_space", "Brown", "added_space" },
	{ topline, N_("top/bottom line"), "topline", "Brown", "topline" },
	{ tabularline, N_("tabular line"), "tabularline", "black",
	     "tabularline" },
	{ tabularonoffline, N_("tabular on/off line"), "tabularonoffline",
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

	int i = 0;
	while (items[i].guiname) {
		fill(items[i].lcolor, items[i].guiname, items[i].latexname,
		     items[i].x11name, items[i].lyxname);
		++i;
	}
}


string const LColor::getGUIName(LColor::color c) const
{
	InfoTab::const_iterator ici = infotab.find(c);
	if (ici != infotab.end())
		return _(ici->second.guiname);

	return "none";
}


string const LColor::getX11Name(LColor::color c) const
{
	InfoTab::const_iterator ici = infotab.find(c);
	if (ici != infotab.end())
		return ici->second.x11name;

	lyxerr << "LyX internal error: Missing color"
		" entry in LColor.C for " << int(c) << '\n';
	lyxerr << "Using black.\n";
	return "black";
}


string const LColor::getLaTeXName(LColor::color c) const
{
	InfoTab::const_iterator ici = infotab.find(c);
	if (ici != infotab.end())
		return ici->second.latexname;
	return "black";
}


string const LColor::getLyXName(LColor::color c) const
{
	InfoTab::const_iterator ici = infotab.find(c);
	if (ici != infotab.end())
		return ici->second.lyxname;
	return "black";
}


void LColor::setColor(LColor::color col, string const & x11name)
{
	InfoTab::iterator iti = infotab.find(col);
	if (iti != infotab.end()) {
		iti->second.x11name = x11name;
		return;
	}
	lyxerr << "LyX internal error: color and such.\n";
	lyx::Assert(false);
}


bool LColor::setColor(string const & lyxname, string const & x11name)
{
	color col = getFromLyXName (lyxname);

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
	InfoTab::const_iterator ici = infotab.begin();
	InfoTab::const_iterator end = infotab.end();
	for (; ici != end; ++ici) {
		if (!compare_ascii_no_case(_(ici->second.guiname), guiname))
			return ici->first;
	}
	return LColor::inherit;
}


LColor::color LColor::getFromLyXName(string const & lyxname) const
{

	InfoTab::const_iterator ici = infotab.begin();
	InfoTab::const_iterator end = infotab.end();
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
