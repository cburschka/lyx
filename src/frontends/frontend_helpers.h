// -*- C++ -*-
/**
 * \file frontend_helpers.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Herbert Voﬂ
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FRONTEND_HELPERS_H
#define FRONTEND_HELPERS_H

#include "support/docstring.h"

#include <vector>

namespace lyx {

namespace support { class FileFilterList; }

namespace frontend {

///
typedef std::pair<docstring, std::string> LanguagePair;

/** If the caller is the character dialog, add "No change" and "Reset"
*  to the vector.
*/
std::vector<LanguagePair> const getLanguageData(bool character_dlg);

/** Launch a file dialog and return the chosen file.
	filename: a suggested filename.
	title: the title of the dialog.
	pattern: *.ps etc.
	dir1 = (name, dir), dir2 = (name, dir): extra buttons on the dialog.
*/
docstring browseFile(docstring const & filename,
	docstring const & title,
	support::FileFilterList const & filters,
	bool save = false,
	docstring const & label1 = docstring(),
	docstring const & dir1 = docstring(),
	docstring const & label2 = docstring(),
	docstring const & dir2 = docstring());


/** Wrapper around browseFile which tries to provide a filename
	relative to relpath.  If the relative path is of the form "foo.txt"
	or "bar/foo.txt", then it is returned as relative. OTOH, if it is
	of the form "../baz/foo.txt", an absolute path is returned. This is
	intended to be useful for insets which encapsulate files/
*/
docstring browseRelFile(docstring const & filename,
	docstring const & refpath,
	docstring const & title,
	support::FileFilterList const & filters,
	bool save = false,
	docstring const & label1 = docstring(),
	docstring const & dir1 = docstring(),
	docstring const & label2 = docstring(),
	docstring const & dir2 = docstring());


/** Wrapper around browseFile which tries to provide a filename
*  relative to the user or system directory. The dir, name and ext
*  parameters have the same meaning as in the
*  support::LibFileSearch function.
*/
docstring browseLibFile(docstring const & dir,
	docstring const & name,
	docstring const & ext,
	docstring const & title,
	support::FileFilterList const & filters);


/** Launch a file dialog and return the chosen directory.
	pathname: a suggested pathname.
	title: the title of the dialog.
	dir1 = (name, dir), dir2 = (name, dir): extra buttons on the dialog.
*/
docstring browseDir(docstring const & pathname,
	docstring const & title,
	docstring const & label1 = docstring(),
	docstring const & dir1 = docstring(),
	docstring const & label2 = docstring(),
	docstring const & dir2 = docstring());


/** Build filelists of all availabe bst/cls/sty-files. Done through
*  kpsewhich and an external script, saved in *Files.lst.
*/
void rescanTexStyles();

/** Fill \c contents from one of the three texfiles.
 *  Each entry in the file list is returned as a name_with_path
 */
void getTexFileList(std::string const & filename, std::vector<std::string> & contents);

} // namespace frontend
} // namespace lyx

#endif // FRONTEND_HELPERS_H
