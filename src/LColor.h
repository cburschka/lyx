// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1998-2000 The LyX Team
 *
 *======================================================*/

#ifdef USE_PAINTER

#ifndef LCOLOR_H
#define LCOLOR_H

#ifdef __GNUG__
#pragma interface
#endif

#include <map>

#include "LString.h"

/**
  This is a stateless class. 

  It has one basic purposes:
  To serve as a color-namespace container (the Color enum).
  
  A color can be one of the following kinds:

  - A real, predefined color, such as black, white, red or green.
  - A logical color, such as no color, inherit, math

  */

class LColor {
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

		/// Needed interface colors

		/// Background color
		background,
		/// Foreground color
		foreground,
		/// Background color of selected text
		selection,
		/// Text color in LaTeX mode
		latex,
		/// Titles color of floats
		floats,

		/// Text color for notes
		note,
		/// Background color of notes
		notebg,
		/// Frame color for notes
		noteframe,


		/// Text color for command insets
		command,
		/// Background color for command insets
		commandbg,
		/// Frame color for command insets
		commandframe,

		/// Text color for accents we can't handle nicely
		accent,
		///
		accentbg,
		///
		accentframe,

		/// Minipage line color
		minipageline,

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
		footnote,
		/// Footnote marker background color
		footnotebg,
		/// Footnote line color
		footnoteframe,

		/// ERT marker text
		ert,
		
		/// Text color for inset marker
		inset,
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
		tableline,
		/// Bottom area color
		bottomarea,
		/// Page break color
		pagebreak,

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

	        /// Logical attributes

		/// Color is inherited
		inherit,
		/// For ignoring updates of a color
		ignore
	};

	///
	LColor();
	///
	void setColor(LColor::color col, string const & x11name);
	/// Get GUI name of color
	string getGUIName(LColor::color c) const;

	/// Get X11 name of color
	string getX11Name(LColor::color c) const;

	/// Get LaTeX name of color
	string getLaTeXName(LColor::color c) const;

	/// Get LyX name of color
	string getLyXName(LColor::color c) const;
	///
	LColor::color getFromGUIName(string const & guiname) const;
	///
	LColor::color getFromLyXName(string const & lyxname) const;
private:
	///
	struct information {
		string guiname;
		string latexname;
		string x11name;
		string lyxname;
	};

	///
	void fill(LColor::color col, string const & gui,
		  string const & latex, string const & x11,
		  string const & lyx);

	///
	typedef map<LColor::color, information, less<LColor::color> > InfoTab;

	InfoTab infotab;
};

extern LColor lcolor;

#endif

#endif
