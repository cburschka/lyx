// File modified by fdfix.sh for use by lyx (with xforms 0.81) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "lyx.h"

FD_form_title *create_form_form_title(void)
{
  FL_OBJECT *obj;
  FD_form_title *fdui = (FD_form_title *) fl_calloc(1, sizeof(FD_form_title));

  fdui->form_title = fl_bgn_form(FL_NO_BOX, 425, 290);
  obj = fl_add_box(FL_UP_BOX, 0, 0, 425, 290, "");
    fl_set_object_color(obj, FL_BLACK, FL_TOP_BCOL);
  fdui->timer_title = obj = fl_add_timer(FL_HIDDEN_TIMER, 110, 170, 190, 60, "");
    fl_set_object_callback(obj, TimerCB, 0);
  fl_end_form();

  //fdui->form_title->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_figure *create_form_form_figure(void)
{
  FL_OBJECT *obj;
  FD_form_figure *fdui = (FD_form_figure *) fl_calloc(1, sizeof(FD_form_figure));

  fdui->form_figure = fl_bgn_form(FL_NO_BOX, 340, 150);
  obj = fl_add_box(FL_UP_BOX, 0, 0, 340, 150, "");
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 20, 320, 80, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);

  fdui->group_radio_fugre = fl_bgn_group();
  fdui->radio_postscript = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 10, 30, 320, 30, idex(_("Encapsulated Postscript (*.eps, *.ps)|#E")));fl_set_button_shortcut(obj, scex(_("Encapsulated Postscript (*.eps, *.ps)|#E")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->radio_inline = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 10, 60, 320, 30, idex(_("Inlined EPS (*.eps, *.ps)|#I")));fl_set_button_shortcut(obj, scex(_("Inlined EPS (*.eps, *.ps)|#I")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_end_group();

  obj = fl_add_button(FL_RETURN_BUTTON, 10, 110, 100, 30, _("OK"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, FigureOKCB, 0);
  obj = fl_add_button(FL_NORMAL_BUTTON, 120, 110, 100, 30, idex(_("Apply|#A")));fl_set_button_shortcut(obj, scex(_("Apply|#A")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, FigureApplyCB, 0);
  obj = fl_add_button(FL_NORMAL_BUTTON, 230, 110, 100, 30, idex(_("Cancel|^[")));fl_set_button_shortcut(obj, scex(_("Cancel|^[")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, FigureCancelCB, 0);
  obj = fl_add_text(FL_NORMAL_TEXT, 20, 10, 50, 20, _("Type"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fl_end_form();

  //fdui->form_figure->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_screen *create_form_form_screen(void)
{
  FL_OBJECT *obj;
  FD_form_screen *fdui = (FD_form_screen *) fl_calloc(1, sizeof(FD_form_screen));

  fdui->form_screen = fl_bgn_form(FL_NO_BOX, 390, 220);
  obj = fl_add_box(FL_UP_BOX, 0, 0, 390, 220, "");
  fdui->input_roman = obj = fl_add_input(FL_NORMAL_INPUT, 140, 10, 240, 30, idex(_("Roman Font|#R")));fl_set_button_shortcut(obj, scex(_("Roman Font|#R")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->input_sans = obj = fl_add_input(FL_NORMAL_INPUT, 140, 40, 240, 30, idex(_("Sans Serif Font|#S")));fl_set_button_shortcut(obj, scex(_("Sans Serif Font|#S")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->input_typewriter = obj = fl_add_input(FL_NORMAL_INPUT, 140, 70, 240, 30, idex(_("Typewriter Font|#T")));fl_set_button_shortcut(obj, scex(_("Typewriter Font|#T")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->input_font_norm = obj = fl_add_input(FL_NORMAL_INPUT, 140, 100, 240, 30, idex(_("Font Norm|#N")));fl_set_button_shortcut(obj, scex(_("Font Norm|#N")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->intinput_size = obj = fl_add_input(FL_INT_INPUT, 140, 140, 50, 30, idex(_("Font Zoom|#Z")));fl_set_button_shortcut(obj, scex(_("Font Zoom|#Z")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
  obj = fl_add_button(FL_RETURN_BUTTON, 60, 180, 100, 30, _("OK"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, ScreenOKCB, 0);
  obj = fl_add_button(FL_NORMAL_BUTTON, 170, 180, 100, 30, idex(_("Apply|#A")));fl_set_button_shortcut(obj, scex(_("Apply|#A")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, ScreenApplyCB, 0);
  obj = fl_add_button(FL_NORMAL_BUTTON, 280, 180, 100, 30, idex(_("Cancel|^[")));fl_set_button_shortcut(obj, scex(_("Cancel|^[")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, ScreenCancelCB, 0);
  fl_end_form();

  //fdui->form_screen->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_toc *create_form_form_toc(void)
{
  FL_OBJECT *obj;
  FD_form_toc *fdui = (FD_form_toc *) fl_calloc(1, sizeof(FD_form_toc));

  fdui->form_toc = fl_bgn_form(FL_NO_BOX, 420, 340);
  obj = fl_add_box(FL_UP_BOX, 0, 0, 420, 340, "");
  fdui->browser_toc = obj = fl_add_browser(FL_HOLD_BROWSER, 10, 10, 400, 280, "");
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
    fl_set_object_callback(obj, TocSelectCB, 0);
  obj = fl_add_button(FL_RETURN_BUTTON, 310, 300, 100, 30, _("Close"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, TocCancelCB, 0);
  obj = fl_add_button(FL_NORMAL_BUTTON, 200, 300, 100, 30, idex(_("Update|Uu#u")));fl_set_button_shortcut(obj, scex(_("Update|Uu#u")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, TocUpdateCB, 0);
  fdui->toctype = obj = fl_add_choice(FL_NORMAL_CHOICE, 60, 300, 130, 30, idex(_("Type|Tt#t")));fl_set_button_shortcut(obj,scex(_("Type|Tt#t")),1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_callback(obj, TocUpdateCB, 0);
  fl_end_form();

  //fdui->form_toc->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_ref *create_form_form_ref(void)
{
  FL_OBJECT *obj;
  FD_form_ref *fdui = (FD_form_ref *) fl_calloc(1, sizeof(FD_form_ref));

  fdui->form_ref = fl_bgn_form(FL_NO_BOX, 590, 400);
  obj = fl_add_box(FL_UP_BOX, 0, 0, 590, 400, "");
  fdui->browser_ref = obj = fl_add_browser(FL_HOLD_BROWSER, 10, 10, 280, 380, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  obj = fl_add_button(FL_NORMAL_BUTTON, 480, 60, 100, 30, idex(_("Update|#U")));fl_set_button_shortcut(obj, scex(_("Update|#U")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, RefUpdateCB, 0);
  obj = fl_add_button(FL_NORMAL_BUTTON, 480, 100, 100, 30, idex(_("Close|#C^[")));fl_set_button_shortcut(obj, scex(_("Close|#C^[")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, RefHideCB, 0);
  obj = fl_add_button(FL_NORMAL_BUTTON, 310, 60, 160, 30, idex(_("Insert Reference|#I^M")));fl_set_button_shortcut(obj, scex(_("Insert Reference|#I^M")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthWest);
    fl_set_object_callback(obj, RefSelectCB, 0);
  obj = fl_add_button(FL_NORMAL_BUTTON, 310, 100, 160, 30, idex(_("Insert Page Number|#P")));fl_set_button_shortcut(obj, scex(_("Insert Page Number|#P")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthWest);
    fl_set_object_callback(obj, RefSelectCB, 1);
  obj = fl_add_button(FL_NORMAL_BUTTON, 310, 280, 160, 30, idex(_("Go to Reference|#G")));fl_set_button_shortcut(obj, scex(_("Go to Reference|#G")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthWest);
    fl_set_object_callback(obj, RefSelectCB, 5);
  fdui->ref_name = obj = fl_add_input(FL_NORMAL_INPUT, 380, 20, 200, 30, idex(_("Name|#N")));fl_set_button_shortcut(obj, scex(_("Name|#N")), 1);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->sort = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 310, 360, 110, 30, idex(_("Sort keys|#S")));fl_set_button_shortcut(obj, scex(_("Sort keys|#S")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, RefUpdateCB, 0);
    fl_set_button(obj, 1);
  fdui->vref = obj = fl_add_button(FL_NORMAL_BUTTON, 310, 140, 160, 30, idex(_("Insert vReference|#V")));fl_set_button_shortcut(obj, scex(_("Insert vReference|#V")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthWest);
    fl_set_object_callback(obj, RefSelectCB, 2);
  fdui->vpageref = obj = fl_add_button(FL_NORMAL_BUTTON, 310, 180, 160, 30, idex(_("Insert vPage Number|#N")));fl_set_button_shortcut(obj, scex(_("Insert vPage Number|#N")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthWest);
    fl_set_object_callback(obj, RefSelectCB, 3);
  fdui->prettyref = obj = fl_add_button(FL_NORMAL_BUTTON, 310, 220, 160, 30, idex(_("Insert Pretty Ref|#T")));fl_set_button_shortcut(obj, scex(_("Insert Pretty Ref|#T")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthWest);
    fl_set_object_callback(obj, RefSelectCB, 4);
  fl_end_form();

  //fdui->form_ref->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

