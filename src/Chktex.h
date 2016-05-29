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

#include <string>


namespace lyx {

class TeXErrors;


///
class Chktex {
public:
	/**
	  @param cmd the chktex command.
	  @param file name of the (temporary) latex file.
	  @param path name of the files original path.
	*/
	Chktex(std::string const & cmd, std::string const & file,
	       std::string const & path);

	/** Runs chktex.
	    @return -1 if fail, number of messages otherwise.
	  */
	int run(TeXErrors &);
private:
	///
	int scanLogFile(TeXErrors &);
	///
	std::string cmd;
	///
	std::string file;
	///
	std::string path;
};


} // namespace lyx

#endif
