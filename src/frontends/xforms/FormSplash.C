/**
 * \file FormSplash.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Allan Rae
 * \author John Levon
 */

#include <config.h>

#include FORMS_H_LOCATION
 
#ifdef __GNUG__
#pragma implementation
#endif

#include "Dialogs.h"
#include "LyXView.h"
#include "form_splash.h"
#include "FormSplash.h"
#include "version.h"
#include "support/filetools.h"
#include "lyxrc.h"

 
extern "C" int C_FormSplashCloseCB(FL_FORM * forms, void *)
{
	Assert(forms);
	FormSplash * form = static_cast<FormSplash*>(forms->u_vdata);
	form->hide();
	return 0;
}


extern "C" void C_FormSplashCB(FL_OBJECT * ob, long)
{
	FormSplash * form = static_cast<FormSplash*>(ob->form->u_vdata);
	form->hide();
}

 
FormSplash::FormSplash(LyXView *, Dialogs * d)
	: d_(d)
{
	c_ = d->showSplash.connect(slot(this, &FormSplash::show));
}


void FormSplash::show()
{
	if (!lyxrc.show_banner)
		return;

	if (!dialog_.get()) {
		build();
		fl_set_form_atclose(dialog_->form, C_FormSplashCloseCB, 0);
	}

	int const xpos = WidthOfScreen(ScreenOfDisplay(fl_get_display(), fl_screen));
	int const ypos = HeightOfScreen(ScreenOfDisplay(fl_get_display(), fl_screen));

	fl_set_form_position(dialog_->form, xpos, ypos);

	// Show the title form at most 5 secs
	fl_set_timer(dialog_->splash_timer, 5);

	if (dialog_->form->visible)
		fl_raise_form(dialog_->form);
	else
		fl_show_form(dialog_->form, FL_PLACE_CENTER, FL_NOBORDER, "");
}


void FormSplash::hide()
{
	c_.disconnect();
	if (dialog_->form && dialog_->form->visible)
		fl_hide_form(dialog_->form);
	d_->destroySplash();
}

 
void FormSplash::build()
{
	string banner_file = LibFileSearch("images", "banner", "xpm");
	if (banner_file.empty())
		return;
 
	dialog_.reset(build_splash());

	// Workaround dumb xforms sizing bug
	fl_set_form_minsize(dialog_->form, dialog_->form->w, dialog_->form->h);
 
	fl_set_form_dblbuffer(dialog_->form, 1); // use dbl buffer
	fl_addto_form(dialog_->form);
	FL_OBJECT * obj = fl_add_pixmapbutton(FL_NORMAL_BUTTON, 0, 0, 425, 290, "");
	fl_set_pixmapbutton_file(obj, banner_file.c_str());
	
	fl_set_pixmapbutton_focus_outline(obj, 3);
	fl_set_button_shortcut(obj, "^M ^[", 1);
	fl_set_object_boxtype(obj, FL_NO_BOX);
	fl_set_object_callback(obj, C_FormSplashCB, 0);
	
	obj = fl_add_text(FL_NORMAL_TEXT, 248, 265, 170, 16, LYX_VERSION);
	fl_set_object_lsize(obj, FL_NORMAL_SIZE);
	fl_mapcolor(FL_FREE_COL2, 0x05, 0x2e, 0x4c);
	fl_mapcolor(FL_FREE_COL3, 0xe1, 0xd2, 0x9b);
	fl_set_object_color(obj, FL_FREE_COL2, FL_FREE_COL2);
	fl_set_object_lcol(obj, FL_FREE_COL3);
	fl_set_object_lalign(obj, FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
	fl_set_object_lstyle(obj, FL_BOLD_STYLE);
	fl_end_form();
}
