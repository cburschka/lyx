#include <config.h>

#include <cstdlib>

#include FORMS_H_LOCATION
#include "layout_forms.h"
#include "lyx_main.h"
#include "lyxrc.h"
#include "LString.h"
#include "support/filetools.h"
#include "buffer.h"
#include "minibuffer.h"
#include "vspace.h"
#include "lyx_gui_misc.h"
#include "BufferView.h"
#include "gettext.h"
#include "LyXView.h"

extern FD_form_paper * fd_form_paper;
extern BufferView * current_view;


inline
void DeactivatePaperButtons ()
{
	fl_deactivate_object (fd_form_paper->button_ok);
	fl_deactivate_object (fd_form_paper->button_apply);
	fl_set_object_lcol (fd_form_paper->button_ok, FL_INACTIVE);
	fl_set_object_lcol (fd_form_paper->button_apply, FL_INACTIVE);
}


inline
void ActivatePaperButtons ()
{
	fl_activate_object (fd_form_paper->button_ok);
	fl_activate_object (fd_form_paper->button_apply);
	fl_set_object_lcol (fd_form_paper->button_ok, FL_BLACK);
	fl_set_object_lcol (fd_form_paper->button_apply, FL_BLACK);
}


inline
void DisablePaperLayout()
{
        DeactivatePaperButtons();
        fl_deactivate_object (fd_form_paper->choice_paperpackage);
	fl_deactivate_object (fd_form_paper->greoup_radio_orientation);
	fl_deactivate_object (fd_form_paper->radio_portrait);
	fl_deactivate_object (fd_form_paper->radio_landscape);
	fl_deactivate_object (fd_form_paper->choice_papersize2);
	fl_deactivate_object (fd_form_paper->push_use_geometry);
	fl_deactivate_object (fd_form_paper->input_custom_width);
	fl_deactivate_object (fd_form_paper->input_custom_height);
	fl_deactivate_object (fd_form_paper->input_top_margin);
	fl_deactivate_object (fd_form_paper->input_bottom_margin);
	fl_deactivate_object (fd_form_paper->input_left_margin);
	fl_deactivate_object (fd_form_paper->input_right_margin);
	fl_deactivate_object (fd_form_paper->input_head_height);
	fl_deactivate_object (fd_form_paper->input_head_sep);
	fl_deactivate_object (fd_form_paper->input_foot_skip);
	fl_deactivate_object (fd_form_paper->text_warning);
}


inline
void EnablePaperLayout()
{
        ActivatePaperButtons();
        fl_activate_object (fd_form_paper->choice_paperpackage);
	fl_activate_object (fd_form_paper->greoup_radio_orientation);
	fl_activate_object (fd_form_paper->radio_portrait);
	fl_activate_object (fd_form_paper->radio_landscape);
	fl_activate_object (fd_form_paper->choice_papersize2);
	fl_activate_object (fd_form_paper->push_use_geometry);
	fl_activate_object (fd_form_paper->input_custom_width);
	fl_activate_object (fd_form_paper->input_custom_height);
	fl_activate_object (fd_form_paper->input_top_margin);
	fl_activate_object (fd_form_paper->input_bottom_margin);
	fl_activate_object (fd_form_paper->input_left_margin);
	fl_activate_object (fd_form_paper->input_right_margin);
	fl_activate_object (fd_form_paper->input_head_height);
	fl_activate_object (fd_form_paper->input_head_sep);
	fl_activate_object (fd_form_paper->input_foot_skip);
	fl_activate_object (fd_form_paper->text_warning);
}


static
void checkMarginValues()
{
	int allEmpty;
    
	fl_hide_object(fd_form_paper->text_warning);
	ActivatePaperButtons();

	allEmpty = (!strlen(fl_get_input(fd_form_paper->input_top_margin)) &&
		    !strlen(fl_get_input(fd_form_paper->input_bottom_margin)) &&
		    !strlen(fl_get_input(fd_form_paper->input_left_margin)) &&
		    !strlen(fl_get_input(fd_form_paper->input_right_margin)) &&
		    !strlen(fl_get_input(fd_form_paper->input_head_height)) &&
		    !strlen(fl_get_input(fd_form_paper->input_head_sep)) &&
		    !strlen(fl_get_input(fd_form_paper->input_foot_skip)) &&
		    !strlen(fl_get_input(fd_form_paper->input_custom_width)) &&
		    !strlen(fl_get_input(fd_form_paper->input_custom_height)));
	if (!allEmpty) {
		fl_set_button(fd_form_paper->push_use_geometry, 1);
	}
}


