#ifndef XFORMSHELPERS_H
#define XFORMSHELPERS_H

#ifdef __GNUG_
#pragma interface
#endif

#include <utility> // pair
//#include <config.h>
#include "LString.h"
#include "Color.h"

// Set an FL_OBJECT to activated or deactivated
void setEnabled(FL_OBJECT *, bool enable);

// Take a string and add breaks so that it fits into a desired label width, w
string formatted(string const &label, int w, int size, int style);

/** Launch a file dialog and return the chosen file.
    filename: a suggested filename.
    title: the title of the dialog.
    pattern: *.ps etc.
    dir1 = (name, dir), dir2 = (name, dir): extra buttons on the dialog.
*/
string const browseFile(string const & filename,
			string const & title,
			string const & pattern, 
			std::pair<string,string> const & dir1,
			std::pair<string,string> const & dir2);

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
