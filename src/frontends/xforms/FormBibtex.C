/**
 * \file FormBibtex.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Angus Leeming
 * \author John Levon
 * \author Herbert Voss <voss@lyx.org>
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>
#include "xformsBC.h"
#include "ControlBibtex.h"
#include "FormBibtex.h"
#include "form_bibtex.h"
#include "gettext.h"
#include "debug.h"
#include "helper_funcs.h"
#include "support/lstrings.h"
#include "support/filetools.h"


typedef FormCB<ControlBibtex, FormDB<FD_form_bibtex> > base_class;

FormBibtex::FormBibtex(ControlBibtex & c)
	: base_class(c, _("BibTeX Database"))
{}


void FormBibtex::build()
{
	dialog_.reset(build_bibtex());

	fl_set_input_return(dialog_->database, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->style, FL_RETURN_CHANGED);

        // Manage the ok, apply, restore and cancel/close buttons
	bc().setOK(dialog_->button_ok);
	bc().setCancel(dialog_->button_cancel);

	bc().addReadOnly(dialog_->database_browse);
	bc().addReadOnly(dialog_->database);
	bc().addReadOnly(dialog_->style_browse);
	bc().addReadOnly(dialog_->style);
	bc().addReadOnly(dialog_->radio_bibtotoc);
}


ButtonPolicy::SMInput FormBibtex::input(FL_OBJECT * ob, long)
{
	if (ob == dialog_->database_browse) {
		// When browsing, take the first file only 
		string const in_name = fl_get_input(dialog_->database);
		string first;
		split(in_name, first, ',');
		first = strip(first);

		string out_name = 
			controller().Browse(first,
					    "Select Database",
					    "*.bib| BibTeX Databases (*.bib)");
		if (!out_name.empty()) {
			// add the database to any existing ones
			if (!in_name.empty())
				out_name = in_name + ", " + out_name;

			fl_freeze_form(form()); 
			fl_set_input(dialog_->database, out_name.c_str());
			fl_unfreeze_form(form()); 
		}
	}

	if (ob == dialog_->style_browse) {
		string const in_name = fl_get_input(dialog_->style);
		string out_name = 
			controller().Browse(in_name,
					    "Select BibTeX-Style",
					    "*.bst| BibTeX Styles (*.bst)");
		if (!out_name.empty()) {
			fl_freeze_form(form()); 
			fl_set_input(dialog_->style, out_name.c_str());
			fl_unfreeze_form(form()); 
		}
	}
  
	if (!compare(fl_get_input(dialog_->database),"")) {
		return ButtonPolicy::SMI_NOOP;
	}

	return ButtonPolicy::SMI_VALID;
}


void FormBibtex::update()
{
	fl_set_input(dialog_->database,
		     controller().params().getContents().c_str());
        string bibtotoc = "bibtotoc";
	string bibstyle (controller().params().getOptions().c_str());
	if (prefixIs(bibstyle,bibtotoc)) { // bibtotoc exists?
		fl_set_button(dialog_->radio_bibtotoc,1);
		if (contains(bibstyle,',')) { // bibstyle exists?
			bibstyle = split(bibstyle,bibtotoc,',');
		} else {
			bibstyle = "";
		}

		fl_set_input(dialog_->style,bibstyle.c_str());

	} else {
		fl_set_button(dialog_->radio_bibtotoc,0);
		fl_set_input(dialog_->style,bibstyle.c_str());
	}
}

namespace {

// Remove all duplicate entries in c.
// Taken stright out of Stroustrup
template<class C> void eliminate_duplicates(C & c)
{
	std::sort(c.begin(), c.end()); // sort
	typename C::iterator p = std::unique(c.begin(), c.end()); // compact
	c.erase(p, c.end()); // shrink
}

string const unique_and_no_extensions(string const & str_in)
{
	std::vector<string> dbase = getVectorFromString(str_in);
	for (std::vector<string>::iterator it = dbase.begin();
	     it != dbase.end(); ++it) {
		*it = ChangeExtension(*it, "");
	}
	eliminate_duplicates(dbase);
	return getStringFromVector(dbase);
}
 
} // namespace anon


void FormBibtex::apply()
{
	string db = fl_get_input(dialog_->database);
	controller().params().setContents(unique_and_no_extensions(db));

	// empty is valid!
	string bibstyle = fl_get_input(dialog_->style);
	if (!bibstyle.empty()) {
		// save the BibTeX style without any ".bst" extension
		bibstyle = ChangeExtension(OnlyFilename(bibstyle), "");
	}

	if ((fl_get_button(dialog_->radio_bibtotoc) > 0) &&
	    (!bibstyle.empty())) {
		// both bibtotoc and style
		controller().params().setOptions("bibtotoc,"+bibstyle);
	} else {
		if (fl_get_button(dialog_->radio_bibtotoc) > 0) {
			// bibtotoc and no style
			controller().params().setOptions("bibtotoc");
		} else {
			// only style
			controller().params().setOptions(bibstyle);
		}
	}
}
