// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_print.h"
#include "FormPrint.h"

FD_form_print::~FD_form_print()
{
  if (form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_print * FormPrint::build_print()
{
  FL_OBJECT *obj;
  FD_form_print *fdui = new FD_form_print;

  fdui->form = fl_bgn_form(FL_NO_BOX, 340, 360);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 340, 360, "");
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 215, 320, 90, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
  obj = fl_add_text(FL_NORMAL_TEXT, 20, 205, 70, 20, _("Print to"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fdui->input_printer = obj = fl_add_input(FL_NORMAL_INPUT, 90, 225, 230, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_file = obj = fl_add_input(FL_NORMAL_INPUT, 90, 265, 230, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);

  fdui->group_radio_printto = fl_bgn_group();
  fdui->radio_printer = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 10, 225, 80, 30, idex(_("Printer|#P")));
    fl_set_button_shortcut(obj, scex(_("Printer|#P")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->radio_file = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 10, 265, 80, 30, idex(_("File|#F")));
    fl_set_button_shortcut(obj, scex(_("File|#F")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_group();

  obj = fl_add_frame(FL_ENGRAVED_FRAME, 180, 20, 150, 70, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 20, 160, 180, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
  fdui->button_ok = obj = fl_add_button(FL_RETURN_BUTTON, 10, 315, 100, 30, _("OK"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseOKCB, 0);
  fdui->button_apply = obj = fl_add_button(FL_NORMAL_BUTTON, 120, 315, 100, 30, idex(_("Apply|#A")));
    fl_set_button_shortcut(obj, scex(_("Apply|#A")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseApplyCB, 0);
  fdui->button_cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 230, 315, 100, 30, idex(_("Cancel|C#C^[")));
    fl_set_button_shortcut(obj, scex(_("Cancel|C#C^[")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseCancelCB, 0);

  fdui->group_radio_pages = fl_bgn_group();
  fdui->radio_all_pages = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 10, 30, 160, 30, idex(_("All Pages|#G")));
    fl_set_button_shortcut(obj, scex(_("All Pages|#G")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->radio_odd_pages = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 10, 60, 160, 30, idex(_("Only Odd Pages|#O")));
    fl_set_button_shortcut(obj, scex(_("Only Odd Pages|#O")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->radio_even_pages = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 10, 90, 160, 30, idex(_("Only Even Pages|#E")));
    fl_set_button_shortcut(obj, scex(_("Only Even Pages|#E")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_group();


  fdui->group_radio_order = fl_bgn_group();
  fdui->radio_order_normal = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 180, 30, 150, 30, idex(_("Normal Order|#N")));
    fl_set_button_shortcut(obj, scex(_("Normal Order|#N")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->radio_order_reverse = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 180, 60, 150, 30, idex(_("Reverse Order|#R")));
    fl_set_button_shortcut(obj, scex(_("Reverse Order|#R")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_group();

  obj = fl_add_text(FL_NORMAL_TEXT, 200, 10, 60, 20, _("Order"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT, 20, 10, 50, 20, _("Print"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fdui->input_from_page = obj = fl_add_input(FL_INT_INPUT, 20, 160, 50, 30, _("Pages:"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 180, 110, 150, 90, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
  obj = fl_add_text(FL_NORMAL_TEXT, 200, 95, 50, 20, _("Copies"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fdui->input_count = obj = fl_add_input(FL_INT_INPUT, 190, 160, 130, 30, _("Count:"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->radio_collated = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 180, 115, 140, 30, idex(_("Collated|#C")));
    fl_set_button_shortcut(obj, scex(_("Collated|#C")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_to_page = obj = fl_add_input(FL_INT_INPUT, 110, 160, 50, 30, _("to"));
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

