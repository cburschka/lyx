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

#ifdef HAVE_LIBMYTHES
#include MYTHES_H_LOCATION
#else
#ifdef HAVE_LIBAIKSAURUS
#include AIKSAURUS_H_LOCATION
#endif // HAVE_LIBAIKSAURUS
#endif // !HAVE_LIBMYTHES

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
#ifdef HAVE_LIBMYTHES
	MyThes * thes_;
#else
#ifdef HAVE_LIBAIKSAURUS
	Aiksaurus * thes_;
#endif // HAVE_LIBAIKSAURUS
#endif // !HAVE_LIBMYTHES
};

extern Thesaurus thesaurus;


} // namespace lyx

#endif
