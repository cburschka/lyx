// -*- C++ -*-
/**
 * \file ControlThesaurus.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */


#ifndef CONTROLTHESAURUS_H
#define CONTROLTHESAURUS_H

#ifdef __GNUG__
#pragma interface
#endif

#include <vector>

#include "LString.h"
#include "Thesaurus.h"
#include "ControlDialogs.h"

/** A controller for Thesaurus dialogs.
 */
class ControlThesaurus : public ControlDialog<ControlConnectBD> {
public:
	///
	ControlThesaurus(LyXView &, Dialogs &);
 
	/// replace the particular string
	void replace(string const & newstr);

	/// show dialog
	virtual void showEntry(string const & str);

	/// get noun entries
	std::vector<string> getNouns(string const & str) {
		return getEntries(str, Thesaurus::NOUN);
	}
	/// get verb entries
	std::vector<string> getVerbs(string const & str) {
		return getEntries(str, Thesaurus::VERB);
	}
	/// get adjective entries
	std::vector<string> getAdjectives(string const & str) {
		return getEntries(str, Thesaurus::ADJECTIVE);
	}
	/// get adverb entries
	std::vector<string> getAdverbs(string const & str) {
		return getEntries(str, Thesaurus::ADVERB);
	}
	/// get other entries
	std::vector<string> getOthers(string const & str) {
		return getEntries(str, Thesaurus::OTHER);
	}

	/// the text
	string const & text() {
		return oldstr_;
	}

private:
	/// get entries for a str
	std::vector<string> getEntries(string const & str, Thesaurus::POS pos);

	/// last string looked up
	string laststr_;

	/// entries for last string
	std::vector<Thesaurus::ThesaurusEntry> entries_;

	/// original string
	string oldstr_;

	/// not needed.
	virtual void apply() {}
};

#endif // CONTROLTHESAURUS_H
