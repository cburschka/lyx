// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright (C) 1997 Asger Alstrup
 *           and the LyX Team.
 *
 *======================================================*/

#ifndef _FONTINFO_H_
#define _FONTINFO_H_ 

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"

/** This class manages a font.
The idea is to create a FontInfo object with a font name pattern with a 
wildcard at the size field. Then this object can host request for font-
instances of any given size. If no exact match is found, the closest size
is chosen instead. If the font is scalable, the flag lyxrc->use_scalable_fonts
determines whether to allow scalable fonts to give an exact match. */
class FontInfo {
public:
	///
	FontInfo() { init(); }

	///
	FontInfo(LString const & pat)
	: pattern(pat) { init(); }

	/// Destructor
	~FontInfo() { release(); }

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
	LString getPattern() const { return pattern; }

	/// Set new pattern
	void setPattern(LString const & pat);

	/** Return full name of font close to this size.
	  If impossible, result is the empty string */
	LString getFontname(int size);
private:
	/// Font pattern (with wildcard for size)
	LString pattern;

	/// Available size list
	int * sizes;

	/// Corresponding name list
	LString * strings;

	/// Number of matches
	int matches;

	/// Did we query X about this font?
	bool queried;

	/// Is this font scalable?
	bool scalable;

	/// Which index points to scalable font entry?
	int scaleindex;

	/// Initialize empty record
	void init()
	{
		sizes = 0;
		strings = 0;
		matches = 0;
		queried = false;
		scalable = false;
		scaleindex = -1;
	}

	/// Release allocated stuff
	void release();

	/// Ask X11 about this font pattern
	void query();

	/// Build newly sized font string 
	LString resize(LString const &, int size) const;
};
#endif
