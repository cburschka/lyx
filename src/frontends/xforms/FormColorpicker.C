/**
 * \file FormColorpicker.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "FormColorpicker.h"
#include "forms/form_colorpicker.h"

#include "Tooltips.h"
#include "xforms_resize.h"

#include "gettext.h"
#include "lyxrc.h"

#include "support/filetools.h" //  LibFileSearch
#include "support/tostr.h"

#include "lyx_forms.h"


namespace {

enum GuiColors {
	GUI_COLOR_CHOICE   = FL_FREE_COL13,
	GUI_COLOR_HUE_DIAL = FL_FREE_COL14,
};


string const fillLabel(RGBColor const & rgb)
{
	return tostr(rgb.r) + ", " + tostr(rgb.g) + ", " + tostr(rgb.b);
}


string const fillLabel(HSVColor const & hsv)
{
	int const h = int(hsv.h);
	int const s = int(100.0 * hsv.s);
	int const v = int(100.0 * hsv.v);

	return tostr(h) + ", " + tostr(s) + ", " + tostr(v);
}

} // namespace anon


extern "C" {

// Callback function invoked by xforms when the dialog is closed by the
// window manager.
static int C_WMHideCB(FL_FORM * form, void *);

} // extern "C"


FormColorpicker::FormColorpicker()
	: minw_(0), minh_(0),
	  title_(_("Select Color")),
	  finished_(true),
	  icon_pixmap_(0), icon_mask_(0),
	  tooltips_(new Tooltips)
{}


FormColorpicker::~FormColorpicker()
{
	if (icon_pixmap_)
		XFreePixmap(fl_get_display(), icon_pixmap_);
}


RGBColor const & FormColorpicker::requestColor(RGBColor const & color)
{
	input_color_ = color;
	color_ = color;

	show();

	fl_deactivate_all_forms();
	fl_activate_form(form());

	finished_ = false;
	while (!finished_)
		fl_check_forms();

	fl_activate_all_forms();
	return color_;
}


void FormColorpicker::show()
{
	if (!form()) {
		build();
		prepare_to_show();
	}

	// make sure the form is up to date.
	fl_freeze_form(form());
	update();
	fl_unfreeze_form(form());

	if (form()->visible) {
		fl_raise_form(form());
		/* This XMapWindow() will hopefully ensure that
		 * iconified dialogs are de-iconified. Mad props
		 * out to those crazy Xlib guys for forgetting a
		 * XDeiconifyWindow(). At least WindowMaker, when
		 * being notified of the redirected MapRequest will
		 * specifically de-iconify. From source, fvwm2 seems
		 * to do the same.
		 */
		XMapWindow(fl_get_display(), form()->window);
	} else {
		// calls to fl_set_form_minsize/maxsize apply only to the next
		// fl_show_form(), so this comes first.
		fl_set_form_minsize(form(), minw_, minh_);

		string const maximize_title = "LyX: " + title_;
		int const iconify_policy =
			lyxrc.dialogs_iconify_with_main ? FL_TRANSIENT : 0;

		fl_show_form(form(),
			     FL_PLACE_MOUSE | FL_FREE_SIZE,
			     iconify_policy,
			     maximize_title.c_str());
	}
}


void FormColorpicker::hide() const
{
	// xforms sometimes tries to process a hint-type MotionNotify, and
	// use XQueryPointer, without verifying if the window still exists.
	// So we try to clear out motion events in the queue before the
	// DestroyNotify
	XSync(fl_get_display(), false);

	if (form() && form()->visible)
		fl_hide_form(form());
}


void FormColorpicker::build()
{
	dialog_.reset(build_colorpicker(this));
	rgb_.reset(build_colorpicker_rgb(this));
	hsv_.reset(build_colorpicker_hsv(this));

	fl_set_object_color(dialog_->button_color,
			    GUI_COLOR_CHOICE, GUI_COLOR_CHOICE);

	fl_set_object_color(hsv_->dial_hue, GUI_COLOR_HUE_DIAL, FL_BLACK);
	fl_set_dial_bounds(hsv_->dial_hue, 0.0, 360.0);
	fl_set_dial_step(hsv_->dial_hue, 1.0);
	fl_set_dial_return(hsv_->dial_hue, FL_RETURN_CHANGED);

	fl_set_slider_bounds(hsv_->slider_saturation, 0.0, 1.0);
	fl_set_slider_step(hsv_->slider_saturation, 0.01);
	fl_set_slider_return(hsv_->slider_saturation, FL_RETURN_CHANGED);

	fl_set_slider_bounds(hsv_->slider_value, 0.0, 1.0);
	fl_set_slider_step(hsv_->slider_value, 0.01);
	fl_set_slider_return(hsv_->slider_value, FL_RETURN_CHANGED);

	fl_set_slider_bounds(rgb_->slider_red, 0.0, 255.0);
	fl_set_slider_step(rgb_->slider_red, 1.0);
	fl_set_slider_return(rgb_->slider_red, FL_RETURN_CHANGED);

	fl_set_slider_bounds(rgb_->slider_green, 0.0, 255.0);
	fl_set_slider_step(rgb_->slider_green, 1.0);
	fl_set_slider_return(rgb_->slider_green, FL_RETURN_CHANGED);

	fl_set_slider_bounds(rgb_->slider_blue, 0.0, 255.0);
	fl_set_slider_step(rgb_->slider_blue, 1.0);
	fl_set_slider_return(rgb_->slider_blue, FL_RETURN_CHANGED);

	// Stack tabs
	fl_addto_tabfolder(dialog_->tabfolder,_("RGB").c_str(), rgb_->form);
	fl_addto_tabfolder(dialog_->tabfolder,_("HSV").c_str(), hsv_->form);
}


