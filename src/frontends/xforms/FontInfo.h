// -*- C++ -*-
/**
 * \file FontInfo.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 * \author John Levon 
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef FONTINFO_H
#define FONTINFO_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"

#include <boost/scoped_array.hpp>

/** This class manages a font.
    The idea is to create a FontInfo object with a font name pattern with a
    wildcard at the size field. Then this object can host request for font-
    instances of any given size. If no exact match is found, the closest size
    is chosen instead. If the font is scalable, the flag
    lyxrc.use_scalable_fonts determines whether to allow scalable fonts to
    give an exact match.
*/
class FontInfo {
public:
	///
	FontInfo() { init(); }

	///
	explicit FontInfo(string const & pat)
		: pattern(pat) { init(); }

	/// Does any font match our pattern?
	bool exist() {
		query();
		return matches != 0;
	}

	/// Is this font scalable?
	bool isScalable() {
		query();
		return scalable;
	}

	/// Get existing pattern
	string const & getPattern() const { return pattern; }

	/// Set new pattern
	void setPattern(string const & pat);

	/** Return full name of font close to this size.
	  If impossible, result is the empty string */
	string const getFontname(int size);
private:
	/// Font pattern (with wildcard for size)
	string pattern;

	/// Available size list
	boost::scoped_array<int> sizes;

	/// Corresponding name list
	boost::scoped_array<string> strings;

	/// Number of matches
	int matches;

	/// Did we query X about this font?
	bool queried;

	/// Is this font scalable?
	bool scalable;

	/// Which index points to scalable font entry?
	int scaleindex;

	/// Initialize empty record
	void init();

	/// Ask X11 about this font pattern
	void query();

	/// Build newly sized font string
	string const resize(string const &, int size) const;
};

#endif // FONTINFO_H
