// -*- C++ -*-
/**
 * \file FormVSpace.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FORM_VSPACE_H
#define FORM_VSPACE_H

#include "FormDialogView.h"
#include "RadioButtonGroup.h"

namespace lyx {
namespace frontend {

struct FD_vspace;
class ControlVSpace;

/** This class provides an XForms implementation of the FormVSpace dialog.
 */
class FormVSpace
	: public FormController<ControlVSpace, FormView<FD_vspace> > {
public:
	///
	FormVSpace(Dialog &);
private:
	/// Build the dialog
	virtual void build();
	/// Apply from dialog
	virtual void apply();
	/// Update the dialog
	virtual void update();

	/// Filter the inputs on callback from xforms
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);
};

} // namespace frontend
} // namespace lyx

#endif
