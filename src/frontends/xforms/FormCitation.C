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
#include <algorithm>

#ifdef __GNUG__
#pragma implementation
#endif


#include "Dialogs.h"
#include "FormCitation.h"
#include "LyXView.h"
#include "buffer.h"
#include "form_citation.h"
#include "lyxfunc.h"
#include "support/filetools.h"

using std::find;
using std::max;
using std::min;
using std::pair;
using std::sort;
using std::vector;


FormCitation::FormCitation(LyXView * lv, Dialogs * d)
	: FormCommand(lv, d, _("Citation"), new NoRepeatedApplyReadOnlyPolicy),
	  dialog_(0)
{
	// let the dialog be shown
	// These are permanent connections so we won't bother
	// storing a copy because we won't be disconnecting.
	d->showCitation.connect(slot(this, &FormCitation::showInset));
	d->createCitation.connect(slot(this, &FormCitation::createInset));
}


FormCitation::~FormCitation()
{
	delete dialog_;
}


FL_FORM * FormCitation::form() const
{
	if (dialog_ ) return dialog_->form;
	return 0;
}


void FormCitation::connect()
{
	//fl_set_form_maxsize( dialog_->form, 3*minw_, minh_ );
	FormCommand::connect();
}


void FormCitation::disconnect()
{
	citekeys.clear();
	bibkeys.clear();
	bibkeysInfo.clear();

	FormCommand::disconnect();
}


void FormCitation::build()
{
	dialog_ = build_citation();

	// Workaround dumb xforms sizing bug
	minw_ = form()->w;
	minh_ = form()->h;

        // Manage the ok, apply, restore and cancel/close buttons
	bc_.setOK(dialog_->button_ok);
	bc_.setApply(dialog_->button_apply);
	bc_.setCancel(dialog_->button_cancel);
	bc_.setUndoAll(dialog_->button_restore);
	bc_.refresh();

	bc_.addReadOnly(dialog_->addBtn);
	bc_.addReadOnly(dialog_->delBtn);
	bc_.addReadOnly(dialog_->upBtn);
	bc_.addReadOnly(dialog_->downBtn);
	bc_.addReadOnly(dialog_->textBefore);
	bc_.addReadOnly(dialog_->textAftr);

	bc_.addDontTriggerChange(dialog_->citeBrsr);
	bc_.addDontTriggerChange(dialog_->bibBrsr);
}


void FormCitation::update()
{
	bc_.readOnly(lv_->buffer()->isReadonly());

	bibkeys.clear();
	bibkeysInfo.clear();

	vector<pair<string,string> > blist =
		lv_->buffer()->getBibkeyList();
	sort(blist.begin(), blist.end());

	for (unsigned int i = 0; i < blist.size(); ++i) {
		bibkeys.push_back(blist[i].first);
		bibkeysInfo.push_back(blist[i].second);
	}
	blist.clear();

	citekeys.clear();
	string tmp, keys( params.getContents() );
	keys = frontStrip( split(keys, tmp, ',') );
	while (!tmp.empty()) {
		citekeys.push_back( tmp );
		keys = frontStrip( split(keys, tmp, ',') );
	}

	updateBrowser( dialog_->bibBrsr,  bibkeys );
	updateBrowser( dialog_->citeBrsr, citekeys );
	fl_clear_browser( dialog_->infoBrsr );

	// No keys have been selected yet, so...
	setBibButtons( OFF );
	setCiteButtons( OFF );

	int noKeys = static_cast<int>( max( bibkeys.size(), citekeys.size() ) );

	// Place bounds, so that 4 <= noKeys <= 10
	noKeys = max(4, min(10, noKeys) );

	// Re-size the form to accommodate the new browser size
	int size = 20 * noKeys;
	bool bibPresent = ( bibkeys.size() > 0 );
	setSize( size, bibPresent );

	fl_set_input( dialog_->textAftr, params.getOptions().c_str());
}


