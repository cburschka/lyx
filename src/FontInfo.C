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

#include <config.h>
#include <math.h>	// fabs()
#include <stdlib.h>	// atoi()

#ifdef __GNUG__
#pragma implementation "FontInfo.h"
#endif

#include "FontInfo.h"
#include "error.h"
#include "lyxrc.h"	// lyxrc.use_scalable_fonts
extern LyXRC * lyxrc;

/// Load font close to this size
LString FontInfo::getFontname(int size)
{
	if (!exist())
		return LString();

	int closestind = -1;
	double error = 100000;

	for (int i=0; i<matches; i++) {
		if (sizes[i] == 0) {
			// Scalable font should not be considered close
		} else if (sizes[i] == size) {
			lyxerr.debug(LString("Exact font match with\n")
					      + strings[i], Error::FONT);
			return strings[i];
		} else if (fabs(sizes[i] - size - 0.1) < error) {
			error = fabs(sizes[i] - size - 0.1);
			closestind = i;
		}
	}

	if (scalable && lyxrc->use_scalable_fonts) {
		// We can use scalable
		LString font = resize(strings[scaleindex], size);
		lyxerr.debug("Using scalable font to get\n"
			      + font, Error::FONT);
		return font;
	}

	// Did any fonts get close?
	if (closestind == -1) {
		// No, and we are not allowed to use scalables, so...
		return LString();
	}

	// We use the closest match
	lyxerr.debug(LString("Using closest font match to get size ") + size 
		      + " with\n" + strings[closestind], Error::FONT);
	return strings[closestind];
}

/// Build newly sized font string 
LString FontInfo::resize(LString const & font, int size) const {
	// Find the position of the size spec
	int cut=0, before=0, after=0;
	for (int i=0; i<font.length(); i++) {
		if (font[i] == '-') {
			cut++;
			if (cut==7) {
				before = i;
			} else if (cut==8) {
				after = i;
				break;
			}
		}
	}

	LString head = font;
	head.substring(0, before);
	LString tail = font;
	tail.substring(after,tail.length()-1);
	return head + size + tail;
}

/// Set new pattern
void FontInfo::setPattern(LString const & pat)
{
	release();
	init();
	pattern = pat;
}

/// Query font in X11
void FontInfo::query()
{
	if (queried)
		return;

	if (pattern.empty()) {
		lyxerr.print("Can not use empty font name for font query.");
		queried = true;
		return;
	}

	char ** list = XListFonts(fl_display, pattern.c_str(), 100, &matches);

	if (list == 0) {
		// No fonts matched
		scalable = false;
		sizes = 0;
	} else {
		release();
		sizes = new int[matches];
		strings = new LString[matches];

		// We have matches. Run them through
		for(int i=0; i<matches; i++) {
			LString name(list[i]);
			sizes[i] = atoi(name.token('-',7).c_str());
			strings[i] = name;
			if (sizes[i] == 0) {
				if (scaleindex == -1) {
					scaleindex = i;
				}
				scalable = true;
			};
		};
		XFreeFontNames(list);
	}
	queried = true;
}

/// Release allocated stuff
void FontInfo::release()
{
	if (sizes) {
		delete [] sizes;
		sizes = 0;
	}
	if (strings) {
		delete [] strings;
		strings = 0;
	}
}
