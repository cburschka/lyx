// -*- C++ -*-

#ifndef XFORMSHELPERS_H
#define XFORMSHELPERS_H

#ifdef __GNUG_
#pragma interface
#endif

#include FORMS_H_LOCATION
 
#include "Color.h"
#include "LString.h"
 
#include <vector>

class LyXLength;

// what we always need for lengths
string const choice_Length_All =
    "cm|mm|in|t%%|c%%|p%%|l%%|ex|em|pt|sp|bp|dd|pc|cc|mu";
string const choice_Length_WithUnit =
    "cm|mm|in|ex|em|pt|sp|bp|dd|pc|cc|mu"; 	// all with a Unit

/// Extract shortcut from <ident>|<shortcut> string
char const * flyx_shortcut_extract(char const * sc);
/// Shortcut for flyx_shortcut_extract
#define scex flyx_shortcut_extract

/// Extract shortcut from <ident>|<shortcut> string
char const * flyx_ident_extract(char const * sc);
/// Shortcut for flyx_ident_extract
#define idex flyx_ident_extract

/// Set an FL_OBJECT to activated or deactivated
void setEnabled(FL_OBJECT *, bool enable);

/// Take a string and add breaks so that it fits into a desired label width, w
string formatted(string const &label, int w,
		 int = FL_NORMAL_SIZE, int = FL_NORMAL_STYLE);

/// Given an fl_choice, create a vector of its entries
std::vector<string> const getVectorFromChoice(FL_OBJECT *);

/// Given an fl_browser, create a vector of its entries
std::vector<string> const getVectorFromBrowser(FL_OBJECT *);

/// Given an fl_input, return its contents.
string const getStringFromInput(FL_OBJECT * ob);

/** Given an fl_browser, return the contents of line
    (xforms numbering convention; starts at 1).
*/
string const getStringFromBrowser(FL_OBJECT * ob, int line);
/** Given an fl_browser, return the contents of the currently
    highlighted line.
*/
string const getSelectedStringFromBrowser(FL_OBJECT * ob);

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
