// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"
#include "bmtable.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_maths.h"
#include "FormMaths.h"

FD_form_panel::~FD_form_panel()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_panel * FormMaths::build_panel()
{
  FL_OBJECT *obj;
  FD_form_panel *fdui = new FD_form_panel;

  fdui->form = fl_bgn_form(FL_NO_BOX, 290, 172);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 290, 172, "");
  fdui->button_close = obj = fl_add_button(FL_RETURN_BUTTON, 80, 14, 80, 30, _("Close "));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseCancelCB, 0);
  fdui->browser_functions = obj = fl_add_browser(FL_SELECT_BROWSER, 180, 40, 94, 110, _("Functions"));
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_callback(obj, C_FormMathsButtonCB, MM_FUNC);
    fl_set_browser_hscrollbar(obj, FL_OFF);
  fdui->button_greek = obj = fl_add_button(FL_NORMAL_BUTTON, 10, 90, 50, 30, _("Greek"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, 15);
    fl_set_object_callback(obj, C_FormMathsButtonCB, MM_GREEK);
  fdui->button_arrow = obj = fl_add_button(FL_NORMAL_BUTTON, 10, 120, 50, 30, _("­ Û"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, 15);
    fl_set_object_callback(obj, C_FormMathsButtonCB, MM_ARROW);
  fdui->button_boperator = obj = fl_add_button(FL_NORMAL_BUTTON, 60, 90, 50, 30, _("± ´"));
    fl_set_object_lsize(obj, 0);
    fl_set_object_lstyle(obj, 15);
    fl_set_object_callback(obj, C_FormMathsButtonCB, MM_BOP);
  fdui->button_brelats = obj = fl_add_button(FL_NORMAL_BUTTON, 110, 90, 50, 30, _("£ @"));
    fl_set_object_lsize(obj, 0);
    fl_set_object_lstyle(obj, 15);
    fl_set_object_callback(obj, C_FormMathsButtonCB, MM_BRELATS);
  fdui->button_varsize = obj = fl_add_button(FL_NORMAL_BUTTON, 60, 120, 50, 30, _("S  ò"));
    fl_set_object_lsize(obj, 0);
    fl_set_object_lstyle(obj, 15);
    fl_set_object_callback(obj, C_FormMathsButtonCB, MM_VARSIZE);
  fdui->button_misc = obj = fl_add_button(FL_NORMAL_BUTTON, 110, 120, 50, 30, _("Misc"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_TIMESITALIC_STYLE);
    fl_set_object_callback(obj, C_FormMathsButtonCB, MM_MISC);
  fdui->button_equation = obj = fl_add_pixmapbutton(FL_NORMAL_BUTTON, 130, 50, 30, 30, "");
    fl_set_object_color(obj, FL_MCOL, FL_BLUE);
    fl_set_object_callback(obj, C_FormMathsButtonCB, MM_EQU);
  fdui->button_sqrt = obj = fl_add_pixmapbutton(FL_NORMAL_BUTTON, 10, 15, 30, 30, "");
    fl_set_object_color(obj, FL_MCOL, FL_BLUE);
    fl_set_object_callback(obj, C_FormMathsButtonCB, MM_SQRT);
  fdui->button_frac = obj = fl_add_pixmapbutton(FL_NORMAL_BUTTON, 40, 15, 30, 30, "");
    fl_set_object_color(obj, FL_MCOL, FL_BLUE);
    fl_set_object_lcolor(obj, FL_COL1);
    fl_set_object_callback(obj, C_FormMathsButtonCB, MM_FRAC);
  fdui->button_delim = obj = fl_add_pixmapbutton(FL_NORMAL_BUTTON, 10, 50, 30, 30, "");
    fl_set_object_color(obj, FL_MCOL, FL_BLUE);
    fl_set_object_callback(obj, C_FormMathsButtonCB, MM_DELIM);
  fdui->button_matrix = obj = fl_add_pixmapbutton(FL_NORMAL_BUTTON, 100, 50, 30, 30, "");
    fl_set_object_color(obj, FL_MCOL, FL_BLUE);
    fl_set_object_callback(obj, C_FormMathsButtonCB, MM_MATRIX);
  fdui->button_deco = obj = fl_add_pixmapbutton(FL_NORMAL_BUTTON, 40, 50, 30, 30, "");
    fl_set_object_color(obj, FL_MCOL, FL_BLUE);
    fl_set_object_callback(obj, C_FormMathsButtonCB, MM_DECO);
  fdui->button_space = obj = fl_add_pixmapbutton(FL_NORMAL_BUTTON, 70, 50, 30, 30, "");
    fl_set_object_color(obj, FL_MCOL, FL_BLUE);
    fl_set_object_callback(obj, C_FormMathsButtonCB, MM_SPACE);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_delim::~FD_form_delim()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_delim * FormMaths::build_delim()
{
  FL_OBJECT *obj;
  FD_form_delim *fdui = new FD_form_delim;

  fdui->form = fl_bgn_form(FL_NO_BOX, 250, 260);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 250, 260, "");
  fdui->bmtable_delim = obj = fl_add_bmtable(FL_PUSH_BUTTON, 40, 70, 170, 140, "");
    fl_set_object_lcolor(obj, FL_BLUE);
    fl_set_object_callback(obj, C_FormMathsDelimCB, 2);

  fdui->lado = fl_bgn_group();
  {
    char const * const dummy = N_("Right|#R");
    fdui->radio_right = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 40, 40, 80, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormMathsDelimCB, 4);
  {
    char const * const dummy = N_("Left|#L");
    fdui->radio_left = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 40, 10, 80, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormMathsDelimCB, 3);
  fl_end_group();

  {
    char const * const dummy = N_("Cancel|^[");
    fdui->button_delim_close = obj = fl_add_button(FL_NORMAL_BUTTON, 170, 220, 70, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormMathsDelimCB, MM_CLOSE);
  fdui->button_delim_pix = obj = fl_add_pixmapbutton(FL_NORMAL_BUTTON, 130, 20, 50, 40, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
    fl_set_object_callback(obj, C_FormMathsDelimCB, MM_APPLY);
  {
    char const * const dummy = N_("Apply|#A");
    fdui->button_delim_apply = obj = fl_add_button(FL_NORMAL_BUTTON, 90, 220, 70, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormMathsDelimCB, MM_APPLY);
  fdui->button_delim_ok = obj = fl_add_button(FL_RETURN_BUTTON, 10, 220, 70, 30, _("OK"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormMathsDelimCB, MM_OK);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_matrix::~FD_form_matrix()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_matrix * FormMaths::build_matrix()
{
  FL_OBJECT *obj;
  FD_form_matrix *fdui = new FD_form_matrix;

  fdui->form = fl_bgn_form(FL_NO_BOX, 280, 170);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 280, 170, "");
  fdui->button_matrix_ok = obj = fl_add_button(FL_RETURN_BUTTON, 10, 130, 80, 30, _("OK  "));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormMathsMatrixCB, MM_OK);
  {
    char const * const dummy = N_("Cancel|^[");
    fdui->button_matrix_close = obj = fl_add_button(FL_NORMAL_BUTTON, 190, 130, 80, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormMathsMatrixCB, MM_CLOSE);
  fdui->slider_matrix_rows = obj = fl_add_valslider(FL_HOR_NICE_SLIDER, 10, 30, 160, 30, _("Rows"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_callback(obj, C_FormMathsMatrixCB, -1);
    fl_set_slider_precision(obj, 0);
    fl_set_slider_bounds(obj, 1, 20);
     fl_set_slider_return(obj, FL_RETURN_END_CHANGED);
  fdui->slider_matrix_columns = obj = fl_add_valslider(FL_HOR_NICE_SLIDER, 10, 90, 160, 30, _("Columns "));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_callback(obj, C_FormMathsMatrixCB, 2);
    fl_set_slider_precision(obj, 0);
    fl_set_slider_bounds(obj, 1, 20);
     fl_set_slider_return(obj, FL_RETURN_END_CHANGED);
  {
    char const * const dummy = N_("Vertical align|#V");
    fdui->choice_matrix_valign = obj = fl_add_choice(FL_NORMAL_CHOICE, 180, 30, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_callback(obj, C_FormMathsMatrixCB, -1);
  {
    char const * const dummy = N_("Horizontal align|#H");
    fdui->input_matrix_halign = obj = fl_add_input(FL_NORMAL_INPUT, 180, 90, 90, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_callback(obj, C_FormMathsMatrixCB, 2);
  {
    char const * const dummy = N_("Apply|#A");
    fdui->button_matrix_apply = obj = fl_add_button(FL_NORMAL_BUTTON, 100, 130, 80, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormMathsMatrixCB, MM_APPLY);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_deco::~FD_form_deco()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_deco * FormMaths::build_deco()
{
  FL_OBJECT *obj;
  FD_form_deco *fdui = new FD_form_deco;

  fdui->form = fl_bgn_form(FL_NO_BOX, 160, 150);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 160, 150, "");
  fdui->bmtable_deco = obj = fl_add_bmtable(FL_PUSH_BUTTON, 10, 10, 140, 90, "");
    fl_set_object_lcolor(obj, FL_BLUE);
    fl_set_object_callback(obj, C_FormMathsDecoCB, MM_APPLY);
  fdui->button_deco_close = obj = fl_add_button(FL_RETURN_BUTTON, 30, 110, 100, 30, _("Close"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormMathsDecoCB, MM_CLOSE);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_space::~FD_form_space()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_space * FormMaths::build_space()
{
  FL_OBJECT *obj;
  FD_form_space *fdui = new FD_form_space;

  fdui->form = fl_bgn_form(FL_NO_BOX, 280, 150);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 280, 150, "");
  fdui->button_space_menu = obj = fl_add_button(FL_RETURN_BUTTON, 10, 110, 80, 30, _("OK "));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormMathsSpaceCB, MM_OK);
  {
    char const * const dummy = N_("Cancel|^[");
    fdui->button_space_cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 190, 110, 80, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormMathsSpaceCB, MM_CLOSE);
  {
    char const * const dummy = N_("Apply|#A");
    fdui->button_space_apply = obj = fl_add_button(FL_NORMAL_BUTTON, 100, 110, 80, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormMathsSpaceCB, MM_APPLY);

  fdui->spaces = fl_bgn_group();
  {
    char const * const dummy = N_("Thin|#T");
    fdui->radio_space_thin = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 20, 10, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_MCOL, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormMathsSpaceCB, 1);
  {
    char const * const dummy = N_("Medium|#M");
    fdui->radio_space_medium = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 20, 40, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_MCOL, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormMathsSpaceCB, 2);
  {
    char const * const dummy = N_("Thick|#H");
    fdui->radio_space_thick = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 20, 70, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_MCOL, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormMathsSpaceCB, 3);
  {
    char const * const dummy = N_("Negative|#N");
    fdui->radio_space_negative = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 140, 10, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_MCOL, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormMathsSpaceCB, 0);
  {
    char const * const dummy = N_("Quadratin|#Q");
    fdui->radio_space_quadratin = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 140, 40, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_MCOL, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormMathsSpaceCB, 4);
  {
    char const * const dummy = N_("2Quadratin|#2");
    fdui->radio_space_twoquadratin = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 140, 70, 120, 30, idex(_(dummy)));
    fl_set_button_shortcut(obj, scex(_(dummy)), 1);
  }
    fl_set_object_color(obj, FL_MCOL, FL_YELLOW);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormMathsSpaceCB, 5);
  fl_end_group();

  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

