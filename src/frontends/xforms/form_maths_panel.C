// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_maths_panel.h"
#include "FormMathsPanel.h"

FD_form_maths_panel::~FD_form_maths_panel()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_maths_panel * FormMathsPanel::build_maths_panel()
{
  FL_OBJECT *obj;
  FD_form_maths_panel *fdui = new FD_form_maths_panel;

  fdui->form = fl_bgn_form(FL_NO_BOX, 274, 226);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 274, 226, "");
  fdui->button_close = obj = fl_add_button(FL_RETURN_BUTTON, 100, 180, 80, 30, _("Close "));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedCancelCB, 0);
  fdui->browser_funcs = obj = fl_add_browser(FL_SELECT_BROWSER, 170, 30, 90, 140, _("Functions"));
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, MM_FUNC);
    fl_set_browser_hscrollbar(obj, FL_OFF);
  fdui->button_greek = obj = fl_add_button(FL_NORMAL_BUTTON, 60, 80, 50, 30, _("Greek"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, 15);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, MM_GREEK);
  fdui->button_arrow = obj = fl_add_button(FL_NORMAL_BUTTON, 110, 110, 50, 30, _("­ Û"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, 15);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, MM_ARROW);
  fdui->button_boperator = obj = fl_add_button(FL_NORMAL_BUTTON, 10, 110, 50, 30, _("± ´"));
    fl_set_object_lsize(obj, 0);
    fl_set_object_lstyle(obj, 15);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, MM_BOP);
  fdui->button_brelats = obj = fl_add_button(FL_NORMAL_BUTTON, 60, 110, 50, 30, _("£ @"));
    fl_set_object_lsize(obj, 0);
    fl_set_object_lstyle(obj, 15);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, MM_BRELATS);
  fdui->button_misc = obj = fl_add_button(FL_NORMAL_BUTTON, 110, 140, 50, 30, _("Misc"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_TIMESITALIC_STYLE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, MM_MISC);
  fdui->button_equation = obj = fl_add_pixmapbutton(FL_NORMAL_BUTTON, 130, 50, 30, 30, "");
    fl_set_object_color(obj, FL_MCOL, FL_BLUE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, MM_EQU);
  fdui->button_sqrt = obj = fl_add_pixmapbutton(FL_NORMAL_BUTTON, 10, 20, 30, 30, "");
    fl_set_object_color(obj, FL_MCOL, FL_BLUE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, MM_SQRT);
  fdui->button_frac = obj = fl_add_pixmapbutton(FL_NORMAL_BUTTON, 40, 20, 30, 30, "");
    fl_set_object_color(obj, FL_MCOL, FL_BLUE);
    fl_set_object_lcolor(obj, FL_COL1);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, MM_FRAC);
  fdui->button_delim = obj = fl_add_pixmapbutton(FL_NORMAL_BUTTON, 70, 20, 30, 30, "");
    fl_set_object_color(obj, FL_MCOL, FL_BLUE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, MM_DELIM);
  fdui->button_matrix = obj = fl_add_pixmapbutton(FL_NORMAL_BUTTON, 10, 50, 30, 30, "");
    fl_set_object_color(obj, FL_MCOL, FL_BLUE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, MM_MATRIX);
  fdui->button_deco = obj = fl_add_pixmapbutton(FL_NORMAL_BUTTON, 100, 20, 30, 30, "");
    fl_set_object_color(obj, FL_MCOL, FL_BLUE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, MM_DECO);
  fdui->button_space = obj = fl_add_pixmapbutton(FL_NORMAL_BUTTON, 130, 20, 30, 30, "");
    fl_set_object_color(obj, FL_MCOL, FL_BLUE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, MM_SPACE);
  fdui->button_dots = obj = fl_add_button(FL_NORMAL_BUTTON, 60, 140, 50, 30, _("Dots"));
    fl_set_object_lsize(obj, 0);
    fl_set_object_lstyle(obj, FL_TIMESITALIC_STYLE);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, MM_DOTS);
  fdui->button_varsize = obj = fl_add_button(FL_NORMAL_BUTTON, 10, 140, 50, 30, _("S  ò"));
    fl_set_object_lsize(obj, 0);
    fl_set_object_lstyle(obj, 15);
    fl_set_object_callback(obj, C_FormBaseDeprecatedInputCB, MM_VARSIZE);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

