/* Bullet form callbacks */
#include <config.h>
#include <cstdlib>
#include FORMS_H_LOCATION
#include XPM_H_LOCATION
#include "bullet_forms.h"
#include "bmtable.h"
#include "buffer.h"
#include "bufferparams.h"
#include "support/filetools.h"
#include "gettext.h"
#include "lyx_gui_misc.h" // CancelCloseBoxCB + WriteAlert

extern BufferView * current_view;
FD_form_bullet * fd_form_bullet;

static int current_bullet_panel;
static int current_bullet_depth;

/** Creates or raises the bullet popup and sets appropriate form values
  */
void bulletForm()
{
	if (!fd_form_bullet) {
		if ((XpmVersion < 4
		     || (XpmVersion == 4 && XpmRevision < 7))) {
			WriteAlert(_("Sorry, your libXpm is too old."),
			_("This feature requires xpm-4.7 (a.k.a 3.4g) or newer."),
			"");
			return;
		}
		fd_form_bullet = create_form_form_bullet();
		fl_addto_choice(fd_form_bullet->choice_bullet_size,
		                _(" default | tiny | script | footnote | small |"
		                " normal | large | Large | LARGE | huge | Huge"));
		fl_set_choice(fd_form_bullet->choice_bullet_size, 1);
		fl_set_form_atclose(fd_form_bullet->form_bullet,
		                    CancelCloseBoxCB, 0);
	}

	if (updateBulletForm()) {
		// Show form
		if (fd_form_bullet->form_bullet->visible) {
			fl_raise_form(fd_form_bullet->form_bullet);
		}
		else {
			fl_show_form(fd_form_bullet->form_bullet,
		        	     FL_PLACE_MOUSE, FL_FULLBORDER,
			             _("Itemize Bullet Selection"));
		}
	}
}

bool updateBulletForm()
{
	bool update = true;
	if (!fd_form_bullet) {
		return false;
	}
	if (!current_view->available()) {
		update = false;
	} else if (current_view->buffer()->isReadonly()
		   || current_view->buffer()->isLinuxDoc()) {
		fl_deactivate_object (fd_form_bullet->button_ok);
		fl_deactivate_object (fd_form_bullet->button_apply);
		fl_set_object_lcol (fd_form_bullet->button_ok, FL_INACTIVE);
		fl_set_object_lcol (fd_form_bullet->button_apply, FL_INACTIVE);
		fl_deactivate_object (fd_form_bullet->bmtable_bullet_panel);
		fl_deactivate_object (fd_form_bullet->choice_bullet_size);
		fl_deactivate_object (fd_form_bullet->input_bullet_latex);
		update = false;
	} else {
		fl_activate_object (fd_form_bullet->button_ok);
		fl_activate_object (fd_form_bullet->button_apply);
		fl_set_object_lcol (fd_form_bullet->button_ok, FL_BLACK);
		fl_set_object_lcol (fd_form_bullet->button_apply, FL_BLACK);
		fl_activate_object (fd_form_bullet->bmtable_bullet_panel);
		fl_activate_object (fd_form_bullet->choice_bullet_size);
		fl_activate_object (fd_form_bullet->input_bullet_latex);
	}

	if (update) {
		// any settings that need doing each time
		fl_set_button(fd_form_bullet->radio_bullet_depth_1, 1);
		fl_set_input(fd_form_bullet->input_bullet_latex,
		             current_view->buffer()
			     ->params.user_defined_bullets[0].c_str());
		fl_set_choice(fd_form_bullet->choice_bullet_size,
			      current_view->buffer()
			      ->params.user_defined_bullets[0].getSize() + 2);
	} else {
		if (fd_form_bullet->form_bullet->visible) {
			fl_hide_form(fd_form_bullet->form_bullet);
		}
	}
	return update;
}

/*---------------------------------------*/
/* callbacks for form form_bullet        */

void BulletOKCB(FL_OBJECT *ob, long data)
{
	BulletApplyCB(ob, data);
	BulletCancelCB(ob, data);
}


void BulletApplyCB(FL_OBJECT * /*ob*/, long /*data*/ )
{
	// update the bullet settings
	BufferParams & param = current_view->buffer()->params;

	// a little bit of loop unrolling
	param.user_defined_bullets[0] = param.temp_bullets[0];
	param.user_defined_bullets[1] = param.temp_bullets[1];
	param.user_defined_bullets[2] = param.temp_bullets[2];
	param.user_defined_bullets[3] = param.temp_bullets[3];
	current_view->buffer()->markDirty();
}


