// File modified by fdfix.sh for use by lyx (with xforms > 0.88) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <cstdlib>
#include "form1.h"

FD_KeyMap *create_form_KeyMap(void)
{
  FL_OBJECT *obj;
  FD_KeyMap *fdui = (FD_KeyMap *) fl_calloc(1, sizeof(FD_KeyMap));

  fdui->KeyMap = fl_bgn_form(FL_NO_BOX, 400, 400);
  obj = fl_add_box(FL_UP_BOX, 0, 0, 400, 400, "");
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 200, 210, 190, 140, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
  fdui->AcceptChset = obj = fl_add_button(FL_NORMAL_BUTTON, 210, 280, 170, 30, idex(_("Set Charset|#C")));fl_set_button_shortcut(obj, scex(_("Set Charset|#C")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->ChsetErr = obj = fl_add_text(FL_NORMAL_TEXT, 210, 310, 170, 30, _("Charset not found!"));
    fl_set_object_lcolor(obj, FL_RED);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE+FL_EMBOSSED_STYLE);
  fdui->KeymapErr = obj = fl_add_text(FL_NORMAL_TEXT, 290, 60, 100, 90, _("Error:\n\nKeymap\nnot found"));
    fl_set_object_lcolor(obj, FL_RED);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE+FL_EMBOSSED_STYLE);
  fdui->Charset = obj = fl_add_input(FL_NORMAL_INPUT, 210, 240, 170, 30, idex(_("Character set:|#H")));fl_set_button_shortcut(obj, scex(_("Character set:|#H")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 210, 180, 100, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 20, 380, 170, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->Accept = obj = fl_add_button(FL_RETURN_BUTTON, 130, 360, 120, 30, _("OK"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->OtherKeymap = obj = fl_add_input(FL_NORMAL_INPUT, 120, 70, 160, 30, idex(_("Other...|#O")));fl_set_button_shortcut(obj, scex(_("Other...|#O")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  fdui->OtherKeymap2 = obj = fl_add_input(FL_NORMAL_INPUT, 120, 150, 160, 30, idex(_("Other...|#T")));fl_set_button_shortcut(obj, scex(_("Other...|#T")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  obj = fl_add_text(FL_NORMAL_TEXT, 20, 10, 90, 20, _("Language"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
  obj = fl_add_text(FL_NORMAL_TEXT, 20, 200, 80, 20, _("Mapping"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);

  fdui->KeyMapOn = fl_bgn_group();
  fdui->KeyOnBtn = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 10, 250, 180, 30, idex(_("Primary key map|#r")));fl_set_button_shortcut(obj, scex(_("Primary key map|#r")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->KeyOffBtn = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 10, 220, 180, 30, idex(_("No key mapping|#N")));fl_set_button_shortcut(obj, scex(_("No key mapping|#N")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fdui->KeyOnBtn2 = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 10, 280, 180, 30, idex(_("Secondary key map|#e")));fl_set_button_shortcut(obj, scex(_("Secondary key map|#e")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  fl_end_group();

  obj = fl_add_text(FL_NORMAL_TEXT, 20, 110, 100, 30, _("Secondary"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT|FL_ALIGN_INSIDE);
  obj = fl_add_text(FL_NORMAL_TEXT, 20, 30, 100, 30, _("Primary"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_RIGHT|FL_ALIGN_INSIDE);
  fl_end_form();

  fdui->KeyMap->fdui = fdui;

  return fdui;
}
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

FD_FileDlg *create_form_FileDlg(void)
{
  FL_OBJECT *obj;
  FD_FileDlg *fdui = (FD_FileDlg *) fl_calloc(1, sizeof(FD_FileDlg));

  fdui->FileDlg = fl_bgn_form(FL_NO_BOX, 440, 380);
  obj = fl_add_box(FL_UP_BOX, 0, 0, 440, 380, "");
  fdui->FileInfo = obj = fl_add_text(FL_NORMAL_TEXT, 10, 260, 420, 30, "");
    fl_set_object_boxtype(obj, FL_SHADOW_BOX);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->DirBox = obj = fl_add_input(FL_NORMAL_INPUT, 100, 10, 330, 30, idex(_("Directory:|#D")));fl_set_button_shortcut(obj, scex(_("Directory:|#D")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->PatBox = obj = fl_add_input(FL_NORMAL_INPUT, 100, 40, 330, 30, idex(_("Pattern:|#P")));fl_set_button_shortcut(obj, scex(_("Pattern:|#P")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->List = obj = fl_add_browser(FL_HOLD_BROWSER, 10, 80, 320, 170, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_gravity(obj, FL_NorthWest, FL_SouthEast);
  fdui->Filename = obj = fl_add_input(FL_NORMAL_INPUT, 100, 300, 330, 30, idex(_("Filename:|#F")));fl_set_button_shortcut(obj, scex(_("Filename:|#F")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->Rescan = obj = fl_add_button(FL_NORMAL_BUTTON, 340, 80, 90, 30, idex(_("Rescan|#R#r")));fl_set_button_shortcut(obj, scex(_("Rescan|#R#r")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
  fdui->Home = obj = fl_add_button(FL_NORMAL_BUTTON, 340, 120, 90, 30, idex(_("Home|#H#h")));fl_set_button_shortcut(obj, scex(_("Home|#H#h")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
  fdui->User1 = obj = fl_add_button(FL_NORMAL_BUTTON, 340, 160, 90, 30, idex(_("User1|#1")));fl_set_button_shortcut(obj, scex(_("User1|#1")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
  fdui->User2 = obj = fl_add_button(FL_NORMAL_BUTTON, 340, 200, 90, 30, idex(_("User2|#2")));fl_set_button_shortcut(obj, scex(_("User2|#2")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
  fdui->Ready = obj = fl_add_button(FL_RETURN_BUTTON, 220, 340, 100, 30, _("OK"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
  fdui->Cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 330, 340, 100, 30, idex(_("Cancel|^[")));fl_set_button_shortcut(obj, scex(_("Cancel|^[")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
  fdui->timer = obj = fl_add_timer(FL_HIDDEN_TIMER, 10, 350, 20, 20, "");
    fl_set_object_gravity(obj, FL_SouthWest, FL_SouthWest);
  fl_end_form();

  fdui->FileDlg->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_search *create_form_form_search(void)
{
  FL_OBJECT *obj;
  FD_form_search *fdui = (FD_form_search *) fl_calloc(1, sizeof(FD_form_search));

  fdui->form_search = fl_bgn_form(FL_NO_BOX, 440, 160);
  obj = fl_add_box(FL_UP_BOX, 0, 0, 440, 160, "");
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 300, 10, 130, 80, "");
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
  fdui->input_search = obj = fl_add_input(FL_NORMAL_INPUT, 110, 10, 180, 30, idex(_("Find|#n")));fl_set_button_shortcut(obj, scex(_("Find|#n")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  fdui->input_replace = obj = fl_add_input(FL_NORMAL_INPUT, 110, 40, 180, 30, idex(_("Replace with|#W")));fl_set_button_shortcut(obj, scex(_("Replace with|#W")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_NorthWest, FL_NorthEast);
    fl_set_object_resize(obj, FL_RESIZE_X);
  obj = fl_add_button(FL_NORMAL_BUTTON, 200, 80, 90, 30, idex(_("@>|#F^s")));fl_set_button_shortcut(obj, scex(_("@>|#F^s")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, SearchForwardCB, 0);
  obj = fl_add_button(FL_NORMAL_BUTTON, 110, 80, 90, 30, idex(_("@<|#B^r")));fl_set_button_shortcut(obj, scex(_("@<|#B^r")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, SearchBackwardCB, 0);
  fdui->replace_button = obj = fl_add_button(FL_NORMAL_BUTTON, 110, 120, 90, 30, idex(_("Replace|#R#r")));fl_set_button_shortcut(obj, scex(_("Replace|#R#r")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, SearchReplaceCB, 0);
  obj = fl_add_button(FL_NORMAL_BUTTON, 350, 120, 80, 30, idex(_("Close|^[")));fl_set_button_shortcut(obj, scex(_("Close|^[")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, SearchCancelCB, 0);
  fdui->btnCaseSensitive = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 300, 20, 150, 30, idex(_("Case sensitive|#s#S")));fl_set_button_shortcut(obj, scex(_("Case sensitive|#s#S")), 1);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
  fdui->btnMatchWord = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 300, 50, 150, 30, idex(_("Match word|#M#m")));fl_set_button_shortcut(obj, scex(_("Match word|#M#m")), 1);
    fl_set_object_gravity(obj, FL_NorthEast, FL_NorthEast);
  fdui->replaceall_button = obj = fl_add_button(FL_NORMAL_BUTTON, 200, 120, 90, 30, idex(_("Replace All|#A#a")));fl_set_button_shortcut(obj, scex(_("Replace All|#A#a")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_SouthEast, FL_SouthEast);
    fl_set_object_callback(obj, SearchReplaceAllCB, 0);
  fl_end_form();

  fdui->form_search->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

