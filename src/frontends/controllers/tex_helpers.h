// -*- C++ -*-
/**
 * \file tex_helpers.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Herbert Voﬂ
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef TEX_HELPERS_H
#define TEX_HELPERS_H

#include <string>
#include <vector>

namespace lyx {
namespace frontend {

// build filelists of all availabe bst/cls/sty-files. done through
// kpsewhich and an external script, saved in *Files.lst
void rescanTexStyles();

/// rebuild the textree
void texhash();

/** Fill \c contents from one of the three texfiles.
 *  Each entry in the file list is returned as a name_with_path
 */
void getTexFileList(std::string const & filename, std::vector<std::string> & contents);

/// get the options of stylefile
std::string const getListOfOptions(std::string const & classname, std::string const & type);

/// get a class with full path from the list
std::string const getTexFileFromList(std::string const & classname, std::string const & type);

} // namespace frontend
} // namespace lyx

#endif // TEX_HELPERS_H
