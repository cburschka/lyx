// -*- C++ -*-
/**
 * \file LColor.h
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

#include <boost/scoped_ptr.hpp>
#include <string>

/**
  This is a stateless class.

  It has one basic purposes:
  To serve as a color-namespace container (the Color enum).


  */
/**
 * \class LColor
 *
 * A class holding color definitions and associated names for
 * LaTeX, X11, the GUI, and LyX internally.
 *
 * A color can be one of the following kinds:
 *
 * - A real, predefined color, such as black, white, red or green.
 * - A logical color, such as no color, inherit, math
 */

class LColor
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
		/// Math inset frame color
		mathframe,
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
		/// strike-out color
		strikeout,
		/// added text color
		newtext,
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
		/// Color used for top of boxes
		top,
		/// Color used for bottom of boxes
		bottom,
		/// Color used for left side of boxes
		left,
		/// Color used for right side of boxes
		right,
		/// Color used for bottom background
		buttonbg,

		// Logical attributes

		/// Color is inherited
		inherit,
		/// For ignoring updates of a color
		ignore
	};


	///
	LColor();
	///
	LColor(LColor const &);
	///
	~LColor();
	///
	void operator=(LColor const &);

	/** set the given LyX color to the color defined by the X11 name given
	 *  \returns true if successful.
	 */
	bool setColor(LColor::color col, std::string const & x11name);

	/** set the given LyX color to the color defined by the X11
	 *  name given \returns true if successful. A new color entry
	 *  is created if the color is unknown
	 */
	bool setColor(std::string const & lyxname, std::string const & x11name);

	/// Get the GUI name of \c color.
	std::string const getGUIName(LColor::color c) const;

	/// Get the X11 name of \c color.
	std::string const getX11Name(LColor::color c) const;

	/// Get the LaTeX name of \c color.
	std::string const getLaTeXName(LColor::color c) const;

	/// Get the LyX name of \c color.
	std::string const getLyXName(LColor::color c) const;

	/// \returns the LColor::color associated with the GUI name.
	LColor::color getFromGUIName(std::string const & guiname) const;
	/// \returns the LColor::color associated with the LyX name.
	LColor::color getFromLyXName(std::string const & lyxname) const;
private:
	///
	void addColor(LColor::color c, std::string const & lyxname) const;
	///
	struct Pimpl;
	///
	boost::scoped_ptr<Pimpl> pimpl_;
};


/** \c LColor_color is a wrapper for LColor::color. It can be forward-declared and
 *  passed as a function argument without having to expose LColor.h.
 */
class LColor_color {
        LColor::color val_;
public:
        /** The default constructor is nasty,
	 *  but allows us to use LColor_color in STL containers.
	 */
	LColor_color() : val_(static_cast<LColor::color>(-1)) {}

	LColor_color(LColor::color val) : val_(val) {}
        operator LColor::color() const{ return val_; }
};


/// the current color definitions
extern LColor lcolor;
/// the system color definitions
extern LColor system_lcolor;

#endif
