// -*- C++ -*-
/**
 * \file Thesaurus.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#ifndef THESAURUS_H
#define THESAURUS_H

#include <vector>
#include <map>

#include "LString.h"
#ifdef HAVE_LIBAIKSAURUS
#include "Aiksaurus.h"
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
