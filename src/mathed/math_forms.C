// File modified by fdfix.sh for use by lyx (with xforms 0.81) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "math_panel.h"

FD_panel *create_form_panel(void)
{
  FL_OBJECT *obj;
  FD_panel *fdui = (FD_panel *) fl_calloc(1, sizeof(FD_panel));

  fdui->panel = fl_bgn_form(FL_NO_BOX, 290, 172);
  obj = fl_add_box(FL_UP_BOX,0,0,290,172,"");
  obj = fl_add_button(FL_RETURN_BUTTON,80,14,80,30,_("Close "));
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,button_cb,100);
  fdui->func_browse = obj = fl_add_browser(FL_SELECT_BROWSER,180,40,94,110,_("Functions"));
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_callback(obj,button_cb,MM_FUNC);
#if FL_REVISION > 85
    fl_set_browser_hscrollbar(obj, FL_OFF);
#endif
  fdui->greek = obj = fl_add_button(FL_NORMAL_BUTTON,10,90,50,30,_("Greek"));
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,15);
    fl_set_object_callback(obj,button_cb,MM_GREEK);
  fdui->arrow = obj = fl_add_button(FL_NORMAL_BUTTON,10,120,50,30,_("­ Û"));
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,15);
    fl_set_object_callback(obj,button_cb,MM_ARROW);
  fdui->boperator = obj = fl_add_button(FL_NORMAL_BUTTON,60,90,50,30,_("± ´"));
    fl_set_object_lsize(obj,0);
    fl_set_object_lstyle(obj,15);
    fl_set_object_callback(obj,button_cb,MM_BOP);
  fdui->brelats = obj = fl_add_button(FL_NORMAL_BUTTON,110,90,50,30,_("£ @"));
    fl_set_object_lsize(obj,0);
    fl_set_object_lstyle(obj,15);
    fl_set_object_callback(obj,button_cb,MM_BRELATS);
  fdui->varsize = obj = fl_add_button(FL_NORMAL_BUTTON,60,120,50,30,_("S  ò"));
    fl_set_object_lsize(obj,0);
    fl_set_object_lstyle(obj,15);
    fl_set_object_callback(obj,button_cb,MM_VARSIZE);
  fdui->misc = obj = fl_add_button(FL_NORMAL_BUTTON,110,120,50,30,_("Misc"));
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_TIMESITALIC_STYLE);
    fl_set_object_callback(obj,button_cb,MM_MISC);
  fdui->equation = obj = fl_add_pixmapbutton(FL_NORMAL_BUTTON,130,50,30,30,"");
    fl_set_object_color(obj,FL_MCOL,FL_BLUE);
    fl_set_object_callback(obj,button_cb,MM_EQU);
  fdui->sqrt = obj = fl_add_pixmapbutton(FL_NORMAL_BUTTON,10,15,30,30,"");
    fl_set_object_color(obj,FL_MCOL,FL_BLUE);
    fl_set_object_callback(obj,button_cb,MM_SQRT);
  fdui->frac = obj = fl_add_pixmapbutton(FL_NORMAL_BUTTON,40,15,30,30,"");
    fl_set_object_color(obj,FL_MCOL,FL_BLUE);
    fl_set_object_lcol(obj,FL_COL1);
    fl_set_object_callback(obj,button_cb,MM_FRAC);
  fdui->delim = obj = fl_add_pixmapbutton(FL_NORMAL_BUTTON,10,50,30,30,"");
    fl_set_object_color(obj,FL_MCOL,FL_BLUE);
    fl_set_object_callback(obj,button_cb,MM_DELIM);
  fdui->matrix = obj = fl_add_pixmapbutton(FL_NORMAL_BUTTON,100,50,30,30,"");
    fl_set_object_color(obj,FL_MCOL,FL_BLUE);
    fl_set_object_callback(obj,button_cb,MM_MATRIX);
  fdui->deco = obj = fl_add_pixmapbutton(FL_NORMAL_BUTTON,40,50,30,30,"");
    fl_set_object_color(obj,FL_MCOL,FL_BLUE);
    fl_set_object_callback(obj,button_cb,MM_DECO);
  fdui->space = obj = fl_add_pixmapbutton(FL_NORMAL_BUTTON,70,50,30,30,"");
    fl_set_object_color(obj,FL_MCOL,FL_BLUE);
    fl_set_object_callback(obj,button_cb,MM_SPACE);
  fl_end_form();

  //fdui->panel->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_delim *create_form_delim(void)
{
  FL_OBJECT *obj;
  FD_delim *fdui = (FD_delim *) fl_calloc(1, sizeof(FD_delim));

  fdui->delim = fl_bgn_form(FL_NO_BOX, 250, 260);
  obj = fl_add_box(FL_UP_BOX,0,0,250,260,"");
  fdui->menu = obj = fl_add_bmtable(FL_PUSH_BUTTON,40,70,170,140,"");
    fl_set_object_lcol(obj,FL_BLUE);
    fl_set_object_callback(obj,delim_cb,2);

  fdui->lado = fl_bgn_group();
  fdui->right = obj = fl_add_checkbutton(FL_RADIO_BUTTON,40,40,80,30,idex(_("Right|#R")));fl_set_button_shortcut(obj,scex(_("Right|#R")),1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,delim_cb,4);
  fdui->left = obj = fl_add_checkbutton(FL_RADIO_BUTTON,40,10,80,30,idex(_("Left|#L")));fl_set_button_shortcut(obj,scex(_("Left|#L")),1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,delim_cb,3);
  fl_end_group();

  obj = fl_add_button(FL_NORMAL_BUTTON,170,220,70,30,idex(_("Cancel|^[")));fl_set_button_shortcut(obj,scex(_("Cancel|^[")),1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,delim_cb,MM_CLOSE);
  fdui->pix = obj = fl_add_pixmapbutton(FL_NORMAL_BUTTON,130,20,50,40,"");
    fl_set_object_color(obj,FL_COL1,FL_COL1);
    fl_set_object_callback(obj,delim_cb,MM_APPLY);
  obj = fl_add_button(FL_NORMAL_BUTTON,90,220,70,30,idex(_("Apply|#A")));fl_set_button_shortcut(obj,scex(_("Apply|#A")),1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,delim_cb,MM_APPLY);
  obj = fl_add_button(FL_RETURN_BUTTON,10,220,70,30,_("OK"));
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,delim_cb,MM_OK);
  fl_end_form();

  //fdui->delim->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_matrix *create_form_matrix(void)
{
  FL_OBJECT *obj;
  FD_matrix *fdui = (FD_matrix *) fl_calloc(1, sizeof(FD_matrix));

  fdui->matrix = fl_bgn_form(FL_NO_BOX, 280, 170);
  obj = fl_add_box(FL_UP_BOX,0,0,280,170,"");
  obj = fl_add_button(FL_RETURN_BUTTON,10,130,80,30,_("OK  "));
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,matrix_cb,MM_OK);
  obj = fl_add_button(FL_NORMAL_BUTTON,190,130,80,30,idex(_("Cancel|^[")));fl_set_button_shortcut(obj,scex(_("Cancel|^[")),1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,matrix_cb,MM_CLOSE);
  fdui->rows = obj = fl_add_valslider(FL_HOR_NICE_SLIDER,10,30,160,30,_("Rows"));
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_callback(obj,matrix_cb,-1);
    fl_set_slider_precision(obj, 0);
    fl_set_slider_bounds(obj, 1, 20);
     fl_set_slider_return(obj, FL_RETURN_END_CHANGED);
  fdui->columns = obj = fl_add_valslider(FL_HOR_NICE_SLIDER,10,90,160,30,_("Columns "));
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_callback(obj,matrix_cb,2);
    fl_set_slider_precision(obj, 0);
    fl_set_slider_bounds(obj, 1, 20);
     fl_set_slider_return(obj, FL_RETURN_END_CHANGED);
  fdui->valign = obj = fl_add_choice(FL_NORMAL_CHOICE,180,30,90,30,idex(_("Vertical align|#V")));fl_set_button_shortcut(obj,scex(_("Vertical align|#V")),1);
    fl_set_object_boxtype(obj,FL_FRAME_BOX);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_callback(obj,matrix_cb,-1);
  fdui->halign = obj = fl_add_input(FL_NORMAL_INPUT,180,90,90,30,idex(_("Horizontal align|#H")));fl_set_button_shortcut(obj,scex(_("Horizontal align|#H")),1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP);
    fl_set_object_callback(obj,matrix_cb,2);
  obj = fl_add_button(FL_NORMAL_BUTTON,100,130,80,30,idex(_("Apply|#A")));fl_set_button_shortcut(obj,scex(_("Apply|#A")),1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,matrix_cb,MM_APPLY);
  fl_end_form();

  //fdui->matrix->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_deco *create_form_deco(void)
{
  FL_OBJECT *obj;
  FD_deco *fdui = (FD_deco *) fl_calloc(1, sizeof(FD_deco));

  fdui->deco = fl_bgn_form(FL_NO_BOX, 160, 150);
  obj = fl_add_box(FL_UP_BOX,0,0,160,150,"");
  fdui->menu = obj = fl_add_bmtable(FL_PUSH_BUTTON,10,10,140,90,"");
    fl_set_object_lcol(obj,FL_BLUE);
    fl_set_object_callback(obj,deco_cb,MM_APPLY);
  obj = fl_add_button(FL_RETURN_BUTTON,30,110,100,30,_("Close"));
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,deco_cb,MM_CLOSE);
  fl_end_form();

  //fdui->deco->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_space *create_form_space(void)
{
  FL_OBJECT *obj;
  FD_space *fdui = (FD_space *) fl_calloc(1, sizeof(FD_space));

  fdui->space = fl_bgn_form(FL_NO_BOX, 280, 150);
  obj = fl_add_box(FL_UP_BOX,0,0,280,150,"");
  obj = fl_add_button(FL_RETURN_BUTTON,10,110,80,30,_("OK "));
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,space_cb,MM_OK);
  obj = fl_add_button(FL_NORMAL_BUTTON,190,110,80,30,idex(_("Cancel|^[")));fl_set_button_shortcut(obj,scex(_("Cancel|^[")),1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,space_cb,MM_CLOSE);
  obj = fl_add_button(FL_NORMAL_BUTTON,100,110,80,30,idex(_("Apply|#A")));fl_set_button_shortcut(obj,scex(_("Apply|#A")),1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,space_cb,MM_APPLY);

  fdui->spaces = fl_bgn_group();
  obj = fl_add_checkbutton(FL_RADIO_BUTTON,20,10,120,30,idex(_("Thin|#T")));fl_set_button_shortcut(obj,scex(_("Thin|#T")),1);
    fl_set_object_color(obj,FL_MCOL,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,space_cb,1);
  obj = fl_add_checkbutton(FL_RADIO_BUTTON,20,40,120,30,idex(_("Medium|#M")));fl_set_button_shortcut(obj,scex(_("Medium|#M")),1);
    fl_set_object_color(obj,FL_MCOL,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,space_cb,2);
  obj = fl_add_checkbutton(FL_RADIO_BUTTON,20,70,120,30,idex(_("Thick|#H")));fl_set_button_shortcut(obj,scex(_("Thick|#H")),1);
    fl_set_object_color(obj,FL_MCOL,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,space_cb,3);
  obj = fl_add_checkbutton(FL_RADIO_BUTTON,140,10,120,30,idex(_("Negative|#N")));fl_set_button_shortcut(obj,scex(_("Negative|#N")),1);
    fl_set_object_color(obj,FL_MCOL,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,space_cb,0);
  obj = fl_add_checkbutton(FL_RADIO_BUTTON,140,40,120,30,idex(_("Quadratin|#Q")));fl_set_button_shortcut(obj,scex(_("Quadratin|#Q")),1);
    fl_set_object_color(obj,FL_MCOL,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,space_cb,4);
  obj = fl_add_checkbutton(FL_RADIO_BUTTON,140,70,120,30,idex(_("2Quadratin|#2")));fl_set_button_shortcut(obj,scex(_("2Quadratin|#2")),1);
    fl_set_object_color(obj,FL_MCOL,FL_YELLOW);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,space_cb,5);
  fl_end_group();

  fl_end_form();

  //fdui->space->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

