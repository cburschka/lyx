// -*- C++ -*-
/**
 * \file FormText.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef FORMTEXT_H
#define FORMTEXT_H


#include "FormDialogView.h"

class ControlCommand;
struct FD_text;


class FormText : public FormController<ControlCommand, FormView<FD_text> > {
public:
	///
	FormText(Dialog &, string const & title);
private:
	/// Set the Params variable for the Controller.
	virtual void apply();
	/// Build the dialog.
	virtual void build();
	/// Update dialog before/whilst showing it.
	virtual void update();
};

#endif // FORMTEXT_H
