// File modified by fdfix.sh for use by lyx (with xforms 0.81) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <cstdlib>
#include "lyx.h"

FD_form_title *create_form_form_title(void)
{
  FL_OBJECT *obj;
  FD_form_title *fdui = (FD_form_title *) fl_calloc(1, sizeof(FD_form_title));

  fdui->form_title = fl_bgn_form(FL_NO_BOX, 420, 290);
  obj = fl_add_box(FL_UP_BOX, 0, 0, 420, 290, "");
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
  fdui->input_roman = obj = fl_add_input(FL_NORMAL_INPUT, 140, 10, 240, 30, idex(_("Roman font|#R")));fl_set_button_shortcut(obj, scex(_("Roman font|#R")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->input_sans = obj = fl_add_input(FL_NORMAL_INPUT, 140, 40, 240, 30, idex(_("Sans serif font|#S")));fl_set_button_shortcut(obj, scex(_("Sans serif font|#S")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->input_typewriter = obj = fl_add_input(FL_NORMAL_INPUT, 140, 70, 240, 30, idex(_("Typewriter font|#T")));fl_set_button_shortcut(obj, scex(_("Typewriter font|#T")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->input_font_norm = obj = fl_add_input(FL_NORMAL_INPUT, 140, 100, 240, 30, idex(_("Font norm|#N")));fl_set_button_shortcut(obj, scex(_("Font norm|#N")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NoGravity);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->intinput_size = obj = fl_add_input(FL_INT_INPUT, 140, 140, 50, 30, idex(_("Font zoom|#Z")));fl_set_button_shortcut(obj, scex(_("Font zoom|#Z")), 1);
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

FD_form_external *create_form_form_external(void)
{
  FL_OBJECT *obj;
  FD_form_external *fdui = (FD_form_external *) fl_calloc(1, sizeof(FD_form_external));

  fdui->form_external = fl_bgn_form(FL_NO_BOX, 560, 310);
  obj = fl_add_box(FL_UP_BOX, 0, 0, 560, 310, "");
  fdui->templatechoice = obj = fl_add_choice(FL_NORMAL_CHOICE, 130, 10, 300, 30, idex(_("Template|#t")));fl_set_button_shortcut(obj, scex(_("Template|#t")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_callback(obj, ExternalTemplateCB, 0);
  fdui->filename = obj = fl_add_input(FL_NORMAL_INPUT, 130, 190, 190, 30, idex(_("File|#F")));fl_set_button_shortcut(obj, scex(_("File|#F")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
  fdui->filenamebrowse = obj = fl_add_button(FL_NORMAL_BUTTON, 330, 190, 100, 30, idex(_("Browse...|#B")));fl_set_button_shortcut(obj, scex(_("Browse...|#B")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, ExternalBrowseCB, 0);
  fdui->parameters = obj = fl_add_input(FL_NORMAL_INPUT, 130, 230, 300, 30, idex(_("Parameters|#P")));fl_set_button_shortcut(obj, scex(_("Parameters|#P")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
  fdui->edit = obj = fl_add_button(FL_NORMAL_BUTTON, 435, 50, 110, 30, idex(_("Edit file|#E")));fl_set_button_shortcut(obj, scex(_("Edit file|#E")), 1);
    fl_set_object_color(obj, FL_COL1, FL_BLACK);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
    fl_set_object_callback(obj, ExternalEditCB, 0);
  fdui->view = obj = fl_add_button(FL_NORMAL_BUTTON, 435, 90, 110, 30, idex(_("View result|#V")));fl_set_button_shortcut(obj, scex(_("View result|#V")), 1);
    fl_set_object_color(obj, FL_COL1, FL_BLACK);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
    fl_set_object_callback(obj, ExternalViewCB, 0);
  fdui->update = obj = fl_add_button(FL_NORMAL_BUTTON, 435, 130, 110, 30, idex(_("Update result|#U")));fl_set_button_shortcut(obj, scex(_("Update result|#U")), 1);
    fl_set_object_color(obj, FL_COL1, FL_BLACK);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
    fl_set_object_callback(obj, ExternalUpdateCB, 0);
  fdui->ok = obj = fl_add_button(FL_RETURN_BUTTON, 315, 270, 110, 30, _("OK"));
    fl_set_object_color(obj, FL_COL1, FL_BLACK);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, ExternalOKCB, 0);
  fdui->cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 435, 270, 110, 30, idex(_("Cancel|#C^[")));fl_set_button_shortcut(obj, scex(_("Cancel|#C^[")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, ExternalCancelCB, 0);
  fdui->helptext = obj = fl_add_browser(FL_NORMAL_BROWSER, 130, 50, 300, 130, "");
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  fl_end_form();

  //fdui->form_external->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

