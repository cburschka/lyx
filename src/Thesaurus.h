// -*- C++ -*-
/**
 * \file Thesaurus.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef THESAURUS_H
#define THESAURUS_H

#include <vector>
#include <map>

#include "LString.h"
#ifdef HAVE_LIBAIKSAURUS
#include AIKSAURUS_H_LOCATION
#endif

/**
 * This class provides an interface to whatever thesauri we might support.
 */

class Thesaurus {
public:
	///
	Thesaurus();
	///
	~Thesaurus();

	typedef std::map<string, std::vector<string> > Meanings;

	/**
	 * look up some text in the thesaurus
	 */
	Meanings lookup(string const & text);

private:
#ifdef HAVE_LIBAIKSAURUS
	Aiksaurus * aik_;
#endif
};

extern Thesaurus thesaurus;

#endif
