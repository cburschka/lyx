// -*- C++ -*-
/**
 * \file xforms/FormCitation.h
 * Read the file COPYING
 *
 * \author Angus Leeming 
 *
 * Full author contact details are available in file CREDITS
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
struct FD_citation;

class FormCitation : public FormCB<ControlCitation, FormDB<FD_citation> > {
public:
	///
	FormCitation();
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
