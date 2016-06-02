// -*- C++ -*-
/**
 * \file WordList.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Stefan Schimanski
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef WORDLIST_H
#define WORDLIST_H

#include "support/docstring.h"

#include <memory>

namespace lyx {

class WordList {
public:
	///
	WordList();
	///
	docstring const & word(size_t idx) const;
	///
	size_t size() const;
	///
	void insert(docstring const & w);
	///
	void remove(docstring const & w);
	///
	static void cleanupWordLists();

private:
	struct Impl;
	std::unique_ptr<Impl> d;
};

WordList * theWordList(std::string const & lang);

} // namespace lyx

#endif // WORDLIST_H
