// -*- C++ -*-
/**
 * \file Chktex.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CHKTEX_H
#define CHKTEX_H

#include "support/std_string.h"

class LyXLex;
class TeXErrors;

///
class Chktex {
public:
	/**
	  @param cmd the chktex command.
	  @param file name of the (temporary) latex file.
	  @param path name of the files original path.
	*/
	Chktex(string const & cmd, string const & file,
	       string const & path);

	/** Runs chktex.
	    @return -1 if fail, number of messages otherwise.
	  */
	int run(TeXErrors &);
private:
	///
	int scanLogFile(TeXErrors &);

	///
	string cmd;

	///
	string file;

	///
	string path;
};

#endif
