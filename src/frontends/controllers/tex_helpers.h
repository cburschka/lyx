/**
 * \file tex_helpers.h
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 *
 * \author Herbert Voss <voss@lyx.org>
 */

#ifndef TEX_HELPERS_H
#define TEX_HELPERS_H

// build filelists of all availabe bst/cls/sty-files. done through
// kpsewhich and an external script, saved in *Files.lst
void rescanTexStyles();

/// rebuild the textree
void texhash();

/// return one of the three texfiles
string const getTexFileList(string const & filename, bool withFullPath);

/// get the options of stylefile
string const getListOfOptions(string const & classname,
			    string const & type);

/// get a class with full path from the list
string const getTexFileFromList(string const & classname,
			    string const & type);

#endif // TEX_HELPERS_H
