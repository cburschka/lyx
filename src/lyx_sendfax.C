// File modified by fdfix.sh for use by lyx (with xforms 0.81) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "lyx_sendfax.h"

FD_xsendfax *create_form_xsendfax(void)
{
  FL_OBJECT *obj;
  FD_xsendfax *fdui = (FD_xsendfax *) fl_calloc(1, sizeof(FD_xsendfax));

  fdui->xsendfax = fl_bgn_form(FL_NO_BOX, 600, 330);
  obj = fl_add_box(FL_UP_BOX,0,0,600,330,"");
  obj = fl_add_frame(FL_ENGRAVED_FRAME,10,15,465,125,"");
    fl_set_object_color(obj,FL_COL1,FL_COL1);
  fdui->Input_Phone = obj = fl_add_input(FL_NORMAL_INPUT,90,30,377,30,idex(_("Fax no.:|#F")));fl_set_button_shortcut(obj,scex(_("Fax no.:|#F")),1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  fdui->Input_Name = obj = fl_add_input(FL_NORMAL_INPUT,90,65,377,29,idex(_("Dest. Name:|#N")));fl_set_button_shortcut(obj,scex(_("Dest. Name:|#N")),1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  fdui->Input_Enterprise = obj = fl_add_input(FL_NORMAL_INPUT,90,100,377,29,idex(_("Enterprise:|#E")));fl_set_button_shortcut(obj,scex(_("Enterprise:|#E")),1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
  fdui->Input_Comment = obj = fl_add_input(FL_MULTILINE_INPUT,15,165,575,106,"");
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_TOP_LEFT);
    fl_set_object_lstyle(obj,FL_NORMAL_STYLE+FL_EMBOSSED_STYLE);
  fdui->Button_Send = obj = fl_add_button(FL_NORMAL_BUTTON,20,290,180,30,idex(_("OK|#O")));fl_set_button_shortcut(obj,scex(_("OK|#O")),1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,FaxSendCB,0);
  fdui->Button_Cancel = obj = fl_add_button(FL_NORMAL_BUTTON,400,290,180,30,idex(_("Cancel|^[")));fl_set_button_shortcut(obj,scex(_("Cancel|^[")),1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,FaxCancelCB,0);
  fdui->Button_Apply = obj = fl_add_button(FL_NORMAL_BUTTON,210,290,180,30,idex(_("Apply|#A")));fl_set_button_shortcut(obj,scex(_("Apply|#A")),1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
    fl_set_object_callback(obj,FaxApplyCB,0);
  obj = fl_add_frame(FL_ENGRAVED_FRAME,485,30,105,100,"");
    fl_set_object_color(obj,FL_COL1,FL_COL1);
  obj = fl_add_text(FL_NORMAL_TEXT,490,20,95,20,_("Phone Book"));
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fdui->Button_SPhone = obj = fl_add_button(FL_NORMAL_BUTTON,490,45,15,15,idex(_("Select from|#S")));fl_set_button_shortcut(obj,scex(_("Select from|#S")),1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_RIGHT);
    fl_set_object_callback(obj,FaxOpenPhonebookCB,0);
  obj = fl_add_button(FL_NORMAL_BUTTON,490,65,15,15,idex(_("Add to|#t")));fl_set_button_shortcut(obj,scex(_("Add to|#t")),1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_RIGHT);
    fl_set_object_callback(obj,cb_add_phoneno,0);
  obj = fl_add_button(FL_NORMAL_BUTTON,490,85,15,15,idex(_("Delete from|#D")));fl_set_button_shortcut(obj,scex(_("Delete from|#D")),1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_RIGHT);
    fl_set_object_callback(obj,cb_delete_phoneno,0);
  fdui->pb_save = obj = fl_add_button(FL_NORMAL_BUTTON,490,105,15,15,idex(_("Save|#V")));fl_set_button_shortcut(obj,scex(_("Save|#V")),1);
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_RIGHT);
    fl_set_object_callback(obj,cb_save_phoneno,0);
  obj = fl_add_text(FL_NORMAL_TEXT,15,5,100,20,_("Destination:"));
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  obj = fl_add_frame(FL_ENGRAVED_FRAME,10,155,585,125,"");
    fl_set_object_color(obj,FL_COL1,FL_COL1);
  obj = fl_add_text(FL_NORMAL_TEXT,15,145,85,20,_("Comment:"));
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_lalign(obj,FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj,FL_BOLD_STYLE);
  fl_end_form();

  //fdui->xsendfax->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_phonebook *create_form_phonebook(void)
{
  FL_OBJECT *obj;
  FD_phonebook *fdui = (FD_phonebook *) fl_calloc(1, sizeof(FD_phonebook));

  fdui->phonebook = fl_bgn_form(FL_NO_BOX, 600, 320);
  obj = fl_add_box(FL_UP_BOX,0,0,600,320,"");
  fdui->browser = obj = fl_add_browser(FL_HOLD_BROWSER,2,2,598,318,"");
    fl_set_object_lsize(obj,FL_MEDIUM_SIZE);
    fl_set_object_callback(obj,cb_select_phoneno,0);
  fl_end_form();

  //fdui->phonebook->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_logfile *create_form_logfile(void)
{
  FL_OBJECT *obj;
  FD_logfile *fdui = (FD_logfile *) fl_calloc(1, sizeof(FD_logfile));

  fdui->logfile = fl_bgn_form(FL_NO_BOX, 510, 190);
  obj = fl_add_box(FL_UP_BOX,0,0,510,190,"");
  fdui->browser = obj = fl_add_browser(FL_NORMAL_BROWSER,10,10,490,130,"");
  fdui->Button_Close = obj = fl_add_button(FL_RETURN_BUTTON,410,150,90,30,_("Close"));
    fl_set_object_lsize(obj,FL_NORMAL_SIZE);
    fl_set_object_callback(obj,FaxLogfileCloseCB,0);
  fl_end_form();

  //fdui->logfile->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

