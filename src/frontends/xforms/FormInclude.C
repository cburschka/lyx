/**
 * \file FormInclude.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Alejandro Aguilar Sierra
 * \author John Levon
 */
#include <config.h>
#include <algorithm>
#include <utility>

#ifdef __GNUG__
#pragma implementation
#endif

#include "Dialogs.h"
#include "FormInclude.h"
#include "insets/insetinclude.h"
#include "frontends/FileDialog.h"
#include "support/filetools.h"
#include "support/lstrings.h"
#include "LyXView.h"
#include "buffer.h"
#include "lyxrc.h"
#include "lyxfunc.h"
#include "xforms_helpers.h"

#include "form_include.h"

using std::make_pair;
using std::pair;

FormInclude::FormInclude(LyXView * lv, Dialogs * d)
	: FormBaseBD(lv, d, _("Include file")),
	  ih_(0), inset_(0)
{
	d->showInclude.connect(slot(this, &FormInclude::showInclude));
}


FL_FORM * FormInclude::form() const
{
	if (dialog_.get()) 
		return dialog_->form;
	return 0;
}


void FormInclude::connect()
{
	u_ = d_->updateBufferDependent.
		 connect(slot(this, &FormInclude::updateSlot));
	h_ = d_->hideBufferDependent.
		 connect(slot(this, &FormInclude::hide));
	FormBaseDeprecated::connect();
}


void FormInclude::disconnect()
{
	ih_.disconnect();
	FormBaseBD::disconnect();
	inset_ = 0;
}


void FormInclude::updateSlot(bool switched)
{
	if (switched)
		hide();
	else
		update();
}


void FormInclude::build()
{
	dialog_.reset(build_include());

	// Workaround dumb xforms sizing bug
	minw_ = form()->w;
	minh_ = form()->h;

	// Manage the ok and cancel buttons
	bc_.setOK(dialog_->button_ok);
	bc_.setCancel(dialog_->button_cancel);
	bc_.refresh();

	bc_.addReadOnly(dialog_->button_browse);
	bc_.addReadOnly(dialog_->check_verbatim);
	bc_.addReadOnly(dialog_->check_typeset);
	bc_.addReadOnly(dialog_->check_useinput);
	bc_.addReadOnly(dialog_->check_useinclude);
}


void FormInclude::showInclude(InsetInclude * inset)
{
	// If connected to another inset, disconnect from it.
	if (inset_)
		ih_.disconnect();

	inset_    = inset;
	params    = inset->params();
	ih_ = inset->hideDialog.connect(slot(this, &FormInclude::hide));
	show();
}


void FormInclude::update()
{
	bc().readOnly(lv_->buffer()->isReadonly());

	if (!inset_) {
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

	fl_set_input(dialog_->input_filename, params.cparams.getContents().c_str());

	string const cmdname = params.cparams.getCmdName();

	fl_set_button(dialog_->check_typeset, int(params.noload));

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
	if (lv_->buffer()->isReadonly())
		return;

	params.noload = fl_get_button(dialog_->check_typeset);

	params.cparams.setContents(fl_get_input(dialog_->input_filename));

	if (fl_get_button(dialog_->check_useinput))
		params.flag = InsetInclude::INPUT;
	else if (fl_get_button(dialog_->check_useinclude))
		params.flag = InsetInclude::INCLUDE;
	else if (fl_get_button(dialog_->check_verbatim)) {
		if (fl_get_button(dialog_->check_visiblespace))
			params.flag = InsetInclude::VERBAST;
		else
			params.flag = InsetInclude::VERB;
	}
	
	inset_->setFromParams(params);
	lv_->view()->updateInset(inset_, true);
}

#ifdef WITH_WARNINGS
#warning convert this to use the buttoncontroller
#endif
bool FormInclude::input(FL_OBJECT *, long data)
{
	State state = static_cast<State>(data);

	switch (state) {
		case BROWSE: {
			// Should browsing too be disabled in RO-mode?
			FileDialog fileDlg(lv_, _("Select document to include"),
				LFUN_SELECT_FILE_SYNC,
				make_pair(string(_("Documents")), string(lyxrc.document_path)));

			string ext;
		   
			/* input TeX, verbatim, or LyX file ? */
			if (fl_get_button(dialog_->check_useinput))
				ext = _("*.tex| LaTeX Documents (*.tex)");
			else if (fl_get_button(dialog_->check_verbatim))
				ext = _("*| All files ");
			else
				ext = _("*.lyx| LyX Documents (*.lyx)");
	
			string mpath;

			mpath = OnlyPath(params.buffer->fileName());

			FileDialog::Result result = fileDlg.Select(mpath, ext, fl_get_input(dialog_->input_filename));
	
			// check selected filename
			if (result.second.empty())
				break;
	
			string const filename2 = MakeRelPath(result.second, mpath);
	
			if (prefixIs(filename2, ".."))
				fl_set_input(dialog_->input_filename, result.second.c_str());
			else
				fl_set_input(dialog_->input_filename, filename2.c_str());

		}	break;

		case LOAD:
			if (compare(fl_get_input(dialog_->input_filename),"")) {
				apply();
				lv_->getLyXFunc()->Dispatch(LFUN_CHILDOPEN, params.cparams.getContents());
			}
			break;

		case VERBATIM:
			setEnabled(dialog_->check_visiblespace, true);
			break;
	
		case INPUTINCLUDE:
			fl_set_button(dialog_->check_visiblespace, 0);
			setEnabled(dialog_->check_visiblespace, false);
			break;
	}
	return true;
}
