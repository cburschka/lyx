// -*- C++ -*-
/**
 * \file FormPreamble.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FORMPREAMBLE_H
#define FORMPREAMBLE_H

#include "FormDialogView.h"

namespace lyx {
namespace frontend {

class ControlPreamble;
struct FD_preamble;

/** This class provides an XForms implementation of the Preamble Dialog.
 */
class FormPreamble
	: public FormController<ControlPreamble, FormView<FD_preamble> > {
public:
	///
	FormPreamble(Dialog &);
private:
	/// Apply from dialog
	virtual void apply();
	/// Build the dialog
	virtual void build();
	/// Update the dialog
	virtual void update();
};

} // namespace frontend
} // namespace lyx

#endif // FORMPREAMBLE_H
