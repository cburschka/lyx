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
#include "ControlDialog_impl.h"

/** A controller for Thesaurus dialogs.
 */
class ControlThesaurus : public ControlDialogBD {
public:
	///
	ControlThesaurus(LyXView &, Dialogs &);
 
	/// replace the particular string
	void replace(string const & newstr);

	/// show dialog
	virtual void showEntry(string const & str);

	/// get meanings
	Thesaurus::Meanings const & getMeanings(string const & str);
 
	/// the text
	string const & text() {
		return oldstr_;
	}

private:
	/// last string looked up
	string laststr_;

	/// entries for last string
	Thesaurus::Meanings meanings_;

	/// original string
	string oldstr_;

	/// not needed.
	virtual void apply() {}
};

#endif // CONTROLTHESAURUS_H
