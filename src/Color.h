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
	/// Names of colors, including all logical colors
	enum color {
		/// No particular color---clear or default
		none,
		/// The different text colors
		black,
		///
		white,
		///
		red,
		///
		green,
		///
		blue,
		///
		cyan,
		///
		magenta,
		///
		yellow,

		// Needed interface colors

		/// Cursor color
		cursor,
		/// Background color
		background,
		/// Foreground color
		foreground,
		/// Background color of selected text
		selection,
		/// Text color in LaTeX mode
		latex,
		/// The color used for previews
		preview,

		/// Text color for notes
		note,
		/// Background color of notes
		notebg,
		/// Text color for comments
		comment,
		/// Background color of comments
		commentbg,
		/// Text color for greyedout inset
		greyedout,
		/// Background color of greyedout inset
		greyedoutbg,
		/// Shaded box background
		shadedbg,

		/// Color for the depth bars in the margin
		depthbar,
		/// Color for marking foreign language words
		language,

		/// Text color for command insets
		command,
		/// Background color for command insets
		commandbg,
		/// Frame color for command insets
		commandframe,

		/// Special chars text color
		special,

		/// Graphics inset background color
		graphicsbg,
		/// Math inset text color
		math,
		/// Math inset background color
		mathbg,
		/// Macro math inset background color
		mathmacrobg,
		/// Math inset frame color under focus
		mathframe,
		/// Math inset frame color not under focus
		mathcorners,
		/// Math line color
		mathline,

		/// caption frame color
		captionframe,

		/// collapsable insets text
		collapsable,
		/// collapsable insets frame
		collapsableframe,

		/// Inset marker background color
		insetbg,
		/// Inset marker frame color
		insetframe,

		/// Error box text color
		error,
		/// EOL marker color
		eolmarker,
		/// Added space colour
		added_space,
		/// Appendix marker color
		appendix,
		/// changebar color
		changebar,
		/// deleted text color
		deletedtext,
		/// added text color
		addedtext,
		/// Top and bottom line color
		topline,
		/// Table line color
		tabularline,
		/// Table line color
		tabularonoffline,
		/// Bottom area color
		bottomarea,
		/// Page break color
		pagebreak,

		// FIXME: why are the next four separate ??
		/// Color used for button frame
		buttonframe,
		/// Color used for bottom background
		buttonbg,
		/// Color used for buttom under focus
		buttonhoverbg,

		// Logical attributes

		/// Color is inherited
		inherit,
		/// For ignoring updates of a color
		ignore
	};


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
	bool setColor(Color::color col, std::string const & x11name);

	/** set the given LyX color to the color defined by the X11
	 *  name given \returns true if successful. A new color entry
	 *  is created if the color is unknown
	 */
	bool setColor(std::string const & lyxname, std::string const & x11name);

	/// Get the GUI name of \c color.
	docstring const getGUIName(Color::color c) const;

	/// Get the X11 name of \c color.
	std::string const getX11Name(Color::color c) const;

	/// Get the LaTeX name of \c color.
	std::string const getLaTeXName(Color::color c) const;

	/// Get the LyX name of \c color.
	std::string const getLyXName(Color::color c) const;

	/// \returns the Color::color associated with the LyX name.
	Color::color getFromLyXName(std::string const & lyxname) const;
	/// \returns the Color::color associated with the LaTeX name.
	Color::color getFromLaTeXName(std::string const & latexname) const;
private:
	///
	void addColor(Color::color c, std::string const & lyxname) const;
	///
	class Pimpl;
	///
	boost::scoped_ptr<Pimpl> pimpl_;
};


/** \c Color_color is a wrapper for Color::color. It can be forward-declared and
 *  passed as a function argument without having to expose Color.h.
 */
class Color_color {
	Color::color val_;
public:
	/** The default constructor is nasty,
	 *  but allows us to use Color_color in STL containers.
	 */
	Color_color() : val_(static_cast<Color::color>(-1)) {}

	Color_color(Color::color val) : val_(val) {}
	operator Color::color() const{ return val_; }
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
