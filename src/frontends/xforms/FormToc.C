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


#include "gettext.h"
#include "Dialogs.h"
#include "FormToc.h"
#include "LyXView.h"
#include "buffer.h"
#include "form_toc.h"
#include "lyxtext.h"

static vector<Buffer::TocItem> toclist;

FormToc::FormToc(LyXView * lv, Dialogs * d)
	: FormCommand(lv, d, _("Table of Contents")), dialog_(0)
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


void FormToc::clearStore()
{
	toclist.clear();
}


void FormToc::build()
{
	dialog_ = build_toc();
	fl_addto_choice(dialog_->type,
			_(" TOC | LOF | LOT | LOA "));
}


FL_FORM * const FormToc::form() const
{
	if( dialog_ && dialog_->form_toc )
		return dialog_->form_toc;
	else
		return 0;
}


void FormToc::update()
{
	static int ow = -1, oh;
		
	if (ow < 0) {
		ow = form()->w;
		oh = form()->h;

		fl_set_form_minsize(form(), ow, oh);
		fl_set_form_maxsize(form(), 2*ow, oh);
	}

	Buffer::TocType type;

	if( params.getCmdName() == "tableofcontents" )
		type = Buffer::TOC_TOC;

	else if( params.getCmdName() == "listofalgorithms" )
		type = Buffer::TOC_LOA;

	else if( params.getCmdName() == "listoffigures" )
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
		fl_add_browser_line( dialog_->browser, _("*** No Document ***"));
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
				     (string(4*(*it).depth,' ')+(*it).str).c_str());

	fl_set_browser_topline( dialog_->browser, topline );
	fl_select_browser_line( dialog_->browser, line );
}

 
void FormToc::apply()
{
  	if (!lv_->view()->available())
		return;

	updateToc();

	unsigned int choice = fl_get_browser( dialog_->browser );
	if (0 < choice && choice - 1 < toclist.size()) {
		lv_->view()->beforeChange();
		lv_->view()->text->SetCursor( lv_->view(), toclist[choice-1].par, 0 );
		lv_->view()->text->sel_cursor = 
			lv_->view()->text->cursor;
		lv_->view()->update(BufferView::SELECT|BufferView::FITCUR);
	}
}
