// -*- C++ -*-
/**
 * \file Variables.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef VARIABLES_H
#define VARIABLES_H

#include "LString.h"
#include <map>


///
class Variables {
public:
	///
	void set(string const &, string const &);
	///
	string const get(string const &) const;
	///
	bool isSet(string const & var) const;
	///
	string const expand(string const &) const;
private:
	///
	typedef std::map<string, string> Vars;
	///
	Vars vars_;
};

#endif
