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

#include FORMS_H_LOCATION

#include <algorithm>

#ifdef __GNUG__
#pragma implementation
#endif


#include "gettext.h"
#include "Dialogs.h"
#include "FormCitation.h"
#include "LyXView.h"
#include "buffer.h"
#include "form_citation.h"
#include "lyxfunc.h"
#include "support/filetools.h"

using std::vector;
using std::pair;
using std::max;
using std::min;
using std::find;

static vector<string> citekeys;
static vector<string> bibkeys;
static vector<string> bibkeysInfo;

FormCitation::FormCitation(LyXView * lv, Dialogs * d)
	: FormCommand(lv, d, _("Citation")), dialog_(0)
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
	delete dialog_;
}


void FormCitation::clearStore()
{
	citekeys.clear();
	bibkeys.clear();
	bibkeysInfo.clear();
}


void FormCitation::build()
{
	dialog_ = build_citation();
}


FL_FORM * const FormCitation::form() const
{
	if( dialog_ && dialog_->form_citation )
		return dialog_->form_citation;
	else
		return 0;
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

	citekeys.clear();
	string tmp, keys( params.getContents() );
	keys = frontStrip( split(keys, tmp, ',') );
	while( !tmp.empty() ) {
		citekeys.push_back( tmp );
		keys = frontStrip( split(keys, tmp, ',') );
	}

	fl_freeze_form( dialog_->form_citation );

	updateBrowser( dialog_->bibBrsr,  bibkeys );
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

	fl_set_input( dialog_->textAftr, params.getOptions().c_str() );

	fl_unfreeze_form( dialog_->form_citation );
}


void FormCitation::updateBrowser( FL_OBJECT * browser,
				  vector<string> const & keys ) const
{
	fl_clear_browser( browser );

	for( unsigned int i = 0; i < keys.size(); ++i )
		fl_add_browser_line( browser, keys[i].c_str() );
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


void FormCitation::setSize( int hbrsr, bool bibPresent ) const
{
	int const hinfo  = dialog_->infoBrsr->h;
	int const hother = 140;
	hbrsr = max( hbrsr, 175 );
	int wform = dialog_->form_citation->w;
	int hform = hbrsr + hother;

	if( bibPresent ) hform += hinfo + 30;
	fl_set_form_size( dialog_->form_citation, wform, hform );

	// No resizing is alowed in the y-direction
	fl_set_form_minsize( dialog_->form_citation, wform,   hform );
	fl_set_form_maxsize( dialog_->form_citation, 3*wform, hform );

	int y = 0;
	fl_set_object_geometry( dialog_->box, 0, y, wform, hform );
	y += 30;
	fl_set_object_geometry( dialog_->citeBrsr, 10, y, 180, hbrsr );
	fl_set_object_geometry( dialog_->bibBrsr, 240, y, 180, hbrsr );

	fl_set_object_position( dialog_->addBtn,  200, y );
	y += 5 + dialog_->addBtn->h;
	fl_set_object_position( dialog_->delBtn,  200, y );
	y += 5 + dialog_->delBtn->h;
	fl_set_object_position( dialog_->upBtn,   200, y );
	y += 5 + dialog_->upBtn->h;
	fl_set_object_position( dialog_->downBtn, 200, y );

	y = dialog_->bibBrsr->y + dialog_->bibBrsr->h;

	// awaiting natbib support
	fl_hide_object( dialog_->style );

	if( bibPresent ) {
		y += 30;
		fl_set_object_position( dialog_->infoBrsr, 10, y );
		fl_show_object( dialog_->infoBrsr );
		y += hinfo;
	}
	else
		fl_hide_object( dialog_->infoBrsr );

	y += 20;
	// awaiting natbib support
	fl_hide_object( dialog_->textBefore );

	fl_set_object_position( dialog_->textAftr, 100, y );
	fl_set_object_position( dialog_->ok,       230, y+50 );
	fl_set_object_position( dialog_->cancel,   330, y+50 );
}


void FormCitation::input( long data )
{
	State cb = static_cast<FormCitation::State>( data );

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

	string contents;
	for( unsigned int i = 0; i < citekeys.size(); ++i ) {
		if (i > 0) contents += ", ";
		contents += citekeys[i];
	}

	params.setContents( contents );
	params.setOptions( fl_get_input(dialog_->textAftr) );

	if( inset_ != 0 )
	{
		// Only update if contents have changed
		if( params.getCmdName()  != inset_->getCmdName()  ||
		    params.getContents() != inset_->getContents() ||
		    params.getOptions()  != inset_->getOptions() ) {
			inset_->setParams( params );
			lv_->view()->updateInset( inset_, true );
		}
	} else {
		lv_->getLyXFunc()->Dispatch( LFUN_CITATION_INSERT,
					     params.getAsString().c_str() );
	}
}
