// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "xforms_helpers.h"
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

  fdui->form = fl_bgn_form(FL_NO_BOX, 510, 320);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 510, 320, "");
  fdui->tabFolder = obj = fl_add_tabfolder(FL_TOP_TABFOLDER, 10, 5, 490, 270, _("Tabbed folder"));
    fl_set_object_resize(obj, FL_RESIZE_ALL);
  {
    char const * const dummy = N_("Help|#H");
    fdui->button_help = obj = fl_add_button(FL_NORMAL_BUTTON, 440, 280, 60, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Restore|#R");
    fdui->button_restore = obj = fl_add_button(FL_NORMAL_BUTTON, 135, 280, 75, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthWest);
    fl_set_object_callback(obj, C_FormBaseRestoreCB, 0);
  fdui->button_ok = obj = fl_add_button(FL_RETURN_BUTTON, 10, 280, 75, 30, _("Ok"));
    fl_set_object_callback(obj, C_FormBaseOKCB, 0);
  {
    char const * const dummy = N_("Cancel|^[");
    fdui->button_cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 260, 280, 75, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseCancelCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_file::~FD_form_file()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_file * FormGraphics::build_file()
{
  FL_OBJECT *obj;
  FD_form_file *fdui = new FD_form_file;

  fdui->form = fl_bgn_form(FL_NO_BOX, 505, 235);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 505, 235, "");
  {
    char const * const dummy = N_("File|#F");
    fdui->input_filename = obj = fl_add_input(FL_NORMAL_INPUT, 85, 10, 270, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Browse...|#B");
    fdui->button_browse = obj = fl_add_button(FL_NORMAL_BUTTON, 365, 10, 100, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 15, 50, 260, 115, _("Bounding Box"));
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  {
    char const * const dummy = N_("xLeftBottom|#x");
    fdui->input_bbx0 = obj = fl_add_input(FL_NORMAL_INPUT, 25, 75, 50, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("yLeftBottom|#y");
    fdui->input_bby0 = obj = fl_add_input(FL_NORMAL_INPUT, 160, 75, 50, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("xRightTop|#R");
    fdui->input_bbx1 = obj = fl_add_input(FL_NORMAL_INPUT, 25, 125, 50, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("yRightTop|#T");
    fdui->input_bby1 = obj = fl_add_input(FL_NORMAL_INPUT, 160, 125, 50, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("clip to bounding box|#c");
    fdui->button_clip = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 290, 100, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 15, 165, 465, 60, _("Subfigure"));
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  {
    char const * const dummy = N_("On/Off|#O");
    fdui->check_subcaption = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 15, 180, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Title|#T");
    fdui->input_subcaption = obj = fl_add_input(FL_NORMAL_INPUT, 110, 185, 330, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 285, 50, 195, 115, _("Options"));
  {
    char const * const dummy = N_("draft mode|#D");
    fdui->button_draft = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 290, 125, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->choice_bb_x0 = obj = fl_add_choice(FL_NORMAL_CHOICE, 80, 75, 50, 30, "");
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->choice_bb_y0 = obj = fl_add_choice(FL_NORMAL_CHOICE, 220, 75, 45, 30, "");
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->choice_bb_x1 = obj = fl_add_choice(FL_NORMAL_CHOICE, 80, 125, 50, 30, "");
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->choice_bb_y1 = obj = fl_add_choice(FL_NORMAL_CHOICE, 220, 125, 45, 30, "");
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Get BB from file|#G");
    fdui->button_getBB = obj = fl_add_button(FL_NORMAL_BUTTON, 295, 65, 170, 25, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_size::~FD_form_size()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_size * FormGraphics::build_size()
{
  FL_OBJECT *obj;
  FD_form_size *fdui = new FD_form_size;

  fdui->form = fl_bgn_form(FL_NO_BOX, 505, 235);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 505, 235, "");
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 15, 10, 225, 30, _("LaTeX"));
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  {
    char const * const dummy = N_("Width|#W");
    fdui->input_width = obj = fl_add_input(FL_NORMAL_INPUT, 80, 80, 85, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->choice_width_units = obj = fl_add_choice(FL_NORMAL_CHOICE, 170, 80, 60, 30, "");
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Height|#H");
    fdui->input_height = obj = fl_add_input(FL_NORMAL_INPUT, 80, 120, 85, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->choice_height_units = obj = fl_add_choice(FL_NORMAL_CHOICE, 170, 120, 60, 30, "");
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Scale|#S");
    fdui->input_scale = obj = fl_add_input(FL_NORMAL_INPUT, 80, 185, 85, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  // xgettext:no-c-format
  obj = fl_add_text(FL_NORMAL_TEXT, 170, 185, 25, 30, _("%"));
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 255, 10, 230, 215, _("LyX Screen"));
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  {
    char const * const dummy = N_("Width|#w");
    fdui->input_lyxwidth = obj = fl_add_input(FL_NORMAL_INPUT, 315, 145, 85, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->choice_width_lyxwidth = obj = fl_add_choice(FL_NORMAL_CHOICE, 405, 145, 60, 30, "");
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Height|#h");
    fdui->input_lyxheight = obj = fl_add_input(FL_NORMAL_INPUT, 315, 185, 85, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->choice_width_lyxheight = obj = fl_add_choice(FL_NORMAL_CHOICE, 405, 185, 60, 30, "");
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 15, 40, 225, 120, "");
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 15, 160, 225, 65, "");

  fdui->radio_display = fl_bgn_group();
  {
    char const * const dummy = N_("Don't display|#D");
    fdui->radio_check_display = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 260, 20, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
    fl_set_button(obj, 1);
  {
    char const * const dummy = N_("in Grayscale|#G");
    fdui->radio_display_gray = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 260, 75, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("in Color|#C");
    fdui->radio_display_color = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 260, 105, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("in Monochrome|#M");
    fdui->radio_display_mono = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 260, 45, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_group();


  fdui->radio_size = fl_bgn_group();
  {
    char const * const dummy = N_("Default|#D");
    fdui->button_default = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 15, 10, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("On/Off|#O");
    fdui->button_wh = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 15, 40, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("On/Off|#n");
    fdui->button_scale = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 15, 155, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_group();

  {
    char const * const dummy = N_("keep aspectratio|#k");
    fdui->check_aspectratio = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 100, 40, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_special::~FD_form_special()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_special * FormGraphics::build_special()
{
  FL_OBJECT *obj;
  FD_form_special *fdui = new FD_form_special;

  fdui->form = fl_bgn_form(FL_NO_BOX, 505, 235);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 505, 235, "");
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 15, 15, 255, 65, _("Rotation"));
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  {
    char const * const dummy = N_("Angle|#n");
    fdui->input_rotate_angle = obj = fl_add_input(FL_INT_INPUT, 25, 40, 70, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  obj = fl_add_text(FL_NORMAL_TEXT, 100, 45, 45, 25, _("degrees"));
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 275, 15, 205, 65, _("Special Options"));
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  {
    char const * const dummy = N_("Userdefined LaTeX-Options|#U");
    fdui->input_special = obj = fl_add_input(FL_NORMAL_INPUT, 285, 42, 180, 27, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Origin|#O");
    fdui->choice_origin = obj = fl_add_choice(FL_NORMAL_CHOICE2, 150, 40, 115, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

