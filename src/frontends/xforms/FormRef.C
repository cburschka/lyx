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

#ifdef __GNUG__
#pragma implementation
#endif


#include "Dialogs.h"
#include "FormRef.h"
#include "LyXView.h"
#include "buffer.h"
#include "form_ref.h"
#include "lyxfunc.h"

#include <algorithm>

using std::sort;
using std::vector;

static int formw;
static int formh;

FormRef::FormRef(LyXView * lv, Dialogs * d)
	: FormCommand(lv, d, _("Reference")), toggle(GOBACK), dialog_(0)
{
	// let the dialog be shown
	// These are permanent connections so we won't bother
	// storing a copy because we won't be disconnecting.
	d->showRef.connect(slot(this, &FormRef::showInset));
	d->createRef.connect(slot(this, &FormRef::createInset));
}


FormRef::~FormRef()
{
	delete dialog_;
}


FL_FORM * FormRef::form() const
{
	if ( dialog_ ) return dialog_->form;
	return 0;
}


void FormRef::disconnect()
{
	refs.clear();
	FormCommand::disconnect();
}


void FormRef::build()
{
	dialog_ = build_ref();

	fl_addto_choice(dialog_->type,
			_(" Ref | Page | TextRef | TextPage | PrettyRef "));

	// XFORMS bug workaround
	// Define the min/max dimensions. Actually applied in update()
	formw = form()->w, formh = form()->h;

	// Name is irrelevant to LaTeX documents
	if ( lv_->buffer()->isLatex() ) {
		fl_deactivate_object( dialog_->name );
		fl_set_object_lcol( dialog_->name, FL_INACTIVE );
	}
	  
	// Can change reference only through browser
	fl_deactivate_object( dialog_->ref );

	bc_.setOK( dialog_->button_ok );
	bc_.setCancel( dialog_->button_cancel );
	bc_.addReadOnly( dialog_->type );
	bc_.refresh();
}


void FormRef::update(bool switched)
{
	if (switched) {
		hide();
		return;
	}

	fl_set_input(dialog_->ref,  params.getContents().c_str());
	fl_set_input(dialog_->name, params.getOptions().c_str());

	Type type = getType();
	fl_set_choice( dialog_->type, type+1 );

	toggle = GOBACK;
	fl_set_object_label(dialog_->button_go, _("Goto reference"));

	refs.clear();
	if ( inset_ == 0 ) {
		refs = lv_->buffer()->getLabelList();
		updateBrowser( refs );
		showBrowser();
	} else {
		hideBrowser();
	}
	bc_.readOnly( lv_->buffer()->isReadonly() );
}


void FormRef::updateBrowser( vector<string> keys ) const
{
	if ( fl_get_button( dialog_->sort ) )
		sort( keys.begin(), keys.end() );

	fl_clear_browser( dialog_->browser );
	for( vector<string>::const_iterator it = keys.begin();
	     it != keys.end(); ++it )
		fl_add_browser_line( dialog_->browser, (*it).c_str());

	if ( keys.empty() ) {
		fl_add_browser_line( dialog_->browser,
				     _("*** No labels found in document ***"));

		fl_deactivate_object( dialog_->browser );
		fl_deactivate_object( dialog_->button_update );
		fl_deactivate_object( dialog_->sort );
		fl_set_object_lcol( dialog_->browser, FL_INACTIVE );
		fl_set_object_lcol( dialog_->button_update, FL_INACTIVE );
		fl_set_object_lcol( dialog_->sort, FL_INACTIVE );
	} else {
		fl_set_browser_topline( dialog_->browser, 1 );
		fl_activate_object( dialog_->browser );
		fl_set_object_lcol( dialog_->browser, FL_BLACK );
		fl_activate_object( dialog_->button_update );
		fl_set_object_lcol( dialog_->button_update, FL_BLACK );
		fl_activate_object( dialog_->sort );
		fl_set_object_lcol( dialog_->sort, FL_BLACK );
	}
}


void FormRef::showBrowser() const
{
	fl_show_object( dialog_->browser );
	fl_show_object( dialog_->button_update );
	fl_show_object( dialog_->sort );

	setSize( formw, formh, 0 );

	fl_deactivate_object( dialog_->type );
	fl_set_object_lcol( dialog_->type, FL_INACTIVE );
	fl_deactivate_object( dialog_->button_go );
	fl_set_object_lcol( dialog_->button_go, FL_INACTIVE );
	fl_set_object_lcol( dialog_->ref, FL_INACTIVE );
	bc_.valid(false);
}


void FormRef::hideBrowser() const
{
	fl_hide_object( dialog_->browser );
	fl_hide_object( dialog_->button_update );
	fl_hide_object( dialog_->sort );

	setSize( 250, formh, 280 );

	fl_activate_object( dialog_->type );
	fl_set_object_lcol( dialog_->type, FL_BLACK );
	fl_activate_object( dialog_->button_go );
	fl_set_object_lcol( dialog_->button_go, FL_BLACK );
	fl_set_object_lcol( dialog_->ref, FL_BLACK );
	bc_.invalid();
}


