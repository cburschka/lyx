// -*- C++ -*-
/**
 * \file FormWrap.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FORMWRAP_H
#define FORMWRAP_H

#include "FormDialogView.h"
#include "RadioButtonGroup.h"

namespace lyx {
namespace frontend {

class ControlWrap;
struct FD_wrap;

/** This class provides an XForms implementation of the Wrap
    Dialog.
 */
class FormWrap
	: public FormController<ControlWrap, FormView<FD_wrap> > {
public:
	///
	FormWrap(Dialog &);
private:
	/// Set the Params variable for the Controller.
	virtual void apply();
	/// Build the dialog.
	virtual void build();
	/// Update dialog before/whilst showing it.
	virtual void update();

	/// placement
	RadioButtonGroup placement_;
};

} // namespace frontend
} // namespace lyx

#endif // FORMWRAP_H
