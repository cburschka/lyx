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
  fdui->tabFolder = obj = fl_add_tabfolder(FL_TOP_TABFOLDER, 10, 5, 490, 270, _("Tabbed folder"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_resize(obj, FL_RESIZE_ALL);
  {
    char const * const dummy = N_("Help|#H");
    fdui->button_help = obj = fl_add_button(FL_NORMAL_BUTTON, 15, 320, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Restore|#R");
    fdui->button_restore = obj = fl_add_button(FL_NORMAL_BUTTON, 165, 320, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthWest);
    fl_set_object_callback(obj, C_FormBaseRestoreCB, 0);
  fdui->button_ok = obj = fl_add_button(FL_RETURN_BUTTON, 315, 320, 90, 30, _("Ok"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseOKCB, 0);
  {
    char const * const dummy = N_("Cancel|^[");
    fdui->button_cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 410, 320, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseCancelCB, 0);
  fdui->text_warning = obj = fl_add_text(FL_NORMAL_TEXT, 10, 280, 485, 35, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
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
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 15, 50, 460, 50, _("Subfigure"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  {
    char const * const dummy = N_("Subfigure|#S");
    fdui->check_subcaption = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 20, 60, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Title:|#T");
    fdui->input_subcaption = obj = fl_add_input(FL_NORMAL_INPUT, 180, 60, 280, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 15, 110, 460, 60, _("Rotation"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  {
    char const * const dummy = N_("Angle:|#n");
    fdui->input_rotate_angle = obj = fl_add_input(FL_INT_INPUT, 105, 130, 72, 28, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  obj = fl_add_text(FL_NORMAL_TEXT, 175, 130, 66, 30, _("degrees"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  {
    char const * const dummy = N_("Origin:|#O");
    fdui->choice_origin = obj = fl_add_choice(FL_NORMAL_CHOICE, 340, 130, 118, 28, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 15, 180, 460, 50, _("Options"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  {
    char const * const dummy = N_("clip to bounding box|#b");
    fdui->button_clip = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 145, 190, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("draft mode|#m");
    fdui->button_draft = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 20, 190, 30, 30, idex(_(dummy)));
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
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 15, 10, 470, 210, _("Size"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);

  fdui->radio_size = fl_bgn_group();
  {
    char const * const dummy = N_("Default|#D");
    fdui->button_default = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 50, 20, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Custom|#C");
    fdui->button_wh = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 50, 80, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Scale|#S");
    fdui->button_scale = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 50, 175, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_group();

  {
    char const * const dummy = N_("Width:|#W");
    fdui->input_width = obj = fl_add_input(FL_NORMAL_INPUT, 325, 80, 85, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->choice_width_units = obj = fl_add_choice(FL_NORMAL_CHOICE, 415, 80, 60, 30, "");
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Height:|#H");
    fdui->input_height = obj = fl_add_input(FL_NORMAL_INPUT, 325, 115, 85, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->choice_height_units = obj = fl_add_choice(FL_NORMAL_CHOICE, 415, 115, 60, 30, "");
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Value:|#V");
    fdui->input_scale = obj = fl_add_input(FL_NORMAL_INPUT, 325, 175, 85, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  // xgettext:no-c-format
  obj = fl_add_text(FL_NORMAL_TEXT, 420, 175, 25, 30, _("%"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  {
    char const * const dummy = N_("keep aspectratio|#k");
    fdui->check_aspectratio = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 85, 115, 30, 30, idex(_(dummy)));
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
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 20, 25, 455, 195, _("Bounding Box"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  obj = fl_add_text(FL_NORMAL_TEXT, 135, 35, 35, 25, _("X"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT, 360, 35, 30, 25, _("Y"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  {
    char const * const dummy = N_("Left Bottom:|#L");
    fdui->input_bb_x0 = obj = fl_add_input(FL_NORMAL_INPUT, 135, 60, 50, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->choice_bb_x0 = obj = fl_add_choice(FL_NORMAL_CHOICE, 190, 60, 50, 30, "");
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Right Top:|#R");
    fdui->input_bb_x1 = obj = fl_add_input(FL_NORMAL_INPUT, 135, 100, 50, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->choice_bb_x1 = obj = fl_add_choice(FL_NORMAL_CHOICE, 190, 100, 50, 30, "");
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Left Bottom:|#f");
    fdui->input_bb_y0 = obj = fl_add_input(FL_NORMAL_INPUT, 360, 60, 50, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->choice_bb_y0 = obj = fl_add_choice(FL_NORMAL_CHOICE, 415, 60, 45, 30, "");
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Right Top:|#T");
    fdui->input_bb_y1 = obj = fl_add_input(FL_NORMAL_INPUT, 360, 100, 50, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->choice_bb_y1 = obj = fl_add_choice(FL_NORMAL_CHOICE, 415, 100, 45, 30, "");
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Get bounding box from file|#G");
    fdui->button_getBB = obj = fl_add_button(FL_NORMAL_BUTTON, 135, 160, 220, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
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
    char const * const dummy = N_("LaTeX Options:|#L");
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
  obj = fl_add_labelframe(FL_ENGRAVED_FRAME, 15, 10, 470, 205, _("Screen Display"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  {
    char const * const dummy = N_("Show:|#o");
    fdui->choice_display = obj = fl_add_choice(FL_NORMAL_CHOICE, 30, 45, 100, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Width:|#w");
    fdui->input_lyxwidth = obj = fl_add_input(FL_NORMAL_INPUT, 305, 70, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->choice_width_lyxwidth = obj = fl_add_choice(FL_NORMAL_CHOICE, 410, 70, 60, 30, "");
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Height:|#h");
    fdui->input_lyxheight = obj = fl_add_input(FL_NORMAL_INPUT, 305, 110, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->choice_width_lyxheight = obj = fl_add_choice(FL_NORMAL_CHOICE, 410, 110, 60, 30, "");
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Value:|#V");
    fdui->input_lyxscale = obj = fl_add_input(FL_NORMAL_INPUT, 305, 175, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
 fl_bgn_group();
  obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 205, 20, 30, 30, "Default|#D");
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 205, 70, 30, 30, "Custom|#C");
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Scale|#S");
    fdui->button_lyxscale = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 205, 175, 30, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_COL1, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_group();

  // xgettext:no-c-format
  obj = fl_add_text(FL_NORMAL_TEXT, 410, 175, 25, 30, _("%"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

