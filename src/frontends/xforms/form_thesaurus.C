// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_thesaurus.h"
#include "FormThesaurus.h"

FD_form_tabbed_thesaurus::~FD_form_tabbed_thesaurus()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_tabbed_thesaurus * FormThesaurus::build_tabbed_thesaurus()
{
  FL_OBJECT *obj;
  FD_form_tabbed_thesaurus *fdui = new FD_form_tabbed_thesaurus;

  fdui->form = fl_bgn_form(FL_NO_BOX, 465, 450);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 465, 450, "");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
  fdui->tabbed_folder = obj = fl_add_tabfolder(FL_TOP_TABFOLDER, 15, 55, 440, 310, _("Tabbed folder"));
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
  {
    char const * const dummy = N_("Replace|^R");
    fdui->button_replace = obj = fl_add_button(FL_NORMAL_BUTTON, 355, 375, 100, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  {
    char const * const dummy = N_("Close|^C");
    fdui->button_close = obj = fl_add_button(FL_NORMAL_BUTTON, 180, 410, 100, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
    fl_set_object_callback(obj, C_FormBaseOKCB, 0);
  fdui->input_entry = obj = fl_add_input(FL_NORMAL_INPUT, 60, 10, 315, 25, _("Entry : "));
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_replace = obj = fl_add_input(FL_NORMAL_INPUT, 75, 375, 260, 30, _("Selection :"));
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_noun::~FD_form_noun()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_noun * FormThesaurus::build_noun()
{
  FL_OBJECT *obj;
  FD_form_noun *fdui = new FD_form_noun;

  fdui->form = fl_bgn_form(FL_NO_BOX, 440, 290);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 440, 290, "");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
  fdui->browser_noun = obj = fl_add_browser(FL_HOLD_BROWSER, 0, 0, 440, 290, "");
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_verb::~FD_form_verb()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_verb * FormThesaurus::build_verb()
{
  FL_OBJECT *obj;
  FD_form_verb *fdui = new FD_form_verb;

  fdui->form = fl_bgn_form(FL_NO_BOX, 440, 290);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 440, 290, "");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
  fdui->browser_verb = obj = fl_add_browser(FL_HOLD_BROWSER, 0, 0, 440, 290, "");
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_adjective::~FD_form_adjective()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_adjective * FormThesaurus::build_adjective()
{
  FL_OBJECT *obj;
  FD_form_adjective *fdui = new FD_form_adjective;

  fdui->form = fl_bgn_form(FL_NO_BOX, 440, 290);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 440, 290, "");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
  fdui->browser_adjective = obj = fl_add_browser(FL_HOLD_BROWSER, 0, 0, 440, 290, "");
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_adverb::~FD_form_adverb()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_adverb * FormThesaurus::build_adverb()
{
  FL_OBJECT *obj;
  FD_form_adverb *fdui = new FD_form_adverb;

  fdui->form = fl_bgn_form(FL_NO_BOX, 440, 290);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 440, 290, "");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
  fdui->browser_adverb = obj = fl_add_browser(FL_HOLD_BROWSER, 0, 0, 440, 290, "");
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_other::~FD_form_other()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_other * FormThesaurus::build_other()
{
  FL_OBJECT *obj;
  FD_form_other *fdui = new FD_form_other;

  fdui->form = fl_bgn_form(FL_NO_BOX, 440, 290);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 440, 290, "");
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthWest);
  fdui->browser_other = obj = fl_add_browser(FL_HOLD_BROWSER, 0, 0, 440, 290, "");
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

