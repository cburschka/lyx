/**
 * \file Color.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 * \author Lars Gullik Bjønnes
 * \author Matthias Ettrich
 * \author Jean-Marc Lasgouttes
 * \author John Levon
 * \author André Pönitz
 * \author Martin Vermeer
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Color.h"

#include "support/debug.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/lassert.h"

#include <map>
#include <cmath>
#include <sstream>
#include <iomanip>


using namespace std;
using namespace lyx::support;

namespace lyx {


struct ColorSet::ColorEntry {
	ColorCode lcolor;
	char const * guiname;
	char const * latexname;
	char const * x11name;
	char const * lyxname;
};


static int hexstrToInt(string const & str)
{
	int val = 0;
	istringstream is(str);
	is >> setbase(16) >> val;
	return val;
}


/////////////////////////////////////////////////////////////////////
//
// RGBColor
//
/////////////////////////////////////////////////////////////////////


string const X11hexname(RGBColor const & col)
{
	ostringstream ostr;

	ostr << '#' << setbase(16) << setfill('0')
	     << setw(2) << col.r
	     << setw(2) << col.g
	     << setw(2) << col.b;

	return ostr.str();
}


RGBColor rgbFromHexName(string const & x11hexname)
{
	RGBColor c;
	LASSERT(x11hexname.size() == 7 && x11hexname[0] == '#', /**/);
	c.r = hexstrToInt(x11hexname.substr(1, 2));
	c.g = hexstrToInt(x11hexname.substr(3, 2));
	c.b = hexstrToInt(x11hexname.substr(5, 2));
	return c;
}


