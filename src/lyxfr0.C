/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich,
 *          Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "lyxfr0.h"
#include "lyxfr1.h"
#include "lyx_gui_misc.h"
#include "frontends/Dialogs.h" // redrawGUI

#ifdef SIGC_CXX_NAMESPACES
using SigC::slot;
#endif

// callbacks for form form_search
void SearchCancelCB(FL_OBJECT * ob, long)
{
	fl_hide_form(ob->form);
}


void SearchForwardCB(FL_OBJECT * ob, long)
{
	LyXFindReplace * lfr = static_cast<LyXFindReplace*>(ob->form->u_vdata);
	lfr->SearchCB(true);
}


void SearchBackwardCB(FL_OBJECT * ob, long)
{
	LyXFindReplace * lfr = static_cast<LyXFindReplace*>(ob->form->u_vdata);
	lfr->SearchCB(false);
}


void SearchReplaceAllCB(FL_OBJECT * ob, long)
{
	LyXFindReplace * lfr = static_cast<LyXFindReplace*>(ob->form->u_vdata);
	lfr->SearchReplaceAllCB();
}


void SearchReplaceCB(FL_OBJECT * ob, long)
{
	LyXFindReplace * lfr = static_cast<LyXFindReplace*>(ob->form->u_vdata);
	lfr->SearchReplaceCB();
}



SearchForm::SearchForm()
	: search_form(0)
{
	r_ = Dialogs::redrawGUI.connect(slot(this, &SearchForm::redraw));
}


SearchForm::~SearchForm()
{
	// The search_form should be closed and freed when SearchForm
	// is destructed.
	r_.disconnect();
}


void SearchForm::redraw()
{
	if (search_form && search_form->form_search->visible)
		fl_redraw_form(search_form->form_search);
}


void SearchForm::SearchCancelCB()
{
	fl_hide_form(search_form->form_search);
}


void SearchForm::StartSearch(LyXFindReplace * lfr)
{
	static int ow = -1, oh;

	if (!search_form) {
		search_form = create_form_form_search();
		fl_set_form_atclose(search_form->form_search,
				    CancelCloseBoxCB, 0);
	}
	// Set the u_vdata
	search_form->form_search->u_vdata = lfr;
		
	
	if (search_form->form_search->visible) {
		fl_raise_form(search_form->form_search);
	} else {
		fl_show_form(search_form->form_search,
			     FL_PLACE_MOUSE | FL_FREE_SIZE, FL_TRANSIENT,
			     _("Find & Replace"));	// RVDK_PATCH_5
		if (ow < 0) {
			ow = search_form->form_search->w;
			oh = search_form->form_search->h;
		}
		fl_set_form_minsize(search_form->form_search, ow, oh);
	}
}


// Returns the value of the replace string in the form
string const SearchForm::ReplaceString() const
{
	return fl_get_input(search_form->input_replace);
}


void SearchForm::replaceEnabled(bool fEnable)
{
	fReplaceEnabled = fEnable;
	if (fEnable) {
		fl_activate_object(search_form->replace_button);
		fl_activate_object(search_form->replaceall_button);
		fl_activate_object(search_form->input_replace);
		fl_set_object_lcol(search_form->replace_button, FL_BLACK);
		fl_set_object_lcol(search_form->replaceall_button, FL_BLACK);
		fl_set_object_lcol(search_form->input_replace, FL_BLACK);
	} else {
		fl_deactivate_object(search_form->replace_button);
		fl_deactivate_object(search_form->replaceall_button);
		fl_deactivate_object(search_form->input_replace);
		fl_set_object_lcol(search_form->replace_button, FL_INACTIVE);
		fl_set_object_lcol(search_form->replaceall_button, FL_INACTIVE);
		fl_set_object_lcol(search_form->input_replace, FL_INACTIVE);
	}
}


void SearchForm::SetSearchString(string const & ls)
{
	fl_set_input(search_form->input_search, ls.c_str());	
}
