// -*- C++ -*-
/**
 * \file Thesaurus.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef THESAURUS_H
#define THESAURUS_H

#include "support/docstring.h"

#include <vector>
#include <map>
#include <string>

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
	Meanings lookup(docstring const & text, docstring const & lang);
	/// check if a thesaurus for a given language \p lang is available
	bool thesaurusAvailable(docstring const & lang) const;

private:
#ifdef HAVE_LIBMYTHES
	/// add a thesaurus to the list
	bool addThesaurus(docstring const & lang);

	typedef std::map<docstring, MyThes *> Thesauri;
	/// the thesauri
	Thesauri thes_;
#else
#ifdef HAVE_LIBAIKSAURUS
	Aiksaurus * thes_;
#endif // HAVE_LIBAIKSAURUS
#endif // !HAVE_LIBMYTHES
};

extern Thesaurus thesaurus;


} // namespace lyx

#endif
