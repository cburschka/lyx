// -*- C++ -*-
/**
 * \file PersonalWordList.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Stephan Witt
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef PERSONAL_WORD_LIST_H
#define PERSONAL_WORD_LIST_H

#include "support/strfwd.h"
#include "support/docstring_list.h"
#include "support/FileName.h"

#include <string>

namespace lyx {

/// A PersonalWordList holds a word list with persistent state
class PersonalWordList {
public:
	/// the word list has an associated language
	PersonalWordList(std::string const & lang) : lang_(lang), dirty_(false) {}
	/// the location of the file to hold to word list
	lyx::support::FileName dictfile() const;
	/// (re)load the word list from file
	void load();
	/// save word list to file
	void save();
	/// check for presence of given word
	bool exists(docstring const & word) const;
	/// add given word to list
	void insert(docstring const & word);
	/// remove given word from list
	void remove(docstring const & word);
	/// is word list modified after load/save?
	bool isDirty() const { return dirty_; }
	/// first item in word list
	docstring_list::const_iterator begin() const;
	/// end of word list
	docstring_list::const_iterator end() const;
private:
	///
	docstring_list words_;
	///
	std::string lang_;
	///
	bool dirty_;
	///
	bool equalwords(docstring const & w1, docstring const & w2) const;
	///
	std::string header() const { return "# personal word list"; }
	///
	void dirty(bool flag) { dirty_ = flag; }
};

} // namespace lyx

#endif // PERSONAL_WORD_LIST_H