void FormColorpicker::update() const
{
	fl_mapcolor(GUI_COLOR_CHOICE, color_.r, color_.g, color_.b);

	FL_FORM * folder = fl_get_active_folder(dialog_->tabfolder);

	if (!folder)
		folder = rgb_->form;

	if (folder == rgb_->form) {
		fl_set_slider_value(rgb_->slider_red,   color_.r);
		fl_set_slider_value(rgb_->slider_green, color_.g);
		fl_set_slider_value(rgb_->slider_blue,  color_.b);

		fl_set_object_label(dialog_->text_color_values,
				    fillLabel(color_).c_str());

	} else if (folder == hsv_->form) {
		HSVColor hsv = HSVColor(color_);
		hsv.h = std::max(hsv.h, 0.0);

		fl_set_dial_value(hsv_->dial_hue, hsv.h);
		fl_set_slider_value(hsv_->slider_saturation, hsv.s);
		fl_set_slider_value(hsv_->slider_value, hsv.v);

		fl_set_object_label(dialog_->text_color_values,
				    fillLabel(hsv).c_str());

		RGBColor col = HSVColor(hsv.h, 1.0, 1.0);
		col.r = std::max(col.r, 0u);
		fl_mapcolor(GUI_COLOR_HUE_DIAL, col.r, col.g, col.b);
		fl_redraw_object(hsv_->dial_hue);
	}
}


void FormColorpicker::input(FL_OBJECT * ob, long)
{
	if (ob == dialog_->tabfolder) {
		update();

	} else if (ob == hsv_->dial_hue ||
		   ob == hsv_->slider_saturation ||
		   ob == hsv_->slider_value) {
		InputHSV();

	} else if (ob == rgb_->slider_red ||
		   ob == rgb_->slider_green ||
		   ob == rgb_->slider_blue) {
		InputRGB();

	} else if (ob == dialog_->button_ok) {
		hide();
		finished_ = true;

	} else if (ob == dialog_->button_close || ob == 0) {
		color_ = input_color_;
		hide();
		finished_ = true;
	}
}


FL_FORM * FormColorpicker::form() const
{
	return dialog_.get() ? dialog_->form : 0;
}


Tooltips & FormColorpicker::tooltips() const
{
	return *tooltips_;
}


void FormColorpicker::prepare_to_show()
{
	double const scale = get_scale_to_fit(form());
	if (scale > 1.001)
		scale_form_horizontally(form(), scale);

	// work around dumb xforms sizing bug
	minw_ = form()->w;
	minh_ = form()->h;

	fl_set_form_atclose(form(), C_WMHideCB, 0);

	// set the title for the minimized form
	if (!lyxrc.dialogs_iconify_with_main)
		fl_winicontitle(form()->window, title_.c_str());

	//  assign an icon to the form
	string const iconname =
		lyx::support::LibFileSearch("images", "lyx", "xpm");

	if (!iconname.empty()) {
		unsigned int w, h;
		icon_pixmap_ = fl_read_pixmapfile(fl_root,
						  iconname.c_str(),
						  &w,
						  &h,
						  &icon_mask_,
						  0, 0, 0);
		fl_set_form_icon(form(), icon_pixmap_, icon_mask_);
	}
}


void FormColorpicker::InputRGB()
{
	int const red   = int(fl_get_slider_value(rgb_->slider_red));
	int const green = int(fl_get_slider_value(rgb_->slider_green));
	int const blue  = int(fl_get_slider_value(rgb_->slider_blue));

	color_ = RGBColor(red, green, blue);

	fl_freeze_form(dialog_->form);

	fl_set_object_label(dialog_->text_color_values,
			    fillLabel(color_).c_str());

	fl_mapcolor(GUI_COLOR_CHOICE, color_.r, color_.g, color_.b);
	fl_redraw_object(dialog_->button_color);

	fl_unfreeze_form(dialog_->form);
}


void FormColorpicker::InputHSV()
{
	double const hue = fl_get_dial_value(hsv_->dial_hue);
	double const sat = fl_get_slider_value(hsv_->slider_saturation);
	double const val = fl_get_slider_value(hsv_->slider_value);

	HSVColor hsv = HSVColor(hue, sat, val);
	color_ = hsv;

	fl_freeze_form(dialog_->form);

	fl_set_object_label(dialog_->text_color_values, fillLabel(hsv).c_str());

	fl_mapcolor(GUI_COLOR_CHOICE, color_.r, color_.g, color_.b);
	fl_redraw_object(dialog_->button_color);

	RGBColor col = HSVColor(hue, 1.0, 1.0);
	col.r = std::max(col.r, 0u);
	fl_mapcolor(GUI_COLOR_HUE_DIAL, col.r, col.g, col.b);
	fl_redraw_object(hsv_->dial_hue);

	fl_unfreeze_form(dialog_->form);
}


extern "C" {

void C_FormColorpickerInputCB(FL_OBJECT * ob, long d)
{
	BOOST_ASSERT(ob && ob->form && ob->form->u_vdata);
	FormColorpicker * ptr =
		static_cast<FormColorpicker *>(ob->form->u_vdata);
	ptr->input(ob, d);
}


static int C_WMHideCB(FL_FORM * form, void *)
{
	// Close the dialog cleanly, even if the WM is used to do so.
	BOOST_ASSERT(form && form->u_vdata);
	FormColorpicker * ptr = static_cast<FormColorpicker *>(form->u_vdata);
	ptr->input(0, 0);
	return FL_CANCEL;
}

} // extern "C"
