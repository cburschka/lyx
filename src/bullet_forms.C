// File modified by fdfix.sh for use by lyx (with xforms 0.81) and gettext
#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

/* Form definition file generated with fdesign. */

#include FORMS_H_LOCATION
#include <cstdlib>
#include "bullet_forms.h"
#include "bmtable.h"
#include "support/filetools.h"

extern int mono_video;

FD_form_bullet *create_form_form_bullet(void)
{
  FL_OBJECT *obj;
  FD_form_bullet *fdui = (FD_form_bullet *) fl_calloc(1, sizeof(FD_form_bullet));

  fdui->form_bullet = fl_bgn_form(FL_NO_BOX, 370, 390);
  fl_set_border_width(-1);
  obj = fl_add_box(FL_UP_BOX, 0, 0, 370, 390, "");
 
  // make the border thick again to improve the bmtable looks
  fl_set_border_width(-3);
 
  fdui->bmtable_bullet_panel = obj = fl_add_bmtable(1, 90, 105, 265, 180, "");
     fl_set_object_callback(obj, BulletBMTableCB, 0);
     fl_set_object_lcol(obj, FL_BLUE);
     fl_set_object_boxtype(obj, FL_UP_BOX);
     if (mono_video) {
        fl_set_bmtable_file(obj, 6, 6,
                           LibFileSearch("images",
                                         "standard.xbm").c_str());
     } else {
        fl_set_bmtable_pixmap_file(obj, 6, 6,
                                  LibFileSearch("images",
                                                "standard.xpm").c_str());
    }

  fl_set_border_width(-1);
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 95, 20, 255, 70, "");
  obj = fl_add_frame(FL_ENGRAVED_FRAME, 0, 340, 370, 50, "");
  fdui->choice_bullet_size = obj = fl_add_choice(FL_NORMAL_CHOICE, 15, 45, 65, 30, idex(_("Size|#z")));fl_set_button_shortcut(obj, scex(_("Size|#z")), 1);
    fl_set_object_boxtype(obj, FL_FRAME_BOX);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_TOP);
    fl_set_object_callback(obj, ChoiceBulletSizeCB, 0);
  fdui->button_ok = obj = fl_add_button(FL_RETURN_BUTTON, 25, 350, 100, 30, _("OK"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, BulletOKCB, 0);
  fdui->button_apply = obj = fl_add_button(FL_NORMAL_BUTTON, 135, 350, 100, 30, idex(_("Apply|#A")));fl_set_button_shortcut(obj, scex(_("Apply|#A")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, BulletApplyCB, 0);
  fdui->button_cancel = obj = fl_add_button(FL_NORMAL_BUTTON, 245, 350, 100, 30, idex(_("Cancel|^[")));fl_set_button_shortcut(obj, scex(_("Cancel|^[")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_callback(obj, BulletCancelCB, 0);
  fdui->input_bullet_latex = obj = fl_add_input(FL_NORMAL_INPUT, 80, 300, 275, 30, idex(_("LaTeX|#L")));fl_set_button_shortcut(obj, scex(_("LaTeX|#L")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lstyle(obj, FL_FIXED_STYLE);
    fl_set_input_return(obj, FL_RETURN_CHANGED);
    fl_set_input_maxchars(obj, 80);
    fl_set_object_callback(obj, InputBulletLaTeXCB, 0);

  fdui->radio_bullet_depth = fl_bgn_group();
  fdui->radio_bullet_depth_1 = obj = fl_add_button(FL_RADIO_BUTTON, 105, 35, 55, 40, idex(_("1|#1")));fl_set_button_shortcut(obj, scex(_("1|#1")), 1);
    fl_set_object_lsize(obj, FL_LARGE_SIZE);
    fl_set_object_callback(obj, BulletDepthCB, 0);
    fl_set_button(obj, 1);
  fdui->radio_bullet_depth_2 = obj = fl_add_button(FL_RADIO_BUTTON, 165, 35, 55, 40, idex(_("2|#2")));fl_set_button_shortcut(obj, scex(_("2|#2")), 1);
    fl_set_object_lsize(obj, FL_LARGE_SIZE);
    fl_set_object_callback(obj, BulletDepthCB, 1);
  fdui->radio_bullet_depth_3 = obj = fl_add_button(FL_RADIO_BUTTON, 225, 35, 55, 40, idex(_("3|#3")));fl_set_button_shortcut(obj, scex(_("3|#3")), 1);
    fl_set_object_lsize(obj, FL_LARGE_SIZE);
    fl_set_object_callback(obj, BulletDepthCB, 2);
  fdui->radio_bullet_depth_4 = obj = fl_add_button(FL_RADIO_BUTTON, 285, 35, 55, 40, idex(_("4|#4")));fl_set_button_shortcut(obj, scex(_("4|#4")), 1);
    fl_set_object_lsize(obj, FL_LARGE_SIZE);
    fl_set_object_callback(obj, BulletDepthCB, 3);
  fl_end_group();

  obj = fl_add_text(FL_NORMAL_TEXT, 105, 10, 85, 20, _("Bullet Depth"));
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);

  fdui->radio_bullet_panel = fl_bgn_group();
  fdui->radio_bullet_panel_standard = obj = fl_add_button(FL_RADIO_BUTTON, 15, 105, 65, 30, idex(_("Standard|#S")));fl_set_button_shortcut(obj, scex(_("Standard|#S")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, BulletPanelCB, 0);
    fl_set_button(obj, 1);
  fdui->radio_bullet_panel_maths = obj = fl_add_button(FL_RADIO_BUTTON, 15, 135, 65, 30, idex(_("Maths|#M")));fl_set_button_shortcut(obj, scex(_("Maths|#M")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, BulletPanelCB, 1);
  fdui->radio_bullet_panel_ding2 = obj = fl_add_button(FL_RADIO_BUTTON, 15, 195, 65, 30, idex(_("Ding 2|#i")));fl_set_button_shortcut(obj, scex(_("Ding 2|#i")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, BulletPanelCB, 3);
  fdui->radio_bullet_panel_ding3 = obj = fl_add_button(FL_RADIO_BUTTON, 15, 225, 65, 30, idex(_("Ding 3|#n")));fl_set_button_shortcut(obj, scex(_("Ding 3|#n")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, BulletPanelCB, 4);
  fdui->radio_bullet_panel_ding4 = obj = fl_add_button(FL_RADIO_BUTTON, 15, 255, 65, 30, idex(_("Ding 4|#g")));fl_set_button_shortcut(obj, scex(_("Ding 4|#g")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, BulletPanelCB, 5);
  fdui->radio_bullet_panel_ding1 = obj = fl_add_button(FL_RADIO_BUTTON, 15, 165, 65, 30, idex(_("Ding 1|#D")));fl_set_button_shortcut(obj, scex(_("Ding 1|#D")), 1);
    fl_set_object_lsize(obj, FL_NORMAL_SIZE);
    fl_set_object_resize(obj, FL_RESIZE_NONE);
    fl_set_object_callback(obj, BulletPanelCB, 2);
  fl_end_group();

  fl_end_form();

  //fdui->form_bullet->fdui = fdui;

  return fdui;
}
/*---------------------------------------*/

