/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1997 Asger Alstrup
 *           and the LyX Team.
 *
 * ====================================================== */

#include <config.h>
#include <cmath>	// fabs()

#ifdef __GNUG__
#pragma implementation "FontInfo.h"
#endif

#include "FontInfo.h"
#include "debug.h"
#include "lyxrc.h"	// lyxrc.use_scalable_fonts
#include "support/lstrings.h"
#include "support/lyxlib.h"
#include "frontends/GUIRunTime.h"

using std::endl;

#ifndef CXX_GLOBAL_CSTD
using std::fabs;
#endif

/// Load font close to this size
string const FontInfo::getFontname(int size)
{
	if (!exist())
		return string();

	int closestind = -1;
	double error = 100000.0;

	for (int i = 0; i < matches; ++i) {
		if (sizes[i] == 0) {
			// Scalable font should not be considered close
		} else if (sizes[i] == size) {
			lyxerr[Debug::FONT] << "Exact font match with\n"
					    << strings[i] << endl;
			return strings[i];
		} else if (fabs(sizes[i] - size - 0.1) < error) {
			error = fabs(sizes[i] - size - 0.1);
			closestind = i;
		}
	}

	if (scalable && lyxrc.use_scalable_fonts) {
		// We can use scalable
		string const font = resize(strings[scaleindex], size);
		lyxerr[Debug::FONT] << "Using scalable font to get\n"
				    << font << endl;
		return font;
	}

	// Did any fonts get close?
	if (closestind == -1) {
		// No, and we are not allowed to use scalables, so...
		return string();
	}

	// We use the closest match
	lyxerr[Debug::FONT] << "Using closest font match to get size "
			    << size 
			    << " with\n" << strings[closestind] << endl;
	return strings[closestind];
}


/// Build newly sized font string 
string const FontInfo::resize(string const & font, int size) const
{
	string ret(font);
	// Find the position of the size spec
	int cut = 0;
	string::iterator before = string::iterator(0);
	string::iterator after = string::iterator(0);
	for (string::iterator sit = ret.begin();
	     sit != ret.end(); ++sit)
		if ((*sit) == '-') {
			++cut;
			if (cut == 7) before = sit + 1;
			else if (cut == 8) {
				after = sit;
				break;
			}
		}
	ret.replace(before, after, tostr(size));
	return ret;
}


/// Set new pattern
void FontInfo::setPattern(string const & pat)
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
		lyxerr << "Can not use empty font name for font query."
		       << endl;
		queried = true;
		return;
	}

	char ** list = 0;
	if (lyxrc.use_gui)
		list = XListFonts(GUIRunTime::x11Display(), pattern.c_str(),
				  100, &matches);

	if (list == 0) {
		// No fonts matched
		scalable = false;
		sizes = 0;
	} else {
		release();
		sizes = new int[matches];
		strings = new string[matches];

		// We have matches. Run them through
		for (int i = 0; i < matches; ++i) {
			string name(list[i]);
			sizes[i] = lyx::atoi(token(name, '-', 7));
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


void FontInfo::init()
{
	sizes = 0;
	strings = 0;
	matches = 0;
	queried = false;
	scalable = false;
	scaleindex = -1;
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
