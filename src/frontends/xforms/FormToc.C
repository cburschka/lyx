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

using std::vector;

// The current code uses the apply() for handling the Update button and the
// type-of-table selection and cancel() for the close button.  This is a little
// confusing to the button controller so I've made an IgnorantPolicy to cover
// this situation since the dialog doesn't care about buttons. ARRae 20001013
FormToc::FormToc(LyXView * lv, Dialogs * d)
	: FormCommand(lv, d, _("Table of Contents")),
	  dialog_(0)
{
	// let the dialog be shown
	// These are permanent connections so we won't bother
	// storing a copy because we won't be disconnecting.
	d->showTOC.connect(slot(this, &FormToc::showInset));
	d->createTOC.connect(slot(this, &FormToc::createInset));
}


FL_FORM * FormToc::form() const
{
	if (dialog_.get())
		return dialog_->form;
	return 0;
}


void FormToc::disconnect()
{
	toclist.clear();
	FormCommand::disconnect();
}


void FormToc::build()
{
	dialog_.reset(build_toc());

#if 0
	fl_addto_choice(dialog_->choice_toc_type,
			_(" TOC | LOF | LOT | LOA "));
#else
	Buffer::Lists const tmp = lv_->view()->buffer()->getLists();
	Buffer::Lists::const_iterator cit = tmp.begin();
	Buffer::Lists::const_iterator end = tmp.end();
	for (; cit != end; ++cit) {
		fl_addto_choice(dialog_->choice_toc_type, cit->first.c_str());
	}
#endif
	// Don't need to limit size of this dialog
	// (but fixing min size is a GOOD thing).
	// Workaround dumb xforms sizing bug
	minw_ = form()->w;
	minh_ = form()->h;

        // Manage the cancel/close button
	bc().setCancel(dialog_->button_cancel);
	bc().refresh();
}


void FormToc::update()
{
#if 0
	Buffer::TocType type;

	if (params.getCmdName() == "tableofcontents" )
		type = Buffer::TOC_TOC;

	else if (params.getCmdName() == "listofalgorithms" )
		type = Buffer::TOC_LOA;

	else if (params.getCmdName() == "listoffigures" )
		type = Buffer::TOC_LOF;

	else
		type = Buffer::TOC_LOT;
	
	fl_set_choice( dialog_->choice_toc_type, type+1 );
#else
#warning Reimplement (Lgb)
#endif
	updateToc();
}


void FormToc::updateToc()
{
#if 0
  	if (!lv_->view()->available()) {
		toclist.clear();
		fl_clear_browser( dialog_->browser_toc );
		fl_add_browser_line( dialog_->browser_toc,
				     _("*** No Document ***"));
		return;
	}

	vector<vector<Buffer::TocItem> > tmp =
		lv_->view()->buffer()->getTocList();
	int type = fl_get_choice( dialog_->choice_toc_type ) - 1;

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
		topline = fl_get_browser_topline( dialog_->browser_toc );
		line = fl_get_browser( dialog_->browser_toc );
	} else
		buffer = lv_->view()->buffer();

	fl_clear_browser( dialog_->browser_toc );

	for (vector<Buffer::TocItem>::const_iterator it = toclist.begin();
	     it != toclist.end(); ++it)
		fl_add_browser_line( dialog_->browser_toc,
				     (string(4 * (*it).depth, ' ')
				      + (*it).str).c_str());

	fl_set_browser_topline( dialog_->browser_toc, topline );
	fl_select_browser_line( dialog_->browser_toc, line );
#else
#warning Fix Me! (Lgb)
  	if (!lv_->view()->available()) {
		toclist.clear();
		fl_clear_browser( dialog_->browser_toc );
		fl_add_browser_line( dialog_->browser_toc,
				     _("*** No Document ***"));
		return;
	}

	Buffer::Lists tmp = lv_->view()->buffer()->getLists();
	string const type =
		fl_get_choice_item_text(dialog_->choice_toc_type,
					fl_get_choice(dialog_->choice_toc_type));

	Buffer::Lists::iterator it = tmp.find(type);

	if (it != tmp.end()) {
		// Check if all elements are the same.
		if (toclist == it->second) {
			return;
		}
	} else if (it == tmp.end()) {
		toclist.clear();
		fl_clear_browser(dialog_->browser_toc);
		fl_add_browser_line(dialog_->browser_toc,
				    _("*** No Lists ***"));
		return;
	}
	
	// List has changed. Update browser
	toclist = it->second;

	static Buffer * buffer = 0;
	int topline = 0;
	int line = 0;
	if (buffer == lv_->view()->buffer()) {
		topline = fl_get_browser_topline(dialog_->browser_toc);
		line = fl_get_browser( dialog_->browser_toc );
	} else
		buffer = lv_->view()->buffer();

	fl_clear_browser(dialog_->browser_toc);

	Buffer::SingleList::const_iterator cit = toclist.begin();
	Buffer::SingleList::const_iterator end = toclist.end();
	
	for (; cit != end; ++cit) {
		string const line = string(4 * cit->depth, ' ') + cit->str;
		fl_add_browser_line(dialog_->browser_toc, line.c_str());
	}
	
	fl_set_browser_topline(dialog_->browser_toc, topline);
	fl_select_browser_line(dialog_->browser_toc, line);
#endif
}

 
bool FormToc::input(FL_OBJECT *, long)
{
	updateToc();

	unsigned int const choice = fl_get_browser( dialog_->browser_toc );
	if (0 < choice && choice - 1 < toclist.size()) {
		string const tmp = tostr(toclist[choice-1].par->id());
		lv_->getLyXFunc()->Dispatch(LFUN_GOTO_PARAGRAPH, tmp);
	}

	return true;
}
