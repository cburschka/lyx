// File modified by fdfix.sh for use by lyx (with xforms > 0.88) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <cstdlib>
#include "form1.h"

/*---------------------------------------*/

FD_Figure *create_form_Figure(void)
{
  FL_OBJECT *obj;
  FD_Figure *fdui = (FD_Figure *) fl_calloc(1, sizeof(FD_Figure));

  fdui->Figure = fl_bgn_form(FL_NO_BOX, 420, 390);
  obj = fl_add_box(FL_UP_BOX, 0, 0, 420, 390, "");
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 230, 220, 180, 80, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 230, 60, 180, 140, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 244, 200, 56, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 60, 200, 160, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
  fdui->EpsFile = obj = fl_add_input(FL_NORMAL_INPUT, 80, 10, 230, 30, idex(_("EPS file|#E")));fl_set_button_shortcut(obj, scex(_("EPS file|#E")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, GraphicsCB, 2);
  fdui->Preview = obj = fl_add_button(FL_NORMAL_BUTTON, 270, 350, 140, 30, idex(_("Full Screen Preview|#v")));fl_set_button_shortcut(obj, scex(_("Full Screen Preview|#v")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, GraphicsCB, 1);
  fdui->Browse = obj = fl_add_button(FL_NORMAL_BUTTON, 320, 10, 90, 30, idex(_("Browse...|#B")));fl_set_button_shortcut(obj, scex(_("Browse...|#B")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, GraphicsCB, 0);
  fdui->Width = obj = fl_add_input(FL_FLOAT_INPUT, 20, 180, 80, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, GraphicsCB, 2);
  fdui->Height = obj = fl_add_input(FL_FLOAT_INPUT, 120, 180, 80, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, GraphicsCB, 2);
  fdui->ApplyBtn = obj = fl_add_button(FL_NORMAL_BUTTON, 100, 350, 60, 30, idex(_("Apply|#A")));fl_set_button_shortcut(obj, scex(_("Apply|#A")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, GraphicsCB, 7);
  fdui->OkBtn = obj = fl_add_button(FL_RETURN_BUTTON, 10, 350, 70, 30, _("OK"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, GraphicsCB, 8);
  fdui->CancelBtn = obj = fl_add_button(FL_NORMAL_BUTTON, 180, 350, 70, 30, idex(_("Cancel|^[")));fl_set_button_shortcut(obj, scex(_("Cancel|^[")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, GraphicsCB, 9);
  fdui->Frame = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 230, 230, 180, 30, idex(_("Display Frame|#F")));fl_set_button_shortcut(obj, scex(_("Display Frame|#F")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, GraphicsCB, 53);
  fdui->Translations = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 230, 260, 180, 30, idex(_("Do Translations|#r")));fl_set_button_shortcut(obj, scex(_("Do Translations|#r")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, GraphicsCB, 54);
  obj = fl_add_text(FL_NORMAL_TEXT, 240, 210, 70, 20, _("Options"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fdui->Angle = obj = fl_add_input(FL_FLOAT_INPUT, 100, 260, 80, 30, idex(_("Angle:|#L")));fl_set_button_shortcut(obj, scex(_("Angle:|#L")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, GraphicsCB, 2);

  fdui->HeightGrp = fl_bgn_group();
  // xgettext:no-c-format
  fdui->page2 = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 110, 130, 110, 30, idex(_("% of Page|#g")));fl_set_button_shortcut(obj, scex(_("% of Page|#g")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, GraphicsCB, 23);
  fdui->Default2 = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 110, 70, 80, 30, idex(_("Default|#t")));fl_set_button_shortcut(obj, scex(_("Default|#t")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, GraphicsCB, 20);
  fdui->cm2 = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 110, 90, 60, 30, idex(_("cm|#m")));fl_set_button_shortcut(obj, scex(_("cm|#m")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, GraphicsCB, 21);
  fdui->in2 = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 110, 110, 80, 30, idex(_("inches|#h")));fl_set_button_shortcut(obj, scex(_("inches|#h")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, GraphicsCB, 22);
  fl_end_group();

  obj = fl_add_text(FL_NORMAL_TEXT, 237, 50, 53, 20, _("Display"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fdui->HeightLabel = obj = fl_add_text(FL_NORMAL_TEXT, 120, 50, 60, 20, _("Height"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fdui->WidthLabel = obj = fl_add_text(FL_NORMAL_TEXT, 20, 50, 60, 20, _("Width"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT, 20, 230, 90, 30, _("Rotation"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);

  fdui->DisplayGrp = fl_bgn_group();
  fdui->Wysiwyg3 = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 230, 130, 180, 30, idex(_("Display in Color|#D")));fl_set_button_shortcut(obj, scex(_("Display in Color|#D")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, GraphicsCB, 63);
  fdui->Wysiwyg0 = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 230, 160, 180, 30, idex(_("Do not display this figure|#y")));fl_set_button_shortcut(obj, scex(_("Do not display this figure|#y")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, GraphicsCB, 3);
  fdui->Wysiwyg2 = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 230, 100, 180, 30, idex(_("Display as Grayscale|#i")));fl_set_button_shortcut(obj, scex(_("Display as Grayscale|#i")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, GraphicsCB, 43);
  fdui->Wysiwyg1 = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 230, 70, 180, 30, idex(_("Display as Monochrome|#s")));fl_set_button_shortcut(obj, scex(_("Display as Monochrome|#s")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, GraphicsCB, 33);
  fl_end_group();


  fdui->WidthGrp = fl_bgn_group();
  fdui->Default1 = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 10, 70, 80, 30, idex(_("Default|#U")));fl_set_button_shortcut(obj, scex(_("Default|#U")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, GraphicsCB, 10);
  fdui->cm1 = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 10, 90, 60, 30, idex(_("cm|#c")));fl_set_button_shortcut(obj, scex(_("cm|#c")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, GraphicsCB, 11);
  fdui->in1 = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 10, 110, 80, 30, idex(_("inches|#n")));fl_set_button_shortcut(obj, scex(_("inches|#n")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, GraphicsCB, 12);
  // xgettext:no-c-format
  fdui->page1 = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 10, 130, 110, 30, idex(_("% of Page|#P")));fl_set_button_shortcut(obj, scex(_("% of Page|#P")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, GraphicsCB, 13);
  // xgettext:no-c-format
  fdui->column1 = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 10, 150, 110, 30, idex(_("% of Column|#o")));fl_set_button_shortcut(obj, scex(_("% of Column|#o")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, GraphicsCB, 14);
  fl_end_group();

  obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 310, 400, 30, "");
  fdui->Subcaption = obj = fl_add_input(FL_NORMAL_INPUT, 150, 310, 260, 30, idex(_("Caption|#k")));fl_set_button_shortcut(obj, scex(_("Caption|#k")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, GraphicsCB, 2);
  fdui->Subfigure = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 10, 310, 90, 30, idex(_("Subfigure|#q")));fl_set_button_shortcut(obj, scex(_("Subfigure|#q")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, GraphicsCB, 70);
  fl_end_form();

  fdui->Figure->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/
