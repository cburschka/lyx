// -*- C++ -*-
/**
 * \file FormThesaurus.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon, moz@compsoc.man.ac.uk
 */

#ifndef FORMTHESAURUS_H
#define FORMTHESAURUS_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

class ControlThesaurus;
struct FD_form_thesaurus;
struct FD_form_noun;
struct FD_form_verb;
struct FD_form_adjective;
struct FD_form_adverb;
struct FD_form_other;

/** This class provides an XForms implementation of the Thesaurus dialog.
 */
class FormThesaurus : public FormCB<ControlThesaurus, FormDB<FD_form_thesaurus> > {
public:
	///
	FormThesaurus(ControlThesaurus &);
 
private:
	/// not needed.
	virtual void apply() {}
	/// Build the dialog
	virtual void build();
	/// update dialog
	virtual void update();

	/// dialog build
	FD_form_thesaurus * build_thesaurus();

	/// set the replace word properly
	void setReplace(const string & templ, const string & nstr);

	/// update browser entries
	void updateMeanings(const string & str);

	/// Filter the inputs
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);
 
	/// for double click handling
	int clickline_;

	/// current string
	string str_;
};

#endif // FORMTHESAURUS_H
