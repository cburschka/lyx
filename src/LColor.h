// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1998-2000 The LyX Team
 *
 *======================================================*/

#ifndef LCOLOR_H
#define LCOLOR_H

#ifdef __GNUG__
#pragma interface
#endif

#include <map>

#include "LString.h"
#include <boost/utility.hpp>

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
 
class LColor // : public boost::noncopyable {
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

		/// Text color for notes
		note,
		/// Background color of notes
		notebg,
		/// Frame color for notes
		noteframe,


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

		/// Math inset text color
		math,
		/// Math inset background color
		mathbg,
		/// Math inset frame color
		mathframe,
		/// Math cursor color
		mathcursor,
		/// Math line color
		mathline,

		/// Footnote marker text
		// FIXME: this is seemingly mis-used all over the place
		footnote,
		/// Footnote line color
		footnoteframe,

		/// ERT marker text
		// FIXME: is there any reason this is different from latex ?
		ert,
		
		/// Inset marker background color
		insetbg,
		/// Inset marker frame color
		insetframe,

		/// Error box text color
		error,
		/// EOL marker color
		eolmarker,
		/// Appendix line color
		appendixline,
		/// VFill line color
		vfillline,
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
	/// set the given LyX color to the color defined by the X11 name given
	void setColor(LColor::color col, string const & x11name);
	/// set the given LyX color to the color defined by the X11 name given
	bool setColor(string const & lyxname, string const & x11name);
 
	/// Get GUI name of color
	string const getGUIName(LColor::color c) const;
 
	/// Get X11 name of color
	string const getX11Name(LColor::color c) const;
 
	/// Get LaTeX name of color
	string const getLaTeXName(LColor::color c) const;
 
	/// Get LyX name of color
	string const getLyXName(LColor::color c) const;
 
	/// get the color from the GUI name
	LColor::color getFromGUIName(string const & guiname) const;
	/// get the color from the LyX name
	LColor::color getFromLyXName(string const & lyxname) const;
private:
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
	void fill(LColor::color col, string const & gui,
		  string const & latex, string const & x11,
		  string const & lyx);

	///
	typedef std::map<LColor::color, information> InfoTab;
	/// the table of color information
	InfoTab infotab;
};

/// the current color definitions
extern LColor lcolor;
/// the system color definitions
extern LColor system_lcolor;

#endif
