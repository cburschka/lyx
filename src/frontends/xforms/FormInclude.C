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
#include <iostream>

#ifdef __GNUG__
#pragma implementation
#endif

#include "Dialogs.h"
#include "FormInclude.h"
#include "insets/insetinclude.h" 
#include "filedlg.h"
#include "support/filetools.C"
#include "support/lstrings.h" 
#include "LyXView.h"
#include "buffer.h"
#include "lyxrc.h" 
#include "lyxfunc.h" 
#include "xforms_helpers.h" 
 
#include "form_include.h"

using std::cout;

FormInclude::FormInclude(LyXView * lv, Dialogs * d)
	: FormCommand(lv, d, _("Include file"), new OkCancelPolicy),
	  dialog_(0)
{
	// let the dialog be shown
	// These are permanent connections so we won't bother
	// storing a copy because we won't be disconnecting.
	d->showInclude.connect(slot(this, &FormInclude::showInset));
	d->createInclude.connect(slot(this, &FormInclude::createInset));
}


FormInclude::~FormInclude()
{
	delete dialog_;
}


FL_FORM * FormInclude::form() const
{
	if (dialog_) 
		return dialog_->form;
	return 0;
}


void FormInclude::build()
{
	dialog_ = build_include();

	// Workaround dumb xforms sizing bug
	minw_ = form()->w;
	minh_ = form()->h;

	// Manage the ok and cancel buttons
	bc_.setOK(dialog_->button_ok);
	bc_.setCancel(dialog_->button_cancel);
	bc_.refresh();

	bc_.addReadOnly(dialog_->browsebt);
	bc_.addReadOnly(dialog_->flag1);
	bc_.addReadOnly(dialog_->flag2);
	bc_.addReadOnly(dialog_->flag3);
	bc_.addReadOnly(dialog_->flag4);
	bc_.addReadOnly(dialog_->filename);
	bc_.addReadOnly(dialog_->flag41);
}


void FormInclude::update()
{
	bc_.readOnly(lv_->buffer()->isReadonly());

	if (!inset_) {
		fl_set_input(dialog_->filename, "");
		fl_set_button(dialog_->flag1, 0);
		fl_set_button(dialog_->flag2, 0);
		fl_set_button(dialog_->flag3, 1);
		fl_set_button(dialog_->flag4, 0);
		fl_set_button(dialog_->flag41, 0);
		fl_deactivate_object(dialog_->flag41);
		fl_set_object_lcol(dialog_->flag41, FL_INACTIVE);
		return;
	}
 
	fl_set_input(dialog_->filename, params.getContents().c_str());
 
	string const cmdname = params.getCmdName();
 
	/* FIXME: what do with Don't typeset here ... */
	//fl_set_button(dialog_->flag1, int(inset_->isNoLoad()));
 
	fl_set_button(dialog_->flag2, cmdname == "input");
	fl_set_button(dialog_->flag3, cmdname == "include");
	if (cmdname == "verbatiminput" || cmdname == "verbatiminput*") {
		fl_set_button(dialog_->flag4, 1);
		fl_set_button(dialog_->flag41, cmdname == "verbatiminput*");
	} else {
		fl_set_button(dialog_->flag41, 0);
		fl_deactivate_object(dialog_->flag41);
		fl_set_object_lcol(dialog_->flag41, FL_INACTIVE);
	}
}


void FormInclude::apply()
{
	if (lv_->buffer()->isReadonly())
		return;

	/* FIXME: no way to update internal flags of inset ??? */
 
	//inset_->setNoLoad(fl_get_button(dialog_->flag1));
 
	params.setContents(fl_get_input(dialog_->filename));
	cout << params.getContents() << endl; 
	if (fl_get_button(dialog_->flag2))
		params.setCmdName("input");
	else if (fl_get_button(dialog_->flag3))
		params.setCmdName("include");
	else if (fl_get_button(dialog_->flag4)) {
		if (fl_get_button(dialog_->flag41))
			params.setCmdName("verbatiminput*");
		else
			params.setCmdName("verbatiminput");	
	}
	
	if (inset_) {
		if (params != inset_->params()) {
			inset_->setParams(params);
			lv_->view()->updateInset(inset_, true);
		}
	} else
		lv_->getLyXFunc()->Dispatch(LFUN_CHILD_INSERT, params.getAsString());
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
			LyXFileDlg fileDlg;
 
			string ext;
		    
			fileDlg.SetButton(0, _("Documents"), lyxrc.document_path);

			/* input TeX, verbatim, or LyX file ? */
			if (fl_get_button(dialog_->flag2))
				ext = "*.tex";
			else if (fl_get_button(dialog_->flag4))
				ext = "*";
			else
				ext = "*.lyx";
	 
			string mpath;
 
			/* FIXME: what do I do here ? */ 
			//if (inset_)
			//	mpath = OnlyPath(inset_->getMasterFilename());
 
			string const filename = fileDlg.Select(_("Select Child Document"),
						mpath, ext, fl_get_input(dialog_->filename));
			XFlush(fl_get_display());
	 
			// check selected filename
			if (filename.empty())
				break;
	 
			string const filename2 = MakeRelPath(filename, mpath);
	 
			if (prefixIs(filename2, ".."))
				fl_set_input(dialog_->filename, filename.c_str());
			else
				fl_set_input(dialog_->filename, filename2.c_str());
 
		}	break;

		case LOAD:
			apply();
			lv_->getLyXFunc()->Dispatch(LFUN_CHILDOPEN, params.getContents());
			break;

		case VERBATIM:
			setEnabled(dialog_->flag41, true);
			break;
	 
		case INPUTINCLUDE:
			cout << "inputinclude" << endl;
			/* huh ? why doesn't this work ? */ 
			setEnabled(dialog_->flag41, false);
			fl_set_button(dialog_->flag41, 0);
			break;
	}
	return true; 
}