void FormCitation::updateBrowser( FL_OBJECT * browser,
				  vector<string> const & keys ) const
{
	fl_clear_browser( browser );

	for (unsigned int i = 0; i < keys.size(); ++i )
		fl_add_browser_line( browser, keys[i].c_str());
}


void FormCitation::setBibButtons( State status ) const
{
	switch (status) {
	case ON:
		fl_activate_object( dialog_->addBtn );
		fl_set_object_lcol( dialog_->addBtn, FL_BLACK );
		break;

	case OFF:
		fl_deactivate_object( dialog_->addBtn );
		fl_set_object_lcol( dialog_->addBtn, FL_INACTIVE );
		break;

	default:
		break;
	}
}


void FormCitation::setCiteButtons( State status ) const
{
	switch (status) {
	case ON:
        {
		fl_activate_object( dialog_->delBtn );
		fl_set_object_lcol( dialog_->delBtn, FL_BLACK );

		int sel = fl_get_browser( dialog_->citeBrsr );

		if (sel != 1) {
			fl_activate_object( dialog_->upBtn );
			fl_set_object_lcol( dialog_->upBtn, FL_BLACK );
		} else {
			fl_deactivate_object( dialog_->upBtn );
			fl_set_object_lcol( dialog_->upBtn, FL_INACTIVE );
		}

		if (sel != fl_get_browser_maxline(dialog_->citeBrsr)) {
			fl_activate_object( dialog_->downBtn );
			fl_set_object_lcol( dialog_->downBtn, FL_BLACK );
		} else {
			fl_deactivate_object( dialog_->downBtn );
			fl_set_object_lcol( dialog_->downBtn, FL_INACTIVE );
		}

		break;
	}
	case OFF:
	{
		fl_deactivate_object( dialog_->delBtn );
		fl_set_object_lcol( dialog_->delBtn, FL_INACTIVE );

		fl_deactivate_object( dialog_->upBtn );
		fl_set_object_lcol( dialog_->upBtn, FL_INACTIVE );

		fl_deactivate_object( dialog_->downBtn );
		fl_set_object_lcol( dialog_->downBtn, FL_INACTIVE );
	}
	default:
		break;
	}
}


