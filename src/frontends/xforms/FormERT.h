// -*- C++ -*-
/**
 * \file FormERT.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FORMERT_H
#define FORMERT_H


#include "FormDialogView.h"


class ControlERT;
struct FD_ert;

/** This class provides an XForms implementation of the ERT
    Dialog.
 */
class FormERT : public FormController<ControlERT, FormView<FD_ert> > {
public:
	///
	FormERT(Dialog &);
private:
	/// Set the Params variable for the Controller.
	virtual void apply();
	/// Build the dialog.
	virtual void build();
	/// Update dialog before/whilst showing it.
	virtual void update();
};

#endif // FORMERT_H
