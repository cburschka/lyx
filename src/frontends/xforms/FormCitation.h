// -*- C++ -*-
/**
 * \file FormCitation.h
 * Copyright 2000-2002 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming, a.leeming@ic.ac.uk
 */

#ifndef FORMCITATION_H
#define FORMCITATION_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"
#include "biblio.h"

/** This class provides an XForms implementation of the Citation Dialog.
 */
class ControlCitation;
struct FD_form_citation;

class FormCitation : public FormCB<ControlCitation, FormDB<FD_form_citation> > {
public:
	///
	FormCitation(ControlCitation &);

private:
	///
	enum State {
		///
		ON,
		///
		OFF
	};

	/// Set the Params variable for the Controller.
	virtual void apply();
	/// Build the dialog.
	virtual void build();
	/// Hide the dialog.
	virtual void hide();
	/// Update dialog before/whilst showing it.
	virtual void update();
	/// Filter the inputs on callback from xforms
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);

	/// Fdesign generated method
	FD_form_citation * build_citation();

	/// search for a citation
	void findBiblio(biblio::Direction const dir);

	///
	void updateBrowser(FL_OBJECT *, std::vector<string> const &) const;
	///
	void setBibButtons(State) const;
	///
	void setCiteButtons(State) const;

	///
	std::vector<string> citekeys;
	///
	std::vector<string> bibkeys;
};

#endif // FORMCITATION_H
