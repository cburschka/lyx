// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "xforms_helpers.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_maths_style.h"
#include "FormMathsStyle.h"

FD_form_maths_style::~FD_form_maths_style()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_maths_style * FormMathsStyle::build_maths_style()
{
  FL_OBJECT *obj;
  FD_form_maths_style *fdui = new FD_form_maths_style;

  fdui->form = fl_bgn_form(FL_NO_BOX, 373, 251);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 373, 251, "");
  {
    char const * const dummy = N_("Apply|#A");
    fdui->button_apply = obj = fl_add_button(FL_NORMAL_BUTTON, 152, 212, 80, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedApplyCB, 0);
  fdui->button_ok = obj = fl_add_button(FL_RETURN_BUTTON, 62, 212, 80, 30, _("OK "));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedOKCB, 0);
  {
    char const * const dummy = N_("Cancel|^[");
    fdui->button_cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 244, 212, 80, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedCancelCB, 0);

  fdui->styles = fl_bgn_group();
  {
    char const * const dummy = N_("Display|#D");
    fdui->radio_display = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 15, 15, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_MCOL, FL_YELLOW);
    fl_set_object_lsize(obj, FL_MEDIUM_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 1);
  {
    char const * const dummy = N_("Text|#T");
    fdui->radio_text = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 15, 45, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_MCOL, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 0);
    fl_set_button(obj, 1);
  {
    char const * const dummy = N_("Script|#S");
    fdui->radio_script = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 15, 75, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_MCOL, FL_YELLOW);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 2);
  {
    char const * const dummy = N_("Bold|#B");
    fdui->radio_bold = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 140, 15, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_MCOL, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 4);
  {
    char const * const dummy = N_("Calligraphy|#C");
    fdui->radio_calligraphic = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 140, 45, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_MCOL, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, 14);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 5);
  {
    char const * const dummy = N_("Roman|#m");
    fdui->radio_roman = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 140, 75, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_MCOL, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_TIMES_STYLE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 6);
  {
    char const * const dummy = N_("scriptscript|#p");
    fdui->radio_scriptscript = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 15, 105, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_MCOL, FL_YELLOW);
    fl_set_object_lsize(obj, FL_TINY_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 3);
  {
    char const * const dummy = N_("Fixed|#x");
    fdui->radio_typewriter = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 140, 105, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_MCOL, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_FIXED_STYLE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 7);
  {
    char const * const dummy = N_("Italic|#I");
    fdui->radio_italic = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 140, 165, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_MCOL, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_TIMESITALIC_STYLE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 9);
  {
    char const * const dummy = N_("BB Bold|#o");
    fdui->radio_bbbold = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 260, 15, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_MCOL, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 10);
  {
    char const * const dummy = N_("Fraktur|#F");
    fdui->radio_fraktur = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 260, 45, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_MCOL, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 11);
  {
    char const * const dummy = N_("Reset|#R");
    fdui->radio_normal = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 260, 165, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_MCOL, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 13);
  {
    char const * const dummy = N_("textrm|#e");
    fdui->radio_textrm = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 260, 105, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_MCOL, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 12);
  {
    char const * const dummy = N_("Sans Serif|#n");
    fdui->radio_sans = obj = fl_add_round3dbutton(FL_RADIO_BUTTON, 140, 135, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_MCOL, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, 8);
  fl_end_group();

  obj = fl_add_frame(FL_ENGRAVED_FRAME, 136, 16, 226, 184, "");
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 14, 16, 110, 124, "");
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

