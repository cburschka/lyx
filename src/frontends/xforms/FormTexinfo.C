/**
 * \file FormTexinfo.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Herbert Voss, voss@lyx.org
 * \date 2001-10-01
 */

#include <config.h>
#include <fstream>

#ifdef __GNUG__
#pragma implementation
#endif

#include "xformsBC.h"
#include "FormTexinfo.h"
#include "form_texinfo.h"
#include "Tooltips.h"
#include "gettext.h"
#include "debug.h"
#include "xforms_helpers.h"
#include "support/LAssert.h"
    

typedef FormCB<ControlTexinfo, FormDB<FD_form_texinfo> > base_class;
FormTexinfo::FormTexinfo(ControlTexinfo & c)
	: base_class(c, _("LaTeX Information")),
	  activeStyle(ControlTexinfo::cls)
{}


void FormTexinfo::build() {
	dialog_.reset(build_texinfo());
	// courier medium
	fl_set_browser_fontstyle(dialog_->browser,FL_FIXED_STYLE);
	// with Path is default
	fl_set_button(dialog_->button_fullPath, 1);
	updateStyles(ControlTexinfo::cls);

	// set up the tooltips
	string str = N_("Runs the script \"TexFiles.sh\" to build new file lists.");
	tooltips().initTooltip(dialog_->button_rescan, str);

	str = N_("Shows the contents of the marked file. Only possible in full path mode.");
	tooltips().initTooltip(dialog_->button_view, str);

	str = N_("Runs the script \"texhash\" which builds the a new LaTeX tree. Needed if you install a new TeX class or style. To execute it, you need the write permissions for the tex-dirs, often /var/lib/texmf and other.");
	tooltips().initTooltip(dialog_->button_texhash, str);

	str = N_("View full path or only file name. Full path is needed to view the contents of a file.");
	tooltips().initTooltip(dialog_->button_fullPath, str);

	str = N_("Shows the installed LaTeX Document classes. Remember, that these classes are only available in LyX if a corresponding LyX layout file exists!");
	tooltips().initTooltip(dialog_->radio_cls, str);

	str = N_("Shows the installed LaTeX style files, which are available in LyX by default, like \"babel\" or through \\usepackage{<the stylefile>} in LaTeX preamble.");
	tooltips().initTooltip(dialog_->radio_sty, str);

	str = N_("Shows the installed style files for BibTeX. They can be loaded through insert->Lists&Toc->BibTeX Reference->Style.");
	tooltips().initTooltip(dialog_->radio_bst, str);
}


ButtonPolicy::SMInput FormTexinfo::input(FL_OBJECT * ob, long) {

	if (ob == dialog_->radio_cls) {
		updateStyles(ControlTexinfo::cls);

	} else if (ob == dialog_->radio_sty) {
		updateStyles(ControlTexinfo::sty); 

	} else if (ob == dialog_->radio_bst) {
		updateStyles(ControlTexinfo::bst); 

	} else if (ob == dialog_->button_rescan) {
		// build new *Files.lst
		controller().rescanStyles();
		updateStyles(activeStyle);

	} else if (ob == dialog_->button_fullPath) {
		setEnabled(dialog_->button_view,
			   fl_get_button(dialog_->button_fullPath));
		updateStyles(activeStyle);

	} else if (ob == dialog_->button_texhash) {
		// makes only sense if the rights are set well for
		// users (/var/lib/texmf/ls-R)
		controller().runTexhash();
		// update files in fact of texhash
		controller().rescanStyles();

	} else if (ob == dialog_->button_view) {
		unsigned int selection = fl_get_browser(dialog_->browser);
		// a valid entry?
		if (selection > 0) {
			controller().viewFile(
				fl_get_browser_line(dialog_->browser,
						    selection));
		}
	}

	return ButtonPolicy::SMI_VALID;
}

void FormTexinfo::updateStyles(ControlTexinfo::texFileSuffix whichStyle)
{
	fl_clear_browser(dialog_->browser); 

	bool const withFullPath = fl_get_button(dialog_->button_fullPath);

	string const str = 
		controller().getContents(whichStyle, withFullPath);
	fl_add_browser_line(dialog_->browser, str.c_str());

	activeStyle = whichStyle;
}
