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
#include "xforms_helpers.h"
#include "version.h"
#include "support/filetools.h"
#include "lyxrc.h"

/* FIXME: Really, we shouldn't leave Splash hanging around, but I'm not sure
 * how to make it self-destructive - jbl
 */

extern "C" void C_FormSplashTimerCB(FL_OBJECT * ob, long)
{
        FormSplash::CloseCB(ob);
}

extern "C" int C_FormSplashWMHideCB(FL_FORM * ob, void * d)
{
	return FormBase::WMHideCB(ob, d);
}

FormSplash::FormSplash(LyXView * lv, Dialogs * d)
	: FormBaseBI(lv, d, (string(_("LyX ")) + LYX_VERSION).c_str(), new IgnorantPolicy),
	  dialog_(0)
{
	d->showSplash.connect(slot(this, &FormSplash::show));
	d->hideSplash.connect(slot(this, &FormSplash::hide));
}


FormSplash::~FormSplash()
{
	delete dialog_;
}


void FormSplash::CloseCB(FL_OBJECT * ob)
{
	FormSplash * pre = static_cast<FormSplash*>(ob->form->u_vdata);
	pre->hide();
	delete pre->dialog_;
	pre->dialog_ = 0;
}


void FormSplash::show()
{
	if (!dialog_) {
		build();
		fl_set_form_atclose(dialog_->form, C_FormSplashWMHideCB, 0);
	}

	int const xpos = WidthOfScreen(ScreenOfDisplay(fl_get_display(), fl_screen));
	int const ypos = HeightOfScreen(ScreenOfDisplay(fl_get_display(), fl_screen));

	fl_set_form_position(dialog_->form, xpos, ypos);

	// Show the title form at most 5 secs
	fl_set_timer(dialog_->splash_timer, 5);

	if (form()->visible)
		fl_raise_form(form());
	else {
		connect();
		fl_show_form(dialog_->form, FL_PLACE_CENTER, FL_NOBORDER, title.c_str());
	}
}


FL_FORM * FormSplash::form() const
{
	if (dialog_)
		return dialog_->form;
	return 0;
}


void FormSplash::build()
{
	dialog_ = build_splash();

	// Workaround dumb xforms sizing bug
	minw_ = form()->w;
	minh_ = form()->h;

	string banner_file = LibFileSearch("images", "banner", "xpm");

	if (lyxrc.show_banner && !banner_file.empty()) {
		fl_set_form_dblbuffer(dialog_->form, 1); // use dbl buffer
		fl_addto_form(dialog_->form);
		FL_OBJECT * obj = fl_add_pixmapbutton(FL_NORMAL_BUTTON, 0, 0, 425, 290, "");
		fl_set_pixmapbutton_file(obj, banner_file.c_str());
		
		fl_set_pixmapbutton_focus_outline(obj, 3);
		fl_set_button_shortcut(obj, "^M ^[", 1);
		fl_set_object_boxtype(obj, FL_NO_BOX);
		fl_set_object_callback(obj, C_FormSplashTimerCB, 0);
		
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
}
