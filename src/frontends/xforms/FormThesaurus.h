// -*- C++ -*-
/**
 * \file FormThesaurus.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#ifndef FORMTHESAURUS_H
#define FORMTHESAURUS_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

class ControlThesaurus;
struct FD_form_tabbed_thesaurus;
struct FD_form_noun;
struct FD_form_verb;
struct FD_form_adjective;
struct FD_form_adverb;
struct FD_form_other;

/** This class provides an XForms implementation of the Thesaurus dialog.
 */
class FormThesaurus : public FormCB<ControlThesaurus, FormDB<FD_form_tabbed_thesaurus> > {
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
	/// redraw
	virtual void redraw();

	/// set the replace word properly
	void setReplace(const string & templ, const string & nstr);

	/// update browser entries
	void updateEntries(const string & str);

	/// Filter the inputs
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);
 
	/// Fdesign generated methods
	FD_form_tabbed_thesaurus  * build_tabbed_thesaurus();
	FD_form_noun * build_noun();
	FD_form_verb * build_verb();
	FD_form_adjective * build_adjective();
	FD_form_adverb * build_adverb();
	FD_form_other * build_other();

	/// Real GUI implementations of sub-forms
	boost::scoped_ptr<FD_form_noun> noun_;
	boost::scoped_ptr<FD_form_verb> verb_;
	boost::scoped_ptr<FD_form_adjective> adjective_;
	boost::scoped_ptr<FD_form_adverb> adverb_;
	boost::scoped_ptr<FD_form_other> other_;

	/// for double click handling
	int clickline_;

	/// current string
	string str_;
};

#endif // FORMTHESAURUS_H
