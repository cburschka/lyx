// -*- C++ -*-

#ifndef XFORMSHELPERS_H
#define XFORMSHELPERS_H

#ifdef __GNUG_
#pragma interface
#endif

#include "Color.h"

/// Set an FL_OBJECT to activated or deactivated
void setEnabled(FL_OBJECT *, bool enable);

/// Take a string and add breaks so that it fits into a desired label width, w
string formatted(string const &label, int w,
		 int=FL_NORMAL_SIZE, int=FL_NORMAL_STYLE);

/// Given an fl_choice, create a vector of its entries
std::vector<string> const getVectorFromChoice(FL_OBJECT *);

/// Given an fl_browser, create a vector of its entries
std::vector<string> const getVectorFromBrowser(FL_OBJECT *);

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
