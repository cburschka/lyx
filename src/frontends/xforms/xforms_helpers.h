// -*- C++ -*-
/**
 * \file xforms_helpers.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef XFORMSHELPERS_H
#define XFORMSHELPERS_H


#include "forms_fwd.h"

#include "Color.h"

#include <algorithm>
#include <vector>

class LyXLength;

/// parse "&Save" etc. to <"Save", "#S">. Does not handle &&
std::pair<std::string, std::string> parse_shortcut(std::string const & str);

// A wrapper for the xforms routine, but this one accepts uint args
unsigned long fl_getmcolor(int i, unsigned int * r, unsigned int * g,
			   unsigned int * b);

/** Generate a string of available length units with which to
 *  populate a choice widget.
 */
std::string const buildChoiceLengthString();

/// return the (in)active state of the object
bool isActive(FL_OBJECT * ob);

/// Set an FL_OBJECT to activated or deactivated
void setEnabled(FL_OBJECT *, bool enable);

/// Take a std::string and add breaks so that it fits into a desired label width, w
std::string formatted(std::string const &label, int w,
		 int = 12 /*FL_NORMAL_SIZE*/, int = 0 /*FL_NORMAL_STYLE*/);

/// Given an fl_choice or an fl_browser, create a vector of its entries
std::vector<std::string> const getVector(FL_OBJECT *);

/** Given an fl_input, an fl_choice or an fl_browser, return an entry
    \c num is the position for the string, where 0 means "current item"
 */
std::string const getString(FL_OBJECT * ob, int num = 0);

/// Given input and choice widgets, create a std::string such as "1cm"
std::string getLengthFromWidgets(FL_OBJECT * input, FL_OBJECT * choice);

/** Given a string such as "1cm", set the input and choice widgets.
    If the string is empty, the choice will be set to default_unit.
 */
void updateWidgetsFromLengthString(FL_OBJECT * input, FL_OBJECT * choice,
				   std::string const & str,
				   std::string const & default_unit);

/** Given a LyXLength, set the input and choice widgets.
    If the length is null, the choice will be set to default_unit.
 */
void updateWidgetsFromLength(FL_OBJECT * input, FL_OBJECT * choice,
			     LyXLength const & len,
			     std::string const & default_unit);


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
	static bool read(std::string const &);
	static bool write(std::string const &);
};


/** Some functions that perform some quite detailed tests to ascertain whether
    the directory or file is readable or writeable. If not, then an error
    message is placed in error_message. */
class RWInfo {
public:
	///
	static bool WriteableDir(std::string const & dir);
	///
	static bool ReadableDir(std::string const & dir);
	///
	static bool WriteableFile(std::string const & file);
	///
	static bool ReadableFile(std::string const & file);
	///
	static std::string const & ErrorMessage() { return error_message; }
private:
	///
	static std::string error_message;
};
#endif // XFORMSHELPERS_H