ColorSet::ColorSet()
{
	char const * grey40 = "#666666";
	char const * grey60 = "#999999";
	char const * grey80 = "#cccccc";
	//char const * grey90 = "#e5e5e5";
	//  ColorCode, gui, latex, x11, lyx
	static ColorEntry const items[] = {
	{ Color_none, N_("none"), "none", "black", "none" },
	{ Color_black, N_("black"), "black", "black", "black" },
	{ Color_white, N_("white"), "white", "white", "white" },
	{ Color_red, N_("red"), "red", "red", "red" },
	{ Color_green, N_("green"), "green", "green", "green" },
	{ Color_blue, N_("blue"), "blue", "blue", "blue" },
	{ Color_cyan, N_("cyan"), "cyan", "cyan", "cyan" },
	{ Color_magenta, N_("magenta"), "magenta", "magenta", "magenta" },
	{ Color_yellow, N_("yellow"), "yellow", "yellow", "yellow" },
	{ Color_cursor, N_("cursor"), "cursor", "black", "cursor" },
	{ Color_background, N_("background"), "background", "linen", "background" },
	{ Color_foreground, N_("text"), "foreground", "black", "foreground" },
	{ Color_selection, N_("selection"), "selection", "LightBlue", "selection" },
	{ Color_selectiontext, N_("selected text"),
		"selectiontext", "black", "selectiontext" },
	{ Color_latex, N_("LaTeX text"), "latex", "DarkRed", "latex" },
	{ Color_inlinecompletion, N_("inline completion"),
		"inlinecompletion", grey60, "inlinecompletion" },
	{ Color_nonunique_inlinecompletion, N_("non-unique inline completion"),
		"nonuniqueinlinecompletion", grey80, "nonuniqueinlinecompletion" },
	{ Color_preview, N_("previewed snippet"), "preview", "black", "preview" },
	{ Color_notelabel, N_("note label"), "note", "yellow", "note" },
	{ Color_notebg, N_("note background"), "notebg", "yellow", "notebg" },
	{ Color_commentlabel, N_("comment label"), "comment", "magenta", "comment" },
	{ Color_commentbg, N_("comment background"), "commentbg", "linen", "commentbg" },
	{ Color_greyedoutlabel, N_("greyedout inset label"), "greyedout", "#ff0080", "greyedout" },
	{ Color_greyedoutbg, N_("greyedout inset background"), "greyedoutbg", "linen", "greyedoutbg" },
	{ Color_shadedbg, N_("shaded box"), "shaded", "#ff0000", "shaded" },
	{ Color_listingsbg, N_("listings background"), "listingsbg", "white", "listingsbg" },
	{ Color_branchlabel, N_("branch label"), "branchlabel", "#c88000", "branchlabel" },
	{ Color_footlabel, N_("footnote label"), "footlabel", "#00aaff", "footlabel" },
	{ Color_indexlabel, N_("index label"), "indexlabel", "green", "indexlabel" },
	{ Color_marginlabel, N_("margin note label"), "marginlabel", "#aa55ff", "marginlabel" },
	{ Color_urllabel, N_("URL label"), "urllabel", "blue", "urllabel" },
	{ Color_urltext, N_("URL text"), "urltext", "blue", "urltext" },
	{ Color_depthbar, N_("depth bar"), "depthbar", "IndianRed", "depthbar" },
	{ Color_language, N_("language"), "language", "Blue", "language" },
	{ Color_command, N_("command inset"), "command", "black", "command" },
	{ Color_commandbg, N_("command inset background"), "commandbg", "azure", "commandbg" },
	{ Color_commandframe, N_("command inset frame"), "commandframe", "black", "commandframe" },
	{ Color_special, N_("special character"), "special", "RoyalBlue", "special" },
	{ Color_math, N_("math"), "math", "DarkBlue", "math" },
	{ Color_mathbg, N_("math background"), "mathbg", "linen", "mathbg" },
	{ Color_graphicsbg, N_("graphics background"), "graphicsbg", "linen", "graphicsbg" },
	{ Color_mathmacrobg, N_("math macro background"), "mathmacrobg", "linen", "mathmacrobg" },
	{ Color_mathframe, N_("math frame"), "mathframe", "Magenta", "mathframe" },
	{ Color_mathcorners, N_("math corners"), "mathcorners", "linen", "mathcorners" },
	{ Color_mathline, N_("math line"), "mathline", "Blue", "mathline" },
	{ Color_mathmacrobg, N_("math macro background"), "mathmacrobg", "#ede2d8", "mathmacrobg" },
	{ Color_mathmacrohoverbg, N_("math macro hovered background"), "mathmacrohoverbg", "#cdc3b8", "mathmacrohoverbg" },
	{ Color_mathmacrolabel, N_("math macro label"), "mathmacrolabel", "#a19992", "mathmacrolabel" },
	{ Color_mathmacroframe, N_("math macro frame"), "mathmacroframe", "#ede2d8", "mathmacroframe" },
	{ Color_mathmacroblend, N_("math macro blended out"), "mathmacroblend", "black", "mathmacroblend" },
	{ Color_mathmacrooldarg, N_("math macro old parameter"), "mathmacrooldarg", grey80, "mathmacrooldarg" },
	{ Color_mathmacronewarg, N_("math macro new parameter"), "mathmacronewarg", "black", "mathmacronewarg" },
	{ Color_captionframe, N_("caption frame"), "captionframe", "DarkRed", "captionframe" },
	{ Color_collapsable, N_("collapsable inset text"), "collapsable", "DarkRed", "collapsable" },
	{ Color_collapsableframe, N_("collapsable inset frame"), "collapsableframe", "IndianRed", "collapsableframe" },
	{ Color_insetbg, N_("inset background"), "insetbg", grey80, "insetbg" },
	{ Color_insetframe, N_("inset frame"), "insetframe", "IndianRed", "insetframe" },
	{ Color_error, N_("LaTeX error"), "error", "Red", "error" },
	{ Color_eolmarker, N_("end-of-line marker"), "eolmarker", "Brown", "eolmarker" },
	{ Color_appendix, N_("appendix marker"), "appendix", "Brown", "appendix" },
	{ Color_changebar, N_("change bar"), "changebar", "Blue", "changebar" },
	{ Color_deletedtext, N_("deleted text"), "deletedtext", "#ff0000", "deletedtext" },
	{ Color_addedtext, N_("added text"), "addedtext", "#0000ff", "addedtext" },
	{ Color_changedtextauthor1, N_("changed text 1st author"), "changedtextauthor1", "#0000ff", "changedtextauthor1" },
	{ Color_changedtextauthor2, N_("changed text 2nd author"), "changedtextauthor2", "#ff00ff", "changedtextauthor2" },
	{ Color_changedtextauthor3, N_("changed text 3rd author"), "changedtextauthor3", "#ff0000", "changedtextauthor3" },
	{ Color_changedtextauthor4, N_("changed text 4th author"), "changedtextauthor4", "#aa00ff", "changedtextauthor4" },
	{ Color_changedtextauthor5, N_("changed text 5th author"), "changedtextauthor5", "#55aa00", "changedtextauthor5" },
	{ Color_added_space, N_("added space markers"), "added_space", "Brown", "added_space" },
	{ Color_topline, N_("top/bottom line"), "topline", "Brown", "topline" },
	{ Color_tabularline, N_("table line"), "tabularline", "black", "tabularline" },
	{ Color_tabularonoffline, N_("table on/off line"), "tabularonoffline",
	     "LightSteelBlue", "tabularonoffline" },
	{ Color_bottomarea, N_("bottom area"), "bottomarea", grey40, "bottomarea" },
	{ Color_newpage, N_("new page"), "newpage", "Blue", "newpage" },
	{ Color_pagebreak, N_("page break / line break"), "pagebreak", "RoyalBlue", "pagebreak" },
	{ Color_buttonframe, N_("frame of button"), "buttonframe", "#dcd2c8", "buttonframe" },
	{ Color_buttonbg, N_("button background"), "buttonbg", "#dcd2c8", "buttonbg" },
	{ Color_buttonhoverbg, N_("button background under focus"), "buttonhoverbg", "#C7C7CA", "buttonhoverbg" },
	{ Color_inherit, N_("inherit"), "inherit", "black", "inherit" },
	{ Color_ignore, N_("ignore"), "ignore", "black", "ignore" },
	{ Color_ignore, 0, 0, 0, 0 }
	};

	for (int i = 0; items[i].guiname; ++i)
		fill(items[i]);
}


