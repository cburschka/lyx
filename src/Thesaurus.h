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
#include "WordLangTuple.h"

#include <map>
#include <vector>

namespace lyx {

/**
 * This class provides an interface to whatever thesauri we might support.
 */

class Thesaurus
{
	/// noncopyable
	Thesaurus(Thesaurus const &);
	void operator=(Thesaurus const &);
public:
	///
	Thesaurus();
	///
	~Thesaurus();

	typedef std::map<docstring, std::vector<docstring> > Meanings;

	/**
	 * look up some text in the thesaurus
	 */
	Meanings lookup(WordLangTuple const & wl);
	/** check if a thesaurus for a given language \p lang is available
	 *  (installed and loaded)
	 */
	bool thesaurusAvailable(docstring const & lang) const;
	/// check if a thesaurus for a given language \p lang is installed
	bool thesaurusInstalled(docstring const & lang) const;

private:
	struct Private;
	Private * const d;
};

extern Thesaurus thesaurus;


} // namespace lyx

#endif
