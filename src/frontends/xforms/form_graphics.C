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

  fdui->form = fl_bgn_form(FL_NO_BOX, 510, 360);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 510, 360, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->tabFolder = obj = fl_add_tabfolder(FL_TOP_TABFOLDER, 5, 5, 500, 270, _("Tabbed folder"));
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_resize(obj, FL_RESIZE_ALL);
  {
    char const * const dummy = N_("Restore|#R");
    fdui->button_restore = obj = fl_add_button(FL_NORMAL_BUTTON, 5, 310, 80, 30, idex(_(dummy)));
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

  fdui->form = fl_bgn_form(FL_NO_BOX, 505, 235);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 505, 235, "");
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
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 10, 50, 475, 55, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  {
    char const * const dummy = N_("Subfigure|#S");
    fdui->check_subcaption = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 15, 65, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Title|#T");
    fdui->input_subcaption = obj = fl_add_input(FL_NORMAL_INPUT, 190, 65, 280, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 10, 105, 475, 65, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  {
    char const * const dummy = N_("Angle|#n");
    fdui->input_rotate_angle = obj = fl_add_input(FL_INT_INPUT, 190, 125, 55, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  obj = fl_add_text(FL_NORMAL_TEXT, 247, 125, 38, 30, _("deg"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  {
    char const * const dummy = N_("Origin|#O");
    fdui->choice_origin = obj = fl_add_choice(FL_NORMAL_CHOICE, 350, 125, 118, 28, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 10, 170, 475, 60, _("Options"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  {
    char const * const dummy = N_("draft mode|#m");
    fdui->button_draft = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 15, 190, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Rotate|#a");
    fdui->check_rotate = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 15, 125, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("don't unzip, when \nexporting to LaTeX|#u");
    fdui->button_nounzip = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 320, 190, 30, 30, idex(_(dummy)));
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

  fdui->form = fl_bgn_form(FL_NO_BOX, 505, 235);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 505, 235, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 15, 10, 470, 210, _("Output Size"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);

  fdui->radio_size = fl_bgn_group();
  {
    char const * const dummy = N_("Original Size|#O");
    fdui->button_asis = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 50, 20, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Scale|#S");
    fdui->button_scale = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 50, 75, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Custom|#C");
    fdui->button_wh = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 50, 140, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_group();

  {
    char const * const dummy = N_("Value|#V");
    fdui->input_scale = obj = fl_add_input(FL_NORMAL_INPUT, 325, 75, 85, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  // xgettext:no-c-format
  obj = fl_add_text(FL_NORMAL_TEXT, 420, 75, 25, 30, _("%"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  {
    char const * const dummy = N_("Width|#W");
    fdui->input_width = obj = fl_add_input(FL_NORMAL_INPUT, 325, 140, 85, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->choice_width_units = obj = fl_add_choice(FL_NORMAL_CHOICE, 415, 140, 60, 30, "");
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Height|#g");
    fdui->input_height = obj = fl_add_input(FL_NORMAL_INPUT, 325, 175, 85, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->choice_height_units = obj = fl_add_choice(FL_NORMAL_CHOICE, 415, 175, 60, 30, "");
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("keep Aspect ratio|#k");
    fdui->check_aspectratio = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 85, 175, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Get LyX size|#x");
    fdui->button_lyx_values = obj = fl_add_button(FL_NORMAL_BUTTON, 360, 20, 115, 30, idex(_(dummy)));
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

  fdui->form = fl_bgn_form(FL_NO_BOX, 505, 235);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 505, 235, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 15, 10, 475, 205, _("Corner coordinates (X, Y)"));
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
    fdui->choice_bb_units = obj = fl_add_choice(FL_NORMAL_CHOICE, 370, 70, 45, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Get values from file|#G");
    fdui->button_getBB = obj = fl_add_button(FL_NORMAL_BUTTON, 45, 160, 165, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("clip to bounding box|#b");
    fdui->button_clip = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 245, 160, 30, 30, idex(_(dummy)));
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

  fdui->form = fl_bgn_form(FL_NO_BOX, 505, 235);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 505, 235, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 15, 15, 455, 70, _("Special Options"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  {
    char const * const dummy = N_("LaTeX Options|#L");
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

  fdui->form = fl_bgn_form(FL_NO_BOX, 505, 235);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 505, 235, "");
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 15, 10, 475, 205, _("Screen Display"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  {
    char const * const dummy = N_("Value|#V");
    fdui->input_lyxscale = obj = fl_add_input(FL_NORMAL_INPUT, 350, 75, 70, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
 fl_bgn_group();
  {
    char const * const dummy = N_("Original Size|#n");
    fdui->button_lyxasis = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 255, 20, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Scale|#S");
    fdui->button_lyxscale = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 255, 75, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Custom|#C");
    fdui->button_lyxwh = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 255, 130, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_group();

  // xgettext:no-c-format
  obj = fl_add_text(FL_NORMAL_TEXT, 430, 75, 25, 30, _("%"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);

  fdui->radio_display = fl_bgn_group();
  {
    char const * const dummy = N_("Default|#f");
    fdui->radio_pref = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 20, 20, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Monochrome|#M");
    fdui->radio_mono = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 20, 70, 30, 30, idex(_(dummy)));
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
    char const * const dummy = N_("Color|#o");
    fdui->radio_color = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 20, 140, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Don't display|#y");
    fdui->radio_nodisplay = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 20, 175, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_group();

  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 150, 10, 340, 205, _("Screen Size"));
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  {
    char const * const dummy = N_("Width|#w");
    fdui->input_lyxwidth = obj = fl_add_input(FL_NORMAL_INPUT, 350, 130, 70, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->choice_width_lyxwidth = obj = fl_add_choice(FL_NORMAL_CHOICE, 430, 130, 50, 30, "");
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Height|#h");
    fdui->input_lyxheight = obj = fl_add_input(FL_NORMAL_INPUT, 350, 170, 70, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->choice_width_lyxheight = obj = fl_add_choice(FL_NORMAL_CHOICE, 430, 170, 50, 30, "");
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  obj = fl_add_text(FL_NORMAL_TEXT, 30, 45, 115, 25, _("(from Preferences)"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  {
    char const * const dummy = N_("Get LaTeX size|#x");
    fdui->button_latex_values = obj = fl_add_button(FL_NORMAL_BUTTON, 350, 20, 115, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

