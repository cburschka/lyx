// -*- C++ -*-
/**
 * \file font_metrics.h
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 *
 * \author unknown
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef FONT_METRICS_H
#define FONT_METRICS_H

#include "LString.h"

class LyXFont;

/**
 * A namespace holding helper functions for determining
 * the screen dimensions of fonts.
 *
 * The geometry is the standard typographical geometry,
 * as follows :
 *                                                                
 * --------------+------------------<maxAscent
 *               |          |
 *               <-------> (right bearing)
 *               <-> (left bearing)
 * char ascent>___          |
 *               ^   oooo   |  oooo              
 *   origin>____ |  oo  oo  | oo  oo             
 *              \|  oo  oo  | oo  oo             
 * --------------+---ooooo--|--oooo-<baseline  
 *               |      oo  |                 
 * char          |  oo  oo  |                
 * descent>______|   oooo   |                
 *               <-  width ->
 * --------------+----------+-------<maxDescent
 *  
 */
namespace font_metrics {
	/// return the maximum ascent of the font
	int maxAscent(LyXFont const & f);
	/// return the maximum descent of the font
	int maxDescent(LyXFont const & f);
	/// return the ascent of the char in the font
	int ascent(char c, LyXFont const & f);
	/// return the descent of the char in the font
	int descent(char c, LyXFont const & f);
	/// return the left bearing of the char in the font
	int lbearing(char c, LyXFont const & f);
	/// return the right bearing of the char in the font
	int rbearing(char c, LyXFont const & f);
	/// return the width of the string in the font
	int width(char const * s, size_t n, LyXFont const & f);
	/// return the width of the char in the font
	inline int width(char c, LyXFont const & f) {
		return width(&c, 1, f);
	}
	/// return the width of the string in the font
	inline int width(string const & s, LyXFont const & f) {
		if (s.empty()) return 0;
		return width(s.data(), s.length(), f);
	}
	/// FIXME ??
	int signedWidth(string const & s, LyXFont const & f);
	/** 
	 * fill in width,ascent,descent with the values for the
	 * given string in the font.
	 */
	void rectText(string const & str, LyXFont const & font,
		int & width, 
		int & ascent, 
		int & descent);
	/** 
	 * fill in width,ascent,descent with the values for the
	 * given string in the font for a button.
	 */
	void buttonText(string const & str, LyXFont const & font,
		int & width, 
		int & ascent, 
		int & descent);
};

#endif // FONT_METRICS_H
