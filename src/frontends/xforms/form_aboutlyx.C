// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "xforms_helpers.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_aboutlyx.h"
#include "FormAboutlyx.h"

FD_form_aboutlyx::~FD_form_aboutlyx()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_aboutlyx * FormAboutlyx::build_aboutlyx()
{
  FL_OBJECT *obj;
  FD_form_aboutlyx *fdui = new FD_form_aboutlyx;

  fdui->form = fl_bgn_form(FL_NO_BOX, 450, 400);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 450, 400, "");
  {
    char const * const dummy = N_("Close|^[");
    fdui->button_close = obj = fl_add_button(FL_NORMAL_BUTTON, 320, 360, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseCancelCB, 0);
  fdui->tabfolder = obj = fl_add_tabfolder(FL_TOP_TABFOLDER, 10, 20, 430, 330, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_tab_version::~FD_form_tab_version()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_tab_version * FormAboutlyx::build_tab_version()
{
  FL_OBJECT *obj;
  FD_form_tab_version *fdui = new FD_form_tab_version;

  fdui->form = fl_bgn_form(FL_NO_BOX, 430, 310);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 430, 310, "");
  fdui->text_copyright = obj = fl_add_text(FL_NORMAL_TEXT, 10, 10, 410, 110, _("Text"));
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lalign(obj, FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  fdui->text_version = obj = fl_add_text(FL_NORMAL_TEXT, 10, 130, 410, 170, _("Text"));
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_tab_credits::~FD_form_tab_credits()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_tab_credits * FormAboutlyx::build_tab_credits()
{
  FL_OBJECT *obj;
  FD_form_tab_credits *fdui = new FD_form_tab_credits;

  fdui->form = fl_bgn_form(FL_NO_BOX, 430, 310);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 430, 310, "");
  fdui->browser_credits = obj = fl_add_browser(FL_NORMAL_BROWSER, 10, 10, 410, 290, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_tab_license::~FD_form_tab_license()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_tab_license * FormAboutlyx::build_tab_license()
{
  FL_OBJECT *obj;
  FD_form_tab_license *fdui = new FD_form_tab_license;

  fdui->form = fl_bgn_form(FL_NO_BOX, 430, 310);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 430, 310, "");
  fdui->text_license = obj = fl_add_text(FL_NORMAL_TEXT, 10, 10, 410, 100, _("Text"));
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lalign(obj, FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  fdui->text_warranty = obj = fl_add_text(FL_NORMAL_TEXT, 10, 120, 410, 180, _("Text"));
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lalign(obj, FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/
