/**
 * \file FormBibtex.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author John Levon
 * \author Herbert Voﬂ
 * \author Rob Lahaye
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "FormBibtex.h"
#include "ControlBibtex.h"
#include "forms/form_bibtex.h"

#include "Tooltips.h"
#include "xforms_helpers.h"
#include "xformsBC.h"

#include "support/filetools.h"
#include "support/globbing.h"
#include "support/lstrings.h"
#include "support/lyxalgo.h"

#include "lyx_forms.h"

using lyx::support::ChangeExtension;
using lyx::support::compare;
using lyx::support::contains;
using lyx::support::FileFilterList;
using lyx::support::getStringFromVector;
using lyx::support::getVectorFromString;
using lyx::support::OnlyFilename;
using lyx::support::prefixIs;
using lyx::support::split;

using std::vector;
using std::string;


typedef FormController<ControlBibtex, FormView<FD_bibtex> > base_class;

FormBibtex::FormBibtex(Dialog & parent)
	: base_class(parent, _("BibTeX Database"))
{}


void FormBibtex::build()
{
	dialog_.reset(build_bibtex(this));

	// Manage the ok, apply, restore and cancel/close buttons
	bcview().setOK(dialog_->button_ok);
	bcview().setApply(dialog_->button_apply);
	bcview().setCancel(dialog_->button_close);
	bcview().setRestore(dialog_->button_restore);

	// disable for read-only documents
	bcview().addReadOnly(dialog_->input_database);
	bcview().addReadOnly(dialog_->button_database_browse);
	bcview().addReadOnly(dialog_->button_style_browse);
	bcview().addReadOnly(dialog_->button_rescan);
	bcview().addReadOnly(dialog_->input_style);
	bcview().addReadOnly(dialog_->check_bibtotoc);
	bcview().addReadOnly(dialog_->choice_btprint);

	// trigger an input event for cut&paste with middle mouse button.
	setPrehandler(dialog_->input_database);
	setPrehandler(dialog_->input_style);

	fl_set_input_return(dialog_->input_database, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_style, FL_RETURN_CHANGED);

	// callback for double click in browser
	fl_set_browser_dblclick_callback(dialog_->browser_styles,
					 C_FormDialogView_InputCB, 2);
					 
	fl_addto_choice(dialog_->choice_btprint,
			_(" all cited references "
			  "| all uncited references "
			  "| all references ").c_str());

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

	str = _("Double click to choose a BibTeX style from the list.");
	tooltips().init(dialog_->browser_styles, str);

#if FL_VERSION == 0 || (FL_REVISION == 0 && FL_FIXLEVEL == 0)
	// Work-around xforms' bug; enable tooltips for browser widgets.
	setPrehandler(dialog_->browser_styles);
#endif

	str = _("Updates your TeX system for a new bibstyle list. Only "
		"the styles which are in directories where TeX finds them "
		"are listed!");
	tooltips().init(dialog_->button_rescan, str);
	
	str = _("The bibliography section contains...");
	tooltips().init(dialog_->choice_btprint, str);
}


ButtonPolicy::SMInput FormBibtex::input(FL_OBJECT * ob, long ob_value)
{
	if (ob == dialog_->button_database_browse) {
		// When browsing, take the first file only
		string const in_name = getString(dialog_->input_database);
		FileFilterList const
			filter(_("*.bib| BibTeX Databases (*.bib)"));
		string out_name =
			controller().browse("", _("Select Database"),
					    filter);
		if (!out_name.empty()) {
			// add the database to any existing ones
			if (!in_name.empty())
				out_name = in_name + ',' + out_name;

			fl_set_input(dialog_->input_database, out_name.c_str());
		}

	} else if (ob == dialog_->button_style_browse) {
		string const in_name = getString(dialog_->input_style);
		FileFilterList const
			filter(_("*.bst| BibTeX Styles (*.bst)"));
		string const style = controller()
			.browse(in_name, _("Select BibTeX-Style"), filter);
		if (!style.empty()) {
			fl_set_input(dialog_->input_style, style.c_str());
		}

	} else if (ob == dialog_->browser_styles && ob_value == 2) {
		// double clicked in styles browser
		string const style = getString(dialog_->browser_styles);
		if (style.empty()) {
			return ButtonPolicy::SMI_NOOP;
		} else {
			fl_set_input(dialog_->input_style,
					ChangeExtension(style, "").c_str());
		}
		// reset the browser so that the following
		// single-click callback doesn't do anything
		fl_deselect_browser(dialog_->browser_styles);

	} else if (ob == dialog_->button_rescan) {
		fl_clear_browser(dialog_->browser_styles);
		controller().rescanBibStyles();
		vector<string> styles;
		controller().getBibStyles(styles);
		fl_add_browser_line(dialog_->browser_styles,
				    getStringFromVector(styles, "\n").c_str());
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

	bool const bibtopic = controller().usingBibtopic();
	bool const bibtotoc_exists = prefixIs(bibstyle, bibtotoc);
	fl_set_button(dialog_->check_bibtotoc, bibtotoc_exists && !bibtopic);
	setEnabled(dialog_->check_bibtotoc, !bibtopic);
	if (bibtotoc_exists) {
		if (contains(bibstyle, ',')) { // bibstyle exists?
			bibstyle = split(bibstyle, bibtotoc, ',');
		} else {
			bibstyle.erase();
		}
	}
	fl_set_input(dialog_->input_style, bibstyle.c_str());
	
	string btprint = controller().params().getSecOptions();
	int btp = 1;
	if (btprint == "btPrintNotCited")
		btp = 2;
	else if (btprint == "btPrintAll")
		btp = 3;
	
	fl_set_choice(dialog_->choice_btprint, btp);
	setEnabled(dialog_->choice_btprint, bibtopic);

	vector<string> styles;
	controller().getBibStyles(styles);

	fl_clear_browser(dialog_->browser_styles);
	fl_add_browser_line(dialog_->browser_styles,
			    getStringFromVector(styles, "\n").c_str());
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
		controller().params().setSecOptions("");
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
		controller().params().setOptions(bibtotoc + ',' + bibstyle);

	} else {
		// At least one of addtotoc and bibstyle is empty. 
		// No harm to output both!
		controller().params().setOptions(bibtotoc + bibstyle);
	}
	
	// bibtopic allows three kinds of sections:
	// 1. sections that include all cited references of the database(s)
	// 2. sec. that include all uncited references of the database(s)
	// 3. sec. that include all references of the database(s), cited or not
	if (controller().usingBibtopic()){
		int btp = fl_get_choice(dialog_->choice_btprint);
		switch (btp) {
		case 1:
			controller().params().setSecOptions("btPrintCited");
			break;
		case 2:
			controller().params().setSecOptions("btPrintNotCited");
			break;
		case 3:
			controller().params().setSecOptions("btPrintAll");
			break;
		}
	}
	
	else
		controller().params().setSecOptions("");
}
