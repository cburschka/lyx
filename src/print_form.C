// File modified by fdfix.sh for use by lyx (with xforms > 0.88) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <cstdlib>
#include "print_form.h"

FD_form_sendto *create_form_form_sendto(void)
{
  FL_OBJECT *obj;
  FD_form_sendto *fdui = (FD_form_sendto *) fl_calloc(1, sizeof(FD_form_sendto));

  fdui->form_sendto = fl_bgn_form(FL_NO_BOX, 340, 210);
  obj = fl_add_box(FL_UP_BOX, 0, 0, 340, 210, "");
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 20, 310, 80, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
  obj = fl_add_text(FL_NORMAL_TEXT, 20, 10, 80, 20, _("File Type"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fdui->input_cmd = obj = fl_add_input(FL_NORMAL_INPUT, 10, 130, 320, 30, idex(_("Command:|#C")));fl_set_button_shortcut(obj, scex(_("Command:|#C")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
  obj = fl_add_button(FL_RETURN_BUTTON, 10, 170, 100, 30, _("OK"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, SendtoOKCB, 0);
  obj = fl_add_button(FL_NORMAL_BUTTON, 120, 170, 100, 30, idex(_("Apply|#A")));fl_set_button_shortcut(obj, scex(_("Apply|#A")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, SendtoApplyCB, 0);
  obj = fl_add_button(FL_NORMAL_BUTTON, 230, 170, 100, 30, idex(_("Cancel|^[")));fl_set_button_shortcut(obj, scex(_("Cancel|^[")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, SendtoCancelCB, 0);

  fdui->group_ftype = fl_bgn_group();
  fdui->radio_ftype_dvi = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 110, 30, 110, 30, idex(_("DVI|#D")));fl_set_button_shortcut(obj, scex(_("DVI|#D")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->radio_ftype_ps = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 110, 60, 110, 30, idex(_("Postscript|#P")));fl_set_button_shortcut(obj, scex(_("Postscript|#P")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->radio_ftype_latex = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 10, 60, 100, 30, idex(_("LaTeX|#T")));fl_set_button_shortcut(obj, scex(_("LaTeX|#T")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_button(obj, 1);
  fdui->radio_ftype_lyx = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 10, 30, 100, 30, idex(_("LyX|#L")));fl_set_button_shortcut(obj, scex(_("LyX|#L")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->radio_ftype_ascii = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 220, 30, 90, 30, idex(_("Ascii|#s")));fl_set_button_shortcut(obj, scex(_("Ascii|#s")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_end_group();

  fl_end_form();

  fdui->form_sendto->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

