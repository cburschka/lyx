// File modified by fdfix.sh for use by lyx (with xforms 0.81) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <cstdlib>
#include "cite.h"

FD_citation_form *create_form_citation_form(void)
{
  FL_OBJECT *obj;
  FD_citation_form *fdui = (FD_citation_form *) fl_calloc(1, sizeof(FD_citation_form));

  fdui->citation_form = fl_bgn_form(FL_NO_BOX, 440, 790);
  obj = fl_add_box(FL_UP_BOX, 0, 0, 440, 790, "");
  obj = fl_add_browser(FL_HOLD_BROWSER, 20, 40, 170, 370, _("Inset keys"));
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
  obj = fl_add_browser(FL_HOLD_BROWSER, 250, 40, 170, 370, _("Bibliography keys"));
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
  obj = fl_add_button(FL_NORMAL_BUTTON, 200, 40, 40, 40, _("@4->"));
  obj = fl_add_button(FL_NORMAL_BUTTON, 200, 90, 40, 40, _("@9+"));
  obj = fl_add_button(FL_NORMAL_BUTTON, 200, 140, 40, 40, _("@8->"));
  obj = fl_add_button(FL_NORMAL_BUTTON, 200, 190, 40, 40, _("@2->"));
  obj = fl_add_browser(FL_NORMAL_BROWSER, 20, 440, 400, 110, _("Info"));
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
  obj = fl_add_choice(FL_NORMAL_CHOICE, 160, 570, 130, 30, _("Citation style"));
    fl_set_object_boxtype(obj, FL_DOWN_BOX);
  obj = fl_add_input(FL_NORMAL_INPUT, 100, 620, 250, 30, _("Text before"));
  obj = fl_add_input(FL_NORMAL_INPUT, 100, 660, 250, 30, _("Text after"));
  obj = fl_add_button(FL_RETURN_BUTTON, 190, 730, 110, 40, _("OK"));
  obj = fl_add_button(FL_NORMAL_BUTTON, 310, 730, 110, 40, _("Cancel"));
  fl_end_form();

  //fdui->citation_form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

