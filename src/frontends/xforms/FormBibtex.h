// -*- C++ -*-
/**
 * \file FormBibtex.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FORMBIBTEX_H
#define FORMBIBTEX_H


#include "FormDialogView.h"
#include "RadioButtonGroup.h"
#include <vector>

class ControlBibtex;
struct FD_bibtex;

/**
 * For bibtex database setting
 */
class FormBibtex : public FormController<ControlBibtex, FormView<FD_bibtex> > {
public:
	///
	FormBibtex(Dialog &);

	enum PathStyle {ABSOLUTE_PATH = 1,
			RELATIVE_PATH,
			NAME_ONLY};

private:
	/// Set the Params variable for the Controller.
	virtual void apply();
	/// Build the dialog.
	virtual void build();
	/// Update dialog before/whilst showing it.
	virtual void update();
	/// Filter the inputs on callback from xforms
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);

	ButtonPolicy::SMInput add_database();
	ButtonPolicy::SMInput browser_add_db(string const & name);
	ButtonPolicy::SMInput browser_selected();
	ButtonPolicy::SMInput browser_double_click();
	ButtonPolicy::SMInput browser_delete();
	ButtonPolicy::SMInput set_path(PathStyle);
	ButtonPolicy::SMInput database_browse();
	ButtonPolicy::SMInput style_browse();
	void browser_deactivated();
	void input_clear();
	void input_database();

	RadioButtonGroup storage_;
	std::vector<string> bib_files_;
	std::vector<string> bib_files_namesonly_;
};

#endif // FORMBIBTEX_H
