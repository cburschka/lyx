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

using std::find;
using std::max;
using std::sort;
using std::vector;

FormRef::FormRef(LyXView * lv, Dialogs * d)
	: FormCommand(lv, d, _("Reference"), new NoRepeatedApplyPolicy),
	  toggle(GOBACK), dialog_(0)
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
	if (dialog_) return dialog_->form;
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

	// Workaround dumb xforms sizing bug
	minw_ = form()->w;
	minh_ = form()->h;

	// Force the user to use the browser to change refs.
	fl_deactivate_object(dialog_->ref);

        // Manage the ok and cancel/close buttons
	bc_.setOK(dialog_->button_ok);
	bc_.setApply(dialog_->button_apply);
	bc_.setCancel(dialog_->button_cancel);
	bc_.setUndoAll(dialog_->button_restore);
	bc_.refresh();

	bc_.addReadOnly(dialog_->type);
	bc_.addReadOnly(dialog_->name);
}


void FormRef::update()
{
	fl_set_input(dialog_->ref,  params.getContents().c_str());
	fl_set_input(dialog_->name, params.getOptions().c_str());

	if (inset_)
		fl_set_choice(dialog_->type, getType()+1);

	toggle = GOBACK;
	fl_set_object_label(dialog_->button_go, _("Goto reference"));

	// Name is irrelevant to LaTeX documents
	if (lv_->buffer()->isLatex()) {
		fl_deactivate_object(dialog_->name);
		fl_set_object_lcol(dialog_->name, FL_INACTIVE);
	}

	refs = lv_->buffer()->getLabelList();
	updateBrowser(refs);

	bc_.readOnly(lv_->buffer()->isReadonly());
}


void FormRef::updateBrowser(vector<string> const & akeys) const
{
	vector<string> keys(akeys);
	if (fl_get_button(dialog_->sort))
		sort(keys.begin(), keys.end());

	fl_clear_browser(dialog_->browser);
	for (vector<string>::const_iterator it = keys.begin();
	     it != keys.end(); ++it)
		fl_add_browser_line(dialog_->browser, (*it).c_str());

	if (keys.empty()) {
		fl_add_browser_line(dialog_->browser,
				    _("*** No labels found in document ***"));

		fl_deactivate_object(dialog_->browser);
		fl_deactivate_object(dialog_->button_update);
		fl_deactivate_object(dialog_->sort);
		fl_set_object_lcol(dialog_->browser, FL_INACTIVE);
		fl_set_object_lcol(dialog_->button_update, FL_INACTIVE);
		fl_set_object_lcol(dialog_->sort, FL_INACTIVE);
	} else {
		fl_set_browser_topline(dialog_->browser, 1);
		fl_activate_object(dialog_->browser);
		fl_set_object_lcol(dialog_->browser, FL_BLACK);
		fl_activate_object(dialog_->button_update);
		fl_set_object_lcol(dialog_->button_update, FL_BLACK);
		fl_activate_object(dialog_->sort);
		fl_set_object_lcol(dialog_->sort, FL_BLACK);

		string ref = fl_get_input(dialog_->ref);
		vector<string>::const_iterator cit =
			find(keys.begin(), keys.end(), ref);

		if (cit != keys.end()) {
			int const i = static_cast<int>(cit - keys.begin());
			fl_set_browser_topline(dialog_->browser, max(i-5, 1));
			fl_select_browser_line(dialog_->browser, i+1);
		}
	}
}


void FormRef::apply()
{
  	if (!lv_->view()->available())
		return;

	Type const type = static_cast<Type>(fl_get_choice(dialog_->type) - 1);
	params.setCmdName(getName(type));

	params.setOptions(fl_get_input(dialog_->name));
	params.setContents(fl_get_input(dialog_->ref));

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


bool FormRef::input(FL_OBJECT *, long data)
{
	bool activate(true);
	switch (data) {
	// goto reference / go back
	case 1:
	{
		// No change to data
		activate = false;
		
		toggle = static_cast<Goto>(toggle + 1);
		if (toggle == GOFIRST ) toggle = GOREF;
	
		switch (toggle) {
		case GOREF:
		{
			lv_->getLyXFunc()->
				Dispatch(LFUN_REF_GOTO,
					 fl_get_input(dialog_->ref));
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
		unsigned int sel = fl_get_browser(dialog_->browser);
		if (sel < 1 || sel > refs.size()) break;

		if (!lv_->buffer()->isReadonly()) {
			string s = fl_get_browser_line(dialog_->browser, sel);
			fl_set_input(dialog_->ref, s.c_str());
		}

		toggle = GOBACK;
		lv_->getLyXFunc()->Dispatch(LFUN_REF_BACK);
		fl_set_object_label(dialog_->button_go, _("Goto reference"));

		fl_activate_object(dialog_->type);
		fl_set_object_lcol(dialog_->type, FL_BLACK);
		fl_activate_object(dialog_->button_go);
		fl_set_object_lcol(dialog_->button_go, FL_BLACK);
		fl_set_object_lcol(dialog_->ref, FL_BLACK);
	}
	break;

	// update or sort
	case 3:
		refs = lv_->buffer()->getLabelList();

		// fall through to...
	case 4:
		fl_freeze_form(form());
		updateBrowser(refs);
		fl_unfreeze_form(form());
		break;

	// changed reference type
	case 5:
	{
		Type type = static_cast<Type>( 
			fl_get_choice(dialog_->type) - 1);
		if (params.getCmdName() == getName(type)
		    && inset_) {
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

	if (params.getCmdName() == "ref" )
		type = REF;

	else if (params.getCmdName() == "pageref" )
		type = PAGEREF;

	else if (params.getCmdName() == "vref" )
		type = VREF;

	else if (params.getCmdName() == "vpageref" )
		type = VPAGEREF;

	else
		type = PRETTYREF;
	
	return type;
}


string const FormRef::getName(Type type) const
{
	string name;

	switch (type) {
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
