// -*- C++ -*-
/**
 * \file Color.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 * \author Lars Gullik Bjønnes
 * \author Matthias Ettrich
 * \author Jean-Marc Lasgouttes
 * \author Angus Leeming
 * \author John Levon
 * \author André Pönitz
 * \author Martin Vermeer
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LCOLOR_H
#define LCOLOR_H

#include "ColorCode.h"

#include "support/strfwd.h"

#include <map>
#include <string>

namespace lyx {

/**
 * \class ColorSet
 *
 * A class holding color definitions and associated names for
 * LaTeX, X11, the GUI, and LyX internally.
 *
 * A color can be one of the following kinds:
 *
 * - A real, predefined color, such as black, white, red or green.
 * - A logical color, such as no color, inherit, math
 */


// made copyable for same reasons as LyXRC was made copyable. See there for
// explanation.

class ColorSet
{
public:
	///
	ColorSet();

	/** set the given LyX color to the color defined by the X11 name given
	 *  \returns true if successful.
	 */
	bool setColor(ColorCode col, std::string const & x11name);

	/** set the given LyX color to the color defined by the X11
	 *  name given \returns true if successful. A new color entry
	 *  is created if the color is unknown
	 */
	bool setColor(std::string const & lyxname, std::string const & x11name);

	/// Get the GUI name of \c color.
	docstring const getGUIName(ColorCode c) const;

	/// Get the X11 name of \c color.
	std::string const getX11Name(ColorCode c) const;

	/// Get the LaTeX name of \c color.
	std::string const getLaTeXName(ColorCode c) const;

	/// Get the LyX name of \c color.
	std::string const getLyXName(ColorCode c) const;

	/// \returns the ColorCode associated with the LyX name.
	ColorCode getFromLyXName(std::string const & lyxname) const;
	/// \returns the ColorCode associated with the LaTeX name.
	ColorCode getFromLaTeXName(std::string const & latexname) const;

private:
	///
	void addColor(ColorCode c, std::string const & lyxname); 
	///
	class Information {
	public:
		/// the name as it appears in the GUI
		std::string guiname;
		/// the name used in LaTeX
		std::string latexname;
		/// the name for X11
		std::string x11name;
		/// the name for LyX
		std::string lyxname;
	};

	/// initialise a color entry
	struct ColorEntry;
	void fill(ColorEntry const & entry);

	///
	typedef std::map<ColorCode, Information> InfoTab;
	/// the table of color Information
	InfoTab infotab;

	typedef std::map<std::string, ColorCode> Transform;
	/// the transform between LyX color name string and integer code.
	Transform lyxcolors;
	/// the transform between LaTeX color name string and integer code.
	Transform latexcolors;
};


/// the current color definitions
extern ColorSet lcolor;
/// the system color definitions
extern ColorSet system_lcolor;

std::string const X11hexname(RGBColor const & col);
RGBColor rgbFromHexName(std::string const & x11hexname);

} // namespace lyx

#endif
