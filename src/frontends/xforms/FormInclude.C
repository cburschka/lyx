/**
 * \file FormInclude.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Alejandro Aguilar Sierra
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Angus Leeming, a.leeming@.ac.uk
 */

#include <algorithm>
#include <utility>

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>
#include "xformsBC.h"
#include "ControlInclude.h"
#include "FormInclude.h"
#include "form_include.h"
#include "insets/insetinclude.h"

#include "frontends/FileDialog.h"
#include "LyXView.h"
#include "buffer.h"

#include "xforms_helpers.h" // setEnabled
#include "support/filetools.h"
#include "support/lstrings.h"
#include "lyxrc.h"

using std::make_pair;
using std::pair;

typedef FormCB<ControlInclude, FormDB<FD_form_include> > base_class;

FormInclude::FormInclude(ControlInclude & c)
	: base_class(c, _("Include file"))
{}


void FormInclude::build()
{
	dialog_.reset(build_include());

	// Manage the ok and cancel buttons
	bc().setOK(dialog_->button_ok);
	bc().setCancel(dialog_->button_cancel);
	bc().refresh();

	bc().addReadOnly(dialog_->button_browse);
	bc().addReadOnly(dialog_->check_verbatim);
	bc().addReadOnly(dialog_->check_typeset);
	bc().addReadOnly(dialog_->check_useinput);
	bc().addReadOnly(dialog_->check_useinclude);
}


void FormInclude::update()
{
	if (controller().params().noload) {
		fl_set_input(dialog_->input_filename, "");
		fl_set_button(dialog_->check_typeset, 0);
		fl_set_button(dialog_->check_useinput, 0);
		fl_set_button(dialog_->check_useinclude, 1);
		fl_set_button(dialog_->check_verbatim, 0);
		fl_set_button(dialog_->check_visiblespace, 0);
		fl_deactivate_object(dialog_->check_visiblespace);
		fl_set_object_lcol(dialog_->check_visiblespace, FL_INACTIVE);
		return;
	}

	fl_set_input(dialog_->input_filename,
		     controller().params().cparams.getContents().c_str());

	string const cmdname = controller().params().cparams.getCmdName();

	fl_set_button(dialog_->check_typeset,
		      int(controller().params().noload));

	fl_set_button(dialog_->check_useinput, cmdname == "input");
	fl_set_button(dialog_->check_useinclude, cmdname == "include");
	if (cmdname == "verbatiminput" || cmdname == "verbatiminput*") {
		fl_set_button(dialog_->check_verbatim, 1);
		fl_set_button(dialog_->check_visiblespace, cmdname == "verbatiminput*");
		setEnabled(dialog_->check_visiblespace, true);
	} else {
		fl_set_button(dialog_->check_visiblespace, 0);
		setEnabled(dialog_->check_visiblespace, false);
	}
 
	if (cmdname.empty())
		fl_set_button(dialog_->check_useinclude, 1);
}


void FormInclude::apply()
{
	controller().params().noload = fl_get_button(dialog_->check_typeset);

	controller().params().cparams.
		setContents(fl_get_input(dialog_->input_filename));

	if (fl_get_button(dialog_->check_useinput))
		controller().params().flag = InsetInclude::INPUT;
	else if (fl_get_button(dialog_->check_useinclude))
		controller().params().flag = InsetInclude::INCLUDE;
	else if (fl_get_button(dialog_->check_verbatim)) {
		if (fl_get_button(dialog_->check_visiblespace))
			controller().params().flag = InsetInclude::VERBAST;
		else
			controller().params().flag = InsetInclude::VERB;
	}
}


ButtonPolicy::SMInput FormInclude::input(FL_OBJECT * ob, long)
{
	if (ob == dialog_->button_browse)
		return inputBrowse();

	if (ob == dialog_->button_load) {
		if (compare(fl_get_input(dialog_->input_filename),"")) {
			ApplyButton();
			return ButtonPolicy::SMI_NOOP;
		}
	}

	if (ob == dialog_->check_verbatim) {
		setEnabled(dialog_->check_visiblespace, true);

	} else if (ob == dialog_->check_useinclude ||
		   ob == dialog_->check_useinput) {
		fl_set_button(dialog_->check_visiblespace, 0);
		setEnabled(dialog_->check_visiblespace, false);
	}
	
	return ButtonPolicy::SMI_VALID;
}
	

ButtonPolicy::SMInput FormInclude::inputBrowse()
{
	// Should browsing too be disabled in RO-mode?
	FileDialog fileDlg(controller().lv(),
			   _("Select document to include"),
			   LFUN_SELECT_FILE_SYNC,
			   make_pair(string(_("Documents")),
				     string(lyxrc.document_path)));

	string ext;
		   
	// input TeX, verbatim, or LyX file ?
	if (fl_get_button(dialog_->check_useinput))
		ext = _("*.tex| LaTeX Documents (*.tex)");
	else if (fl_get_button(dialog_->check_verbatim))
		ext = _("*| All files ");
	else
		ext = _("*.lyx| LyX Documents (*.lyx)");
	
	string const mpath =
		OnlyPath(controller().params().masterFilename_);

	FileDialog::Result const result =
		fileDlg.Select(mpath, ext,
			       fl_get_input(dialog_->input_filename));
	
	// check selected filename
	if (result.second.empty())
		return ButtonPolicy::SMI_NOOP;
	
	string const filename2 = MakeRelPath(result.second, mpath);

	if (prefixIs(filename2, ".."))
		fl_set_input(dialog_->input_filename, result.second.c_str());
	else
		fl_set_input(dialog_->input_filename, filename2.c_str());

	return ButtonPolicy::SMI_VALID;
}
