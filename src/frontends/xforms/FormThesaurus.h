// -*- C++ -*-
/**
 * \file FormThesaurus.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FORMTHESAURUS_H
#define FORMTHESAURUS_H

#include "FormDialogView.h"

namespace lyx {
namespace frontend {

class ControlThesaurus;
struct FD_thesaurus;

/** This class provides an XForms implementation of the Thesaurus dialog.
 */
class FormThesaurus
	: public FormController<ControlThesaurus, FormView<FD_thesaurus> > {
public:
	///
	FormThesaurus(Dialog &);
private:
	/// not needed.
	virtual void apply() {}
	/// Build the dialog
	virtual void build();
	/// update dialog
	virtual void update();

	/// set the replace word properly
	void setReplace(std::string const & templ, std::string const & nstr);

	/// update browser entries
	void updateMeanings(std::string const & str);

	/// Filter the inputs
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);

	/// for double click handling
	int clickline_;

	/// current string
	std::string str_;
};

} // namespace frontend
} // namespace lyx

#endif // FORMTHESAURUS_H
