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

#include "ControlSplash.h"
#include "FormSplash.h"
#include "form_splash.h"
#include "form_splash.h"
#include "support/LAssert.h"

extern "C" {
	
	static
	int C_FormSplashCloseCB(FL_FORM * form, void *)
	{
		lyx::Assert(form && form->u_vdata);
		FormSplash * pre = static_cast<FormSplash *>(form->u_vdata);
		pre->Hide();
		return FL_CANCEL;
	}
	
	void C_FormSplashCB(FL_OBJECT * ob, long)
	{
		lyx::Assert(ob && ob->form && ob->form->u_vdata);
		FormSplash * pre = static_cast<FormSplash*>(ob->form->u_vdata);
		pre->Hide();
	}
	
}

 
FormSplash::FormSplash(ControlSplash & c)
	: ViewSplash(c)
{}


ControlSplash & FormSplash::controller() const
{
	return static_cast<ControlSplash &>(controller_);
	//return dynamic_cast<ControlSplash &>(controller_);
}


void FormSplash::show()
{
	if (!dialog_.get()) {
		build();
		fl_set_form_atclose(dialog_->form, C_FormSplashCloseCB, 0);
	}

	int const xpos = WidthOfScreen(ScreenOfDisplay(fl_get_display(),
						       fl_screen));
	int const ypos = HeightOfScreen(ScreenOfDisplay(fl_get_display(),
							fl_screen));

	fl_set_form_position(dialog_->form, xpos, ypos);

	// Show the title form at most 5 secs
	fl_set_timer(dialog_->splash_timer, 5);

	if (dialog_->form->visible)
		fl_raise_form(dialog_->form);
	else
		// Workaround dumb xforms sizing bug
		fl_set_form_minsize(dialog_->form,
				    dialog_->form->w,
				    dialog_->form->h);
		fl_show_form(dialog_->form, FL_PLACE_CENTER, FL_NOBORDER, "");
}


void FormSplash::hide()
{
	if (dialog_->form && dialog_->form->visible)
		fl_hide_form(dialog_->form);
}

 
void FormSplash::build()
{
	dialog_.reset(build_splash());

	fl_set_form_dblbuffer(dialog_->form, 1); // use dbl buffer
	fl_addto_form(dialog_->form);
	FL_OBJECT * obj = fl_add_pixmapbutton(FL_NORMAL_BUTTON, 0, 0, 425, 290, "");
	fl_set_pixmapbutton_file(obj, controller().bannerFile().c_str());
	
	fl_set_pixmapbutton_focus_outline(obj, 3);
	fl_set_button_shortcut(obj, "^M ^[", 1);
	fl_set_object_boxtype(obj, FL_NO_BOX);
	fl_set_object_callback(obj, C_FormSplashCB, 0);
	
	obj = fl_add_text(FL_NORMAL_TEXT, 248, 265, 170, 16,
			  controller().LyXVersion().c_str());
	fl_set_object_lsize(obj, FL_NORMAL_SIZE);
	fl_mapcolor(FL_FREE_COL2, 0x05, 0x2e, 0x4c);
	fl_mapcolor(FL_FREE_COL3, 0xe1, 0xd2, 0x9b);
	fl_set_object_color(obj, FL_FREE_COL2, FL_FREE_COL2);
	fl_set_object_lcol(obj, FL_FREE_COL3);
	fl_set_object_lalign(obj, FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
	fl_set_object_lstyle(obj, FL_BOLD_STYLE);
	fl_end_form();
}