void FormRef::setSize( int w, int h, int dx ) const
{
	static int x1 = dialog_->name->x;
	static int y1 = dialog_->name->y;
	static int x2 = dialog_->ref->x;
	static int y2 = dialog_->ref->y;
	static int x3 = dialog_->type->x;
	static int y3 = dialog_->type->y;
	static int x4 = dialog_->button_go->x;
	static int y4 = dialog_->button_go->y;
	static int x5 = dialog_->button_ok->x;
	static int y5 = dialog_->button_ok->y;
	static int x6 = dialog_->button_cancel->x;
	static int y6 = dialog_->button_cancel->y;

	if ( form()->w != w )
		fl_set_form_size( form(), w, h );

	fl_set_form_minsize( form(), w, h );
	fl_set_form_maxsize( form(), 2*w, h );

	if ( form()->w == w ) return;

	fl_set_object_position( dialog_->name,   x1-dx, y1 );
	fl_set_object_position( dialog_->ref,    x2-dx, y2 );
	fl_set_object_position( dialog_->type,   x3-dx, y3 );
	fl_set_object_position( dialog_->button_go,     x4-dx, y4 );
	fl_set_object_position( dialog_->button_ok,     x5-dx, y5 );
	fl_set_object_position( dialog_->button_cancel, x6-dx, y6 );

	// These two must be reset apparently
	// Name is irrelevant to LaTeX documents
	if ( lv_->buffer()->isLatex() ) {
		fl_deactivate_object( dialog_->name );
		fl_set_object_lcol( dialog_->name, FL_INACTIVE );
	}
	  
	// Can change reference only through browser
	fl_deactivate_object( dialog_->ref );
}


void FormRef::apply()
{
  	if (!lv_->view()->available())
		return;

	Type const type = static_cast<Type>(fl_get_choice(dialog_->type) - 1);
	params.setCmdName(getName(type));

	params.setOptions(fl_get_input(dialog_->name));

	if (inset_ != 0) {
		// Only update if contents have changed
		if (params != inset_->params()) {
			inset_->setParams(params);
			lv_->view()->updateInset(inset_, true);
		}
	} else {
		lv_->getLyXFunc()->Dispatch(LFUN_REF_INSERT,
					    params.getAsString());
	}
}


#ifdef WITH_WARNINGS
#warning check use of buttoncontroller
// Seems okay except that goref and goback shouldn't
// affect the status of ok.
#endif
bool FormRef::input( FL_OBJECT *, long data )
{
	bool activate( true );
	switch( data ) {
	// goto reference / go back
	case 1:
	{
		toggle = static_cast<Goto>(toggle + 1);
		if ( toggle == GOFIRST ) toggle = GOREF;
	
		switch (toggle) {
		case GOREF:
		{
			lv_->getLyXFunc()->
				Dispatch(LFUN_REF_GOTO,
					 params.getContents());
	  		fl_set_object_label(dialog_->button_go, _("Go back"));
		}
		break;

		case GOBACK:
		{
			lv_->getLyXFunc()->Dispatch(LFUN_REF_BACK);
			fl_set_object_label(dialog_->button_go,
					    _("Goto reference"));
		}
		break;

		default:
			break;
		}
	}
	break;

	// choose browser key
	case 2:
	{
		unsigned int sel = fl_get_browser( dialog_->browser );
		if ( sel < 1 || sel > refs.size() ) break;

		string s = fl_get_browser_line( dialog_->browser, sel );
		fl_set_input( dialog_->ref, s.c_str());
		params.setContents( s );

		toggle = GOBACK;
		lv_->getLyXFunc()->Dispatch(LFUN_REF_BACK);
		fl_set_object_label(dialog_->button_go, _("Goto reference"));

		fl_activate_object( dialog_->type );
		fl_set_object_lcol( dialog_->type, FL_BLACK );
		fl_activate_object( dialog_->button_go );
		fl_set_object_lcol( dialog_->button_go, FL_BLACK );
		fl_set_object_lcol( dialog_->ref, FL_BLACK );
	}
	break;

	// update or sort
	case 3:
	{
		fl_freeze_form( form() );
		updateBrowser( refs );
		fl_unfreeze_form( form() );
	}
	break;

	// changed reference type
	case 4:
	{
		Type type = static_cast<Type>( 
			fl_get_choice(dialog_->type) - 1 );
		if ( params.getCmdName() == getName( type )
		    && inset_ ) {
			activate = false;
		}
	}
	break;

	default:
		break;
	}
	return activate;
}


FormRef::Type FormRef::getType() const
{
	Type type;

	if ( params.getCmdName() == "ref" )
		type = REF;

	else if ( params.getCmdName() == "pageref" )
		type = PAGEREF;

	else if ( params.getCmdName() == "vref" )
		type = VREF;

	else if ( params.getCmdName() == "vpageref" )
		type = VPAGEREF;

	else
		type = PRETTYREF;
	
	return type;
}


string FormRef::getName( Type type ) const
{
	string name;

	switch( type ) {
	case REF:
		name = "ref";
		break;
	case PAGEREF:
		name = "pageref";
		break;
	case VREF:
		name = "vref";
		break;
	case VPAGEREF:
		name = "vpageref";
		break;
	case PRETTYREF:
		name = "prettyref";
		break;
	}
	
	return name;
}
