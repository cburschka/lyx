// -*- C++ -*-
/**
 * \file FormThesaurus.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef FORMTHESAURUS_H
#define FORMTHESAURUS_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

class ControlThesaurus;
struct FD_thesaurus;

/** This class provides an XForms implementation of the Thesaurus dialog.
 */
class FormThesaurus : public FormCB<ControlThesaurus, FormDB<FD_thesaurus> > {
public:
	///
	FormThesaurus();
private:
	/// not needed.
	virtual void apply() {}
	/// Build the dialog
	virtual void build();
	/// update dialog
	virtual void update();

	/// set the replace word properly
	void setReplace(string const & templ, string const & nstr);

	/// update browser entries
	void updateMeanings(string const & str);

	/// Filter the inputs
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);

	/// for double click handling
	int clickline_;

	/// current string
	string str_;
};

#endif // FORMTHESAURUS_H
