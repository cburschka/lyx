// File modified by fdfix.sh for use by lyx (with xforms 0.81) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "credits_form.h"

FD_form_credits *create_form_form_credits(void)
{
  FL_OBJECT *obj;
  FD_form_credits *fdui = (FD_form_credits *) fl_calloc(1, sizeof(FD_form_credits));

  fdui->form_credits = fl_bgn_form(FL_NO_BOX, 500, 330);
  obj = fl_add_box(FL_UP_BOX,0,0,500,330,"");
  obj = fl_add_button(FL_RETURN_BUTTON,180,290,140,30,_("OK"));
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,CreditsOKCB,0);
  obj = fl_add_text(FL_NORMAL_TEXT,10,40,480,30,_("Matthias"));
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_ITALIC_STYLE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
  obj = fl_add_text(FL_NORMAL_TEXT,10,10,480,30,_("All these people have contributed to the LyX project. Thanks,"));
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
  fdui->browser_credits = obj = fl_add_browser(FL_NORMAL_BROWSER,10,80,480,200,"");
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  fl_end_form();

  //fdui->form_credits->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_copyright *create_form_copyright(void)
{
  FL_OBJECT *obj;
  FD_copyright *fdui = (FD_copyright *) fl_calloc(1, sizeof(FD_copyright));

  fdui->copyright = fl_bgn_form(FL_NO_BOX, 450, 430);
  obj = fl_add_box(FL_UP_BOX,0,0,450,430,"");
  obj = fl_add_text(FL_NORMAL_TEXT,10,10,430,50,_("LyX is Copyright (C) 1995 by Matthias Ettrich,\n1995-1999 LyX Team"));
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
  obj = fl_add_text(FL_NORMAL_TEXT,10,70,430,110,_("This program is free software; you can redistribute it\nand/or modify it under the terms of the GNU General\nPublic License as published by the Free Software\nFoundation; either version 2 of the License, or\n(at your option) any later version."));
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  obj = fl_add_button(FL_RETURN_BUTTON,160,390,140,30,_("OK"));
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_South, FL_South);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj,CopyrightOKCB,0);
  obj = fl_add_text(FL_NORMAL_TEXT,10,190,430,190,_("LyX is distributed in the hope that it will\nbe useful, but WITHOUT ANY WARRANTY;\nwithout even the implied warranty of MERCHANTABILITY\nor FITNESS FOR A PARTICULAR PURPOSE.\nSee the GNU General Public License for more details.\nYou should have received a copy of\nthe GNU General Public License\nalong with this program; if not, write to\nthe Free Software Foundation, Inc.,\n675 Mass Ave, Cambridge, MA 02139, USA."));
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  fl_end_form();

  //fdui->copyright->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