void BulletCancelCB(FL_OBJECT * /*ob*/, long /*data*/ )
{
	fl_hide_form(fd_form_bullet->form_bullet);
	// this avoids confusion when reopening
	BufferParams & param = current_view->buffer()->params;
	param.temp_bullets[0] = param.user_defined_bullets[0];
	param.temp_bullets[1] = param.user_defined_bullets[1];
	param.temp_bullets[2] = param.user_defined_bullets[2];
	param.temp_bullets[3] = param.user_defined_bullets[3];
}


void InputBulletLaTeXCB(FL_OBJECT *, long)
{
	// fill-in code for callback
	BufferParams & param = current_view->buffer()->params;

	param.temp_bullets[current_bullet_depth].setText(
		fl_get_input(fd_form_bullet->input_bullet_latex));
}


void ChoiceBulletSizeCB(FL_OBJECT * ob, long /*data*/ )
{
	BufferParams & param = current_view->buffer()->params;

	// convert from 1-6 range to -1-4 
	param.temp_bullets[current_bullet_depth].setSize(fl_get_choice(ob) - 2);
	fl_set_input(fd_form_bullet->input_bullet_latex,
				 param.temp_bullets[current_bullet_depth].c_str());
}


void BulletDepthCB(FL_OBJECT * ob, long data)
{
	/* Should I do the following:                                 */
	/*  1. change to the panel that the current bullet belongs in */
	/*  2. show that bullet as selected                           */
	/*  3. change the size setting to the size of the bullet in Q.*/
	/*  4. display the latex equivalent in the latex box          */
	/*                                                            */
	/* I'm inclined to just go with 3 and 4 at the moment and     */
	/* maybe try to support the others later                      */
	BufferParams & param = current_view->buffer()->params;

	switch (fl_get_button_numb(ob)) {
	case 3:
		// right mouse button resets to default
		param.temp_bullets[data] = ITEMIZE_DEFAULTS[data];
	default:
		current_bullet_depth = data;
		fl_set_input(fd_form_bullet->input_bullet_latex,
		             param.temp_bullets[data].c_str());
		fl_set_choice(fd_form_bullet->choice_bullet_size,
		              param.temp_bullets[data].getSize() + 2);
	}
}


void BulletPanelCB(FL_OBJECT * /*ob*/, long data)
{
	/* Here we have to change the background pixmap to that selected */
	/* by the user. (eg. standard.xpm, psnfss1.xpm etc...)           */

	if (data != current_bullet_panel) {
		fl_freeze_form(fd_form_bullet->form_bullet);
		current_bullet_panel = data;

		/* free the current pixmap */
		fl_free_bmtable_pixmap(fd_form_bullet->bmtable_bullet_panel);
		string new_panel;
		switch (data) {
			/* display the new one */
		case 0 :
			new_panel = "standard";
			break;
		case 1 :
			new_panel = "amssymb";
			break;
		case 2 :
			new_panel = "psnfss1";
			break;
		case 3 :
			new_panel = "psnfss2";
			break;
		case 4 :
			new_panel = "psnfss3";
			break;
		case 5 :
			new_panel = "psnfss4";
			break;
		default :
			/* something very wrong happened */
			// play it safe for now but should be an exception
			current_bullet_panel = 0;  // standard panel
			new_panel = "standard";
			break;
		}
		fl_set_bmtable_pixmap_file(fd_form_bullet->bmtable_bullet_panel, 
					   6, 6,
					   LibFileSearch("images", new_panel, 
							 "xpm").c_str());
		fl_redraw_object(fd_form_bullet->bmtable_bullet_panel);
		fl_unfreeze_form(fd_form_bullet->form_bullet);
	}
}


void BulletBMTableCB(FL_OBJECT *ob, long /*data*/ )
{
	/* handle the user input by setting the current bullet depth's pixmap */
	/* to that extracted from the current chosen position of the BMTable  */
	/* Don't forget to free the button's old pixmap first.                */

	BufferParams & param = current_view->buffer()->params;
	int bmtable_button = fl_get_bmtable(ob);

	/* try to keep the button held down till another is pushed */
	/*  fl_set_bmtable(ob, 1, bmtable_button); */
	param.temp_bullets[current_bullet_depth].setFont(current_bullet_panel);
	param.temp_bullets[current_bullet_depth].setCharacter(bmtable_button);
	fl_set_input(fd_form_bullet->input_bullet_latex,
	             param.temp_bullets[current_bullet_depth].c_str());
}
