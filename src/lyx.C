// File modified by fdfix.sh for use by lyx (with xforms > 0.88) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <cstdlib>
#include "lyx.h"

FD_form_figure *create_form_form_figure(void)
{
  FL_OBJECT *obj;
  FD_form_figure *fdui = (FD_form_figure *) fl_calloc(1, sizeof(FD_form_figure));

  fdui->form_figure = fl_bgn_form(FL_NO_BOX, 340, 150);
  obj = fl_add_box(FL_UP_BOX, 0, 0, 340, 150, "");
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 20, 320, 80, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);

  fdui->group_radio_fugre = fl_bgn_group();
  fdui->radio_postscript = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 10, 30, 320, 30, idex(_("Encapsulated Postscript (*.eps, *.ps)|#E")));fl_set_button_shortcut(obj, scex(_("Encapsulated Postscript (*.eps, *.ps)|#E")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->radio_inline = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 10, 60, 320, 30, idex(_("Inlined EPS (*.eps, *.ps)|#I")));fl_set_button_shortcut(obj, scex(_("Inlined EPS (*.eps, *.ps)|#I")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_end_group();

  obj = fl_add_button(FL_RETURN_BUTTON, 10, 110, 100, 30, _("OK"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, FigureOKCB, 0);
  obj = fl_add_button(FL_NORMAL_BUTTON, 120, 110, 100, 30, idex(_("Apply|#A")));fl_set_button_shortcut(obj, scex(_("Apply|#A")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, FigureApplyCB, 0);
  obj = fl_add_button(FL_NORMAL_BUTTON, 230, 110, 100, 30, idex(_("Cancel|^[")));fl_set_button_shortcut(obj, scex(_("Cancel|^[")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, FigureCancelCB, 0);
  obj = fl_add_text(FL_NORMAL_TEXT, 20, 10, 50, 20, _("Type"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fl_end_form();

  fdui->form_figure->fdui = fdui;

  return fdui;
}
