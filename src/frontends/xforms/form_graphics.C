// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_graphics.h"
#include "FormGraphics.h"

FD_form_graphics::~FD_form_graphics()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_graphics * FormGraphics::build_graphics()
{
  FL_OBJECT *obj;
  FD_form_graphics *fdui = new FD_form_graphics;

  fdui->form = fl_bgn_form(FL_NO_BOX, 490, 390);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 490, 390, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 20, 310, 138, 30, "");
  {
    char const * const dummy = N_("Graphics File|#F");
    fdui->input_filename = obj = fl_add_input(FL_NORMAL_INPUT, 150, 20, 210, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseInputCB, CHECKINPUT);
  {
    char const * const dummy = N_("Browse|#B");
    fdui->button_browse = obj = fl_add_button(FL_NORMAL_BUTTON, 370, 20, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseInputCB, BROWSE);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 20, 70, 240, 160, _("Width"));
    fl_set_object_lalign(obj, FL_ALIGN_CENTER);
  obj = fl_add_text(FL_NORMAL_TEXT, 164, 60, 60, 20, _("Height"));
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fdui->input_width = obj = fl_add_input(FL_NORMAL_INPUT, 32, 190, 108, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, CHECKINPUT);
  fdui->input_height = obj = fl_add_input(FL_NORMAL_INPUT, 152, 190, 96, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, CHECKINPUT);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 270, 70, 210, 160, _("Display"));
    fl_set_object_lalign(obj, FL_ALIGN_CENTER);

  fdui->radio_button_group_width = fl_bgn_group();
  // xgettext:no-c-format
  fdui->radio_width_percent_page = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 20, 140, 80, 30, _("% of Page"));
    fl_set_object_callback(obj, C_FormBaseInputCB, CHECKINPUT);
  fdui->radio_width_default = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 20, 80, 80, 30, _("Default"));
    fl_set_object_callback(obj, C_FormBaseInputCB, CHECKINPUT);
    fl_set_button(obj, 1);
  fdui->radio_width_cm = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 20, 100, 80, 30, _("cm"));
    fl_set_object_callback(obj, C_FormBaseInputCB, CHECKINPUT);
  fdui->radio_width_inch = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 20, 120, 80, 30, _("Inch"));
    fl_set_object_callback(obj, C_FormBaseInputCB, CHECKINPUT);
  // xgettext:no-c-format
  fdui->radio_width_percent_column = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 20, 160, 96, 30, _("% of Column"));
    fl_set_object_callback(obj, C_FormBaseInputCB, CHECKINPUT);
  fl_end_group();


  fdui->radio_button_group_height = fl_bgn_group();
  // xgettext:no-c-format
  fdui->radio_height_percent_page = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 140, 140, 96, 30, _("% of Page"));
    fl_set_object_callback(obj, C_FormBaseInputCB, CHECKINPUT);
  fdui->radio_height_inch = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 140, 120, 96, 30, _("Inch"));
    fl_set_object_callback(obj, C_FormBaseInputCB, CHECKINPUT);
  fdui->radio_height_cm = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 140, 100, 80, 30, _("cm"));
    fl_set_object_callback(obj, C_FormBaseInputCB, CHECKINPUT);
  fdui->radio_height_default = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 140, 80, 96, 30, _("Default"));
    fl_set_object_callback(obj, C_FormBaseInputCB, CHECKINPUT);
    fl_set_button(obj, 1);
  fl_end_group();


  fdui->radio_button_group_display = fl_bgn_group();
  {
    char const * const dummy = N_("in Monochrome|#M");
    fdui->radio_display_monochrome = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 295, 80, 98, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseInputCB, CHECKINPUT);
    fl_set_button(obj, 1);
  {
    char const * const dummy = N_("in Grayscale|#G");
    fdui->radio_display_grayscale = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 295, 110, 98, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseInputCB, CHECKINPUT);
  {
    char const * const dummy = N_("in Color|#C");
    fdui->radio_display_color = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 295, 140, 98, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseInputCB, CHECKINPUT);
  {
    char const * const dummy = N_("Don't display|#D");
    fdui->radio_no_display = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 295, 170, 98, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseInputCB, CHECKINPUT);
  fl_end_group();

  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 20, 250, 240, 50, _("Rotate"));
  {
    char const * const dummy = N_("Angle|#A");
    fdui->input_rotate_angle = obj = fl_add_input(FL_INT_INPUT, 91, 260, 94, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseInputCB, CHECKINPUT);
  fdui->input_subcaption = obj = fl_add_input(FL_NORMAL_INPUT, 158, 310, 322, 30, "");
    fl_set_object_callback(obj, C_FormBaseInputCB, CHECKINPUT);
  {
    char const * const dummy = N_("Subcaption|#S");
    fdui->check_subcaption = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 120, 310, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, CHECKINPUT);
  {
    char const * const dummy = N_("Update|#U");
    fdui->button_restore = obj = fl_add_button(FL_NORMAL_BUTTON, 20, 350, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthWest);
    fl_set_object_callback(obj, C_FormBaseRestoreCB, 0);
  fdui->button_ok = obj = fl_add_button(FL_RETURN_BUTTON, 190, 350, 90, 30, _("Ok"));
    fl_set_object_callback(obj, C_FormBaseOKCB, 0);
  {
    char const * const dummy = N_("Apply|#A");
    fdui->button_apply = obj = fl_add_button(FL_NORMAL_BUTTON, 290, 350, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseApplyCB, 0);
  {
    char const * const dummy = N_("Cancel|^[");
    fdui->button_cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 390, 350, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseCancelCB, 0);
  fdui->radio_height_scale = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 140, 160, 96, 30, _("Scale"));
    fl_set_object_callback(obj, C_FormBaseInputCB, CHECKINPUT);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

