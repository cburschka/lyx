/* This file is part of
* ======================================================
* 
*           LyX, The Document Processor
* 	 
*	    Copyright (C) 1995 Matthias Ettrich,
*           Copyright (C) 1995-1998 The LyX Team.
*
*======================================================*/

#include <config.h>

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "LString.h"
#include "lyx_main.h"
#include FORMS_H_LOCATION
#include "form1.h"
#include "lyxfr0.h"
#include "lyxfr1.h"
#include "lyxfunc.h"
#include "lyxscreen.h"
#include "error.h"
#include "lyxtext.h"
#include "gettext.h"
#include "LyXView.h" // only because of form_main

//---------------------------------------------------------------
// I hate global variables, but the same search object must be used everywhere,
// and the form is also global, so... 
LyXFindReplace1	_FR;

// This one should be a protected member of LyXFindReplace1
// Form creation/destruction must also be done in LyXFindReplace1
extern FD_form_search *fd_form_search;

//---------------------------------------------------------------


// callbacks for form form_search
void SearchCancelCB(FL_OBJECT *, long)
{
	_FR.SearchCancelCB();
}


void SearchForwardCB(FL_OBJECT *, long)
{
	_FR.SearchCB(true);
}


void SearchBackwardCB(FL_OBJECT *, long)
{
	_FR.SearchCB(false);
}


void SearchReplaceAllCB(FL_OBJECT *, long)
{
	_FR.SearchReplaceAllCB();
}


void SearchReplaceCB(FL_OBJECT *, long)
{
	_FR.SearchReplaceCB();
}


//--------------------- LyXFindReplace0's implementation ------------

LyXFindReplace0::LyXFindReplace0()
{
    	fCaseSensitive = false;
	fMatchWord = false;
}


void LyXFindReplace0::StartSearch()
{
	FD_form_search *fd_fs = fd_form_search;

	if (fd_fs->form_search->visible) {
		fl_raise_form(fd_fs->form_search);
	} else {
		fl_show_form(fd_fs->form_search,
			     FL_PLACE_MOUSE | FL_FREE_SIZE, FL_FULLBORDER,
			     _("Find & Replace"));	// RVDK_PATCH_5
	}
	ReInitFromForm();
}


void LyXFindReplace0::ReInitFromForm()
{
	FD_form_search *fd_fs = fd_form_search;

	lsSearch = fl_get_input(fd_fs->input_search);
	fCaseSensitive = fl_get_button(fd_fs->btnCaseSensitive);
	fMatchWord = fl_get_button(fd_fs->btnMatchWord);
}


// Returns the value of the replace string in the form
LString const LyXFindReplace0::ReplaceString()
{
	return LString(fl_get_input(fd_form_search->input_replace));
}


void LyXFindReplace0::SearchCancelCB()
{
	fl_hide_form(fd_form_search->form_search);
}


void LyXFindReplace0::SetReplaceEnabled(bool fEnable)
{
	FD_form_search *fd_fs = fd_form_search;
	fReplaceEnabled = fEnable;
	if (fEnable) {
        fl_activate_object(fd_fs->replace_button);
        fl_activate_object(fd_fs->replaceall_button);
        fl_activate_object(fd_fs->input_replace);
		fl_set_object_lcol(fd_fs->replace_button, FL_BLACK);
		fl_set_object_lcol(fd_fs->replaceall_button, FL_BLACK);
		fl_set_object_lcol(fd_fs->input_replace, FL_BLACK);
	} else {
        fl_deactivate_object(fd_fs->replace_button);
        fl_deactivate_object(fd_fs->replaceall_button);
        fl_deactivate_object(fd_fs->input_replace);
		fl_set_object_lcol(fd_fs->replace_button,FL_INACTIVE);
		fl_set_object_lcol(fd_fs->replaceall_button,FL_INACTIVE);
		fl_set_object_lcol(fd_fs->input_replace, FL_INACTIVE);
	}
}


void LyXFindReplace0::SetSearchString(LString const &ls)
{
	lsSearch = ls;
	fl_set_input(fd_form_search->input_search, ls.c_str());	
}


//---------------------------------------------------------------
//HB??: Maybe _FR.StartSearch should be called in lyxfunc.C instead of MenuSearch() ?

void MenuSearch()
{
	_FR.StartSearch();   
}
