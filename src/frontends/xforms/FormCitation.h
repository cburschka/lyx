// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2000 The LyX Team.
 *
 * ======================================================
 *
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#ifndef FORMCITATION_H
#define FORMCITATION_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBase.h"

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

	///
	void updateBrowser(FL_OBJECT *, std::vector<string> const &) const;
	///
	void setBibButtons(State) const;
	///
	void setCiteButtons(State) const;
	///
	void setSize(int, bool) const;

	///
	std::vector<string> citekeys;
	///
	std::vector<string> bibkeys;
};

#endif // FORMCITATION_H
