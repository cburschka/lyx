/**
 * \file FormTexinfo.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Herbert Voss <voss@lyx.org>
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

	setPrehandler(dialog_->button_rescan);
	setPrehandler(dialog_->button_view);
	setPrehandler(dialog_->button_texhash);
	setPrehandler(dialog_->button_fullPath);
	setPrehandler(dialog_->browser);
	setPrehandler(dialog_->radio_cls);
	setPrehandler(dialog_->radio_sty);
	setPrehandler(dialog_->radio_bst);
	setPrehandler(dialog_->message);

	setTooltipHandler(dialog_->button_rescan);
	setTooltipHandler(dialog_->button_view);
	setTooltipHandler(dialog_->button_texhash);
	setTooltipHandler(dialog_->button_fullPath);
	setTooltipHandler(dialog_->browser);
	setTooltipHandler(dialog_->radio_cls);
	setTooltipHandler(dialog_->radio_sty);
	setTooltipHandler(dialog_->radio_bst);
	setTooltipHandler(dialog_->message);
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


string const FormTexinfo::getMinimalTooltip(FL_OBJECT * ob) const
{
	string str;

	if (ob == dialog_->radio_cls) {
		str = N_("");

	} else if (ob == dialog_->radio_sty) {
		str = _("");

	} else if (ob == dialog_->radio_bst) {
		str = _("");

	} else if (ob == dialog_->button_rescan) {
		str = _("Rescan File List");

	} else if (ob == dialog_->button_fullPath) {
		str = _("Show Full Path or not");

	} else if (ob == dialog_->button_texhash) {
		str = _("");

	} else if (ob == dialog_->button_view) {
		str = N_("View Content of the File");

	}

	return str;
}


string const FormTexinfo::getVerboseTooltip(FL_OBJECT * ob) const
{
	string str;

	if (ob == dialog_->radio_cls) {
		str = N_("Shows the installed LaTeX Document classes. Remember, that these classes are only available in LyX if a corresponding LyX layout file exists!");

	} else if (ob == dialog_->radio_sty) {
		str = _("Shows the installed LaTeX style files, which are available in LyX by default, like \"babel\" or through \\usepackage{<the stylefile>} in LaTeX preamble.");

	} else if (ob == dialog_->radio_bst) {
		str = _("Shows the installed style files for BibTeX. They can be loaded through insert->Lists&Toc->BibTeX Reference->Style.");

	} else if (ob == dialog_->button_rescan) {
		str = _("Runs the script \"TexFiles.sh\" to build new file lists.");

	} else if (ob == dialog_->button_fullPath) {
		str = _("View full path or only file name. Full path is needed to view the contents of a file.");

	} else if (ob == dialog_->button_texhash) {
		str = _("Runs the script \"texhash\" which builds the a new LaTeX tree. Needed if you install a new TeX class or style. To execute it, you need the write permissions for the tex-dirs, often /var/lib/texmf and other.");

	} else if (ob == dialog_->button_view) {
		str = N_("Shows the contents of the marked file. Only possible in full path mode.");

	}

	return str;
}
