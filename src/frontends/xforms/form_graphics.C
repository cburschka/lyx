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

  fdui->form = fl_bgn_form(FL_NO_BOX, 510, 345);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 510, 345, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->tabFolder = obj = fl_add_tabfolder(FL_TOP_TABFOLDER, 5, 5, 500, 270, _("Tabbed folder"));
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_resize(obj, FL_RESIZE_ALL);
  {
    char const * const dummy = N_("Restore|#R");
    fdui->button_restore = obj = fl_add_button(FL_NORMAL_BUTTON, 5, 310, 100, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthWest);
    fl_set_object_callback(obj, C_FormBaseRestoreCB, 0);
  fdui->button_ok = obj = fl_add_button(FL_RETURN_BUTTON, 225, 310, 90, 30, _("Ok"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseOKCB, 0);
  {
    char const * const dummy = N_("Cancel|^[");
    fdui->button_cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 415, 310, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseCancelCB, 0);
  {
    char const * const dummy = N_("Apply|#A");
    fdui->button_apply = obj = fl_add_button(FL_NORMAL_BUTTON, 320, 310, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseApplyCB, 0);
  fdui->text_warning = obj = fl_add_text(FL_NORMAL_TEXT, 5, 280, 495, 25, "");
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
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

  fdui->form = fl_bgn_form(FL_NO_BOX, 500, 245);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 500, 245, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  {
    char const * const dummy = N_("File|#F");
    fdui->input_filename = obj = fl_add_input(FL_NORMAL_INPUT, 75, 10, 285, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Browse...|#B");
    fdui->button_browse = obj = fl_add_button(FL_NORMAL_BUTTON, 365, 10, 100, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 10, 55, 480, 60, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  {
    char const * const dummy = N_("Subfigure|#S");
    fdui->check_subcaption = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 15, 70, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Title|#T");
    fdui->input_subcaption = obj = fl_add_input(FL_NORMAL_INPUT, 190, 70, 290, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 10, 115, 480, 60, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  {
    char const * const dummy = N_("Angle|#A");
    fdui->input_rotate_angle = obj = fl_add_input(FL_INT_INPUT, 190, 130, 55, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  obj = fl_add_text(FL_NORMAL_TEXT, 245, 130, 38, 30, _("deg"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  {
    char const * const dummy = N_("Origin|#O");
    fdui->choice_origin = obj = fl_add_choice(FL_NORMAL_CHOICE, 360, 130, 118, 28, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Draft mode|#D");
    fdui->button_draft = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 15, 195, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Rotate|#R");
    fdui->check_rotate = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 15, 130, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Don't unzip, when exporting to LaTeX|#u");
    fdui->button_nounzip = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 175, 195, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
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

  fdui->form = fl_bgn_form(FL_NO_BOX, 500, 245);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 500, 245, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 355, 10, 135, 50, "");
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 10, 10, 480, 225, _("Output size"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);

  fdui->radio_size = fl_bgn_group();
  {
    char const * const dummy = N_("Original size|#O");
    fdui->button_asis = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 25, 30, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Scale|#S");
    fdui->button_scale = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 25, 80, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Custom|#C");
    fdui->button_wh = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 25, 130, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_group();

  {
    char const * const dummy = N_("Value|#V");
    fdui->input_scale = obj = fl_add_input(FL_NORMAL_INPUT, 200, 80, 85, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  // xgettext:no-c-format
  obj = fl_add_text(FL_NORMAL_TEXT, 285, 80, 25, 30, _("%"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  {
    char const * const dummy = N_("Width|#W");
    fdui->input_width = obj = fl_add_input(FL_NORMAL_INPUT, 200, 130, 85, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->choice_width_units = obj = fl_add_choice(FL_NORMAL_CHOICE, 290, 130, 60, 30, "");
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Height|#H");
    fdui->input_height = obj = fl_add_input(FL_NORMAL_INPUT, 200, 165, 85, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->choice_height_units = obj = fl_add_choice(FL_NORMAL_CHOICE, 290, 165, 60, 30, "");
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("keep Aspect ratio|#A");
    fdui->check_aspectratio = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 200, 200, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Get LyX size|#L");
    fdui->button_lyx_values = obj = fl_add_button(FL_NORMAL_BUTTON, 365, 20, 115, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_bbox::~FD_form_bbox()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_bbox * FormGraphics::build_bbox()
{
  FL_OBJECT *obj;
  FD_form_bbox *fdui = new FD_form_bbox;

  fdui->form = fl_bgn_form(FL_NO_BOX, 500, 245);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 500, 245, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 10, 10, 480, 180, _("Corner coordinates (X, Y)"));
    fl_set_object_boxtype(obj, FL_UP_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  obj = fl_add_text(FL_NORMAL_TEXT, 195, 110, 30, 25, _(", "));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT, 195, 55, 30, 25, _(", "));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  {
    char const * const dummy = N_("Top right  ( |#T");
    fdui->input_bb_x1 = obj = fl_add_input(FL_NORMAL_INPUT, 150, 45, 50, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_bb_y1 = obj = fl_add_input(FL_NORMAL_INPUT, 220, 45, 50, 30, _(" )"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Bottom left  ( |#B");
    fdui->input_bb_x0 = obj = fl_add_input(FL_NORMAL_INPUT, 150, 95, 50, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_bb_y0 = obj = fl_add_input(FL_NORMAL_INPUT, 220, 95, 50, 30, _(" )"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Units|#U");
    fdui->choice_bb_units = obj = fl_add_choice(FL_NORMAL_CHOICE, 350, 70, 45, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Get values from file|#G");
    fdui->button_getBB = obj = fl_add_button(FL_NORMAL_BUTTON, 20, 150, 165, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Clip to bounding box|#C");
    fdui->button_clip = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 20, 200, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
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

  fdui->form = fl_bgn_form(FL_NO_BOX, 500, 245);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 500, 245, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  {
    char const * const dummy = N_("Additional LaTeX options|#L");
    fdui->input_special = obj = fl_add_input(FL_NORMAL_INPUT, 190, 40, 265, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_lyxview::~FD_form_lyxview()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_lyxview * FormGraphics::build_lyxview()
{
  FL_OBJECT *obj;
  FD_form_lyxview *fdui = new FD_form_lyxview;

  fdui->form = fl_bgn_form(FL_NO_BOX, 500, 245);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 500, 245, "");
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 155, 10, 335, 225, _("Screen size"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 355, 10, 135, 50, "");
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 10, 10, 145, 225, _("Screen display"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  {
    char const * const dummy = N_("Value|#V");
    fdui->input_lyxscale = obj = fl_add_input(FL_NORMAL_INPUT, 350, 95, 70, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
 fl_bgn_group();
  {
    char const * const dummy = N_("Original size|#O");
    fdui->button_lyxasis = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 255, 25, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Scale|#S");
    fdui->button_lyxscale = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 255, 90, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Custom|#u");
    fdui->button_lyxwh = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 255, 155, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_group();

  // xgettext:no-c-format
  obj = fl_add_text(FL_NORMAL_TEXT, 420, 95, 25, 30, _("%"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);

  fdui->radio_display = fl_bgn_group();
  {
    char const * const dummy = N_("Default|#f");
    fdui->radio_pref = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 20, 25, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Monochrome|#M");
    fdui->radio_mono = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 20, 65, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Grayscale|#G");
    fdui->radio_gray = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 20, 105, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Color|#C");
    fdui->radio_color = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 20, 145, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Don't display|#D");
    fdui->radio_nodisplay = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 20, 185, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_group();

  {
    char const * const dummy = N_("Width|#W");
    fdui->input_lyxwidth = obj = fl_add_input(FL_NORMAL_INPUT, 350, 155, 70, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->choice_width_lyxwidth = obj = fl_add_choice(FL_NORMAL_CHOICE, 425, 155, 50, 30, "");
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Height|#H");
    fdui->input_lyxheight = obj = fl_add_input(FL_NORMAL_INPUT, 350, 190, 70, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->choice_width_lyxheight = obj = fl_add_choice(FL_NORMAL_CHOICE, 425, 190, 50, 30, "");
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Get LaTeX size|#L");
    fdui->button_latex_values = obj = fl_add_button(FL_NORMAL_BUTTON, 365, 20, 115, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

