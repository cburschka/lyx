// -*- C++ -*-
/**
 * \file FormParagraph.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FORM_PARAGRAPH_H
#define FORM_PARAGRAPH_H

#include "FormDialogView.h"
#include "RadioButtonGroup.h"

namespace lyx {
namespace frontend {

class ControlParagraph;
struct FD_paragraph;

/** This class provides an XForms implementation of the FormParagraph dialog.
 */
class FormParagraph
	: public FormController<ControlParagraph, FormView<FD_paragraph> > {
public:
	///
	FormParagraph(Dialog &);
private:
	/// Build the dialog
	virtual void build();
	/// Apply from dialog
	virtual void apply();
	/// Update the dialog
	virtual void update();

	/// Filter the inputs on callback from xforms
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);

	/// alignment radio buttons
	RadioButtonGroup alignment_;
};

} // namespace frontend
} // namespace lyx

#endif
