// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2000 The LyX Team.
 *
 * ======================================================
 */

#include <config.h>
#include <vector>

#include FORMS_H_LOCATION

#ifdef __GNUG__
#pragma implementation
#endif


#include "Dialogs.h"
#include "FormToc.h"
#include "LyXView.h"
#include "form_toc.h"
#include "lyxtext.h"
#include "lyxfunc.h"
#include "support/lstrings.h"

// The current code uses the apply() for handling the Update button and the
// type-of-table selection and cancel() for the close button.  This is a little
// confusing to the button controller so I've made an IgnorantPolicy to cover
// this situation since the dialog doesn't care about buttons. ARRae 20001013
FormToc::FormToc(LyXView * lv, Dialogs * d)
	: FormCommand(lv, d, _("Table of Contents"), new IgnorantPolicy),
	  dialog_(0)
{
	// let the dialog be shown
	// These are permanent connections so we won't bother
	// storing a copy because we won't be disconnecting.
	d->showTOC.connect(slot(this, &FormToc::showInset));
	d->createTOC.connect(slot(this, &FormToc::createInset));
}


FormToc::~FormToc()
{
	delete dialog_;
}


FL_FORM * FormToc::form() const
{
	if ( dialog_ ) return dialog_->form;
	return 0;
}


void FormToc::disconnect()
{
	toclist.clear();
	FormCommand::disconnect();
}


void FormToc::build()
{
	dialog_ = build_toc();
	fl_addto_choice(dialog_->type,
			_(" TOC | LOF | LOT | LOA "));

	// Don't need to limit size of this dialog
}


// we can safely ignore the parameter because we can always update
void FormToc::update(bool)
{
	Buffer::TocType type;

	if ( params.getCmdName() == "tableofcontents" )
		type = Buffer::TOC_TOC;

	else if ( params.getCmdName() == "listofalgorithms" )
		type = Buffer::TOC_LOA;

	else if ( params.getCmdName() == "listoffigures" )
		type = Buffer::TOC_LOF;

	else
		type = Buffer::TOC_LOT;
	
	fl_set_choice( dialog_->type, type+1 );

	updateToc();
}


void FormToc::updateToc()
{
  	if (!lv_->view()->available()) {
		toclist.clear();
		fl_clear_browser( dialog_->browser );
		fl_add_browser_line( dialog_->browser,
				     _("*** No Document ***"));
		return;
	}

	vector<vector<Buffer::TocItem> > tmp =
		lv_->view()->buffer()->getTocList();
	int type = fl_get_choice( dialog_->type ) - 1;

	// Check if all elements are the same.
	if (toclist.size() == tmp[type].size()) {
		unsigned int i = 0;
		for (; i < toclist.size(); ++i) {
			if (toclist[i] !=  tmp[type][i])
				break;
		}
		if (i >= toclist.size()) return;
	}

	// List has changed. Update browser
	toclist = tmp[type];

	static Buffer * buffer = 0;
	int topline = 0;
	int line = 0;
	if (buffer == lv_->view()->buffer()) {
		topline = fl_get_browser_topline( dialog_->browser );
		line = fl_get_browser( dialog_->browser );
	} else
		buffer = lv_->view()->buffer();

	fl_clear_browser( dialog_->browser );

	for (vector<Buffer::TocItem>::const_iterator it = toclist.begin();
	     it != toclist.end(); ++it)
		fl_add_browser_line( dialog_->browser,
				     (string(4 * (*it).depth, ' ')
				      + (*it).str).c_str());

	fl_set_browser_topline( dialog_->browser, topline );
	fl_select_browser_line( dialog_->browser, line );
}

 
void FormToc::apply()
{
  	if (!lv_->view()->available())
		return;

	updateToc();

	unsigned int const choice = fl_get_browser( dialog_->browser );
	if (0 < choice && choice - 1 < toclist.size()) {
		string const tmp = tostr(toclist[choice-1].par->id());
		lv_->getLyXFunc()->Dispatch(LFUN_GOTO_PARAGRAPH, tmp);
	}
}
