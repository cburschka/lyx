/**
 * \file FormTexinfo.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Herbert Voss
 *
 * Full author contact details are available in file CREDITS
 * \date 2001-10-01
 */

#include <config.h>
#include <fstream>

#ifdef __GNUG__
#pragma implementation
#endif

#include "xformsBC.h"
#include "FormTexinfo.h"
#include "forms/form_texinfo.h"
#include "Tooltips.h"
#include "gettext.h"
#include "xforms_helpers.h"
#include "support/LAssert.h"
#include "support/lstrings.h"

#include FORMS_H_LOCATION


typedef FormCB<ControlTexinfo, FormDB<FD_texinfo> > base_class;
FormTexinfo::FormTexinfo()
	: base_class(_("LaTeX Information")),
	  activeStyle(ControlTexinfo::cls)
{}


void FormTexinfo::build() {
	dialog_.reset(build_texinfo(this));

	// callback for double click in browser to view the selected file
	fl_set_browser_dblclick_callback(dialog_->browser, C_FormBaseInputCB, 2);

	string const classes_List = _("LaTeX classes|LaTeX styles|BibTeX styles");
	fl_addto_choice(dialog_->choice_classes, classes_List.c_str());

	updateStyles(activeStyle);

	// set up the tooltips
	string str = _("Shows the installed classses and styles for LaTeX/BibTeX; "
			"available only if the corresponding LyX layout file exists.");
	tooltips().init(dialog_->choice_classes, str);

	str = _("Show full path or only file name.");
	tooltips().init(dialog_->check_fullpath, str);

	str = _("Runs the script \"TexFiles.sh\" to build new file lists.");
	tooltips().init(dialog_->button_rescan, str);

	str = _("Double click to view contents of file.");
	tooltips().init(dialog_->browser, str);

	str = _("Runs the script \"texhash\" which builds a new LaTeX tree. "
		"Needed if you install a new TeX class or style. You need write "
		"permissions for the TeX-dirs, often /var/lib/texmf and others.");
	tooltips().init(dialog_->button_texhash, str);
}


ButtonPolicy::SMInput FormTexinfo::input(FL_OBJECT * ob, long ob_value) {

	if (ob == dialog_->button_texhash) {
		// makes only sense if the rights are set well for
		// users (/var/lib/texmf/ls-R)
		controller().runTexhash();
		// texhash requires a rescan and an update of the styles
		controller().rescanStyles();
		updateStyles(activeStyle);


	} else if (ob == dialog_->browser && ob_value == 2) {
		// double click in browser: view selected file
		string selection = string();
		if (fl_get_button(dialog_->check_fullpath)) {
			// contents in browser has full path
			selection = getString(dialog_->browser);
		} else {
			// contents in browser has filenames without path
			// reconstruct path from controller getContents
			string const files = controller().getContents(activeStyle, true);
			vector<string> const vec = getVectorFromString(files, "\n");

			// find line in files vector
			vector<string>::const_iterator it = vec.begin();
			int const line = fl_get_browser(dialog_->browser);
			for (int i = line; it != vec.end() && i > 0; ++it, --i) {
				if (i == 1) selection = *it;
			}
		}

		if (!selection.empty()) {
			controller().viewFile(selection);
		}

		// reset the browser so that the following single-click callback doesn't do anything
		fl_deselect_browser(dialog_->browser);
		
	} else if (ob == dialog_->button_rescan) {
		// build new *Files.lst
		controller().rescanStyles();
		updateStyles(activeStyle);

	} else if (ob == dialog_->check_fullpath) {
		updateStyles(activeStyle);

	} else if (ob == dialog_->choice_classes) {
		switch (fl_get_choice(dialog_->choice_classes)) {
		case 3:
			updateStyles(ControlTexinfo::bst);
			break;
		case 2:
			updateStyles(ControlTexinfo::sty);
			break;
		case 1:
		default:
			updateStyles(ControlTexinfo::cls);
		}
	}

	return ButtonPolicy::SMI_VALID;
}


void FormTexinfo::updateStyles(ControlTexinfo::texFileSuffix whichStyle)
{
	fl_clear_browser(dialog_->browser);

	bool const withFullPath = fl_get_button(dialog_->check_fullpath);

	string const str =
		controller().getContents(whichStyle, withFullPath);
	fl_add_browser_line(dialog_->browser, str.c_str());

	activeStyle = whichStyle;
}
