// File modified by fdfix.sh for use by lyx (with xforms 0.81) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "print_form.h"

FD_form_print *create_form_form_print(void)
{
  FL_OBJECT *obj;
  FD_form_print *fdui = (FD_form_print *) fl_calloc(1, sizeof(FD_form_print));

  fdui->form_print = fl_bgn_form(FL_NO_BOX, 340, 360);
  obj = fl_add_box(FL_UP_BOX,0,0,340,360,"");
  obj = fl_add_frame(FL_ENGRAVED_FRAME,10,215,320,90,"");
    fl_set_object_color(obj,FL_COL1,FL_COL1);
  obj = fl_add_text(FL_NORMAL_TEXT,20,205,70,20,_("Print to"));
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fdui->input_printer = obj = fl_add_input(FL_NORMAL_INPUT,90,225,230,30,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  fdui->input_file = obj = fl_add_input(FL_NORMAL_INPUT,90,265,230,30,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);

  fdui->group_radio_printto = fl_bgn_group();
  fdui->radio_printer = obj = fl_add_checkbutton(FL_RADIO_BUTTON,10,225,80,30,idex(_("Printer|#P")));fl_set_button_shortcut(obj,scex(_("Printer|#P")),1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  fdui->radio_file = obj = fl_add_checkbutton(FL_RADIO_BUTTON,10,265,80,30,idex(_("File|#F")));fl_set_button_shortcut(obj,scex(_("File|#F")),1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  fl_end_group();

  obj = fl_add_frame(FL_ENGRAVED_FRAME,180,20,150,70,"");
    fl_set_object_color(obj,FL_COL1,FL_COL1);
  obj = fl_add_frame(FL_ENGRAVED_FRAME,10,20,160,180,"");
    fl_set_object_color(obj,FL_COL1,FL_COL1);
  obj = fl_add_button(FL_RETURN_BUTTON,10,315,100,30,_("OK"));
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,PrintOKCB,0);
  obj = fl_add_button(FL_NORMAL_BUTTON,120,315,100,30,idex(_("Apply|#A")));fl_set_button_shortcut(obj,scex(_("Apply|#A")),1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,PrintApplyCB,0);
  obj = fl_add_button(FL_NORMAL_BUTTON,230,315,100,30,idex(_("Cancel|^[")));fl_set_button_shortcut(obj,scex(_("Cancel|^[")),1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,PrintCancelCB,0);

  fdui->group_radio_pages = fl_bgn_group();
  fdui->radio_all_pages = obj = fl_add_checkbutton(FL_RADIO_BUTTON,10,30,160,30,idex(_("All Pages|#G")));fl_set_button_shortcut(obj,scex(_("All Pages|#G")),1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  fdui->radio_odd_pages = obj = fl_add_checkbutton(FL_RADIO_BUTTON,10,60,160,30,idex(_("Only Odd Pages|#O")));fl_set_button_shortcut(obj,scex(_("Only Odd Pages|#O")),1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  fdui->radio_even_pages = obj = fl_add_checkbutton(FL_RADIO_BUTTON,10,90,160,30,idex(_("Only Even Pages|#E")));fl_set_button_shortcut(obj,scex(_("Only Even Pages|#E")),1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  fl_end_group();


  fdui->group_radio_order = fl_bgn_group();
  fdui->radio_order_normal = obj = fl_add_checkbutton(FL_RADIO_BUTTON,180,30,150,30,idex(_("Normal Order|#N")));fl_set_button_shortcut(obj,scex(_("Normal Order|#N")),1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  fdui->radio_order_reverse = obj = fl_add_checkbutton(FL_RADIO_BUTTON,180,60,150,30,idex(_("Reverse Order|#R")));fl_set_button_shortcut(obj,scex(_("Reverse Order|#R")),1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  fl_end_group();

  obj = fl_add_text(FL_NORMAL_TEXT,200,10,60,20,_("Order"));
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT,20,10,50,20,_("Print"));
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fdui->input_pages = obj = fl_add_input(FL_NORMAL_INPUT,20,160,140,30,_("Pages:"));
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
  obj = fl_add_frame(FL_ENGRAVED_FRAME,180,110,150,90,"");
    fl_set_object_color(obj,FL_COL1,FL_COL1);
  obj = fl_add_text(FL_NORMAL_TEXT,200,95,50,20,_("Copies"));
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fdui->input_copies = obj = fl_add_input(FL_NORMAL_INPUT,190,160,130,30,_("Count:"));
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
  fdui->do_unsorted = obj = fl_add_checkbutton(FL_PUSH_BUTTON,180,115,140,30,idex(_("Unsorted|#U")));fl_set_button_shortcut(obj,scex(_("Unsorted|#U")),1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  fl_end_form();

  //fdui->form_print->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_sendto *create_form_form_sendto(void)
{
  FL_OBJECT *obj;
  FD_form_sendto *fdui = (FD_form_sendto *) fl_calloc(1, sizeof(FD_form_sendto));

  fdui->form_sendto = fl_bgn_form(FL_NO_BOX, 340, 210);
  obj = fl_add_box(FL_UP_BOX,0,0,340,210,"");
  obj = fl_add_frame(FL_ENGRAVED_FRAME,10,20,310,80,"");
    fl_set_object_color(obj,FL_COL1,FL_COL1);
  obj = fl_add_text(FL_NORMAL_TEXT,20,10,80,20,_("File Type"));
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fdui->input_cmd = obj = fl_add_input(FL_NORMAL_INPUT,10,130,320,30,idex(_("Command:|#C")));fl_set_button_shortcut(obj,scex(_("Command:|#C")),1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
  obj = fl_add_button(FL_RETURN_BUTTON,10,170,100,30,_("OK"));
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,SendtoOKCB,0);
  obj = fl_add_button(FL_NORMAL_BUTTON,120,170,100,30,idex(_("Apply|#A")));fl_set_button_shortcut(obj,scex(_("Apply|#A")),1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,SendtoApplyCB,0);
  obj = fl_add_button(FL_NORMAL_BUTTON,230,170,100,30,idex(_("Cancel|^[")));fl_set_button_shortcut(obj,scex(_("Cancel|^[")),1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,SendtoCancelCB,0);

  fdui->group_ftype = fl_bgn_group();
  fdui->radio_ftype_dvi = obj = fl_add_checkbutton(FL_RADIO_BUTTON,110,30,110,30,idex(_("DVI|#D")));fl_set_button_shortcut(obj,scex(_("DVI|#D")),1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  fdui->radio_ftype_ps = obj = fl_add_checkbutton(FL_RADIO_BUTTON,110,60,110,30,idex(_("Postscript|#P")));fl_set_button_shortcut(obj,scex(_("Postscript|#P")),1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  fdui->radio_ftype_latex = obj = fl_add_checkbutton(FL_RADIO_BUTTON,10,60,100,30,idex(_("LaTeX|#T")));fl_set_button_shortcut(obj,scex(_("LaTeX|#T")),1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_button(obj, 1);
  fdui->radio_ftype_lyx = obj = fl_add_checkbutton(FL_RADIO_BUTTON,10,30,100,30,idex(_("LyX|#L")));fl_set_button_shortcut(obj,scex(_("LyX|#L")),1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  fdui->radio_ftype_ascii = obj = fl_add_checkbutton(FL_RADIO_BUTTON,220,30,90,30,idex(_("Ascii|#s")));fl_set_button_shortcut(obj,scex(_("Ascii|#s")),1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  fl_end_group();

  fl_end_form();

  //fdui->form_sendto->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

