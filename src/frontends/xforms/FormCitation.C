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

#include "gettext.h"
#include FORMS_H_LOCATION
#include "xform_macros.h"
#include "FormCitation.h"
#include "Dialogs.h"
#include "LyXView.h"
#include "lyxfunc.h"
#include "insets/insetcite.h"
#include "form_citation.h"
#include "buffer.h"
#include "BufferView.h"
#include "support/filetools.h"

#ifdef __GNUG__
#pragma implementation
#endif

using std::vector;
using std::pair;
using std::max;
using std::min;
using std::find;

C_RETURNCB(FormCitation, WMHideCB)
C_GENERICCB(FormCitation, OKCB)
C_GENERICCB(FormCitation, CancelCB)
C_GENERICCB(FormCitation, InputCB)

FormCitation::FormCitation(LyXView * lv, Dialogs * d)
	: dialog_(0), lv_(lv), d_(d), h_(0), inset_(0), dialogIsOpen(false)
{
	// let the dialog be shown
	// These are permanent connections so we won't bother
	// storing a copy because we won't be disconnecting.
	d->showCitation.connect(slot(this, &FormCitation::showInset));
	d->createCitation.connect(slot(this, &FormCitation::createInset));
}


FormCitation::~FormCitation()
{
	free();
}


void FormCitation::build()
{
  dialog_ = build_citation();
}


void FormCitation::showInset( InsetCitation * inset )
{
	if( dialogIsOpen || inset == 0 ) return;

	inset_ = inset;

	textAfter = inset->getOptions();
	updateCitekeys(inset->getContents());
	show();
}


void FormCitation::createInset( string const & arg )
{
	if( dialogIsOpen ) return;

	string keys;
	if (contains(arg, "|")) {
		keys = token(arg, '|', 0);
		textAfter = token(arg, '|', 1);
	} else {
		keys = arg;
		textAfter.erase();
	}

	updateCitekeys(keys);
	show();
}


void FormCitation::show()
{
	if (!dialog_) {
		build();
		fl_set_form_atclose(dialog_->form_citation,
				    C_FormCitationWMHideCB, 0);
	}

	update();  // make sure its up-to-date

	dialogIsOpen = true;
	if (dialog_->form_citation->visible) {
		fl_raise_form(dialog_->form_citation);
	} else {
		fl_show_form(dialog_->form_citation,
			     FL_PLACE_MOUSE | FL_FREE_SIZE,
			     FL_TRANSIENT,
			     _("Citation"));
		u_ = d_->updateBufferDependent.
		         connect(slot(this, &FormCitation::update));
		h_ = d_->hideBufferDependent.
		         connect(slot(this, &FormCitation::hide));
	}
}


void FormCitation::update()
{
	bibkeys.clear();
	bibkeysInfo.clear();

	vector<pair<string,string> > blist =
		lv_->buffer()->getBibkeyList();

	for( unsigned int i = 0; i < blist.size(); ++i ) {
		bibkeys.push_back(blist[i].first);
		bibkeysInfo.push_back(blist[i].second);
	}

	blist.clear();

	updateBrowser( dialog_->bibBrsr, bibkeys );
	updateBrowser( dialog_->citeBrsr, citekeys );
	fl_clear_browser( dialog_->infoBrsr );

	// No keys have been selected yet, so...
	setBibButtons( OFF );
	setCiteButtons( OFF );

	int noKeys = max( bibkeys.size(), citekeys.size() );

	// Place bounds, so that 4 <= noKeys <= 15
	noKeys = max( 4, min(15, noKeys) );

	// Re-size the form to accommodate the new browser size
	int size = 20 * noKeys;
	bool bibPresent = ( bibkeys.size() > 0 );
	setSize( size, bibPresent );

	fl_set_input( dialog_->textAftr, textAfter.c_str() );
}


void FormCitation::updateCitekeys( string const & keysIn )
{
	citekeys.clear();

	string tmp;
	string keys = keysIn;
	keys = frontStrip( split(keys, tmp, ',') );
	while( !tmp.empty() ) {
		citekeys.push_back( tmp );
		keys = frontStrip( split(keys, tmp, ',') );
	}
}