bool UpdateLayoutPaper()
{
	bool update = true;
	if (!current_view->available())
		update = false;

	if (update) {
		BufferParams * params = &current_view->buffer()->params;

		fl_set_choice(fd_form_paper->choice_papersize2,
			      params->papersize2 + 1);
		fl_set_choice(fd_form_paper->choice_paperpackage,
			      params->paperpackage + 1);
		fl_set_button(fd_form_paper->push_use_geometry,
			      params->use_geometry);
		fl_set_button(fd_form_paper->radio_portrait, 0);
		fl_set_button(fd_form_paper->radio_landscape, 0);
		if (params->orientation == BufferParams::ORIENTATION_LANDSCAPE)
			fl_set_button(fd_form_paper->radio_landscape, 1);
		else
			fl_set_button(fd_form_paper->radio_portrait, 1);
   
		fl_set_input(fd_form_paper->input_custom_width,
			     params->paperwidth.c_str());
		fl_set_input(fd_form_paper->input_custom_height,
			     params->paperheight.c_str());
		fl_set_input(fd_form_paper->input_left_margin,
			     params->leftmargin.c_str());
		fl_set_input(fd_form_paper->input_top_margin,
			     params->topmargin.c_str());
		fl_set_input(fd_form_paper->input_right_margin,
			     params->rightmargin.c_str());
		fl_set_input(fd_form_paper->input_bottom_margin,
			     params->bottommargin.c_str());
		fl_set_input(fd_form_paper->input_head_height,
			     params->headheight.c_str());
		fl_set_input(fd_form_paper->input_head_sep,
			     params->headsep.c_str());
		fl_set_input(fd_form_paper->input_foot_skip,
			     params->footskip.c_str());
		fl_set_focus_object(fd_form_paper->form_paper,
				    fd_form_paper->choice_papersize2);
		fl_hide_object(fd_form_paper->text_warning);
		if (current_view->buffer()->isReadonly()) {
			DisablePaperLayout();
			fl_set_object_label(fd_form_paper->text_warning,
					    _("Document is read-only."
					      "  No changes to layout"
					      " permitted."));
			fl_show_object(fd_form_paper->text_warning);
		} else {
			EnablePaperLayout();
		}
	} else if (fd_form_paper->form_paper->visible) {
		fl_hide_form(fd_form_paper->form_paper);
	}
	return update;
}


void MenuLayoutPaper()
{
	if (UpdateLayoutPaper()) {
		if (fd_form_paper->form_paper->visible) {
			fl_raise_form(fd_form_paper->form_paper);
		} else {
			fl_show_form(fd_form_paper->form_paper,
				     FL_PLACE_MOUSE, FL_FULLBORDER,
				     _("Paper Layout"));
		}
	}
}


void PaperApplyCB(FL_OBJECT *, long)
{
	if (!current_view->available()) 
		return;
	
	BufferParams * params = &current_view->buffer()->params;
	
	FD_form_paper const * fd = fd_form_paper;
    
	params->papersize2 = fl_get_choice(fd->choice_papersize2)-1;
	params->paperpackage = fl_get_choice(fd->choice_paperpackage)-1;
	params->use_geometry = fl_get_button(fd_form_paper->push_use_geometry);
	if (fl_get_button(fd_form_paper->radio_landscape))
		params->orientation = BufferParams::ORIENTATION_LANDSCAPE;
	else
		params->orientation = BufferParams::ORIENTATION_PORTRAIT;
	params->paperwidth = fl_get_input(fd->input_custom_width);
	params->paperheight = fl_get_input(fd->input_custom_height);
	params->leftmargin = fl_get_input(fd->input_left_margin);
	params->topmargin = fl_get_input(fd->input_top_margin);
	params->rightmargin = fl_get_input(fd->input_right_margin);
	params->bottommargin = fl_get_input(fd->input_bottom_margin);
	params->headheight = fl_get_input(fd->input_head_height);
	params->headsep = fl_get_input(fd->input_head_sep);
	params->footskip = fl_get_input(fd->input_foot_skip);
	current_view->buffer()->setPaperStuff();
	current_view->owner()->getMiniBuffer()->Set(_("Paper layout set"));
	current_view->buffer()->markDirty();
	
	return;
}


void PaperCancelCB(FL_OBJECT *, long)
{
	fl_hide_form(fd_form_paper->form_paper);
}


void PaperOKCB(FL_OBJECT * ob, long data)
{
	PaperCancelCB(ob, data);
	PaperApplyCB(ob, data);
}


void PaperMarginsCB(FL_OBJECT * ob, long)
{
	FD_form_paper const * fd = fd_form_paper;
	string str;
	char val;
    
	checkMarginValues();
	if (ob == fd->choice_papersize2) {
		val = fl_get_choice(fd->choice_papersize2)-1;
		if (val == BufferParams::VM_PAPER_DEFAULT) {
			fl_set_button(fd->push_use_geometry, 0);
			checkMarginValues();
		} else {
			if (val != BufferParams::VM_PAPER_USLETTER
			    && val != BufferParams::VM_PAPER_USLEGAL
			    && val != BufferParams::VM_PAPER_USEXECUTIVE
			    && val != BufferParams::VM_PAPER_A4
			    && val != BufferParams::VM_PAPER_A5
			    && val != BufferParams::VM_PAPER_B5) {
				fl_set_button(fd->push_use_geometry, 1);
			}
			fl_set_choice(fd->choice_paperpackage, BufferParams::PACKAGE_NONE + 1);
		}
	} else if (ob == fd->choice_paperpackage) {
		val = fl_get_choice(fd->choice_paperpackage)-1;
		if (val != BufferParams::PACKAGE_NONE) {
			fl_set_choice(fd->choice_papersize2,
				      BufferParams::VM_PAPER_DEFAULT + 1);
			fl_set_button(fd->push_use_geometry, 0);
		}
	} else if (ob == fd->input_custom_width
		   || ob == fd->input_custom_height 
		   || ob == fd->input_left_margin
		   || ob == fd->input_right_margin 
		   || ob == fd->input_top_margin
		   || ob == fd->input_bottom_margin 
		   || ob == fd->input_head_height
		   || ob == fd->input_head_sep
		   || ob == fd->input_foot_skip) {
		str = fl_get_input(ob);
		if (!str.empty() && !isValidLength(str)) {
			fl_set_object_label(fd_form_paper->text_warning,
					    _("Warning: Invalid Length"
					      " (valid example: 10mm)"));
			fl_show_object(fd_form_paper->text_warning);
			DeactivatePaperButtons();
		} else if (!str.empty())
			fl_set_button(fd->push_use_geometry, 1);
	}
	return;
}
