// File modified by fdfix.sh for use by lyx (with xforms >= 0.86) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_graphics.h"
#include "FormGraphics.h" 

FD_form_graphics * FormGraphics::build_graphics()
{
  FL_OBJECT *obj;
  FD_form_graphics *fdui = new FD_form_graphics;

  fdui->form_graphics = fl_bgn_form(FL_NO_BOX, 410, 390);
  fdui->form_graphics->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 410, 390, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 310, 390, 30, "");
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 230, 250, 170, 50, "");
  fdui->input_filename = obj = fl_add_input(FL_NORMAL_INPUT, 90, 20, 210, 30, idex(_("Graphics File|F#F")));fl_set_button_shortcut(obj, scex(_("Graphics File|F#F")), 1);
    fl_set_object_callback(obj, C_FormGraphicsInputCB, 0);
  fdui->button_browse = obj = fl_add_button(FL_NORMAL_BUTTON, 310, 20, 90, 30, idex(_("Browse|B#B")));fl_set_button_shortcut(obj, scex(_("Browse|B#B")), 1);
    fl_set_object_callback(obj, C_FormGraphicsBrowseCB, 0);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 10, 70, 200, 160, _("Width"));
    fl_set_object_lalign(obj, FL_ALIGN_CENTER);
  obj = fl_add_text(FL_NORMAL_TEXT, 130, 60, 50, 20, _("Height"));
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fdui->input_width = obj = fl_add_input(FL_NORMAL_INPUT, 20, 190, 90, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormGraphicsInputCB, 0);
  fdui->input_height = obj = fl_add_input(FL_NORMAL_INPUT, 120, 190, 80, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormGraphicsInputCB, 0);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 230, 70, 170, 160, _("Display"));
    fl_set_object_lalign(obj, FL_ALIGN_CENTER);
  fdui->button_ok = obj = fl_add_button(FL_RETURN_BUTTON, 10, 350, 70, 30, _("Ok"));
    fl_set_object_callback(obj, C_FormGraphicsOKCB, 0);
  fdui->button_apply = obj = fl_add_button(FL_NORMAL_BUTTON, 100, 350, 70, 30, idex(_("Apply|A#A")));fl_set_button_shortcut(obj, scex(_("Apply|A#A")), 1);
    fl_set_object_callback(obj, C_FormGraphicsApplyCB, 0);
  fdui->button_cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 190, 350, 70, 30, idex(_("Cancel|C#C^[")));fl_set_button_shortcut(obj, scex(_("Cancel|C#C^[")), 1);
    fl_set_object_callback(obj, C_FormGraphicsCancelCB, 0);

  fdui->radio_button_group_width = fl_bgn_group();
  fdui->radio_width_percent_page = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 10, 140, 80, 30, _("% of Page"));
    fl_set_object_callback(obj, C_FormGraphicsInputCB, 0);
  fdui->radio_width_default = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 10, 80, 80, 30, _("Default"));
    fl_set_object_callback(obj, C_FormGraphicsInputCB, 0);
    fl_set_button(obj, 1);
  fdui->radio_width_cm = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 10, 100, 80, 30, _("cm"));
    fl_set_object_callback(obj, C_FormGraphicsInputCB, 0);
  fdui->radio_width_inch = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 10, 120, 80, 30, _("Inch"));
    fl_set_object_callback(obj, C_FormGraphicsInputCB, 0);
  fdui->radio_width_percent_column = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 10, 160, 80, 30, _("% of Column"));
    fl_set_object_callback(obj, C_FormGraphicsInputCB, 0);
  fl_end_group();


  fdui->radio_button_group_height = fl_bgn_group();
  fdui->radio_height_percent_page = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 110, 140, 80, 30, _("% of Page"));
    fl_set_object_callback(obj, C_FormGraphicsInputCB, 0);
  fdui->radio_height_inch = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 110, 120, 80, 30, _("Inch"));
    fl_set_object_callback(obj, C_FormGraphicsInputCB, 0);
  fdui->radio_height_cm = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 110, 100, 80, 30, _("cm"));
    fl_set_object_callback(obj, C_FormGraphicsInputCB, 0);
  fdui->radio_height_default = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 110, 80, 80, 30, _("Default"));
    fl_set_object_callback(obj, C_FormGraphicsInputCB, 0);
    fl_set_button(obj, 1);
  fl_end_group();


  fdui->radio_button_group_display = fl_bgn_group();
  fdui->radio_display_monochrome = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 250, 80, 80, 30, _("in Monochrome"));
    fl_set_button(obj, 1);
  fdui->radio_display_grayscale = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 250, 110, 80, 30, _("in Grayscale"));
  fdui->radio_display_color = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 250, 140, 80, 30, _("in Color"));
  fdui->radio_no_display = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 250, 170, 80, 30, _("Don't display"));
  fl_end_group();

  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 10, 250, 200, 50, _("Rotate"));
  fdui->input_rotate_angle = obj = fl_add_input(FL_INT_INPUT, 70, 260, 80, 30, _("Angle"));
    fl_set_object_callback(obj, C_FormGraphicsInputCB, 0);
  fdui->check_inline = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 230, 260, 170, 30, _("Inline Figure"));
    fl_set_object_callback(obj, C_FormGraphicsInputCB, 0);
  fdui->input_subcaption = obj = fl_add_input(FL_NORMAL_INPUT, 120, 310, 280, 30, "");
    fl_set_object_callback(obj, C_FormGraphicsInputCB, 0);
  fdui->check_subcaption = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 10, 310, 110, 30, idex(_("Subcaption|S#S")));fl_set_button_shortcut(obj, scex(_("Subcaption|S#S")), 1);
    fl_set_object_callback(obj, C_FormGraphicsInputCB, 0);
  fl_end_form();

  fdui->form_graphics->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