/// initialise a color entry
void ColorSet::fill(ColorEntry const & entry)
{
	Information in;
	in.lyxname   = entry.lyxname;
	in.latexname = entry.latexname;
	in.x11name   = entry.x11name;
	in.guiname   = entry.guiname;
	infotab[entry.lcolor] = in;
	lyxcolors[entry.lyxname] = entry.lcolor;
	latexcolors[entry.latexname] = entry.lcolor;
}


docstring const ColorSet::getGUIName(ColorCode c) const
{
	InfoTab::const_iterator it = infotab.find(c);
	if (it != infotab.end())
		return _(it->second.guiname);
	return from_ascii("none");
}


string const ColorSet::getX11Name(ColorCode c) const
{
	InfoTab::const_iterator it = infotab.find(c);
	if (it != infotab.end())
		return it->second.x11name;

	lyxerr << "LyX internal error: Missing color"
		  " entry in Color.cpp for " << c << '\n'
	       << "Using black." << endl;
	return "black";
}


string const ColorSet::getLaTeXName(ColorCode c) const
{
	InfoTab::const_iterator it = infotab.find(c);
	if (it != infotab.end())
		return it->second.latexname;
	return "black";
}


string const ColorSet::getLyXName(ColorCode c) const
{
	InfoTab::const_iterator it = infotab.find(c);
	if (it != infotab.end())
		return it->second.lyxname;
	return "black";
}


bool ColorSet::setColor(ColorCode col, string const & x11name)
{
	InfoTab::iterator it = infotab.find(col);
	if (it == infotab.end()) {
		LYXERR0("Color " << col << " not found in database.");
		return false;
	}

	// "inherit" is returned for colors not in the database
	// (and anyway should not be redefined)
	if (col == Color_none || col == Color_inherit || col == Color_ignore) {
		LYXERR0("Color " << getLyXName(col) << " may not be redefined.");
		return false;
	}

	it->second.x11name = x11name;
	return true;
}


bool ColorSet::setColor(string const & lyxname, string const &x11name)
{
	string const lcname = ascii_lowercase(lyxname);
	if (lyxcolors.find(lcname) == lyxcolors.end()) {
		LYXERR(Debug::GUI, "ColorSet::setColor: Unknown color \""
		       << lyxname << '"');
		addColor(static_cast<ColorCode>(infotab.size()), lcname);
	}

	return setColor(lyxcolors[lcname], x11name);
}


void ColorSet::addColor(ColorCode c, string const & lyxname)
{
	ColorEntry ce = { c, "", "", "", lyxname.c_str() };
	fill(ce);
}


ColorCode ColorSet::getFromLyXName(string const & lyxname) const
{
	string const lcname = ascii_lowercase(lyxname);
	Transform::const_iterator it = lyxcolors.find(lcname);
	if (it == lyxcolors.end()) {
		LYXERR0("ColorSet::getFromLyXName: Unknown color \""
		       << lyxname << '"');
		return Color_none;
	}

	return it->second;
}


ColorCode ColorSet::getFromLaTeXName(string const & latexname) const
{
	Transform::const_iterator it = latexcolors.find(latexname);
	if (it == latexcolors.end()) {
		lyxerr << "ColorSet::getFromLaTeXName: Unknown color \""
		       << latexname << '"' << endl;
		return Color_none;
	}

	return it->second;
}


// The evil global Color instance
ColorSet lcolor;
// An equally evil global system Color instance
ColorSet system_lcolor;


} // namespace lyx
