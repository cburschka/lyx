// -*- C++ -*-
/**
 * \file xforms_helpers.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

#ifndef XFORMSHELPERS_H
#define XFORMSHELPERS_H

#ifdef __GNUG__
#pragma interface
#endif

#include "forms_fwd.h"

#include "Color.h"
#include "LString.h"

#include <algorithm>
#include <vector>

class LyXLength;

// what we always need for lengths
string const choice_Length_All =
    "cm|mm|in|text%%|col%%|page%%|line%%|ex|em|pt|sp|bp|dd|pc|cc|mu";
string const choice_Length_WithUnit =
    "cm|mm|in|ex|em|pt|sp|bp|dd|pc|cc|mu";	// all with a Unit

/// Set an FL_OBJECT to activated or deactivated
void setEnabled(FL_OBJECT *, bool enable);

/// Take a string and add breaks so that it fits into a desired label width, w
string formatted(string const &label, int w,
		 int = 12 /*FL_NORMAL_SIZE*/, int = 0 /*FL_NORMAL_STYLE*/);

/// Given an fl_choice or an fl_browser, create a vector of its entries
std::vector<string> const getVector(FL_OBJECT *);

/// Given an fl_input, an fl_choice or an fl_browser, return an entry
/** \c num is the position for the string, where -1 means "current item" */
string const getString(FL_OBJECT * ob, int num = -1);

/// Given input and choice widgets, create a string such as "1cm"
string getLengthFromWidgets(FL_OBJECT * input, FL_OBJECT * choice);

/** Given a string such as "1cm", set the input and choice widgets.
    If the string is empty, the choice will be set to default_unit.
 */
void updateWidgetsFromLengthString(FL_OBJECT * input, FL_OBJECT * choice,
				   string const & str,
				   string const & default_unit);

/** Given a LyXLength, set the input and choice widgets.
    If the length is null, the choice will be set to default_unit.
 */
void updateWidgetsFromLength(FL_OBJECT * input, FL_OBJECT * choice,
			     LyXLength const & len,
			     string const & default_unit);


/** Return the position of val in the vector if found.
    If not found, return 0.
 */
template<class A>
typename std::vector<A>::size_type
findPos(std::vector<A> const & vec, A const & val)
{
	typename std::vector<A>::const_iterator it =
		std::find(vec.begin(), vec.end(), val);
	if (it == vec.end())
		return 0;
	return std::distance(vec.begin(), it);
}

/// Called from Preferences when the cursor color is changed.
void setCursorColor(int color);


/// struct holding xform-specific colors
struct XformsColor : public NamedColor {
	int colorID;
	XformsColor() : NamedColor(), colorID(0) {}
	static bool read(string const &);
	static bool write(string const &);
};


/** Some functions that perform some quite detailed tests to ascertain whether
    the directory or file is readable or writeable. If not, then an error
    message is placed in error_message. */
class RWInfo {
public:
	///
	static bool WriteableDir(string const & dir);
	///
	static bool ReadableDir(string const & dir);
	///
	static bool WriteableFile(string const & file);
	///
	static bool ReadableFile(string const & file);
	///
	static string const & ErrorMessage() { return error_message; }
private:
	///
	static string error_message;
};
#endif // XFORMSHELPERS_H