void FormCitation::setSize( int hbrsr, bool bibPresent ) const
{
	bool const natbib = false; // will eventually be input
	hbrsr = max( hbrsr, 175 ); // limit max size of cite/bib brsrs

	// dh1, dh2, dh3 are the vertical separation between elements.
	// These can be specified because the browser height is fixed
	// so they are not changed by dynamic resizing
	static int const dh1 = 30; // top of form to top of cite/bib brsrs;
	                           // bottom of cite/bib brsrs to top of info;
	                           // bottom of info to top next element;
	                           // bottom of style to top textBefore;
	                           // bottom of text to top ok/cancel buttons.
	static int const dh2 = 10; // bottom of textBefore to top textAftr;
	                           // bottom of ok/cancel buttons to bottom form
	static int const dh3 = 5;  // spacing between add/delete/... buttons.

	int const wbrsr  = dialog_->citeBrsr->w;
	static int const hinfo  = dialog_->infoBrsr->h;
	static int const hstyle = dialog_->style->h;
	static int const htext  = dialog_->textAftr->h;
	static int const hok    = dialog_->button_ok->h;

	int hform = dh1 + hbrsr + dh1;
	if (bibPresent ) hform += hinfo + dh1;
	if (natbib ) hform += hstyle + dh1 + htext + dh2;
	hform += htext + dh1 + hok + dh2;

	if (hform != minh_) {
		minh_ = hform;
		fl_set_form_size( dialog_->form, minw_, minh_ );
	} else
		return;

	int x = 0;
	int y = 0;
	fl_set_object_geometry( dialog_->box, x, y, minw_, minh_ );

	x = dialog_->citeBrsr->x;
	y += dh1; 
	fl_set_object_geometry( dialog_->citeBrsr, x, y, wbrsr, hbrsr );
	x = dialog_->bibBrsr->x;
	fl_set_object_geometry( dialog_->bibBrsr,  x, y, wbrsr, hbrsr );

	x = dialog_->addBtn->x;
	fl_set_object_position( dialog_->addBtn,  x, y );
	y += dh3 + dialog_->addBtn->h;
	fl_set_object_position( dialog_->delBtn,  x, y );
	y += dh3 + dialog_->delBtn->h;
	fl_set_object_position( dialog_->upBtn,   x, y );
	y += dh3 + dialog_->upBtn->h;
	fl_set_object_position( dialog_->downBtn, x, y );

	y = dh1 + hbrsr + dh1; // in position for next element

	if (bibPresent) {
		x = dialog_->infoBrsr->x;
		fl_set_object_position( dialog_->infoBrsr, x, y );
		fl_show_object( dialog_->infoBrsr );
		y += hinfo + dh1;
	} else
		fl_hide_object( dialog_->infoBrsr );

	if (natbib) {
		x = dialog_->style->x;
		fl_set_object_position( dialog_->style, x, y );
		fl_show_object( dialog_->style );
		x = dialog_->textBefore->x;
		y += hstyle + dh1;
		fl_set_object_position( dialog_->textBefore, x, y );
		fl_show_object( dialog_->textBefore );
		y += htext + dh2;
	} else {
		fl_hide_object( dialog_->style );
		fl_hide_object( dialog_->textBefore );
	}

	x = dialog_->textAftr->x;
	fl_set_object_position( dialog_->textAftr, x, y );

	y += htext + dh1;
	x = dialog_->button_restore->x;
	fl_set_object_position( dialog_->button_restore,     x, y );
	x = dialog_->button_ok->x;
	fl_set_object_position( dialog_->button_ok,     x, y );
	x = dialog_->button_apply->x;
	fl_set_object_position( dialog_->button_apply,  x, y );
	x = dialog_->button_cancel->x;
	fl_set_object_position( dialog_->button_cancel, x, y );
}


