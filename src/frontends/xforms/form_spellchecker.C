// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "xforms_helpers.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_spellchecker.h"
#include "FormSpellchecker.h"

FD_form_spellchecker::~FD_form_spellchecker()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_spellchecker * FormSpellchecker::build_spellchecker()
{
  FL_OBJECT *obj;
  FD_form_spellchecker *fdui = new FD_form_spellchecker;

  fdui->form = fl_bgn_form(FL_NO_BOX, 540, 280);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 540, 280, "");
    fl_set_object_lsize(obj, 0);
  fdui->text = obj = fl_add_text(FL_NORMAL_TEXT, 80, 10, 220, 30, "");
    fl_set_object_boxtype(obj, FL_DOWN_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input = obj = fl_add_input(FL_NORMAL_INPUT, 80, 40, 220, 30, _("Replace"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->browser = obj = fl_add_browser(FL_SELECT_BROWSER, 80, 70, 220, 150, _("Suggestions"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Spellchecker Options...|#O");
    fdui->button_options = obj = fl_add_button(FL_NORMAL_BUTTON, 310, 210, 220, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Start spellchecking|#S");
    fdui->button_start = obj = fl_add_button(FL_NORMAL_BUTTON, 310, 10, 220, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Insert in personal dictionary|#I");
    fdui->button_insert = obj = fl_add_button(FL_NORMAL_BUTTON, 310, 50, 220, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Ignore word|#g");
    fdui->button_ignore = obj = fl_add_button(FL_NORMAL_BUTTON, 310, 110, 220, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Accept word in this session|#A");
    fdui->button_accept = obj = fl_add_button(FL_NORMAL_BUTTON, 310, 80, 220, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Stop spellchecking|#T");
    fdui->button_stop = obj = fl_add_button(FL_NORMAL_BUTTON, 310, 180, 220, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Close Spellchecker|#C^[");
    fdui->button_close = obj = fl_add_button(FL_NORMAL_BUTTON, 310, 240, 220, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseCancelCB, 0);
  // xgettext:no-c-format
  obj = fl_add_box(FL_NO_BOX, 10, 250, 50, 20, _("0 %"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
  // xgettext:no-c-format
  obj = fl_add_box(FL_NO_BOX, 250, 250, 50, 20, _("100 %"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT|FL_ALIGN_INSIDE);
  {
    char const * const dummy = N_("Replace word|#R");
    fdui->button_replace = obj = fl_add_button(FL_NORMAL_BUTTON, 310, 140, 220, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->slider = obj = fl_add_slider(FL_HOR_FILL_SLIDER, 10, 230, 290, 20, "");
    fl_set_object_color(obj, FL_BLUE, FL_COL1);
    fl_set_object_lsize(obj, 0);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

