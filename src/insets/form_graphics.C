#include <config.h>
#include "lyx_gui_misc.h"
#include "gettext.h"

#include FORMS_H_LOCATION
#include <cstdlib>
#include "form_graphics.h"

extern "C" void GraphicxCB(FL_OBJECT*,long);

FD_Graphics * create_form_Graphics(void)
{
	FL_OBJECT * obj;
	FD_Graphics * fdui = (FD_Graphics *) fl_calloc(1, sizeof(FD_Graphics));
	
	fdui->Graphics = fl_bgn_form(FL_NO_BOX, 420, 390);
	
	obj = fl_add_box(FL_UP_BOX, 0, 0, 420, 390, "");
	
	obj = fl_add_frame(FL_ENGRAVED_FRAME, 230, 220, 180, 80, "");
	fl_set_object_color(obj, FL_COL1, FL_COL1);
	
	obj = fl_add_frame(FL_ENGRAVED_FRAME, 230, 60, 180, 140, "");
	fl_set_object_color(obj, FL_COL1, FL_COL1);
	
	obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 244, 200, 56, "");
	fl_set_object_color(obj, FL_COL1, FL_COL1);
	
	obj = fl_add_frame(FL_ENGRAVED_FRAME, 10, 60, 200, 160, "");
	fl_set_object_color(obj, FL_COL1, FL_COL1);
	
	fdui->graphicsFile = obj = fl_add_input(FL_NORMAL_INPUT,
						80, 10, 230, 30,
						idex(_("Graphics file|#G")));
	fl_set_button_shortcut(obj, scex(_("Graphics file|#G")), 1);
	fl_set_object_lsize(obj, FL_NORMAL_SIZE);
	//fl_set_object_callback(obj, GraphicxCB, 0);
	
	fdui->Browse = obj = fl_add_button(FL_NORMAL_BUTTON, 320, 10, 90, 30,
					   idex(_("Browse...|#B")));
	fl_set_button_shortcut(obj, scex(_("Browse...|#B")), 1);
	fl_set_object_lsize(obj, FL_NORMAL_SIZE);
	fl_set_object_callback(obj, GraphicxCB, 1);
	
	fdui->Width = obj = fl_add_input(FL_FLOAT_INPUT, 20, 180, 80, 30, "");
	fl_set_object_lsize(obj, FL_NORMAL_SIZE);
	fl_set_object_callback(obj, GraphicxCB, 99);
	
	fdui->Height = obj = fl_add_input(FL_FLOAT_INPUT,
					  120, 180, 80, 30, "");
	fl_set_object_lsize(obj, FL_NORMAL_SIZE);
	fl_set_object_callback(obj, GraphicxCB, 99);
	
	fdui->ApplyBtn = obj = fl_add_button(FL_NORMAL_BUTTON,
					     100, 350, 60, 30,
					     idex(_("Apply|#A")));
	fl_set_button_shortcut(obj, scex(_("Apply|#A")), 1);
	fl_set_object_lsize(obj, FL_NORMAL_SIZE);
	fl_set_object_callback(obj, GraphicxCB, 2);
	
	fdui->OkBtn = obj = fl_add_button(FL_RETURN_BUTTON,
					  10, 350, 70, 30, _("OK"));
	fl_set_object_lsize(obj, FL_NORMAL_SIZE);
	fl_set_object_callback(obj, GraphicxCB, 3);
	
	fdui->CancelBtn = obj = fl_add_button(FL_NORMAL_BUTTON,
					      180, 350, 70, 30,
					      idex(_("Cancel|^[")));
	fl_set_button_shortcut(obj, scex(_("Cancel|^[")), 1);
	fl_set_object_lsize(obj, FL_NORMAL_SIZE);
	fl_set_object_callback(obj, GraphicxCB, 4);
	
	obj = fl_add_text(FL_NORMAL_TEXT, 240, 210, 70, 20, _("Options"));
	fl_set_object_lsize(obj, FL_NORMAL_SIZE);
	fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
	fl_set_object_lstyle(obj, FL_BOLD_STYLE);
	
	fdui->Angle = obj = fl_add_input(FL_FLOAT_INPUT, 100, 260, 80, 30,
					 idex(_("Angle:|#L")));
	fl_set_button_shortcut(obj, scex(_("Angle:|#L")), 1);
	fl_set_object_lsize(obj, FL_NORMAL_SIZE);
	fl_set_object_callback(obj, GraphicxCB, 99);
	
	obj = fl_add_text(FL_NORMAL_TEXT, 237, 50, 53, 20, _("Display"));
	fl_set_object_lsize(obj, FL_NORMAL_SIZE);
	fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
	fl_set_object_lstyle(obj, FL_BOLD_STYLE);
	
	fdui->HeightLabel = obj = fl_add_text(FL_NORMAL_TEXT,
					      120, 50, 60, 20, _("Height"));
	fl_set_object_lsize(obj, FL_NORMAL_SIZE);
	fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
	fl_set_object_lstyle(obj, FL_BOLD_STYLE);
	
	fdui->WidthLabel = obj = fl_add_text(FL_NORMAL_TEXT,
					     20, 50, 60, 20, _("Width"));
	fl_set_object_lsize(obj, FL_NORMAL_SIZE);
	fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
	fl_set_object_lstyle(obj, FL_BOLD_STYLE);
	
	obj = fl_add_text(FL_NORMAL_TEXT, 20, 230, 90, 30, _("Rotation"));
	fl_set_object_lsize(obj, FL_NORMAL_SIZE);
	fl_set_object_lalign(obj, FL_ALIGN_LEFT|FL_ALIGN_INSIDE);
	fl_set_object_lstyle(obj, FL_BOLD_STYLE);
	
	fl_end_form();
	
	//fdui->Figure->fdui = fdui;
	
	return fdui;
}
