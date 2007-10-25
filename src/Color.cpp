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

#include "debug.h"
#include "gettext.h"
#include "Color.h"

#include "support/lstrings.h"

#include <map>
#include <cmath>
#include <sstream>
#include <iomanip>


#ifndef CXX_GLOBAL_CSTD
using std::floor;
#endif

using std::max;
using std::min;
using std::setw;

using std::istringstream;
using std::ostringstream;
using std::string;
using std::endl;

namespace lyx {

using support::compare_ascii_no_case;
using support::ascii_lowercase;

namespace {

struct ColorEntry {
	ColorCode lcolor;
	char const * guiname;
	char const * latexname;
	char const * x11name;
	char const * lyxname;
};

int const nohue = -1;

int hexstrToInt(string const & str)
{
	int val = 0;
	istringstream is(str);
	is >> std::setbase(16) >> val;
	return val;
}

} // namespace anon


/////////////////////////////////////////////////////////////////////
//
// RGBColor
//
/////////////////////////////////////////////////////////////////////


string const X11hexname(RGBColor const & col)
{
	ostringstream ostr;

	ostr << '#' << std::setbase(16) << std::setfill('0')
	     << setw(2) << col.r
	     << setw(2) << col.g
	     << setw(2) << col.b;

	return ostr.str();
}


RGBColor::RGBColor(string const & x11hexname)
	: r(0), g(0), b(0)
{
	BOOST_ASSERT(x11hexname.size() == 7 && x11hexname[0] == '#');
	r = hexstrToInt(x11hexname.substr(1,2));
	g = hexstrToInt(x11hexname.substr(3,2));
	b = hexstrToInt(x11hexname.substr(5,2));
}


/////////////////////////////////////////////////////////////////////
//
// Color::Pimpl
//
/////////////////////////////////////////////////////////////////////

class Color::Pimpl {
public:
	///
	class information {
	public:
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
		information in;
		in.lyxname   = entry.lyxname;
		in.latexname = entry.latexname;
		in.x11name   = entry.x11name;
		in.guiname   = entry.guiname;
		infotab[entry.lcolor] = in;
		lyxcolors[entry.lyxname] = entry.lcolor;
		latexcolors[entry.latexname] = entry.lcolor;
	}

	///
	typedef std::map<ColorCode, information> InfoTab;
	/// the table of color information
	InfoTab infotab;

	typedef std::map<string, ColorCode> Transform;
	/// the transform between LyX color name string and integer code.
	Transform lyxcolors;
	/// the transform between LaTeX color name string and integer code.
	Transform latexcolors;

};


Color::Color()
	: pimpl_(new Pimpl)
{
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
	{ Color_latex, N_("LaTeX text"), "latex", "DarkRed", "latex" },
	{ Color_preview, N_("previewed snippet"), "preview", "black", "preview" },
	{ Color_note, N_("note"), "note", "blue", "note" },
	{ Color_notebg, N_("note background"), "notebg", "yellow", "notebg" },
	{ Color_comment, N_("comment"), "comment", "magenta", "comment" },
	{ Color_commentbg, N_("comment background"), "commentbg", "linen", "commentbg" },
	{ Color_greyedout, N_("greyedout inset"), "greyedout", "red", "greyedout" },
	{ Color_greyedoutbg, N_("greyedout inset background"), "greyedoutbg", "linen", "greyedoutbg" },
	{ Color_shadedbg, N_("shaded box"), "shaded", "#ff0000", "shaded" },
	{ Color_depthbar, N_("depth bar"), "depthbar", "IndianRed", "depthbar" },
	{ Color_language, N_("language"), "language", "Blue", "language" },
	{ Color_command, N_("command inset"), "command", "black", "command" },
	{ Color_commandbg, N_("command inset background"), "commandbg", "azure", "commandbg" },
	{ Color_commandframe, N_("command inset frame"), "commandframe", "black", "commandframe" },
	{ Color_special, N_("special character"), "special", "RoyalBlue", "special" },
	{ Color_math, N_("math"), "math", "DarkBlue", "math" },
	{ Color_mathbg, N_("math background"), "mathbg", "linen", "mathbg" },
	{ Color_graphicsbg, N_("graphics background"), "graphicsbg", "linen", "graphicsbg" },
	{ Color_mathmacrobg, N_("Math macro background"), "mathmacrobg", "linen", "mathmacrobg" },
	{ Color_mathframe, N_("math frame"), "mathframe", "Magenta", "mathframe" },
	{ Color_mathcorners, N_("math corners"), "mathcorners", "linen", "mathcorners" },
	{ Color_mathline, N_("math line"), "mathline", "Blue", "mathline" },
	{ Color_captionframe, N_("caption frame"), "captionframe", "DarkRed", "captionframe" },
	{ Color_collapsable, N_("collapsable inset text"), "collapsable", "DarkRed", "collapsable" },
	{ Color_collapsableframe, N_("collapsable inset frame"), "collapsableframe", "IndianRed", "collapsableframe" },
	{ Color_insetbg, N_("inset background"), "insetbg", "grey80", "insetbg" },
	{ Color_insetframe, N_("inset frame"), "insetframe", "IndianRed", "insetframe" },
	{ Color_error, N_("LaTeX error"), "error", "Red", "error" },
	{ Color_eolmarker, N_("end-of-line marker"), "eolmarker", "Brown", "eolmarker" },
	{ Color_appendix, N_("appendix marker"), "appendix", "Brown", "appendix" },
	{ Color_changebar, N_("change bar"), "changebar", "Blue", "changebar" },
	{ Color_deletedtext, N_("Deleted text"), "deletedtext", "#ff0000", "deletedtext" },
	{ Color_addedtext, N_("Added text"), "addedtext", "#0000ff", "addedtext" },
	{ Color_added_space, N_("added space markers"), "added_space", "Brown", "added_space" },
	{ Color_topline, N_("top/bottom line"), "topline", "Brown", "topline" },
	{ Color_tabularline, N_("table line"), "tabularline", "black", "tabularline" },
	{ Color_tabularonoffline, N_("table on/off line"), "tabularonoffline",
	     "LightSteelBlue", "tabularonoffline" },
	{ Color_bottomarea, N_("bottom area"), "bottomarea", "grey40", "bottomarea" },
	{ Color_pagebreak, N_("page break"), "pagebreak", "RoyalBlue", "pagebreak" },
	{ Color_buttonframe, N_("frame of button"), "buttonframe", "#dcd2c8", "buttonframe" },
	{ Color_buttonbg, N_("button background"), "buttonbg", "#dcd2c8", "buttonbg" },
	{ Color_buttonhoverbg, N_("button background under focus"), "buttonhoverbg", "#C7C7CA", "buttonhoverbg" },
	{ Color_inherit, N_("inherit"), "inherit", "black", "inherit" },
	{ Color_ignore, N_("ignore"), "ignore", "black", "ignore" },
	{ Color_ignore, 0, 0, 0, 0 }
	};

