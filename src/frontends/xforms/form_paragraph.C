// File modified by fdfix.sh for use by lyx (with xforms >= 0.88) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <stdlib.h>
#include "form_paragraph.h"
#include "FormParagraph.h"

FD_form_paragraph_general::~FD_form_paragraph_general()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_paragraph_general * FormParagraph::build_paragraph_general()
{
  FL_OBJECT *obj;
  FD_form_paragraph_general *fdui = new FD_form_paragraph_general;

  fdui->form = fl_bgn_form(FL_NO_BOX, 490, 250);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 490, 250, "");
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 360, 110, 120, 50, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 110, 340, 90, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 360, 20, 120, 70, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 210, 20, 140, 70, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 20, 190, 70, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
  fdui->input_labelwidth = obj = fl_add_input(FL_NORMAL_INPUT, 100, 210, 360, 30, idex(_("Label Width:|#d")));
    fl_set_button_shortcut(obj, scex(_("Label Width:|#d")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_gravity(obj, FL_West, FL_East);
    fl_set_object_resize(obj, FL_RESIZE_X);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  obj = fl_add_text(FL_NORMAL_TEXT, 370, 100, 60, 20, _("Indent"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fdui->check_lines_top = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 230, 30, 120, 30, idex(_("Above|#b")));
    fl_set_button_shortcut(obj, scex(_("Above|#b")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->check_lines_bottom = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 230, 60, 120, 30, idex(_("Below|#E")));
    fl_set_button_shortcut(obj, scex(_("Below|#E")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->check_pagebreaks_top = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 360, 30, 120, 30, idex(_("Above|#o")));
    fl_set_button_shortcut(obj, scex(_("Above|#o")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->check_pagebreaks_bottom = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 360, 60, 120, 30, idex(_("Below|#l")));
    fl_set_button_shortcut(obj, scex(_("Below|#l")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->check_noindent = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 360, 120, 120, 30, idex(_("No Indent|#I")));
    fl_set_button_shortcut(obj, scex(_("No Indent|#I")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);

  fdui->group_radio_alignment = fl_bgn_group();
  fdui->radio_align_right = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 10, 30, 80, 30, idex(_("Right|#R")));
    fl_set_button_shortcut(obj, scex(_("Right|#R")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->radio_align_left = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 10, 60, 80, 30, idex(_("Left|#f")));
    fl_set_button_shortcut(obj, scex(_("Left|#f")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->radio_align_block = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 100, 30, 80, 30, idex(_("Block|#c")));
    fl_set_button_shortcut(obj, scex(_("Block|#c")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->radio_align_center = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 100, 60, 80, 30, idex(_("Center|#n")));
    fl_set_button_shortcut(obj, scex(_("Center|#n")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_group();

  fdui->input_space_above = obj = fl_add_input(FL_NORMAL_INPUT, 180, 120, 90, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_space_below = obj = fl_add_input(FL_NORMAL_INPUT, 180, 160, 90, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->choice_space_above = obj = fl_add_choice(FL_NORMAL_CHOICE, 70, 120, 100, 30, idex(_("Above:|#v")));
    fl_set_button_shortcut(obj, scex(_("Above:|#v")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->choice_space_below = obj = fl_add_choice(FL_NORMAL_CHOICE, 70, 160, 100, 30, idex(_("Below:|#w")));
    fl_set_button_shortcut(obj, scex(_("Below:|#w")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  obj = fl_add_text(FL_NORMAL_TEXT, 370, 10, 100, 20, _("Pagebreaks"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT, 230, 10, 50, 20, _("Lines"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT, 20, 10, 80, 20, _("Alignment"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT, 20, 100, 130, 20, _("Vertical Spaces"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fdui->check_space_above = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 270, 120, 40, 30, idex(_("Keep|#K")));
    fl_set_button_shortcut(obj, scex(_("Keep|#K")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->check_space_below = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 270, 160, 40, 30, idex(_("Keep|#p")));
    fl_set_button_shortcut(obj, scex(_("Keep|#p")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_paragraph_extra::~FD_form_paragraph_extra()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_paragraph_extra * FormParagraph::build_paragraph_extra()
{
  FL_OBJECT *obj;
  FD_form_paragraph_extra *fdui = new FD_form_paragraph_extra;

  fdui->form = fl_bgn_form(FL_NO_BOX, 490, 250);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_FLAT_BOX, 0, 0, 490, 250, "");
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 20, 170, 100, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 350, 20, 100, 100, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 190, 20, 150, 100, "");
    fl_set_object_color(obj, FL_COL1, FL_COL1);
  obj = fl_add_text(FL_NORMAL_TEXT, 20, 10, 120, 20, _("Extra Options"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fdui->input_pextra_width = obj = fl_add_input(FL_NORMAL_INPUT, 240, 30, 90, 30, idex(_("Length|#L")));
    fl_set_button_shortcut(obj, scex(_("Length|#L")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->input_pextra_widthp = obj = fl_add_input(FL_INT_INPUT, 240, 70, 90, 30, idex(_("or %|#o")));
    fl_set_button_shortcut(obj, scex(_("or %|#o")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  obj = fl_add_text(FL_NORMAL_TEXT, 200, 10, 60, 20, _("Width"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  obj = fl_add_text(FL_NORMAL_TEXT, 350, 10, 90, 20, _("Alignment"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);

  fdui->group_alignment = fl_bgn_group();
  fdui->radio_pextra_top = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 360, 30, 80, 30, idex(_("Top|#T")));
    fl_set_button_shortcut(obj, scex(_("Top|#T")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->radio_pextra_middle = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 360, 60, 80, 30, idex(_("Middle|#d")));
    fl_set_button_shortcut(obj, scex(_("Middle|#d")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->radio_pextra_bottom = obj = fl_add_checkbutton(FL_RADIO_BUTTON, 360, 90, 80, 30, idex(_("Bottom|#B")));
    fl_set_button_shortcut(obj, scex(_("Bottom|#B")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_group();

  fdui->radio_pextra_hfill = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 10, 130, 300, 30, idex(_("HFill between Minipage paragraphs|#H")));
    fl_set_button_shortcut(obj, scex(_("HFill between Minipage paragraphs|#H")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->radio_pextra_startmp = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 10, 160, 260, 30, idex(_("Start new Minipage|#S")));
    fl_set_button_shortcut(obj, scex(_("Start new Minipage|#S")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);

  fdui->group_extraopt = fl_bgn_group();
  fdui->radio_pextra_indent = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 10, 30, 160, 30, idex(_("Indented Paragraph|#I")));
    fl_set_button_shortcut(obj, scex(_("Indented Paragraph|#I")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->radio_pextra_minipage = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 10, 60, 160, 30, idex(_("Minipage|#M")));
    fl_set_button_shortcut(obj, scex(_("Minipage|#M")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fdui->radio_pextra_floatflt = obj = fl_add_checkbutton(FL_PUSH_BUTTON, 10, 90, 160, 30, idex(_("Floatflt|#F")));
    fl_set_button_shortcut(obj, scex(_("Floatflt|#F")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseInputCB, 0);
  fl_end_group();

  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

FD_form_tabbed_paragraph::~FD_form_tabbed_paragraph()
{
  if ( form->visible ) fl_hide_form( form );
  fl_free_form( form );
}


FD_form_tabbed_paragraph * FormParagraph::build_tabbed_paragraph()
{
  FL_OBJECT *obj;
  FD_form_tabbed_paragraph *fdui = new FD_form_tabbed_paragraph;

  fdui->form = fl_bgn_form(FL_NO_BOX, 510, 350);
  fdui->form->u_vdata = this;
  obj = fl_add_box(FL_UP_BOX, 0, 0, 510, 350, "");
  fdui->tabbed_folder = obj = fl_add_tabfolder(FL_TOP_TABFOLDER, 10, 10, 490, 270, _("Tabbed folder"));
    fl_set_object_resize(obj, FL_RESIZE_ALL);
  fdui->button_cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 400, 310, 100, 30, idex(_("Cancel|C#C^[")));
    fl_set_button_shortcut(obj, scex(_("Cancel|C#C^[")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseCancelCB, 0);
  fdui->button_apply = obj = fl_add_button(FL_NORMAL_BUTTON, 290, 310, 100, 30, idex(_("Apply|#A")));
    fl_set_button_shortcut(obj, scex(_("Apply|#A")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseApplyCB, 0);
  fdui->button_ok = obj = fl_add_button(FL_RETURN_BUTTON, 180, 310, 100, 30, _("OK"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseOKCB, 0);
  fdui->button_restore = obj = fl_add_button(FL_NORMAL_BUTTON, 10, 310, 100, 30, idex(_("Restore|#R")));
    fl_set_button_shortcut(obj, scex(_("Restore|#R")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, C_FormBaseRestoreCB, 0);
  fdui->text_warning = obj = fl_add_text(FL_NORMAL_TEXT, 10, 280, 490, 30, "");
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
    fl_set_object_lstyle(obj, FL_BOLD_STYLE);
  fl_end_form();

  fdui->form->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