void FormCitation::updateBrowser( FL_OBJECT * browser,
				  vector<string> const & keys ) const
{
	fl_clear_browser( browser );

	fl_freeze_form( browser->form );
	for( unsigned int i = 0; i < keys.size(); ++i )
		fl_add_browser_line( browser, keys[i].c_str() );
	fl_unfreeze_form( browser->form );
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
	switch( status ) {
	case ON:
        {
		fl_activate_object( dialog_->delBtn );
		fl_set_object_lcol( dialog_->delBtn, FL_BLACK );

		int sel = fl_get_browser( dialog_->citeBrsr );

		if( sel != 1 ) {
			fl_activate_object( dialog_->upBtn );
			fl_set_object_lcol( dialog_->upBtn, FL_BLACK );
		} else {
			fl_deactivate_object( dialog_->upBtn );
			fl_set_object_lcol( dialog_->upBtn, FL_INACTIVE );
		}

		if( sel != fl_get_browser_maxline(dialog_->citeBrsr)) {
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


void FormCitation::setSize( int brsrHeight, bool bibPresent ) const
{
	int const infoHeight  = 110;
	int const otherHeight = 140;
	brsrHeight = max( brsrHeight, 175 );
	int formHeight = brsrHeight + otherHeight;

	if( bibPresent ) formHeight += infoHeight + 30;
	fl_set_form_size( dialog_->form_citation, 430, formHeight );

	// No resizing is alowed in the y-direction
	fl_set_form_minsize( dialog_->form_citation, 430, formHeight );
	fl_set_form_maxsize( dialog_->form_citation, 1000, formHeight );

	int ypos = 0;
	fl_set_object_geometry( dialog_->box,      0,   ypos, 430, formHeight );
	ypos += 30;
	fl_set_object_geometry( dialog_->citeBrsr, 10,  ypos, 180, brsrHeight );
	fl_set_object_geometry( dialog_->bibBrsr,  240, ypos, 180, brsrHeight );
	fl_set_object_geometry( dialog_->addBtn,   200, ypos,  30, 30 );
	ypos += 35;
	fl_set_object_geometry( dialog_->delBtn,   200, ypos,  30, 30 );
	ypos += 35;
	fl_set_object_geometry( dialog_->upBtn,    200, ypos,  30, 30 );
	ypos += 35;
	fl_set_object_geometry( dialog_->downBtn,  200, ypos,  30, 30 );

	ypos = brsrHeight+30; // base of Citation/Bibliography browsers

	// awaiting natbib support
	fl_hide_object( dialog_->style );

	if( bibPresent ) {
		ypos += 30;
		fl_set_object_geometry( dialog_->infoBrsr, 10, ypos, 410, infoHeight );
		fl_show_object( dialog_->infoBrsr );
		ypos += infoHeight;
	}
	else
		fl_hide_object( dialog_->infoBrsr );

	ypos += 20;
	// awaiting natbib support
	fl_hide_object( dialog_->textBefore );

	fl_set_object_geometry( dialog_->textAftr, 100, ypos,   250, 30 );
	fl_set_object_geometry( dialog_->ok,       230, ypos+50, 90, 30 );
	fl_set_object_geometry( dialog_->cancel,   330, ypos+50, 90, 30 );
}


void FormCitation::input( State cb )
{
	switch( cb ) {
	case BIBBRSR:
	{
		fl_deselect_browser( dialog_->citeBrsr );
		
		unsigned int sel = fl_get_browser( dialog_->bibBrsr );
		if( sel < 1 || sel > bibkeys.size() ) break;

		// Put into infoBrsr the additional info associated with
		// the selected bibBrsr key
		fl_clear_browser( dialog_->infoBrsr );
		fl_add_browser_line( dialog_->infoBrsr,
				     bibkeysInfo[sel-1].c_str() );

		// Highlight the selected bibBrsr key in citeBrsr if present
		vector<string>::iterator it =
			find( citekeys.begin(), citekeys.end(), bibkeys[sel-1] );

		if( it != citekeys.end() ) {
			int n = it - citekeys.begin();
			fl_select_browser_line( dialog_->citeBrsr, n+1 );
			fl_set_browser_topline( dialog_->citeBrsr, n+1 );
		}

		if( !lv_->buffer()->isReadonly() ) {
			if( it != citekeys.end() ) {
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
		if( sel < 1 || sel > citekeys.size() ) break;

		if( !lv_->buffer()->isReadonly() ) {
			setBibButtons( OFF );
			setCiteButtons( ON );
		}

		// Highlight the selected citeBrsr key in bibBrsr
		vector<string>::iterator it =
			find( bibkeys.begin(), bibkeys.end(), citekeys[sel-1] );

		if (it != bibkeys.end()) {
			int n = it - bibkeys.begin();
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
		if( lv_->buffer()->isReadonly() ) break;

		unsigned int sel = fl_get_browser( dialog_->bibBrsr );
		if( sel < 1 || sel > bibkeys.size() ) break;

		// Add the selected bibBrsr key to citeBrsr
		fl_addto_browser( dialog_->citeBrsr,
				  bibkeys[sel-1].c_str() );
		citekeys.push_back( bibkeys[sel-1] );

		int n = citekeys.size();
		fl_select_browser_line( dialog_->citeBrsr, n );

		setBibButtons( OFF );
		setCiteButtons( ON );
	}
	break;
	case DELETE:
	{
		if( lv_->buffer()->isReadonly() ) break;

		unsigned int sel = fl_get_browser( dialog_->citeBrsr );
		if( sel < 1 || sel > citekeys.size() ) break;

		// Remove the selected key from citeBrsr
		fl_delete_browser_line( dialog_->citeBrsr, sel ) ;
		citekeys.erase( citekeys.begin() + sel-1 );

		setBibButtons( ON );
		setCiteButtons( OFF );
	}
	break;
	case UP:
	{
		if( lv_->buffer()->isReadonly() ) break;

		unsigned int sel = fl_get_browser( dialog_->citeBrsr );
		if( sel < 2 || sel > citekeys.size() ) break;

		// Move the selected key up one line
		vector<string>::iterator it = citekeys.begin() + sel-1;
		string tmp = *it;

		fl_delete_browser_line( dialog_->citeBrsr, sel );
		citekeys.erase( it );

		fl_insert_browser_line( dialog_->citeBrsr, sel-1, tmp.c_str() );
		fl_select_browser_line( dialog_->citeBrsr, sel-1 );
		citekeys.insert( it-1, tmp );
		setCiteButtons( ON );
	}
	break;
	case DOWN:
	{
		if( lv_->buffer()->isReadonly() ) break;

		unsigned int sel = fl_get_browser( dialog_->citeBrsr );
		if( sel < 1 || sel > citekeys.size()-1 ) break;

		// Move the selected key down one line
		vector<string>::iterator it = citekeys.begin() + sel-1;
		string tmp = *it;

		fl_delete_browser_line( dialog_->citeBrsr, sel );
		citekeys.erase( it );

		fl_insert_browser_line( dialog_->citeBrsr, sel+1, tmp.c_str() );
		fl_select_browser_line( dialog_->citeBrsr, sel+1 );
		citekeys.insert( it+1, tmp );
		setCiteButtons( ON );
	}
	break;
	default:
		break;
	}
}


void FormCitation::apply()
{
	if( lv_->buffer()->isReadonly() ) return;

	string tmp;
	for( unsigned int i = 0; i < citekeys.size(); ++i ) {
		if (i > 0) tmp += ", ";
		tmp += citekeys[i];
	}

	textAfter = fl_get_input(dialog_->textAftr);

	if( inset_ != 0 )
	{
		inset_->setContents( tmp );
		inset_->setOptions( textAfter );
		lv_->view()->updateInset( inset_, true );
	} else {
		string arg = tmp + '|' + textAfter;
		lv_->getLyXFunc()->Dispatch( LFUN_INSERT_CITATION, arg.c_str() );
	}
}


void FormCitation::hide()
{
	if (dialog_
	    && dialog_->form_citation
	    && dialog_->form_citation->visible) {
		fl_hide_form(dialog_->form_citation);
		u_.disconnect();
		h_.disconnect();
	}

	// free up the dialog for another inset
	inset_ = 0;
	dialogIsOpen = false;
}


void FormCitation::free()
{
	// we don't need to delete u and h here because
	// hide() does that after disconnecting.
	if (dialog_) {
		if (dialog_->form_citation
		    && dialog_->form_citation->visible) {
			hide();
		}
		fl_free_form(dialog_->form_citation);
		delete dialog_;
		dialog_ = 0;
	}
}


int FormCitation::WMHideCB(FL_FORM * form, void *)
{
	// Ensure that the signals (u and h) are disconnected even if the
	// window manager is used to close the dialog.
	FormCitation * pre = static_cast<FormCitation*>(form->u_vdata);
	pre->hide();
	return FL_CANCEL;
}


void FormCitation::OKCB(FL_OBJECT * ob, long)
{
	FormCitation * pre = static_cast<FormCitation*>(ob->form->u_vdata);
	pre->apply();
	pre->hide();
}


void FormCitation::CancelCB(FL_OBJECT * ob, long)
{
	FormCitation * pre = static_cast<FormCitation*>(ob->form->u_vdata);
	pre->hide();
}


void FormCitation::InputCB(FL_OBJECT * ob, long data)
{
	FormCitation * pre = static_cast<FormCitation*>(ob->form->u_vdata);
	pre->input( static_cast<FormCitation::State>(data) );
}
