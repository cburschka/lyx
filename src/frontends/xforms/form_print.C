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
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_print * FormPrint::build_print()
{
  FL_OBJECT *obj;
  FD_form_print *fdui = new FD_form_print;

  fdui->form = fl_bgn_form(FL_NO_BOX, 340, 360);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 340, 360, "");
  fdui->input_printer = obj = fl_add_input(FL_NORMAL_INPUT, 90, 225, 230, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_file = obj = fl_add_input(FL_NORMAL_INPUT, 90, 265, 230, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);

  fdui->group_radio_printto = fl_bgn_group();
  {
    char const * const dummy = N_("Printer|#P");
    fdui->radio_printer = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 10, 225, 80, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("File|#F");
    fdui->radio_file = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 10, 265, 80, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_group();

  fdui->button_ok = obj = fl_add_button(FL_RETURN_BUTTON, 10, 315, 100, 30, _("OK"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseOKCB, 0);
  {
    char const * const dummy = N_("Apply|#A");
    fdui->button_apply = obj = fl_add_button(FL_NORMAL_BUTTON, 120, 315, 100, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseApplyCB, 0);
  {
    char const * const dummy = N_("Cancel|C#C^[");
    fdui->button_cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 230, 315, 100, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseCancelCB, 0);

  fdui->group_radio_pages = fl_bgn_group();
  {
    char const * const dummy = N_("All Pages|#G");
    fdui->radio_all_pages = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 10, 30, 160, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Only Odd Pages|#O");
    fdui->radio_odd_pages = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 10, 60, 160, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Only Even Pages|#E");
    fdui->radio_even_pages = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 10, 90, 160, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_group();


  fdui->group_radio_order = fl_bgn_group();
  {
    char const * const dummy = N_("Normal Order|#N");
    fdui->radio_order_normal = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 180, 30, 150, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Reverse Order|#R");
    fdui->radio_order_reverse = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 180, 60, 150, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_group();

  fdui->input_from_page = obj = fl_add_input(FL_INT_INPUT, 20, 160, 50, 30, _("Pages:"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_count = obj = fl_add_input(FL_INT_INPUT, 190, 160, 130, 30, _("Count:"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Collated|#C");
    fdui->radio_collated = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 180, 115, 140, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_to_page = obj = fl_add_input(FL_INT_INPUT, 110, 160, 50, 30, _("to"));
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 10, 20, 160, 180, _("Print"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 180, 20, 150, 70, _("Order"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 180, 110, 150, 90, _("Copies"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 10, 210, 320, 100, _("Print to"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

