// -*- C++ -*-
/**
 * \file FormBox.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Martin Vermeer
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FORMBOX_H
#define FORMBOX_H

#include "FormDialogView.h"

namespace lyx {
namespace frontend {

class ControlBox;
struct FD_box;

/** This class provides an XForms implementation of the Box Dialog.
 */
class FormBox : public FormController<ControlBox, FormView<FD_box> > {
public:
	/// Constructor
	FormBox(Dialog &);
private:
	///
	virtual void apply();
	/// Build the dialog
	virtual void build();
	/// Update dialog before showing it
	virtual void update();
	///
	virtual ButtonPolicy::SMInput input( FL_OBJECT *, long);
	///
	std::vector<std::string> ids_;
	///
	std::vector<std::string> gui_names_;
	///
	std::vector<std::string> ids_spec_;
	///
	std::vector<std::string> gui_names_spec_;
	///
	enum Specials {
		NONE = 1,
		HEIGHT,
		DEPTH,
		TOTALHEIGHT,
		WIDTH
	};
};

} // namespace frontend
} // namespace lyx

#endif // FORMBOX_H