	for (int i = 0; items[i].guiname; ++i)
		pimpl_->fill(items[i]);
}


Color::Color(Color const & c)
	: pimpl_(new Pimpl(*c.pimpl_))
{}


Color::~Color()
{}


Color & Color::operator=(Color tmp)
{
	boost::swap(pimpl_, tmp.pimpl_);
	return *this;
}


docstring const Color::getGUIName(ColorCode c) const
{
	Pimpl::InfoTab::const_iterator it = pimpl_->infotab.find(c);
	if (it != pimpl_->infotab.end())
		return _(it->second.guiname);
	return from_ascii("none");
}


string const Color::getX11Name(ColorCode c) const
{
	Pimpl::InfoTab::const_iterator it = pimpl_->infotab.find(c);
	if (it != pimpl_->infotab.end())
		return it->second.x11name;

	lyxerr << "LyX internal error: Missing color"
		  " entry in Color.cpp for " << c << '\n'
	       << "Using black." << endl;
	return "black";
}


string const Color::getLaTeXName(ColorCode c) const
{
	Pimpl::InfoTab::const_iterator it = pimpl_->infotab.find(c);
	if (it != pimpl_->infotab.end())
		return it->second.latexname;
	return "black";
}


string const Color::getLyXName(ColorCode c) const
{
	Pimpl::InfoTab::const_iterator it = pimpl_->infotab.find(c);
	if (it != pimpl_->infotab.end())
		return it->second.lyxname;
	return "black";
}


bool Color::setColor(ColorCode col, string const & x11name)
{
	Pimpl::InfoTab::iterator it = pimpl_->infotab.find(col);
	if (it == pimpl_->infotab.end()) {
		lyxerr << "Color " << col << " not found in database."
		       << std::endl;
		return false;
	}

	// "inherit" is returned for colors not in the database
	// (and anyway should not be redefined)
	if (col == Color_none || col == Color_inherit || col == Color_ignore) {
		lyxerr << "Color " << getLyXName(col)
		       << " may not be redefined" << endl;
		return false;
	}

	it->second.x11name = x11name;
	return true;
}


bool Color::setColor(string const & lyxname, string const &x11name)
{
	string const lcname = ascii_lowercase(lyxname);
	if (pimpl_->lyxcolors.find(lcname) == pimpl_->lyxcolors.end()) {
		LYXERR(Debug::GUI)
			<< "Color::setColor: Unknown color \""
		       << lyxname << '"' << endl;
		addColor(static_cast<ColorCode>(pimpl_->infotab.size()), lcname);
	}

	return setColor(pimpl_->lyxcolors[lcname], x11name);
}


void Color::addColor(ColorCode c, string const & lyxname) const
{
	ColorEntry ce = { c, "", "", "", lyxname.c_str() };
	pimpl_->fill(ce);
}


ColorCode Color::getFromLyXName(string const & lyxname) const
{
	string const lcname = ascii_lowercase(lyxname);
	if (pimpl_->lyxcolors.find(lcname) == pimpl_->lyxcolors.end()) {
		lyxerr << "Color::getFromLyXName: Unknown color \""
		       << lyxname << '"' << endl;
		return Color_none;
	}

	return pimpl_->lyxcolors[lcname];
}


ColorCode Color::getFromLaTeXName(string const & latexname) const
{
	if (pimpl_->latexcolors.find(latexname) == pimpl_->latexcolors.end()) {
		lyxerr << "Color::getFromLaTeXName: Unknown color \""
		       << latexname << '"' << endl;
		return Color_none;
	}

	return pimpl_->latexcolors[latexname];
}


// The evil global Color instance
Color lcolor;
// An equally evil global system Color instance
Color system_lcolor;


} // namespace lyx