#ifdef WITH_WARNINGS
#warning convert this to use the buttoncontroller
#endif
bool FormCitation::input( FL_OBJECT *, long data )
{
	bool activate = false;
	State cb = static_cast<State>( data );

	switch (cb) {
	case BIBBRSR:
	{
		fl_deselect_browser( dialog_->citeBrsr );
		
		unsigned int sel = fl_get_browser( dialog_->bibBrsr );
		if (sel < 1 || sel > bibkeys.size() ) break;

		// Put into infoBrsr the additional info associated with
		// the selected bibBrsr key
		fl_clear_browser( dialog_->infoBrsr );
		fl_add_browser_line( dialog_->infoBrsr,
				     bibkeysInfo[sel - 1].c_str() );

		// Highlight the selected bibBrsr key in citeBrsr if present
		vector<string>::iterator it =
			find( citekeys.begin(), citekeys.end(), bibkeys[sel-1] );

		if (it != citekeys.end()) {
			int n = static_cast<int>( it - citekeys.begin() );
			fl_select_browser_line( dialog_->citeBrsr, n+1 );
			fl_set_browser_topline( dialog_->citeBrsr, n+1 );
		}

		if (!lv_->buffer()->isReadonly()) {
			if (it != citekeys.end()) {
				setBibButtons( OFF );
				setCiteButtons( ON );
			} else {
				setBibButtons( ON );
				setCiteButtons( OFF );
			}
		}
	}
	break;
	case CITEBRSR:
	{
		unsigned int sel = fl_get_browser( dialog_->citeBrsr );
		if (sel < 1 || sel > citekeys.size() ) break;

		if (!lv_->buffer()->isReadonly()) {
			setBibButtons( OFF );
			setCiteButtons( ON );
		}

		// Highlight the selected citeBrsr key in bibBrsr
		vector<string>::iterator it =
			find( bibkeys.begin(), bibkeys.end(), citekeys[sel-1] );

		if (it != bibkeys.end()) {
			int n = static_cast<int>( it - bibkeys.begin() );
			fl_select_browser_line( dialog_->bibBrsr, n+1 );
			fl_set_browser_topline( dialog_->bibBrsr, n+1 );

			// Put into infoBrsr the additional info associated with
			// the selected citeBrsr key
			fl_clear_browser( dialog_->infoBrsr );
			fl_add_browser_line( dialog_->infoBrsr,
					     bibkeysInfo[n].c_str() );
		}
	}
	break;
	case ADD:
	{
		if (lv_->buffer()->isReadonly() ) break;

		unsigned int sel = fl_get_browser( dialog_->bibBrsr );
		if (sel < 1 || sel > bibkeys.size() ) break;

		// Add the selected bibBrsr key to citeBrsr
		fl_addto_browser( dialog_->citeBrsr,
				  bibkeys[sel-1].c_str() );
		citekeys.push_back( bibkeys[sel-1] );

		int n = static_cast<int>( citekeys.size() );
		fl_select_browser_line( dialog_->citeBrsr, n );

		setBibButtons( OFF );
		setCiteButtons( ON );
		activate = true;
	}
	break;
	case DELETE:
	{
		if (lv_->buffer()->isReadonly() ) break;

		unsigned int sel = fl_get_browser( dialog_->citeBrsr );
		if (sel < 1 || sel > citekeys.size() ) break;

		// Remove the selected key from citeBrsr
		fl_delete_browser_line( dialog_->citeBrsr, sel ) ;
		citekeys.erase( citekeys.begin() + sel-1 );

		setBibButtons( ON );
		setCiteButtons( OFF );
		activate = true;
	}
	break;
	case UP:
	{
		if (lv_->buffer()->isReadonly() ) break;

		unsigned int sel = fl_get_browser( dialog_->citeBrsr );
		if (sel < 2 || sel > citekeys.size() ) break;

		// Move the selected key up one line
		vector<string>::iterator it = citekeys.begin() + sel-1;
		string tmp = *it;

		fl_delete_browser_line( dialog_->citeBrsr, sel );
		citekeys.erase( it );

		fl_insert_browser_line( dialog_->citeBrsr, sel-1, tmp.c_str() );
		fl_select_browser_line( dialog_->citeBrsr, sel-1 );
		citekeys.insert( it-1, tmp );
		setCiteButtons( ON );
		activate = true;
	}
	break;
	case DOWN:
	{
		if (lv_->buffer()->isReadonly() ) break;

		unsigned int sel = fl_get_browser( dialog_->citeBrsr );
		if (sel < 1 || sel > citekeys.size()-1 ) break;

		// Move the selected key down one line
		vector<string>::iterator it = citekeys.begin() + sel-1;
		string tmp = *it;

		fl_delete_browser_line( dialog_->citeBrsr, sel );
		citekeys.erase( it );

		fl_insert_browser_line( dialog_->citeBrsr, sel+1, tmp.c_str() );
		fl_select_browser_line( dialog_->citeBrsr, sel+1 );
		citekeys.insert( it+1, tmp );
		setCiteButtons( ON );
		activate = true;
	}
	break;
	default:
		break;
	}
	return activate;
}


void FormCitation::apply()
{
	if (lv_->buffer()->isReadonly()) return;

	string contents;
	for (unsigned int i = 0; i < citekeys.size(); ++i) {
		if (i > 0) contents += ", ";
		contents += citekeys[i];
	}

	params.setContents(contents);
	params.setOptions(fl_get_input(dialog_->textAftr));

	if (inset_ != 0) {
		// Only update if contents have changed
		if (params != inset_->params()) {
			inset_->setParams(params);
			lv_->view()->updateInset(inset_, true);
		}
	} else {
		lv_->getLyXFunc()->Dispatch(LFUN_CITATION_INSERT,
					    params.getAsString());
	}
}
