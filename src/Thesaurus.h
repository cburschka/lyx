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

#include "support/docstring.h"

#include <vector>
#include <map>

#ifdef HAVE_LIBAIKSAURUS
#include AIKSAURUS_H_LOCATION
#endif

namespace lyx {

/**
 * This class provides an interface to whatever thesauri we might support.
 */

class Thesaurus {
public:
	///
	Thesaurus();
	///
	~Thesaurus();

	typedef std::map<docstring, std::vector<docstring> > Meanings;

	/**
	 * look up some text in the thesaurus
	 */
	Meanings lookup(docstring const & text);

private:
#ifdef HAVE_LIBAIKSAURUS
	Aiksaurus * aik_;
#endif
};

extern Thesaurus thesaurus;


} // namespace lyx

#endif
