// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_splash.h"
#include "FormSplash.h"

FD_form_splash::~FD_form_splash()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_splash * FormSplash::build_splash()
{
  FL_OBJECT *obj;
  FD_form_splash *fdui = new FD_form_splash;

  fdui->form = fl_bgn_form(FL_NO_BOX, 420, 290);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 420, 290, "");
    fl_set_object_color(obj, FL_BLACK, FL_TOP_BCOL);
  fdui->splash_timer = obj = fl_add_timer(FL_HIDDEN_TIMER, 110, 170, 190, 60, "");
    fl_set_object_callback(obj, C_FormSplashTimerCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

