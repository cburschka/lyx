/**
 * \file FormBibtex.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming, a.leeming@ic.ac.uk
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Herbert Voss, voss@lyx.org
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>
#include "xformsBC.h"
#include "ControlBibtex.h"
#include "FormBibtex.h"
#include "form_bibtex.h"
#include "Tooltips.h"
#include "xforms_helpers.h"
#include "helper_funcs.h"
#include "gettext.h"
#include "debug.h"
#include "support/LAssert.h"
#include "support/lstrings.h"
#include "support/filetools.h"


using std::vector;
using std::sort;


typedef FormCB<ControlBibtex, FormDB<FD_form_bibtex> > base_class;

FormBibtex::FormBibtex(ControlBibtex & c)
	: base_class(c, _("BibTeX Database"))
{}


void FormBibtex::build()
{
	dialog_.reset(build_bibtex());

	fl_set_input_return(dialog_->input_database, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_style, FL_RETURN_CHANGED);

	setPrehandler(dialog_->input_database);
	setPrehandler(dialog_->input_style);

        // Manage the ok, apply, restore and cancel/close buttons
	bc().setOK(dialog_->button_ok);
	bc().setCancel(dialog_->button_close);

	bc().addReadOnly(dialog_->button_database_browse);
	bc().addReadOnly(dialog_->input_database);
	bc().addReadOnly(dialog_->button_style_browse);
	bc().addReadOnly(dialog_->input_style);
	bc().addReadOnly(dialog_->check_bibtotoc);

	// set up the tooltips
	string str = N_("The database you want to cite from. Insert it without the default extension \".bib\". If you insert it with the browser, LyX strips the extension. Several databases must be separated by a comma: \"natbib, books\".");
	tooltips().initTooltip(dialog_->button_database_browse, str);

	str = N_("Browse your directory for BibTeX stylefiles.");
	tooltips().initTooltip(dialog_->button_style_browse, str);

	str = N_("The BibTeX style to use (only one allowed). Insert it without the default extension \".bst\" and without path. Most of the bibstyles are stored in $TEXMF/bibtex/bst. $TEXMF is the root dir of the local TeX tree. In \"Help->TeX Info\" you can list all installed styles.");
	tooltips().initTooltip(dialog_->input_style, str);

	str = N_("Activate this option if you want the bibliography to appear in the Table of Contents (which doesn't happen by default).");
	tooltips().initTooltip(dialog_->check_bibtotoc, str);
}


ButtonPolicy::SMInput FormBibtex::input(FL_OBJECT * ob, long)
{
	if (ob == dialog_->button_database_browse) {
		// When browsing, take the first file only 
		string const in_name = fl_get_input(dialog_->input_database);
		string out_name = 
			controller().Browse("",
					    "Select Database",
					    "*.bib| BibTeX Databases (*.bib)");
		if (!out_name.empty()) {
			// add the database to any existing ones
			if (!in_name.empty())
				out_name = in_name + ", " + out_name;

			fl_freeze_form(form()); 
			fl_set_input(dialog_->input_database, out_name.c_str());
			fl_unfreeze_form(form()); 
		}
	}

	if (ob == dialog_->button_style_browse) {
		string const in_name = fl_get_input(dialog_->input_style);
		string out_name = 
			controller().Browse(in_name,
					    "Select BibTeX-Style",
					    "*.bst| BibTeX Styles (*.bst)");
		if (!out_name.empty()) {
			fl_freeze_form(form()); 
			fl_set_input(dialog_->input_style, out_name.c_str());
			fl_unfreeze_form(form()); 
		}
	}
  
	if (!compare(fl_get_input(dialog_->input_database),"")) {
		return ButtonPolicy::SMI_NOOP;
	}

	return ButtonPolicy::SMI_VALID;
}


void FormBibtex::update()
{
	fl_set_input(dialog_->input_database,
		     controller().params().getContents().c_str());
        string bibtotoc = "bibtotoc";
	string bibstyle (controller().params().getOptions().c_str());
	if (prefixIs(bibstyle,bibtotoc)) { // bibtotoc exists?
		fl_set_button(dialog_->check_bibtotoc,1);
		if (contains(bibstyle,',')) { // bibstyle exists?
			bibstyle = split(bibstyle,bibtotoc,',');
		} else {
			bibstyle = "";
		}

		fl_set_input(dialog_->input_style,bibstyle.c_str());

	} else {
		fl_set_button(dialog_->check_bibtotoc,0);
		fl_set_input(dialog_->input_style,bibstyle.c_str());
	}
}

namespace {

// Remove all duplicate entries in c.
// Taken stright out of Stroustrup
template<class C>
void eliminate_duplicates(C & c)
{
	sort(c.begin(), c.end());
	typename C::iterator p = std::unique(c.begin(), c.end());
	c.erase(p, c.end());
}


string const unique_and_no_extensions(string const & str_in)
{
	vector<string> dbase = getVectorFromString(str_in);
	for (vector<string>::iterator it = dbase.begin();
	     it != dbase.end(); ++it) {
		*it = ChangeExtension(*it, "");
	}
	eliminate_duplicates(dbase);
	return subst(getStringFromVector(dbase),",",", ");
}
 
} // namespace anon


void FormBibtex::apply()
{
	string const db = fl_get_input(dialog_->input_database);
	if (db.empty()) {
		// no database -> no bibtex-command and no options!
		controller().params().setContents("");
		controller().params().setOptions("");
		return;
	}
	
	controller().params().setContents(unique_and_no_extensions(db));

	// empty is valid!
	string bibstyle = fl_get_input(dialog_->input_style);
	if (!bibstyle.empty()) {
		// save the BibTeX style without any ".bst" extension
		bibstyle = ChangeExtension(OnlyFilename(bibstyle), "");
	}

	bool const bibtotoc = fl_get_button(dialog_->check_bibtotoc);
	
	if (bibtotoc && (!bibstyle.empty())) {
		// both bibtotoc and style
		controller().params().setOptions("bibtotoc,"+bibstyle);

	} else if (bibtotoc) {
		// bibtotoc and no style
		controller().params().setOptions("bibtotoc");

	} else if (!bibstyle.empty()) {
		// only style
		controller().params().setOptions(bibstyle);
	}
}
