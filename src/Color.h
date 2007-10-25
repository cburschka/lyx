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

#include "support/docstring.h"

#include <boost/scoped_ptr.hpp>


namespace lyx {

/**
 * This is a stateless class.
 *
 * It has one basic purposes:
 * To serve as a color-namespace container (the Color enum).
 */
/**
 * \class Color
 *
 * A class holding color definitions and associated names for
 * LaTeX, X11, the GUI, and LyX internally.
 *
 * A color can be one of the following kinds:
 *
 * - A real, predefined color, such as black, white, red or green.
 * - A logical color, such as no color, inherit, math
 */

class Color
// made copyable for same reasons as LyXRC was made copyable. See there for
// explanation.
{
public:
	///
	Color();
	///
	Color(Color const &);
	///
	~Color();
	///
	Color & operator=(Color);

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
	void addColor(ColorCode c, std::string const & lyxname) const;
	///
	class Pimpl;
	///
	boost::scoped_ptr<Pimpl> pimpl_;
};


/// the current color definitions
extern Color lcolor;
/// the system color definitions
extern Color system_lcolor;


struct RGBColor {
	unsigned int r;
	unsigned int g;
	unsigned int b;
	RGBColor() : r(0), g(0), b(0) {}
	RGBColor(unsigned int red, unsigned int green, unsigned int blue)
		: r(red), g(green), b(blue) {}
	/// \param x11hexname is of the form "#ffa071"
	RGBColor(std::string const & x11hexname);
};

inline
bool operator==(RGBColor const & c1, RGBColor const & c2)
{
	return (c1.r == c2.r && c1.g == c2.g && c1.b == c2.b);
}


inline
bool operator!=(RGBColor const & c1, RGBColor const & c2)
{
	return !(c1 == c2);
}

/// returns a string of form #rrggbb, given an RGBColor struct
std::string const X11hexname(RGBColor const & col);

} // namespace lyx

#endif
