// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1998-2000 The LyX Team
 *
 *======================================================*/

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include <X11/Xlib.h>

#include "debug.h"
#include "LColor.h"
#include "support/LAssert.h"
#include "gettext.h"
#include "support/lstrings.h"


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


LColor::LColor()
{
	//  LColor::color, gui, latex, x11, lyx
	fill(none, _("none"), "none", "black", "none");
	fill(black, _("black"), "black", "black", "black");
	fill(white, _("white"), "white", "white", "white");
	fill(red, _("red"), "red", "red", "red");
	fill(green, _("green"), "green", "green", "green");
	fill(blue, _("blue"), "blue", "blue", "blue");
	fill(cyan, _("cyan"), "cyan", "cyan", "cyan");
	fill(magenta, _("magenta"), "magenta", "magenta", "magenta");
	fill(yellow, _("yellow"), "yellow", "yellow", "yellow");
	fill(background, _("background"), "background", "linen", "background");
	fill(foreground, _("foreground"), "foreground", "black", "foreground");
	fill(selection, _("selection"), "selection", "LightBlue", "selection");
	fill(latex, _("latex"), "latex", "DarkRed", "latex");
	fill(floats, _("floats"), "floats", "red", "floats");
	fill(note, _("note"), "note", "black", "note");
	fill(notebg, _("note background"), "notebg", "yellow", "notebg");
	fill(noteframe, _("note frame"), "noteframe", "black", "noteframe");
	fill(command, _("command-inset"), "command", "black", "command");
	fill(commandbg, _("command-inset background"), "commandbg", "grey80", "commandbg");
	fill(commandframe, _("inset frame"), "commandframe", "black", "commandframe");
	fill(accent, _("accent"), "accent", "black", "accent");
	fill(accentbg, _("accent background"), "accentbg", "offwhite", "accentbg");
	fill(accentframe, _("accent frame"), "accentframe", "linen", "accentframe");
	fill(minipageline, _("minipage line"), "minipageline", "violet", "minipageline");
	fill(special, _("special char"), "special", "RoyalBlue", "special");
	fill(math, _("math"), "math", "DarkBlue", "math");
	fill(mathbg, _("math background"), "mathbg", "AntiqueWhite", "mathbg");
	fill(mathframe, _("math frame"), "mathframe", "Magenta", "mathframe");
	fill(mathcursor, _("math cursor"), "mathcursor", "black", "mathcursor");
	fill(mathline, _("math line"), "mathline", "Blue", "mathline");
	fill(footnote, _("footnote"), "footnote", "DarkRed", "footnote");
	fill(footnotebg, _("footnote background"), "footnotebg", "grey40", "footnotebg");
	fill(footnoteframe, _("footnote frame"), "footnoteframe", "IndianRed", "footnoteframe");
	fill(ert, _("ert"), "ert", "DarkRed", "ert");
	fill(inset, _("inset"), "inset", "black", "inset");
	fill(insetbg, _("inset background"), "insetbg", "grey40", "insetbg");
	fill(insetframe, _("inset frame"), "insetframe", "IndianRed", "insetframe");
	fill(error, _("error"), "error", "Red", "error");
	fill(eolmarker, _("end-of-line marker"), "eolmarker", "Brown", "eolmarker");
	fill(appendixline, _("appendix line"), "appendixline", "Brown", "appendixline");
	fill(vfillline, _("vfill line"), "vfillline", "Brown", "vfillline");
	fill(topline, _("top/bottom line"), "topline", "Brown", "topline");
	fill(tableline, _("table line"), "tableline", "black", "tableline");
	fill(bottomarea, _("bottom area"), "bottomarea", "grey40", "bottomarea");
	fill(pagebreak, _("page break"), "pagebreak", "RoyalBlue", "pagebreak");
	fill(top, _("top of button"), "top", "grey80", "top");
	fill(bottom, _("bottom of button"), "bottom", "grey40", "bottom");
	fill(left, _("left of button"), "left", "grey80", "left");
	fill(right, _("right of button"), "right", "grey40", "right");
	fill(buttonbg, _("button background"), "buttonbg", "grey60", "buttonbg");
	fill(inherit, _("inherit"), "inherit", "black", "inherit");
	fill(ignore, _("ignore"), "ignore", "black", "ignore");
}


string LColor::getGUIName(LColor::color c) const
{
	InfoTab::const_iterator ici = infotab.find(c);
	if (ici != infotab.end())
		return (*ici).second.guiname;

	return "none";
}


string LColor::getX11Name(LColor::color c) const
{
	InfoTab::const_iterator ici = infotab.find(c);
	if (ici != infotab.end()) 
		return (*ici).second.x11name;

	lyxerr << "LyX internal error: Missing color"
		" entry in LColor.C for " << int(c) << '\n';
	lyxerr << "Using black.\n";
	return "black";
}


string LColor::getLaTeXName(LColor::color c) const
{
	InfoTab::const_iterator ici = infotab.find(c);
	if (ici != infotab.end())
		return (*ici).second.latexname;
	return "black";
}


string LColor::getLyXName(LColor::color c) const
{
	InfoTab::const_iterator ici = infotab.find(c);
	if (ici != infotab.end())
		return (*ici).second.lyxname;
	return "black";
}


void LColor::setColor(LColor::color col, string const & x11name)
{
	InfoTab::iterator iti = infotab.find(col);
	if (iti != infotab.end()) {
		(*iti).second.x11name = x11name;
		return;
	}
	lyxerr << "LyX internal error: color and such.\n";
	Assert(false);
}


LColor::color LColor::getFromGUIName(string const & guiname) const
{
	InfoTab::const_iterator ici = infotab.begin();
	for (; ici != infotab.end(); ++ici) {
		if (!compare_no_case((*ici).second.guiname, guiname))
			return (*ici).first;
	}
	return LColor::ignore;
}


LColor::color LColor::getFromLyXName(string const & lyxname) const
{
	InfoTab::const_iterator ici = infotab.begin();
	for (; ici != infotab.end(); ++ici) {
		if (!compare_no_case((*ici).second.lyxname, lyxname))
			return (*ici).first;
	}
	return LColor::ignore;
}

// The evil global LColor instance
LColor lcolor;
