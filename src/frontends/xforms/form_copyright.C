// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_copyright.h"
#include "FormCopyright.h"

FD_form_copyright::~FD_form_copyright()
{
  if (form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_copyright * FormCopyright::build_copyright()
{
  FL_OBJECT *obj;
  FD_form_copyright *fdui = new FD_form_copyright;

  fdui->form = fl_bgn_form(FL_NO_BOX, 450, 430);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 450, 430, "");
  obj = fl_add_text(FL_NORMAL_TEXT, 10, 10, 430, 50, _("LyX is Copyright (C) 1995 by Matthias Ettrich, \n1995-2000 LyX Team"));
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
  obj = fl_add_text(FL_NORMAL_TEXT, 10, 70, 430, 110, _("This program is free software; you can redistribute it\nand/or modify it under the terms of the GNU General\nPublic License as published by the Free Software\nFoundation; either version 2 of the License, or\n(at your option) any later version."));
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  fdui->button_ok = obj = fl_add_button(FL_RETURN_BUTTON, 160, 390, 140, 30, idex(_("Close|#C^[^M")));
    fl_set_button_shortcut(obj, scex(_("Close|#C^[^M")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormBaseCancelCB, 0);
  obj = fl_add_text(FL_NORMAL_TEXT, 10, 190, 430, 190, _("LyX is distributed in the hope that it will\nbe useful, but WITHOUT ANY WARRANTY;\nwithout even the implied warranty of MERCHANTABILITY\nor FITNESS FOR A PARTICULAR PURPOSE.\nSee the GNU General Public License for more details.\nYou should have received a copy of\nthe GNU General Public License\nalong with this program; if not, write to\nthe Free Software Foundation, Inc., \n675 Mass Ave, Cambridge, MA 02139, USA."));
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

