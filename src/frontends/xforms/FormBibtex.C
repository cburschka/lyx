/**
 * \file FormBibtex.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming 
 * \author John Levon
 * \author Herbert Voss
 * \author Rob Lahaye
 *
 * Full author contact details are available in file CREDITS
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>
#include "xformsBC.h"
#include "ControlBibtex.h"
#include "FormBibtex.h"
#include "forms/form_bibtex.h"
#include "Tooltips.h"
#include "xforms_helpers.h"
#include FORMS_H_LOCATION

#include "helper_funcs.h"
#include "gettext.h"
#include "debug.h"
#include "support/LAssert.h"
#include "support/lstrings.h"
#include "support/filetools.h"
#include "support/lyxalgo.h"


using std::vector;
using std::sort;


typedef FormCB<ControlBibtex, FormDB<FD_bibtex> > base_class;

FormBibtex::FormBibtex()
	: base_class(_("BibTeX Database"))
{}


void FormBibtex::build()
{
	dialog_.reset(build_bibtex(this));

	// Manage the ok, apply, restore and cancel/close buttons
	bc().setOK(dialog_->button_ok);
	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_close);
	bc().setRestore(dialog_->button_restore);

	// disable for read-only documents
	bc().addReadOnly(dialog_->input_database);
	bc().addReadOnly(dialog_->button_database_browse);
	bc().addReadOnly(dialog_->button_style_browse);
	bc().addReadOnly(dialog_->button_rescan);
	bc().addReadOnly(dialog_->input_style);
	bc().addReadOnly(dialog_->check_bibtotoc);

	// trigger an input event for cut&paste with middle mouse button.
	setPrehandler(dialog_->input_database);
	setPrehandler(dialog_->input_style);

	fl_set_input_return(dialog_->input_database, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_style, FL_RETURN_CHANGED);

	// set up the tooltips
	string str = _("The database you want to cite from. Insert it "
		       "without the default extension \".bib\". Use comma "
		       "to separate databases.");
	tooltips().init(dialog_->button_database_browse, str);

	str = _("Browse directory for BibTeX stylefiles");
	tooltips().init(dialog_->button_style_browse, str);

	str = _("The BibTeX style to use (only one allowed). Insert it without "
		"the default extension \".bst\" and without path.");
	tooltips().init(dialog_->input_style, str);

	str = _("Select if the bibliography should appear in the Table "
		"of Contents");
	tooltips().init(dialog_->check_bibtotoc, str);

	str = _("Choose a BibTeX style from the list.");
	tooltips().init(dialog_->browser_styles, str);

	str = _("Updates your TeX system for a new bibstyle list. Only "
		"the styles which are in directories where TeX finds them "
		"are listed!");
	tooltips().init(dialog_->button_rescan, str);
}


ButtonPolicy::SMInput FormBibtex::input(FL_OBJECT * ob, long)
{
	if (ob == dialog_->button_database_browse) {
		// When browsing, take the first file only
		string const in_name = getString(dialog_->input_database);
		string out_name =
			controller().Browse("",
				_("Select Database"),
				_("*.bib| BibTeX Databases (*.bib)"));
		if (!out_name.empty()) {
			// add the database to any existing ones
			if (!in_name.empty())
				out_name = in_name + ',' + out_name;

			fl_set_input(dialog_->input_database, out_name.c_str());
		}

	} else if (ob == dialog_->button_style_browse) {
		string const in_name = getString(dialog_->input_style);
		string const style = controller().Browse(in_name,
					_("Select BibTeX-Style"),
					_("*.bst| BibTeX Styles (*.bst)"));
		if (!style.empty()) {
			fl_set_input(dialog_->input_style, style.c_str());
		}

	} else if (ob == dialog_->browser_styles) {
		string const style = getString(dialog_->browser_styles);
		if (style.empty()) {
			return ButtonPolicy::SMI_NOOP;
		} else {
			fl_set_input(dialog_->input_style,
					ChangeExtension(style, "").c_str());
		}

	} else if (ob == dialog_->button_rescan) {
		fl_clear_browser(dialog_->browser_styles);
		controller().rescanBibStyles();
		string const str =
			controller().getBibStyles();
		fl_add_browser_line(dialog_->browser_styles, str.c_str());
	}

	// with an empty database nothing makes sense ...
	if (!compare(fl_get_input(dialog_->input_database), "")) {
		return ButtonPolicy::SMI_NOOP;
	}

	return ButtonPolicy::SMI_VALID;
}


void FormBibtex::update()
{
	fl_set_input(dialog_->input_database,
		     controller().params().getContents().c_str());

	string bibtotoc = "bibtotoc";
	string bibstyle = controller().params().getOptions();

	bool const bibtotoc_exists = prefixIs(bibstyle, bibtotoc);
	fl_set_button(dialog_->check_bibtotoc, bibtotoc_exists);
	if (bibtotoc_exists) {
		if (contains(bibstyle, ',')) { // bibstyle exists?
			bibstyle = split(bibstyle, bibtotoc, ',');
		} else {
			bibstyle.erase();
		}
	}
	fl_set_input(dialog_->input_style, bibstyle.c_str());

	fl_clear_browser(dialog_->browser_styles);
	string const str = controller().getBibStyles();
	fl_add_browser_line(dialog_->browser_styles, str.c_str());
}

namespace {

string const unique_and_no_extensions(string const & str_in)
{
	vector<string> dbase = getVectorFromString(str_in);
	for (vector<string>::iterator it = dbase.begin();
	     it != dbase.end(); ++it) {
		*it = ChangeExtension(*it, string());
	}
	lyx::eliminate_duplicates(dbase);
	return getStringFromVector(dbase);
}

} // namespace anon


void FormBibtex::apply()
{
	string const db = getString(dialog_->input_database);
	if (db.empty()) {
		// no database -> no bibtex-command and no options!
		controller().params().setContents("");
		controller().params().setOptions("");
		return;
	}

	controller().params().setContents(unique_and_no_extensions(db));

	// empty is valid!
	string bibstyle = getString(dialog_->input_style);
	if (!bibstyle.empty()) {
		// save the BibTeX style without any ".bst" extension
		bibstyle = ChangeExtension(OnlyFilename(bibstyle), "");
	}

	bool const addtotoc = fl_get_button(dialog_->check_bibtotoc);
	string const bibtotoc = addtotoc ? "bibtotoc" : "";
	if (addtotoc && !bibstyle.empty()) {
		// Both bibtotoc and style.
		controller().params().setOptions(bibtotoc + "," + bibstyle);

	} else {
		// At least one of addtotoc and bibstyle is empty. No harm to output both!
		controller().params().setOptions(bibtotoc + bibstyle);
	}
}
