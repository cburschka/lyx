/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2001 The LyX Team.
 *
 *======================================================*/
/* FormPreferences.C
 * FormPreferences Interface Class Implementation
 */

#include <config.h>

#include <utility>
#include <iomanip>
#include <X11/Xlib.h>

#include FORMS_H_LOCATION

#ifdef __GNUG_
#pragma implementation
#endif

#include "Color.h"
#include "LColor.h"
#include "Lsstream.h"
#include "FormPreferences.h"
#include "form_preferences.h"
#include "ButtonController.tmpl"
#include "input_validators.h"
#include "LyXView.h"
#include "language.h"
#include "lyxfunc.h"
#include "Dialogs.h"
#include "lyxrc.h"
#include "combox.h"
#include "debug.h"
#include "lyxlex.h"
#include "input_validators.h"
#include "xforms_helpers.h"
#include "helper_funcs.h"
#include "converter.h"

#include "support/lyxfunctional.h"
#include "support/lyxmanip.h"
#include "support/filetools.h"
#include "support/LAssert.h"

#include "graphics/GraphicsCache.h"

using std::endl;
using std::pair;
using std::make_pair;
using std::max;
using std::min;
using std::vector;
using std::setw;
using std::setfill;

using SigC::slot;

extern string system_lyxdir;
extern string user_lyxdir;
extern Languages languages;

namespace {

// These should probably go inside the class definition...
Formats    local_formats;
Converters local_converters;

} // namespace anon


FormPreferences::FormPreferences(LyXView * lv, Dialogs * d)
	: FormBaseBI(lv, d, _("Preferences"), false),
	  warningPosted(false),
	  colors_(*this), converters_(*this), inputs_misc_(*this),
	  formats_(*this), interface_(*this), language_(*this), 
	  lnf_misc_(*this), outputs_misc_(*this), paths_(*this),
	  printer_(*this), screen_fonts_(*this), spelloptions_(*this)
{
	// let the dialog be shown
	// This is a permanent connection so we won't bother
	// storing a copy because we won't be disconnecting.
	d->showPreferences.connect(slot(this, &FormPreferences::show));
	d->showSpellcheckerPreferences.connect(slot(this, &FormPreferences::showSpellPref));
}


void FormPreferences::redraw()
{
	if (!(form() && form()->visible))
		return;
	fl_redraw_form(form());

	FL_FORM * form2 = fl_get_active_folder(dialog_->tabfolder_prefs);
	if (!(form2 && form2->visible))
		return;
	fl_redraw_form(form2);

	FL_FORM * form3 = 0;
	if (form2 == converters_tab_->form)
		form3 = fl_get_active_folder(converters_tab_->tabfolder_inner);

	else if (form2 == look_n_feel_tab_->form)
		form3 = fl_get_active_folder(look_n_feel_tab_->tabfolder_inner);

	else if (form2 == inputs_tab_->form)
		form3 = fl_get_active_folder(inputs_tab_->tabfolder_inner);

	else if (form2 == outputs_tab_->form)
		form3 = fl_get_active_folder(outputs_tab_->tabfolder_inner);

	else if (form2 == lang_opts_tab_->form)
		form3 = fl_get_active_folder(lang_opts_tab_->tabfolder_inner);

	if (form3 && form3->visible)
		fl_redraw_form(form3);
}


FL_FORM * FormPreferences::form() const
{
	if (dialog_.get()) return dialog_->form;
	return 0;
}


void FormPreferences::showSpellPref()
{
	show();
	fl_set_folder(dialog_->tabfolder_prefs, lang_opts_tab_->form);
	fl_set_folder(lang_opts_tab_->tabfolder_inner, spelloptions_.dialog()->form);
}


void FormPreferences::ok()
{
	FormBaseDeprecated::ok();

	if (colors_.modifiedXformsPrefs) {
		string const filename =
			AddName(user_lyxdir, "preferences.xform");
		colors_.modifiedXformsPrefs = !XformsColor::write(filename);
	}
	
	lv_->getLyXFunc()->dispatch(LFUN_SAVEPREFERENCES);
}


void FormPreferences::hide()
{
	// We need to hide the active tabfolder otherwise we get a
	// BadDrawable error from X window and LyX crashes without saving.
	FL_FORM * inner_form = fl_get_active_folder(dialog_->tabfolder_prefs);
	if (inner_form
	    && inner_form->visible) {
		fl_hide_form(inner_form);
	}
	FormBaseDeprecated::hide();
}


void FormPreferences::build()
{
	dialog_.reset(build_preferences());

	// Manage the restore, save, apply and cancel/close buttons
	bc().setOK(dialog_->button_ok);
	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_cancel);
	bc().setRestore(dialog_->button_restore);

	// build the tab folders
	converters_tab_.reset(build_inner_tab());
	look_n_feel_tab_.reset(build_inner_tab());
	inputs_tab_.reset(build_inner_tab());
	outputs_tab_.reset(build_inner_tab());
	lang_opts_tab_.reset(build_inner_tab());

	// build actual tabfolder contents
	// these will become nested tabfolders
	colors_.build();
	converters_.build();
	formats_.build();
	inputs_misc_.build();
	interface_.build();
	language_.build();
	lnf_misc_.build();
	outputs_misc_.build();
	paths_.build();
	printer_.build();
	screen_fonts_.build();
	spelloptions_.build();

	// Now add them to the tabfolder
	fl_addto_tabfolder(dialog_->tabfolder_prefs,
			   _("Look & Feel"),
			   look_n_feel_tab_->form);
	fl_addto_tabfolder(dialog_->tabfolder_prefs,
			   _("Lang Opts"),
			   lang_opts_tab_->form);
	fl_addto_tabfolder(dialog_->tabfolder_prefs,
			   _("Conversion"),
			   converters_tab_->form);
	fl_addto_tabfolder(dialog_->tabfolder_prefs,
			   _("Inputs"),
			   inputs_tab_->form);
	fl_addto_tabfolder(dialog_->tabfolder_prefs,
			   _("Outputs"),
			   outputs_tab_->form);

	// now build the nested tabfolders
	// Starting with look and feel
	fl_addto_tabfolder(look_n_feel_tab_->tabfolder_inner,
			   _("Screen Fonts"),
			   screen_fonts_.dialog()->form);
	fl_addto_tabfolder(look_n_feel_tab_->tabfolder_inner,
			   _("Interface"),
			   interface_.dialog()->form);
	fl_addto_tabfolder(look_n_feel_tab_->tabfolder_inner,
			   _("Colors"),
			   colors_.dialog()->form);
	fl_addto_tabfolder(look_n_feel_tab_->tabfolder_inner,
			   _("Misc"),
			   lnf_misc_.dialog()->form);

	// then build converters
	fl_addto_tabfolder(converters_tab_->tabfolder_inner,
			   _("Formats"),
			   formats_.dialog()->form);
	fl_addto_tabfolder(converters_tab_->tabfolder_inner,
			   _("Converters"),
			   converters_.dialog()->form);

	// then build inputs
	// Paths should probably go in a few inner_tab called Files
	fl_addto_tabfolder(inputs_tab_->tabfolder_inner,
			   _("Paths"),
			   paths_.dialog()->form);
	fl_addto_tabfolder(inputs_tab_->tabfolder_inner,
			   _("Misc"),
			   inputs_misc_.dialog()->form);

	// then building outputs
	fl_addto_tabfolder(outputs_tab_->tabfolder_inner,
			   _("Printer"),
			   printer_.dialog()->form);
	fl_addto_tabfolder(outputs_tab_->tabfolder_inner,
			   _("Misc"),
			   outputs_misc_.dialog()->form);

	// then building usage
	fl_addto_tabfolder(lang_opts_tab_->tabfolder_inner,
			   _("Spell checker"),
			   spelloptions_.dialog()->form);
	fl_addto_tabfolder(lang_opts_tab_->tabfolder_inner,
			   _("Language"),
			   language_.dialog()->form);
}


void FormPreferences::apply()
{
	// set the new lyxrc entries
	// many of these need to trigger other functions when the assignment
	// is made.  For example, screen zoom and font types.  These could be
	// handled either by signals/slots in lyxrc or just directly call the
	// associated functions here.
	// There are other problems with this scheme.  We really should check
	// what we copy to make sure that it really is necessary to do things
	// like update the screen fonts because that flushes the textcache
	// and other stuff which may cost us a lot on slower/high-load
	// machines.

	colors_.apply();
	formats_.apply();    // Must be before converters_.apply()
	converters_.apply();
	inputs_misc_.apply();
	interface_.apply();
	language_.apply();
	lnf_misc_.apply();
	outputs_misc_.apply();
	paths_.apply();
	printer_.apply();
	screen_fonts_.apply();
	spelloptions_.apply();
}


void FormPreferences::feedback(FL_OBJECT * ob)
{
	lyx::Assert(ob);

	string str;

	if (ob->form->fdui == colors_.dialog()) {
		str = colors_.feedback(ob);
	} else if (ob->form->fdui == converters_.dialog()) {
		str = converters_.feedback(ob);
	} else if (ob->form->fdui == formats_.dialog()) {
		str = formats_.feedback(ob);
	} else if (ob->form->fdui == inputs_misc_.dialog()) {
		str = inputs_misc_.feedback(ob);
	} else if (ob->form->fdui == interface_.dialog()) {
		str = interface_.feedback(ob);
	} else if (ob->form->fdui == language_.dialog()) {
		str = language_.feedback(ob);
	} else if (ob->form->fdui == lnf_misc_.dialog()) {
		str = lnf_misc_.feedback(ob);
	} else if (ob->form->fdui == outputs_misc_.dialog()) {
		str = outputs_misc_.feedback(ob);
	} else if (ob->form->fdui == paths_.dialog()) {
		str = paths_.feedback(ob);
	} else if (ob->form->fdui == printer_.dialog()) {
		str = printer_.feedback(ob);
	} else if (ob->form->fdui == screen_fonts_.dialog()) {
		str = screen_fonts_.feedback(ob);
	} else if (ob->form->fdui == spelloptions_.dialog()) {
		str = spelloptions_.feedback(ob);
	}

	str = formatted(_(str), dialog_->text_warning->w-10, FL_SMALL_SIZE);

	fl_set_object_label(dialog_->text_warning, str.c_str());
	fl_set_object_lsize(dialog_->text_warning, FL_SMALL_SIZE);
}


bool FormPreferences::input(FL_OBJECT * ob, long)
{
	lyx::Assert(ob);
	
	// whatever checks you need to ensure the user hasn't entered
	// some totally ridiculous value somewhere.  Change activate to suit.
	// comments before each test describe what is _valid_

	if (ob->form->fdui == colors_.dialog()) {
		colors_.input(ob);
		return true;
	} else if (ob->form->fdui == converters_.dialog())
		return converters_.input(ob);
	else if (ob->form->fdui == formats_.dialog())
		return formats_.input(ob);
	else if (ob->form->fdui == interface_.dialog())
		return interface_.input(ob);
	else if (ob->form->fdui == language_.dialog())
		return language_.input(ob);
	else if (ob->form->fdui == paths_.dialog())
		return paths_.input(ob);
	else if (ob->form->fdui == screen_fonts_.dialog())
		return screen_fonts_.input();
	else if (ob->form->fdui == spelloptions_.dialog())
		return spelloptions_.input(ob);

	return true;
}


void FormPreferences::update()
{
	if (!dialog_.get()) return;
    
	// read lyxrc entries
	colors_.update();
	formats_.update();   // Must be before converters_.update()
	converters_.update();
	inputs_misc_.update();
	interface_.update();
	language_.update();
	lnf_misc_.update();
	outputs_misc_.update();
	paths_.update();
	printer_.update();
	screen_fonts_.update();
	spelloptions_.update();
}


FormPreferences::Colors::Colors(FormPreferences & p)
	: modifiedXformsPrefs(false), parent_(p)
{}


FD_form_colors const * FormPreferences::Colors::dialog()
{
	return dialog_.get();
}


void FormPreferences::Colors::apply()
{
	bool modifiedText = false;
	bool modifiedBackground = false;

	for (vector<XformsColor>::const_iterator cit = xformsColorDB.begin();
	     cit != xformsColorDB.end(); ++cit) {
		RGBColor col;
		fl_getmcolor(cit->colorID, &col.r, &col.g, &col.b);
		if (col != cit->color()) {
			modifiedXformsPrefs = true;
			if (cit->colorID == FL_BLACK)
				modifiedText = true;
			if (cit->colorID == FL_COL1)
				modifiedBackground = true;
		}
	}

	if (modifiedXformsPrefs) {
		for (vector<XformsColor>::const_iterator cit =
			     xformsColorDB.begin(); 
		     cit != xformsColorDB.end(); ++cit) {
			fl_mapcolor(cit->colorID, cit->r, cit->g, cit->b);

			if (modifiedText && cit->colorID == FL_BLACK) {
				AdjustVal(FL_INACTIVE, FL_BLACK, 0.5);
			}

			if (modifiedBackground && cit->colorID == FL_COL1) {
				AdjustVal(FL_MCOL,      FL_COL1, 0.1);
				AdjustVal(FL_TOP_BCOL,  FL_COL1, 0.1);
				AdjustVal(FL_LEFT_BCOL, FL_COL1, 0.1);

				AdjustVal(FL_RIGHT_BCOL,  FL_COL1, -0.5);
				AdjustVal(FL_BOTTOM_BCOL, FL_COL1, -0.5);
			}

			if (cit->colorID == GUI_COLOR_CURSOR) {
				fl_mapcolor(GUI_COLOR_CURSOR,
					    cit->r, cit->g, cit->b);
				fl_set_cursor_color(FL_DEFAULT_CURSOR,
						    GUI_COLOR_CURSOR, FL_WHITE);
			}
		}
		Dialogs::redrawGUI();
	}

	// Now do the same for the LyX LColors...
	for (vector<NamedColor>::const_iterator cit = lyxColorDB.begin();
	     cit != lyxColorDB.end(); ++cit) {
		LColor::color lc = lcolor.getFromGUIName(cit->getname());
		if (lc == LColor::inherit) continue;

		// Create a valid X11 name of the form "#rrggbb"
		string const hexname = X11hexname(cit->color());
		
		if (lcolor.getX11Name(lc) != hexname) {
			lyxerr[Debug::GUI]
				<< "FormPreferences::Colors::apply: "
				<< "resetting LColor " << lcolor.getGUIName(lc)
				<< " from \"" << lcolor.getX11Name(lc)
				<< "\" to \"" << hexname << "\"."
				<< endl;

			string const s = lcolor.getLyXName(lc) + string(" ") +
				hexname;
			parent_.lv_->getLyXFunc()->dispatch(LFUN_SET_COLOR, s);
		}
	}
}


void FormPreferences::Colors::build()
{
	dialog_.reset(parent_.build_colors());

	fl_set_object_color(dialog_->button_color,
			    GUI_COLOR_CHOICE, GUI_COLOR_CHOICE);

	fl_set_object_color(dialog_->dial_hue, GUI_COLOR_HUE_DIAL, FL_BLACK);
	fl_set_dial_bounds(dialog_->dial_hue, 0.0, 360.0);
	fl_set_dial_step(dialog_->dial_hue, 1.0);
	fl_set_dial_return(dialog_->dial_hue, FL_RETURN_CHANGED);

	fl_set_slider_bounds(dialog_->slider_saturation, 0.0, 1.0);
	fl_set_slider_step(dialog_->slider_saturation, 0.01);
	fl_set_slider_return(dialog_->slider_saturation, FL_RETURN_CHANGED);

	fl_set_slider_bounds(dialog_->slider_value, 0.0, 1.0);
	fl_set_slider_step(dialog_->slider_value, 0.01);
	fl_set_slider_return(dialog_->slider_value, FL_RETURN_CHANGED);
	
	fl_set_slider_bounds(dialog_->slider_red, 0.0, 255.0);
	fl_set_slider_step(dialog_->slider_red, 1.0);
	fl_set_slider_return(dialog_->slider_red, FL_RETURN_CHANGED);
	
	fl_set_slider_bounds(dialog_->slider_green, 0.0, 255.0);
	fl_set_slider_step(dialog_->slider_green, 1.0);
	fl_set_slider_return(dialog_->slider_green, FL_RETURN_CHANGED);
	
	fl_set_slider_bounds(dialog_->slider_blue, 0.0, 255.0);
	fl_set_slider_step(dialog_->slider_blue, 1.0);
	fl_set_slider_return(dialog_->slider_blue, FL_RETURN_CHANGED);
	
	// set up the feedback mechanism
	setPreHandler(dialog_->browser_lyx_objs);
	setPreHandler(dialog_->button_color);
	setPreHandler(dialog_->button_modify);
	setPreHandler(dialog_->dial_hue);
	setPreHandler(dialog_->slider_saturation);
	setPreHandler(dialog_->slider_value);
	setPreHandler(dialog_->slider_red);
	setPreHandler(dialog_->slider_green);
	setPreHandler(dialog_->slider_blue);
	setPreHandler(dialog_->radio_rgb);
	setPreHandler(dialog_->radio_hsv);
}

string const
FormPreferences::Colors::feedback(FL_OBJECT const * const ob) const
{
	string str;

	if (ob == dialog_->browser_lyx_objs) {
		str = N_("LyX objects that can be assigned a color.");

	} else if (ob == dialog_->button_modify) {
		str = N_("Modify the LyX object's color. Note: you must then \"Apply\" the change.");

	} else if (ob == dialog_->dial_hue ||
		   ob == dialog_->slider_saturation ||
		   ob == dialog_->slider_value ||
		   ob == dialog_->slider_red ||
		   ob == dialog_->slider_green ||
		   ob == dialog_->slider_blue) {
		str = N_("Find a new color.");

	} else if (ob == dialog_->radio_rgb ||
		   ob == dialog_->radio_hsv) {
		str = N_("Toggle between RGB and HSV color spaces.");
	}

	return str;
}


void FormPreferences::Colors::input(FL_OBJECT const * const ob)
{
	if (ob == dialog_->browser_lyx_objs) {
		InputBrowserLyX();
		
	} else if (ob == dialog_->dial_hue ||
		   ob == dialog_->slider_saturation ||
		   ob == dialog_->slider_value) {
		InputHSV();

	} else if (ob == dialog_->slider_red ||
		   ob == dialog_->slider_green ||
		   ob == dialog_->slider_blue) {
		InputRGB();

	} else if (ob == dialog_->radio_rgb ||
		   ob == dialog_->radio_hsv) {
		SwitchColorSpace();

	} else if (ob == dialog_->button_modify) {
		Modify();
	}
}


void FormPreferences::Colors::AdjustVal(int colAdjust, int colParent,
					double addVal) const
{
	RGBColor rgb;
	fl_getmcolor(colParent, &rgb.r, &rgb.g, &rgb.b);

	HSVColor hsv(rgb);
	hsv.v += addVal;
	hsv.v = min(1.0, max(0.0, hsv.v));

	rgb = RGBColor(hsv);
	fl_mapcolor(colAdjust, rgb.r, rgb.g, rgb.b);
}


void FormPreferences::Colors::InputBrowserLyX() const
{
	vector<NamedColor>::size_type const selLyX =
		fl_get_browser(dialog_->browser_lyx_objs);
	if (selLyX < 1) return;

	// Is the choice an Xforms color...
	RGBColor col;

	if (selLyX - 1 < xformsColorDB.size()) {
		vector<XformsColor>::size_type const i = selLyX - 1;
		col = xformsColorDB[i].color();
	}
	// or a LyX Logical color?
	else {
		vector<NamedColor>::size_type const i = selLyX - 1 -
			xformsColorDB.size();
		col = lyxColorDB[i].color();
	}

	fl_freeze_form(dialog_->form);

	fl_mapcolor(GUI_COLOR_CHOICE, col.r, col.g, col.b);
	fl_redraw_object(dialog_->button_color);

	// Display either RGB or HSV but not both!
	SwitchColorSpace();
	
	// Deactivate the modify button to begin with...
	setEnabled(dialog_->button_modify, false);
	
	fl_unfreeze_form(dialog_->form);
}


void FormPreferences::Colors::InputHSV()
{
	double const hue = fl_get_dial_value(dialog_->dial_hue);
	double const sat = fl_get_slider_value(dialog_->slider_saturation);
	double const val = fl_get_slider_value(dialog_->slider_value);

	int const h = int(hue);
	int const s = int(100.0 * sat);
	int const v = int(100.0 * val);
	
	string const label = tostr(h) + string(", ") + tostr(s) + string(", ") +
		tostr(v);
	fl_set_object_label(dialog_->text_color_values, label.c_str());

	RGBColor col = HSVColor(hue, sat, val);
	
	fl_freeze_form(dialog_->form);

	fl_mapcolor(GUI_COLOR_CHOICE, col.r, col.g, col.b);
	fl_redraw_object(dialog_->button_color);

	col = HSVColor(hue, 1.0, 1.0);
	col.r = max(col.r, 0);
	fl_mapcolor(GUI_COLOR_HUE_DIAL, col.r, col.g, col.b);
	fl_redraw_object(dialog_->dial_hue);

	// Ascertain whether to activate the Modify button.
	vector<NamedColor>::size_type const selLyX =
		fl_get_browser(dialog_->browser_lyx_objs);

	fl_unfreeze_form(dialog_->form);
	if (selLyX < 1) return;
	
	fl_getmcolor(GUI_COLOR_CHOICE, &col.r, &col.g, &col.b);
	bool modify = false;
	
	// Is the choice an Xforms color...
	if (selLyX - 1 < xformsColorDB.size()) {
		vector<XformsColor>::size_type const i = selLyX - 1;
		modify = (xformsColorDB[i].color() != col);
	}
	// or a LyX Logical color?
	else {
		vector<NamedColor>::size_type const i = selLyX - 1 -
			xformsColorDB.size();
		modify = (lyxColorDB[i].color() != col);
	}

	setEnabled(dialog_->button_modify, modify);
}


void FormPreferences::Colors::InputRGB()
{
	int const red   = int(fl_get_slider_value(dialog_->slider_red));
	int const green = int(fl_get_slider_value(dialog_->slider_green));
	int const blue  = int(fl_get_slider_value(dialog_->slider_blue));

	string const label = tostr(red) + string(", ") + tostr(green) +
		string(", ") + tostr(blue);
	fl_set_object_label(dialog_->text_color_values, label.c_str());
		
	fl_freeze_form(dialog_->form);

	RGBColor col = RGBColor(red, green, blue);
	fl_mapcolor(GUI_COLOR_CHOICE, col.r, col.g, col.b);
	fl_redraw_object(dialog_->button_color);

	// Ascertain whether to activate the Modify button.
	vector<NamedColor>::size_type const selLyX =
		fl_get_browser(dialog_->browser_lyx_objs);

	fl_unfreeze_form(dialog_->form);
	if (selLyX < 1) return;
	
	bool modify = false;
	
	// Is the choice an Xforms color...
	if (selLyX - 1 < xformsColorDB.size()) {
		vector<XformsColor>::size_type const i = selLyX - 1;
		modify = (xformsColorDB[i].color() != col);
	}
	// or a LyX Logical color?
	else {
		vector<NamedColor>::size_type const i = selLyX - 1 -
			xformsColorDB.size();
		modify = (lyxColorDB[i].color() != col);
	}

	setEnabled(dialog_->button_modify, modify);
}


void FormPreferences::Colors::LoadBrowserLyX()
{
	if (!dialog_->browser_lyx_objs->visible) return;

	// First, define the modifiable xforms colors
	xformsColorDB.clear();
	XformsColor xcol;

	xcol.name = _("GUI background");
	xcol.colorID = FL_COL1;
	fl_getmcolor(FL_COL1, &xcol.r, &xcol.g, &xcol.b);

	xformsColorDB.push_back(xcol);

	xcol.name = _("GUI text");
	xcol.colorID = FL_BLACK;
	fl_getmcolor(FL_BLACK, &xcol.r, &xcol.g, &xcol.b);

	fl_mapcolor(GUI_COLOR_CURSOR, xcol.r, xcol.g, xcol.b);
	fl_set_cursor_color(FL_DEFAULT_CURSOR, GUI_COLOR_CURSOR, FL_WHITE);

	xformsColorDB.push_back(xcol);

	xcol.name = _("GUI selection");
	xcol.colorID = FL_YELLOW;
	fl_getmcolor(FL_YELLOW, &xcol.r, &xcol.g, &xcol.b);

	xformsColorDB.push_back(xcol);

	xcol.name = _("GUI pointer");
	xcol.colorID = GUI_COLOR_CURSOR;
	fl_getmcolor(GUI_COLOR_CURSOR, &xcol.r, &xcol.g, &xcol.b);

	xformsColorDB.push_back(xcol);

	// Now create the the LyX LColors database
	lyxColorDB.clear();
	for (int i=0; i<LColor::ignore; ++i) {
		LColor::color lc = static_cast<LColor::color>(i);
		if (lc == LColor::none
		    || lc == LColor::black
		    || lc == LColor::white
		    || lc == LColor::red
		    || lc == LColor::green
		    || lc == LColor::blue
		    || lc == LColor::cyan
		    || lc == LColor::magenta
		    || lc == LColor::yellow
		    || lc == LColor::inherit
		    || lc == LColor::ignore) continue;

		string const name = lcolor.getX11Name(lc);
		Display * display = fl_get_display();;
		Colormap const colormap = fl_state[fl_get_vclass()].colormap;
		XColor xcol, ccol;

		if (XLookupColor(display, colormap, name.c_str(), &xcol, &ccol)
		    == 0) {
			lyxerr << "FormPreferences::Colors::LoadBrowserLyX:\n"
			       << "LColor " << lcolor.getLyXName(lc)
			       << ": X can't find color \"" << name
			       << "\". Set to \"black\"!" << endl;

			string const arg = lcolor.getLyXName(lc) + " black";
			parent_.lv_->getLyXFunc()->
				dispatch(LFUN_SET_COLOR, arg);
			continue;
		}

		// X has found the color. Now find the "appropriate" X11 name
		// for this color.

		// Note that X stores the RGB values in the range 0 - 65535
		// whilst we require them in the range 0 - 255.
		RGBColor col;
		col.r = xcol.red   / 256;
		col.g = xcol.green / 256;
		col.b = xcol.blue  / 256;

		// Create a valid X11 name of the form "#rrggbb" and change the
		// LColor X11name to this. Don't want to trigger a redraw,
		// as we're just changing the name not the RGB values.
		// Also reset the system_lcolor names, so that we don't output
		// unnecessary changes.
		string const hexname = X11hexname(col);

		if (lcolor.getX11Name(lc) != hexname) {
			lcolor.setColor(lc, hexname);
			system_lcolor.setColor(lc, hexname);
		}

		// Finally, push the color onto the database
		NamedColor ncol(lcolor.getGUIName(lc), col);
		lyxColorDB.push_back(ncol);
	}

	// Now construct the browser
	FL_OBJECT * colbr = dialog_->browser_lyx_objs;
	fl_freeze_form(dialog_->form);
	fl_clear_browser(colbr);
	for (vector<XformsColor>::const_iterator cit = xformsColorDB.begin();
	     cit != xformsColorDB.end(); ++cit) {
		fl_addto_browser(colbr, cit->getname().c_str());
	}
	for (vector<NamedColor>::const_iterator cit = lyxColorDB.begin();
	     cit != lyxColorDB.end(); ++cit) {
		fl_addto_browser(colbr, cit->getname().c_str());
	}

	// just to be safe...
	fl_set_browser_topline(dialog_->browser_lyx_objs, 1);
	fl_select_browser_line(dialog_->browser_lyx_objs, 1);
	fl_unfreeze_form(dialog_->form);

	InputBrowserLyX();
}


void FormPreferences::Colors::Modify()
{
	vector<NamedColor>::size_type const selLyX =
		fl_get_browser(dialog_->browser_lyx_objs);
	if (selLyX < 1) return;

	RGBColor col;
	fl_getmcolor(GUI_COLOR_CHOICE, &col.r, &col.g, &col.b);

	// Is the choice an Xforms color...
	if (selLyX - 1 < xformsColorDB.size()) {
		vector<XformsColor>::size_type const i = selLyX - 1;
		xformsColorDB[i].r  = col.r;
		xformsColorDB[i].g  = col.g;
		xformsColorDB[i].b  = col.b;
	}
	// or a LyX Logical color?
	else {
		vector<NamedColor>::size_type const i = selLyX - 1 -
			xformsColorDB.size();
		lyxColorDB[i].r  = col.r;
		lyxColorDB[i].g  = col.g;
		lyxColorDB[i].b  = col.b;
	}

	fl_freeze_form(dialog_->form);
	setEnabled(dialog_->button_modify, false);	
	fl_unfreeze_form(dialog_->form);
}


void FormPreferences::Colors::SwitchColorSpace() const
{
	bool const hsv = fl_get_button(dialog_->radio_hsv);

	RGBColor col;
	fl_getmcolor(GUI_COLOR_CHOICE, &col.r, &col.g, &col.b);

	fl_freeze_form(dialog_->form);

	if (hsv) {
		fl_hide_object(dialog_->slider_red);
		fl_hide_object(dialog_->slider_blue);
		fl_hide_object(dialog_->slider_green);
		fl_show_object(dialog_->dial_hue);
		fl_show_object(dialog_->slider_saturation);
		fl_show_object(dialog_->slider_value);

		HSVColor hsv = HSVColor(col);
		hsv.h = max(hsv.h, 0.0);
	
		fl_set_dial_value(dialog_->dial_hue, hsv.h);
		fl_set_slider_value(dialog_->slider_saturation, hsv.s);
		fl_set_slider_value(dialog_->slider_value, hsv.v);

		col = HSVColor(hsv.h, 1.0, 1.0);
		col.r = max(col.r,0);
		fl_mapcolor(GUI_COLOR_HUE_DIAL, col.r, col.g, col.b);
		fl_redraw_object(dialog_->dial_hue);

		// Adjust the label a bit, but not the actual values.
		// Means that toggling from one space to the other has no
		// effect on the final color.
		int const h = int(hsv.h);
		int const s = int(100*hsv.s);
		int const v = int(100*hsv.v);
		string const label = tostr(h) + string(", ") + tostr(s) +
			string(", ") + tostr(v);
		fl_set_object_label(dialog_->text_color_values, label.c_str());
		
	} else {
		fl_show_object(dialog_->slider_red);
		fl_show_object(dialog_->slider_blue);
		fl_show_object(dialog_->slider_green);
		fl_hide_object(dialog_->dial_hue);
		fl_hide_object(dialog_->slider_saturation);
		fl_hide_object(dialog_->slider_value);

		fl_set_slider_value(dialog_->slider_red,   col.r);
		fl_set_slider_value(dialog_->slider_green, col.g);
		fl_set_slider_value(dialog_->slider_blue,  col.b);

		// Adjust the label a bit. Same reasoning as above.
		int const r = int(col.r);
		int const g = int(col.g);
		int const b = int(col.b);
		string const label = tostr(r) + string(", ") + tostr(g) +
			string(", ") + tostr(b);
		fl_set_object_label(dialog_->text_color_values, label.c_str());
	}
	
	fl_unfreeze_form(dialog_->form);
}

string const FormPreferences::Colors::X11hexname(RGBColor const & col) const
{
	ostringstream ostr;

	ostr << "#" << std::setbase(16) << setfill('0')
	     << setw(2) << col.r
	     << setw(2) << col.g
	     << setw(2) << col.b;

	return ostr.str().c_str();
}


FormPreferences::Converters::Converters(FormPreferences & p)
	: parent_(p)
{}


FD_form_converters const * FormPreferences::Converters::dialog()
{
	return dialog_.get();
}


void FormPreferences::Converters::apply() const
{
	converters = local_converters;
	converters.update(formats);
	converters.buildGraph();
}


void FormPreferences::Converters::build()
{
	dialog_.reset(parent_.build_converters());

	fl_set_input_return(dialog_->input_converter, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_flags, FL_RETURN_CHANGED);

	// set up the feedback mechanism
	setPreHandler(dialog_->browser_all);
	setPreHandler(dialog_->button_delete);
	setPreHandler(dialog_->button_add);
	setPreHandler(dialog_->input_converter);
	setPreHandler(dialog_->choice_from);
	setPreHandler(dialog_->choice_to);
	setPreHandler(dialog_->input_flags);
}


string const
FormPreferences::Converters::feedback(FL_OBJECT const * const ob) const
{
	string str;

	if (ob == dialog_->browser_all) {
		str = N_("All the currently defined converters known to LyX.");
	} else if (ob == dialog_->choice_from) {
		str = N_("Convert \"from\" this format");
	} else if (ob == dialog_->choice_to) {
		str = N_("Convert \"to\" this format");
	} else if (ob == dialog_->input_converter) {
		str = N_("The conversion command. $$i is the input file name, $$b is the file name without its extension and $$o is the name of the output file.");
	} else if (ob == dialog_->input_flags) {
		str = N_("Flags that control the converter behavior");
	} else if (ob == dialog_->button_delete) {
		str = N_("Remove the current converter from the list of available converters. Note: you must then \"Apply\" the change.");
	} else if (ob == dialog_->button_add) {
		if (string(ob->label) == _("Add"))
			str = N_("Add the current converter to the list of available converters. Note: you must then \"Apply\" the change.");
		else
			str = N_("Modify the contents of the current converter. Note: you must then \"Apply\" the change.");
	}

	return str;
}


bool FormPreferences::Converters::input(FL_OBJECT const * const ob)
{
	if (ob == dialog_->browser_all) {
		return Browser();

	} else if (ob == dialog_->choice_from
		   || ob == dialog_->choice_to
		   || ob == dialog_->input_converter
		   || ob == dialog_->input_flags) {
		return Input();

	} else if (ob == dialog_->button_add) {
		return Add();

	} else if (ob == dialog_->button_delete) {
		return erase();
	}

	return true;
}


void FormPreferences::Converters::update()
{
	local_converters = converters;
	local_converters.update(local_formats);
	UpdateBrowser();
}


void FormPreferences::Converters::UpdateBrowser()
{
	local_converters.sort();

	fl_freeze_form(dialog_->form);
	fl_clear_browser(dialog_->browser_all);
	for (::Converters::const_iterator cit = local_converters.begin();
	     cit != local_converters.end(); ++cit) {
		string const name = cit->From->prettyname() + " -> "
			+ cit->To->prettyname();
		fl_addto_browser(dialog_->browser_all, name.c_str());
	}
	Input();
	fl_unfreeze_form(dialog_->form);
}


bool FormPreferences::Converters::Add()
{
	string const from = GetFrom();
	string const to = GetTo();
	string const command = fl_get_input(dialog_->input_converter);
	string const flags = fl_get_input(dialog_->input_flags);

	Converter const * old = local_converters.getConverter(from, to);
	local_converters.add(from, to, command, flags);
	if (!old) {
		local_converters.updateLast(local_formats);
		UpdateBrowser();
	}
	setEnabled(dialog_->button_add, false);

	return true;
}


bool FormPreferences::Converters::Browser() 
{
	int const i = fl_get_browser(dialog_->browser_all);
	if (i <= 0) return false;

	fl_freeze_form(dialog_->form);

	Converter const & c = local_converters.get(i - 1);
	int j = local_formats.getNumber(c.from);
	if (j >= 0)
		fl_set_choice(dialog_->choice_from, j + 1);

	j = local_formats.getNumber(c.to);
	if (j >= 0)
		fl_set_choice(dialog_->choice_to, j + 1);

	fl_set_input(dialog_->input_converter, c.command.c_str());
	fl_set_input(dialog_->input_flags, c.flags.c_str());

	fl_set_object_label(dialog_->button_add, idex(_("Modify|#M")));
	fl_set_button_shortcut(dialog_->button_add, scex(_("Modify|#M")), 1);

	setEnabled(dialog_->button_add,    false);
	setEnabled(dialog_->button_delete, true);
				
	fl_unfreeze_form(dialog_->form);
	return false;
}


bool FormPreferences::Converters::erase()
{
	string const from = GetFrom();
	string const to = GetTo();

	local_converters.erase(from, to);
	UpdateBrowser();
	return true;
}


bool FormPreferences::Converters::Input()
{
	string const from = GetFrom();
	string const to = GetTo();
	int const sel = local_converters.getNumber(from, to);
	
	fl_freeze_form(dialog_->form);

	if (sel < 0) {
		fl_set_object_label(dialog_->button_add, idex(_("Add|#A")));
		fl_set_button_shortcut(dialog_->button_add, 
				       scex(_("Add|#A")), 1);

		fl_deselect_browser(dialog_->browser_all);
		setEnabled(dialog_->button_delete, false);

	} else {
		fl_set_object_label(dialog_->button_add, idex(_("Modify|#M")));
		fl_set_button_shortcut(dialog_->button_add,
					scex(_("Modify|#M")), 1);
		
		int top = max(sel-5, 0);
		fl_set_browser_topline(dialog_->browser_all, top);
		fl_select_browser_line(dialog_->browser_all, sel+1);
		setEnabled(dialog_->button_delete, true);
	}

	string const command = strip(fl_get_input(dialog_->input_converter));
	bool const enable = !(command.empty() || from == to);
	setEnabled(dialog_->button_add, enable);

	fl_unfreeze_form(dialog_->form);
	return false;
}


string const FormPreferences::Converters::GetFrom() const
{
	::Formats::FormatList::size_type const i =
		fl_get_choice(dialog_->choice_from);

	if (i > 0 && i <= local_formats.size())
		return local_formats.get(i - 1).name();
	else {
		lyxerr << "FormPreferences::Converters::GetFrom: No choice!"
		       << endl;
		return "???";
	}
}


string const FormPreferences::Converters::GetTo() const
{
	::Formats::FormatList::size_type const i =
		fl_get_choice(dialog_->choice_to);

	if (i > 0 && i <= local_formats.size())
		return local_formats.get(i - 1).name();
	else {
		lyxerr << "FormPreferences::Converters::GetTo: No choice!"
		       << endl;
		return "???";
	}
}


void FormPreferences::Converters::UpdateChoices() const
{
	string choice;
	for (::Formats::const_iterator cit = local_formats.begin();
	     cit != local_formats.end(); ++cit) {
		if (!choice.empty())
			choice += " | ";
		else
			choice += " ";
		choice += cit->prettyname();
	}
	choice += " ";

	fl_clear_choice(dialog_->choice_from);
	fl_addto_choice(dialog_->choice_from, choice.c_str());

	fl_clear_choice(dialog_->choice_to);
	fl_addto_choice(dialog_->choice_to, choice.c_str());
}


FormPreferences::Formats::Formats(FormPreferences &  p)
	: parent_(p)
{}


FD_form_formats const * FormPreferences::Formats::dialog()
{
	return dialog_.get();
}


void FormPreferences::Formats::apply() const
{
	formats = local_formats;
}


void FormPreferences::Formats::build()
{
	dialog_.reset(parent_.build_formats());

	fl_set_input_return(dialog_->input_format, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_viewer, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_shrtcut, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_gui_name, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_extension, FL_RETURN_CHANGED);

	fl_set_input_filter(dialog_->input_format, fl_lowercase_filter);

	// set up the feedback mechanism
	setPreHandler(dialog_->browser_all);
	setPreHandler(dialog_->input_format);
	setPreHandler(dialog_->input_gui_name);
	setPreHandler(dialog_->button_delete);
	setPreHandler(dialog_->button_add);
	setPreHandler(dialog_->input_extension);
	setPreHandler(dialog_->input_viewer);
	setPreHandler(dialog_->input_shrtcut);
}


string const
FormPreferences::Formats::feedback(FL_OBJECT const * const ob) const
{
	string str;

	if (ob == dialog_->browser_all) {
		str = N_("All the currently defined formats known to LyX.");
	} else if (ob == dialog_->input_format) {
		str = N_("The format identifier.");
	} else if (ob == dialog_->input_gui_name) {
		str = N_("The format name as it will appear in the menus.");
	} else if (ob == dialog_->input_shrtcut) {
		str = N_("The keyboard accelerator. Use a letter in the GUI name. Case sensitive.");
	} else if (ob == dialog_->input_extension) {
		str = N_("Used to recognize the file. E.g., ps, pdf, tex.");
	} else if (ob == dialog_->input_viewer) {
		str = N_("The command used to launch the viewer application.");
	} else if (ob == dialog_->button_delete) {
		str = N_("Remove the current format from the list of available formats. Note: you must then \"Apply\" the change.");
	} else if (ob == dialog_->button_add) {
		if (string(ob->label) == _("Add"))
			str = N_("Add the current format to the list of available formats. Note: you must then \"Apply\" the change.");
		else
			str = N_("Modify the contents of the current format. Note: you must then \"Apply\" the change.");
	}

	return str;
}


bool FormPreferences::Formats::input(FL_OBJECT const * const ob)
{
	if (ob == dialog_->browser_all) {
		return Browser();

	} else if (ob == dialog_->input_format
		   || ob == dialog_->input_gui_name
		   || ob == dialog_->input_shrtcut
		   || ob == dialog_->input_extension
		   || ob == dialog_->input_viewer) {
		return Input();

	} else if (ob == dialog_->button_add) {
		return Add();

	} else if (ob == dialog_->button_delete) {
		return erase();
	}

	return false;
}


void FormPreferences::Formats::update()
{
	local_formats = formats;
	UpdateBrowser();
}


void FormPreferences::Formats::UpdateBrowser()
{
	local_formats.sort();

	fl_freeze_form(dialog_->form);
	fl_deselect_browser(dialog_->browser_all);
	fl_clear_browser(dialog_->browser_all);
	for (::Formats::const_iterator cit = local_formats.begin();
	     cit != local_formats.end(); ++cit)
		fl_addto_browser(dialog_->browser_all,
				 cit->prettyname().c_str());

	Input();
	fl_unfreeze_form(dialog_->form);

	// Mustn't forget to update the Formats available to the converters_
	parent_.converters_.UpdateChoices();
	local_converters.update(local_formats);
}


bool FormPreferences::Formats::Add()
{
	string const name = fl_get_input(dialog_->input_format);
	string const prettyname = fl_get_input(dialog_->input_gui_name);
	string const extension = fl_get_input(dialog_->input_extension);
	string const shortcut =  fl_get_input(dialog_->input_shrtcut);
	string const viewer =  fl_get_input(dialog_->input_viewer);

	Format const * old = local_formats.getFormat(name);
	string const old_prettyname = old ? old->prettyname() : string();
	local_formats.add(name, extension, prettyname, shortcut);
	local_formats.setViewer(name, viewer);
	if (!old || prettyname != old_prettyname) {
		UpdateBrowser();
		if (old)
			parent_.converters_.UpdateBrowser();
	}
	setEnabled(dialog_->button_add, false);

	return true;
}


bool FormPreferences::Formats::Browser() 
{
	int const i = fl_get_browser(dialog_->browser_all);
	if (i <= 0) return false;

	fl_freeze_form(dialog_->form);

	Format const & f = local_formats.get(i - 1);

	fl_set_input(dialog_->input_format, f.name().c_str());
	fl_set_input(dialog_->input_gui_name, f.prettyname().c_str());
	fl_set_input(dialog_->input_shrtcut, f.shortcut().c_str());
	fl_set_input(dialog_->input_extension, f.extension().c_str());
	fl_set_input(dialog_->input_viewer, f.viewer().c_str());

	fl_set_object_label(dialog_->button_add, idex(_("Modify|#M")));
	fl_set_button_shortcut(dialog_->button_add, scex(_("Modify|#M")), 1);

	setEnabled(dialog_->button_add,    false);
	setEnabled(dialog_->button_delete, true);
				
	fl_unfreeze_form(dialog_->form);
	return false;
}


bool FormPreferences::Formats::erase()
{
	string const name = fl_get_input(dialog_->input_format);

	if (local_converters.formatIsUsed(name)) {
		parent_.printWarning(_("Cannot remove a Format used by a Converter. Remove the converter first."));
		setEnabled(dialog_->button_delete, false);
		return false;
	}

	local_formats.erase(name);
	UpdateBrowser();
	return true;
}


bool FormPreferences::Formats::Input()
{
	string const name = fl_get_input(dialog_->input_format);
	int const sel = local_formats.getNumber(name);
	fl_freeze_form(dialog_->form);

	if (sel < 0) {
		fl_set_object_label(dialog_->button_add,
				     idex(_("Add|#A")));
		fl_set_button_shortcut(dialog_->button_add,
					scex(_("Add|#A")), 1);

		fl_deselect_browser(dialog_->browser_all);
		setEnabled(dialog_->button_delete, false);

	} else {
		fl_set_object_label(dialog_->button_add,
				     idex(_("Modify|#M")));
		fl_set_button_shortcut(dialog_->button_add,
					scex(_("Modify|#M")), 1);

		int const top = max(sel-5, 0);
		fl_set_browser_topline(dialog_->browser_all, top);
		fl_select_browser_line(dialog_->browser_all, sel+1);

		setEnabled(dialog_->button_add, true);
		setEnabled(dialog_->button_delete, true);
	}

	string const prettyname = fl_get_input(dialog_->input_gui_name);
	bool const enable = !(name.empty() || prettyname.empty());
	setEnabled(dialog_->button_add, enable);

	fl_unfreeze_form(dialog_->form);
	return false;
}


FormPreferences::InputsMisc::InputsMisc(FormPreferences &  p)
	: parent_(p)
{}


FD_form_inputs_misc const * FormPreferences::InputsMisc::dialog()
{
	return dialog_.get();
}


void FormPreferences::InputsMisc::apply() const
{
	lyxrc.date_insert_format =
		fl_get_input(dialog_->input_date_format);
}


void FormPreferences::InputsMisc::build()
{
	dialog_.reset(parent_.build_inputs_misc());

	fl_set_input_return(dialog_->input_date_format, FL_RETURN_CHANGED);

	// set up the feedback mechanism
	setPreHandler(dialog_->input_date_format);
}


string const
FormPreferences::InputsMisc::feedback(FL_OBJECT const * const ob) const
{
	string str;

	if (ob == dialog_->input_date_format)
		str = lyxrc.getDescription(LyXRC::RC_DATE_INSERT_FORMAT);

	return str;
}


void FormPreferences::InputsMisc::update()
{
	fl_set_input(dialog_->input_date_format,
		     lyxrc.date_insert_format.c_str());
}


FormPreferences::Interface::Interface(FormPreferences &  p)
	: parent_(p)
{}


FD_form_interface const * FormPreferences::Interface::dialog()
{
	return dialog_.get();
}


void FormPreferences::Interface::apply() const
{
	lyxrc.popup_normal_font =
		fl_get_input(dialog_->input_popup_normal_font);
	lyxrc.popup_bold_font = fl_get_input(dialog_->input_popup_bold_font);
	lyxrc.popup_font_encoding =
		fl_get_input(dialog_->input_popup_font_encoding);
	lyxrc.bind_file = fl_get_input(dialog_->input_bind_file);
	lyxrc.ui_file = fl_get_input(dialog_->input_ui_file);
	lyxrc.override_x_deadkeys =
		fl_get_button(dialog_->check_override_x_dead_keys);
}


void FormPreferences::Interface::build()
{
	dialog_.reset(parent_.build_interface());

	fl_set_input_return(dialog_->input_popup_normal_font, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_popup_bold_font, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_popup_font_encoding, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_bind_file, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_ui_file, FL_RETURN_CHANGED);

	// set up the feedback mechanism
	setPreHandler(dialog_->input_popup_normal_font);
	setPreHandler(dialog_->input_popup_bold_font);
	setPreHandler(dialog_->input_popup_font_encoding);
	setPreHandler(dialog_->input_bind_file);
	setPreHandler(dialog_->button_bind_file_browse);
	setPreHandler(dialog_->input_ui_file);
	setPreHandler(dialog_->button_ui_file_browse);
	setPreHandler(dialog_->check_override_x_dead_keys);
}


string const
FormPreferences::Interface::feedback(FL_OBJECT const * const ob) const
{
	string str;

	if (ob == dialog_->input_popup_normal_font)
		str = lyxrc.getDescription(LyXRC::RC_POPUP_NORMAL_FONT);
	else if (ob == dialog_->input_popup_bold_font)
		str = lyxrc.getDescription(LyXRC::RC_POPUP_BOLD_FONT);
	else if (ob == dialog_->input_popup_font_encoding)
		str = lyxrc.getDescription(LyXRC::RC_POPUP_FONT_ENCODING);
	else if (ob == dialog_->input_bind_file)
		str = lyxrc.getDescription(LyXRC::RC_BINDFILE);
	else if (ob == dialog_->input_ui_file)
		str = lyxrc.getDescription(LyXRC::RC_UIFILE);
	else if (ob == dialog_->check_override_x_dead_keys)
		str = lyxrc.getDescription(LyXRC::RC_OVERRIDE_X_DEADKEYS);

	return str;
}


bool FormPreferences::Interface::input(FL_OBJECT const * const ob)
{
	if (ob == dialog_->button_bind_file_browse) {
		string dir  = AddName(system_lyxdir, "bind");
		string name = N_("Sys Bind|#S#s");
		pair<string,string> dir1(name, dir);

		dir = AddName(user_lyxdir, "bind");
		name = N_("User Bind|#U#u");
		pair<string,string> dir2(name, dir);

		parent_.browse(dialog_->input_bind_file,
			       N_("Bind file"), "*.bind", dir1, dir2);
		
	} else if (ob == dialog_->button_ui_file_browse) {
		string dir  = AddName(system_lyxdir, "ui");
		string name = N_("Sys UI|#S#s");
		pair<string,string> dir1(name, dir);

		dir = AddName(user_lyxdir, "ui");
		name = N_("User UI|#U#u");
		pair<string,string> dir2(name, dir);

		parent_.browse(dialog_->input_ui_file,
			       N_("UI file"), "*.ui", dir1, dir2);
	}
	
	return true;
}


void FormPreferences::Interface::update()
{
	fl_set_input(dialog_->input_popup_normal_font,
		     lyxrc.popup_normal_font.c_str());
	fl_set_input(dialog_->input_popup_bold_font,
		     lyxrc.popup_bold_font.c_str());
	fl_set_input(dialog_->input_popup_font_encoding,
		     lyxrc.popup_font_encoding.c_str());
	fl_set_input(dialog_->input_bind_file,
		     lyxrc.bind_file.c_str());
	fl_set_input(dialog_->input_ui_file,
		     lyxrc.ui_file.c_str());
	fl_set_button(dialog_->check_override_x_dead_keys,
		      lyxrc.override_x_deadkeys);
}


FormPreferences::Language::Language(FormPreferences &  p)
	: parent_(p)
{}


FD_form_language const * FormPreferences::Language::dialog()
{
	return dialog_.get();
}


void FormPreferences::Language::apply()
{
	lyxrc.default_language = combo_default_lang->getline();

	int button = fl_get_button(dialog_->check_use_kbmap);
	string const name_1 = fl_get_input(dialog_->input_kbmap1);
	string const name_2 = fl_get_input(dialog_->input_kbmap2);
	if (button)
		button = !(name_1.empty() && name_2.empty());
	lyxrc.use_kbmap = static_cast<bool>(button);

	if (lyxrc.use_kbmap) {
		lyxrc.primary_kbmap = name_1;
		lyxrc.secondary_kbmap = name_2;
	}
	
	button = fl_get_button(dialog_->check_rtl_support);
	lyxrc.rtl_support = static_cast<bool>(button);

	button = fl_get_button(dialog_->check_mark_foreign);
	lyxrc.mark_foreign_language = static_cast<bool>(button);

	button = fl_get_button(dialog_->check_auto_begin);
	lyxrc.language_auto_begin = static_cast<bool>(button);

	button = fl_get_button(dialog_->check_auto_end);
	lyxrc.language_auto_end = static_cast<bool>(button);

	button = fl_get_button(dialog_->check_use_babel);
	lyxrc.language_use_babel = static_cast<bool>(button);

	button = fl_get_button(dialog_->check_global_options);
	lyxrc.language_global_options = static_cast<bool>(button);

	lyxrc.language_package = fl_get_input(dialog_->input_package);
	lyxrc.language_command_begin = fl_get_input(dialog_->input_command_begin);
	lyxrc.language_command_end = fl_get_input(dialog_->input_command_end);

	// Ensure that all is self-consistent.
	update();
}


void FormPreferences::Language::build()
{
	dialog_.reset(parent_.build_language());

	fl_set_input_return(dialog_->input_package, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_command_begin, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_command_end, FL_RETURN_CHANGED);

	// The default_language is a combo-box and has to be inserted manually
	fl_freeze_form(dialog_->form);
	fl_addto_form(dialog_->form);

	FL_OBJECT * obj = dialog_->choice_default_lang;
	fl_deactivate_object(dialog_->choice_default_lang);
	combo_default_lang.reset(new Combox(FL_COMBOX_DROPLIST));
	combo_default_lang->add(obj->x, obj->y, obj->w, obj->h, 400,
				parent_.lang_opts_tab_->tabfolder_inner,
				parent_.dialog_->tabfolder_prefs);
	combo_default_lang->shortcut("#L",1);
	combo_default_lang->setcallback(ComboCB, &parent_);

	for (Languages::const_iterator cit = languages.begin();
	    cit != languages.end(); ++cit) {
		combo_default_lang->addto(cit->second.lang());
	}

	fl_end_form();
	fl_unfreeze_form(dialog_->form);

	// set up the feedback mechanism
	setPreHandler(dialog_->input_package);
	setPreHandler(dialog_->check_use_kbmap);

	// This is safe, as nothing is done to the pointer, other than
	// to use its address in a block-if statement.
	// No it's not! Leads to crash.
	// setPreHandler(
	// 		reinterpret_cast<FL_OBJECT *>(combo_default_lang),
	//		C_FormPreferencesFeedbackCB);

	setPreHandler(dialog_->input_kbmap1);
	setPreHandler(dialog_->input_kbmap2);
	setPreHandler(dialog_->check_rtl_support);
	setPreHandler(dialog_->check_mark_foreign);
	setPreHandler(dialog_->check_auto_begin);
	setPreHandler(dialog_->check_auto_end);
	setPreHandler(dialog_->check_use_babel);
	setPreHandler(dialog_->check_global_options);
	setPreHandler(dialog_->input_command_begin);
	setPreHandler(dialog_->input_command_end);

	// Activate/Deactivate the input fields dependent on the state of the
	// buttons.
	input(0);
}


string const
FormPreferences::Language::feedback(FL_OBJECT const * const ob) const
{
	string str;

	if (reinterpret_cast<Combox const *>(ob) == combo_default_lang.get())
		str = lyxrc.getDescription(LyXRC::RC_DEFAULT_LANGUAGE);
	else if (ob == dialog_->check_use_kbmap)
		str = lyxrc.getDescription(LyXRC::RC_KBMAP);
	else if (ob == dialog_->input_kbmap1)
		str = lyxrc.getDescription(LyXRC::RC_KBMAP_PRIMARY);
	else if (ob == dialog_->input_kbmap2)
		str = lyxrc.getDescription(LyXRC::RC_KBMAP_SECONDARY);
	else if (ob == dialog_->check_rtl_support)
		str = lyxrc.getDescription(LyXRC::RC_RTL_SUPPORT);
	else if (ob == dialog_->check_mark_foreign)
		str = lyxrc.getDescription(LyXRC::RC_MARK_FOREIGN_LANGUAGE);
	else if (ob == dialog_->check_auto_begin)
		str = lyxrc.getDescription(LyXRC::RC_LANGUAGE_AUTO_BEGIN);
	else if (ob == dialog_->check_auto_end)
		str = lyxrc.getDescription(LyXRC::RC_LANGUAGE_AUTO_END);
	else if (ob == dialog_->check_use_babel)
		str = lyxrc.getDescription(LyXRC::RC_LANGUAGE_USE_BABEL);
	else if (ob == dialog_->check_global_options)
		str = lyxrc.getDescription(LyXRC::RC_LANGUAGE_GLOBAL_OPTIONS);
	else if (ob == dialog_->input_package)
		str = lyxrc.getDescription(LyXRC::RC_LANGUAGE_PACKAGE);
	else if (ob == dialog_->input_command_begin)
		str = lyxrc.getDescription(LyXRC::RC_LANGUAGE_COMMAND_BEGIN);
	else if (ob == dialog_->input_command_end)
		str = lyxrc.getDescription(LyXRC::RC_LANGUAGE_COMMAND_END);

	return str;
}


bool FormPreferences::Language::input(FL_OBJECT const * const ob)
{
	bool activate = true;

	// !ob if function is called from Language::build() to de/activate
	// objects,
	// otherwise the function is called by an xforms CB via input().
	if (!ob || ob == dialog_->check_use_kbmap) {
		bool const enable = fl_get_button(dialog_->check_use_kbmap);
		setEnabled(dialog_->button_kbmap1_browse, enable);
		setEnabled(dialog_->button_kbmap2_browse, enable);
		setEnabled(dialog_->input_kbmap1, enable);
		setEnabled(dialog_->input_kbmap2, enable);
	}

	if (ob == dialog_->button_kbmap1_browse) {
		string const dir  = AddName(system_lyxdir, "kbd");
		string const name = N_("Key maps|#K#k");
		pair<string, string> dir1(name, dir);

		parent_.browse(dialog_->input_kbmap1,
			       N_("Keyboard map"), "*.kmap", dir1);
	} else if (ob == dialog_->button_kbmap2_browse) {
		string const dir  = AddName(system_lyxdir, "kbd");
		string const name = N_("Key maps|#K#k");
		pair<string, string> dir1(name, dir);

		parent_.browse(dialog_->input_kbmap2,
			       N_("Keyboard map"), "*.kmap", dir1);
	}

	return activate;
}


void FormPreferences::Language::update()
{
	fl_set_button(dialog_->check_use_kbmap,
		      lyxrc.use_kbmap);

	combo_default_lang->select(lyxrc.default_language);

	if (lyxrc.use_kbmap) {
		fl_set_input(dialog_->input_kbmap1,
			     lyxrc.primary_kbmap.c_str());
		fl_set_input(dialog_->input_kbmap2,
			     lyxrc.secondary_kbmap.c_str());
	} else {
		fl_set_input(dialog_->input_kbmap1, "");
		fl_set_input(dialog_->input_kbmap2, "");
	}
	
	fl_set_button(dialog_->check_rtl_support, lyxrc.rtl_support);
	fl_set_button(dialog_->check_mark_foreign,
		      lyxrc.mark_foreign_language);
	fl_set_button(dialog_->check_auto_begin, lyxrc.language_auto_begin);
	fl_set_button(dialog_->check_auto_end, lyxrc.language_auto_end);
	fl_set_button(dialog_->check_use_babel, lyxrc.language_use_babel);
	fl_set_button(dialog_->check_global_options,
		      lyxrc.language_global_options);

	fl_set_input(dialog_->input_package,
		     lyxrc.language_package.c_str());
	fl_set_input(dialog_->input_command_begin,
		     lyxrc.language_command_begin.c_str());
	fl_set_input(dialog_->input_command_end,
		     lyxrc.language_command_end.c_str());

	// Activate/Deactivate the input fields dependent on the state of the
	// buttons.
	input(0);
}


void FormPreferences::Language::ComboCB(int, void * v, Combox * combox)
{
    FormPreferences * pre = static_cast<FormPreferences*>(v);
    // This is safe, as nothing is done to the pointer, other than
    // to use its address in a block-if statement.
    pre->bc().valid(pre->input(reinterpret_cast<FL_OBJECT *>(combox), 0));
}


FormPreferences::LnFmisc::LnFmisc(FormPreferences &  p)
	: parent_(p)
{}


FD_form_lnf_misc const * FormPreferences::LnFmisc::dialog()
{
	return dialog_.get();
}


void FormPreferences::LnFmisc::apply() const
{
	lyxrc.show_banner = fl_get_button(dialog_->check_banner);
	lyxrc.auto_region_delete =
		fl_get_button(dialog_->check_auto_region_delete);
	lyxrc.exit_confirmation = fl_get_button(dialog_->check_exit_confirm);
	lyxrc.display_shortcuts =
		fl_get_button(dialog_->check_display_shrtcuts);
	lyxrc.new_ask_filename = fl_get_button(dialog_->check_ask_new_file);
	lyxrc.cursor_follows_scrollbar =
		fl_get_button(dialog_->check_cursor_follows_scrollbar);
	lyxrc.dialogs_iconify_with_main =
		fl_get_button(dialog_->check_dialogs_iconify_with_main);
	lyxrc.autosave = static_cast<unsigned int>
		(fl_get_counter_value(dialog_->counter_autosave));
	lyxrc.wheel_jump = static_cast<unsigned int>
		(fl_get_counter_value(dialog_->counter_wm_jump));

	string const old_value = lyxrc.display_graphics;
	if (fl_get_button(dialog_->radio_display_monochrome)) {
		lyxrc.display_graphics = "mono";
	} else if (fl_get_button(dialog_->radio_display_grayscale)) {
		lyxrc.display_graphics = "gray";
	} else if (fl_get_button(dialog_->radio_display_color)) {
		lyxrc.display_graphics = "color";
	} else {
		lyxrc.display_graphics = "no";
	}
	if (old_value != lyxrc.display_graphics) {
		grfx::GCache & gc = grfx::GCache::get();
		gc.changeDisplay();
	}
}


void FormPreferences::LnFmisc::build()
{
	dialog_.reset(parent_.build_lnf_misc());

	fl_set_counter_step(dialog_->counter_autosave, 1, 10);
	fl_set_counter_step(dialog_->counter_wm_jump, 1, 10);

	fl_set_counter_return(dialog_->counter_autosave, FL_RETURN_CHANGED);
	fl_set_counter_return(dialog_->counter_wm_jump, FL_RETURN_CHANGED);

	// set up the feedback mechanism
	setPreHandler(dialog_->check_banner);
	setPreHandler(dialog_->check_auto_region_delete);
	setPreHandler(dialog_->check_exit_confirm);
	setPreHandler(dialog_->check_display_shrtcuts);
	setPreHandler(dialog_->counter_autosave);
	setPreHandler(dialog_->check_ask_new_file);
	setPreHandler(dialog_->check_cursor_follows_scrollbar);
	setPreHandler(dialog_->check_dialogs_iconify_with_main);
	setPreHandler(dialog_->counter_wm_jump);
	setPreHandler(dialog_->radio_display_monochrome);
	setPreHandler(dialog_->radio_display_grayscale);
	setPreHandler(dialog_->radio_display_color);
	setPreHandler(dialog_->radio_no_display);
}


string const
FormPreferences::LnFmisc::feedback(FL_OBJECT const * const ob) const
{
	string str;

	if (ob == dialog_->check_banner)
		str = lyxrc.getDescription(LyXRC::RC_SHOW_BANNER);
	else if (ob == dialog_->check_auto_region_delete)
		str = lyxrc.getDescription(LyXRC::RC_AUTOREGIONDELETE);
	else if (ob == dialog_->check_exit_confirm)
		str = lyxrc.getDescription(LyXRC::RC_EXIT_CONFIRMATION);
	else if (ob == dialog_->check_display_shrtcuts)
		str = lyxrc.getDescription(LyXRC::RC_DISPLAY_SHORTCUTS);
	else if (ob == dialog_->check_ask_new_file)
		str = lyxrc.getDescription(LyXRC::RC_NEW_ASK_FILENAME);
	else if (ob == dialog_->check_cursor_follows_scrollbar)
		str = lyxrc.getDescription(LyXRC::RC_CURSOR_FOLLOWS_SCROLLBAR);
	else if (ob == dialog_->check_dialogs_iconify_with_main)
		str = lyxrc.getDescription(LyXRC::RC_DIALOGS_ICONIFY_WITH_MAIN);
	else if (ob == dialog_->counter_autosave)
		str = lyxrc.getDescription(LyXRC::RC_AUTOSAVE);
	else if (ob == dialog_->counter_wm_jump)
		str = lyxrc.getDescription(LyXRC::RC_WHEEL_JUMP);
	else if (ob == dialog_->radio_display_monochrome ||
		 ob == dialog_->radio_display_grayscale ||
		 ob == dialog_->radio_display_color ||
		 ob == dialog_->radio_no_display)
		str = lyxrc.getDescription(LyXRC::RC_DISPLAY_GRAPHICS);

	return str;
}


void FormPreferences::LnFmisc::update()
{
	fl_set_button(dialog_->check_banner, lyxrc.show_banner);
	fl_set_button(dialog_->check_auto_region_delete, 
		      lyxrc.auto_region_delete);
	fl_set_button(dialog_->check_exit_confirm, lyxrc.exit_confirmation);
	fl_set_button(dialog_->check_display_shrtcuts, lyxrc.display_shortcuts);
	fl_set_button(dialog_->check_ask_new_file, lyxrc.new_ask_filename);
	fl_set_button(dialog_->check_cursor_follows_scrollbar,
		      lyxrc.cursor_follows_scrollbar);
	fl_set_button(dialog_->check_dialogs_iconify_with_main,
		      lyxrc.dialogs_iconify_with_main);
	fl_set_counter_value(dialog_->counter_autosave, lyxrc.autosave);
	fl_set_counter_value(dialog_->counter_wm_jump, lyxrc.wheel_jump);

	if (lyxrc.display_graphics == "mono") {
		fl_set_button(dialog_->radio_display_monochrome, 1);
	} else if (lyxrc.display_graphics == "gray") {
		fl_set_button(dialog_->radio_display_grayscale, 1);
	} else if (lyxrc.display_graphics == "color") {
		fl_set_button(dialog_->radio_display_color, 1);
	} else {
		fl_set_button(dialog_->radio_no_display, 1);
	}
}


FormPreferences::OutputsMisc::OutputsMisc(FormPreferences &  p)
	: parent_(p)
{}


FD_form_outputs_misc const * FormPreferences::OutputsMisc::dialog()
{
	return dialog_.get();
}


void FormPreferences::OutputsMisc::apply() const
{
	lyxrc.ascii_linelen = static_cast<unsigned int>
		(fl_get_counter_value(dialog_->counter_line_len));
	lyxrc.fontenc = fl_get_input(dialog_->input_tex_encoding);

	int const choice =
		fl_get_choice(dialog_->choice_default_papersize) - 1;
	lyxrc.default_papersize = static_cast<BufferParams::PAPER_SIZE>(choice);

	lyxrc.ascii_roff_command = fl_get_input(dialog_->input_ascii_roff);
	lyxrc.chktex_command = fl_get_input(dialog_->input_checktex);
	lyxrc.view_dvi_paper_option = fl_get_input(dialog_->input_paperoption);
	lyxrc.auto_reset_options = fl_get_button(dialog_->check_autoreset_classopt);
}


void FormPreferences::OutputsMisc::build()
{
	dialog_.reset(parent_.build_outputs_misc());

	fl_set_counter_step(dialog_->counter_line_len, 1, 10);

	fl_set_counter_return(dialog_->counter_line_len, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_tex_encoding, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_ascii_roff,   FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_checktex,     FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_paperoption,  FL_RETURN_CHANGED);

	fl_addto_choice(dialog_->choice_default_papersize,
			_(" default | US letter | legal | executive | A3 | A4 | A5 | B5 "));

	// set up the feedback mechanism
	setPreHandler(dialog_->counter_line_len);
	setPreHandler(dialog_->input_tex_encoding);
	setPreHandler(dialog_->choice_default_papersize);
	setPreHandler(dialog_->input_ascii_roff);
	setPreHandler(dialog_->input_checktex);
	setPreHandler(dialog_->input_paperoption);
	setPreHandler(dialog_->check_autoreset_classopt);
}


string const
FormPreferences::OutputsMisc::feedback(FL_OBJECT const * const ob) const
{
	string str;

	if (ob == dialog_->counter_line_len)
		str = lyxrc.getDescription(LyXRC::RC_ASCII_LINELEN);
	else if (ob == dialog_->input_tex_encoding)
		str = lyxrc.getDescription(LyXRC::RC_FONT_ENCODING);
	else if (ob == dialog_->input_ascii_roff)
		str = lyxrc.getDescription(LyXRC::RC_ASCIIROFF_COMMAND);
	else if (ob == dialog_->input_checktex)
		str = lyxrc.getDescription(LyXRC::RC_CHKTEX_COMMAND);
	else if (ob == dialog_->choice_default_papersize)
		str = lyxrc.getDescription(LyXRC::RC_DEFAULT_PAPERSIZE);
	else if (ob == dialog_->input_paperoption)
		str = lyxrc.getDescription(LyXRC::RC_VIEWDVI_PAPEROPTION);
	else if (ob == dialog_->check_autoreset_classopt)
		str = lyxrc.getDescription(LyXRC::RC_AUTORESET_OPTIONS);

	return str;
}


void FormPreferences::OutputsMisc::update()
{
	fl_set_counter_value(dialog_->counter_line_len,
			     lyxrc.ascii_linelen);
	fl_set_input(dialog_->input_tex_encoding,
		     lyxrc.fontenc.c_str());
	fl_set_choice(dialog_->choice_default_papersize,
		      lyxrc.default_papersize+1);
	fl_set_input(dialog_->input_ascii_roff,
		     lyxrc.ascii_roff_command.c_str());
	fl_set_input(dialog_->input_checktex,
		     lyxrc.chktex_command.c_str());
	fl_set_input(dialog_->input_paperoption,
                     lyxrc.view_dvi_paper_option.c_str());
	fl_set_button(dialog_->check_autoreset_classopt,
                     lyxrc.auto_reset_options);

}


FormPreferences::Paths::Paths(FormPreferences &  p)
	: parent_(p)
{}


FD_form_paths const * FormPreferences::Paths::dialog()
{
	return dialog_.get();
}


void FormPreferences::Paths::apply()
{
	lyxrc.document_path = fl_get_input(dialog_->input_default_path);
	lyxrc.template_path = fl_get_input(dialog_->input_template_path);

	int button = fl_get_button(dialog_->check_use_temp_dir);
	string str  = fl_get_input(dialog_->input_temp_dir);
	if (!button) str.erase();

	lyxrc.use_tempdir = button;
	lyxrc.tempdir_path = str;

	button = fl_get_button(dialog_->check_last_files);
	str = fl_get_input(dialog_->input_lastfiles);
	if (!button) str.erase();
	
	lyxrc.check_lastfiles = button;
	lyxrc.lastfiles = str;
	lyxrc.num_lastfiles = static_cast<unsigned int>
		(fl_get_counter_value(dialog_->counter_lastfiles));

	button = fl_get_button(dialog_->check_make_backups);
	str = fl_get_input(dialog_->input_backup_path);
	if (!button) str.erase();

	lyxrc.make_backup = button;
	lyxrc.backupdir_path = str;

	lyxrc.lyxpipes = fl_get_input(dialog_->input_serverpipe);

	// update view
	update();
}


void FormPreferences::Paths::build()
{
	dialog_.reset(parent_.build_paths());

	fl_set_input_return(dialog_->input_default_path, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_template_path, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_temp_dir, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_lastfiles, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_backup_path, FL_RETURN_CHANGED);
	fl_set_counter_return(dialog_->counter_lastfiles, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_serverpipe, FL_RETURN_CHANGED);

	// set up the feedback mechanism
	setPreHandler(dialog_->input_default_path);
	setPreHandler(dialog_->counter_lastfiles);
	setPreHandler(dialog_->input_template_path);
	setPreHandler(dialog_->check_last_files);
	setPreHandler(dialog_->input_lastfiles);
	setPreHandler(dialog_->check_make_backups);
	setPreHandler(dialog_->input_backup_path);
	setPreHandler(dialog_->input_serverpipe);
	setPreHandler(dialog_->input_temp_dir);
	setPreHandler(dialog_->check_use_temp_dir);
}


string const
FormPreferences::Paths::feedback(FL_OBJECT const * const ob) const
{
	string str;

	if (ob == dialog_->input_default_path)
		str = lyxrc.getDescription(LyXRC::RC_DOCUMENTPATH);
	else if (ob == dialog_->input_template_path)
		str = lyxrc.getDescription(LyXRC::RC_TEMPLATEPATH);
	else if (ob == dialog_->check_use_temp_dir)
		str = lyxrc.getDescription(LyXRC::RC_USETEMPDIR);
	else if (ob == dialog_->input_temp_dir)
		str = lyxrc.getDescription(LyXRC::RC_TEMPDIRPATH);
	else if (ob == dialog_->check_last_files)
		str = lyxrc.getDescription(LyXRC::RC_CHECKLASTFILES);
	else if (ob == dialog_->input_lastfiles)
		str = lyxrc.getDescription(LyXRC::RC_LASTFILES);
	else if (ob == dialog_->counter_lastfiles)
		str = lyxrc.getDescription(LyXRC::RC_NUMLASTFILES);
	else if (ob == dialog_->check_make_backups)
		str = lyxrc.getDescription(LyXRC::RC_MAKE_BACKUP);
	else if (ob == dialog_->input_backup_path)
		str = lyxrc.getDescription(LyXRC::RC_BACKUPDIR_PATH);
	else if (ob == dialog_->input_serverpipe) {
		str = lyxrc.getDescription(LyXRC::RC_SERVERPIPE);
	}

	return str;
}


bool FormPreferences::Paths::input(FL_OBJECT const * const ob)
{
	bool activate = true;
	
	// !ob if function is called from Paths::update() to de/activate
	// objects,
	// otherwise the function is called by an xforms CB via input().
	if (!ob || ob == dialog_->check_use_temp_dir) {
		bool const enable = fl_get_button(dialog_->check_use_temp_dir);
		setEnabled(dialog_->input_temp_dir, enable);
	}

	if (!ob || ob == dialog_->check_last_files) {
		bool const enable = fl_get_button(dialog_->check_last_files);
		setEnabled(dialog_->input_lastfiles, enable);
	}

	if (!ob || ob == dialog_->check_make_backups) {
		bool const enable = fl_get_button(dialog_->check_make_backups);
		setEnabled(dialog_->input_backup_path, enable);
	}

	if (!ob || ob == dialog_->input_default_path) {
		string const name = fl_get_input(dialog_->input_default_path);
		if (!name.empty() && !RWInfo::WriteableDir(name)) {
			parent_.printWarning(RWInfo::ErrorMessage());
			return false;
		}
	}

	if (!ob || ob == dialog_->input_template_path) {
		string const name = fl_get_input(dialog_->input_template_path);
		if (!name.empty() && !RWInfo::ReadableDir(name)) {
			parent_.printWarning(RWInfo::ErrorMessage());
			return false;
		}
	}

	if (!ob || ob == dialog_->input_temp_dir) {
		string const name = fl_get_input(dialog_->input_temp_dir);
		if (fl_get_button(dialog_->check_make_backups)
		    && !name.empty()
		    && !RWInfo::WriteableDir(name)) {
			parent_.printWarning(RWInfo::ErrorMessage());
			return false;
		}
	}

	if (!ob || ob == dialog_->input_backup_path) {
		string const name = fl_get_input(dialog_->input_backup_path);
		if (fl_get_button(dialog_->check_make_backups)
		    && !name.empty()
		    && !RWInfo::WriteableDir(name)) {
			parent_.printWarning(RWInfo::ErrorMessage());
			return false;
		}
	}

	if (!ob || ob == dialog_->input_lastfiles) {
		string const name = fl_get_input(dialog_->input_lastfiles);
		if (fl_get_button(dialog_->check_last_files)
		    && !name.empty()
		    && !RWInfo::WriteableFile(name)) {
			parent_.printWarning(RWInfo::ErrorMessage());
			return false;
		}
	}

	if (!ob || ob == dialog_->input_serverpipe) {
		string const name = fl_get_input(dialog_->input_serverpipe);
		if (!name.empty()) {
			// strip off the extension
			string const str = ChangeExtension(name, "");
			if (!RWInfo::WriteableFile(str + ".in")) {
				parent_.printWarning(RWInfo::ErrorMessage());
				return false;
			}
			if (!RWInfo::WriteableFile(str + ".out")) {
				parent_.printWarning(RWInfo::ErrorMessage());
				return false;
			}
		}
	}

	if (ob == dialog_->button_default_path_browse) {
		parent_.browse(dialog_->input_default_path,
			       N_("Default path"), string());
	} else if (ob == dialog_->button_template_path_browse) {
		parent_.browse(dialog_->input_template_path,
			       N_("Template path"), string());
	} else if (ob == dialog_->button_temp_dir_browse) {
		parent_.browse(dialog_->input_temp_dir,
			       N_("Temp dir"), string());
	} else if (ob == dialog_->button_lastfiles_browse) {
		pair<string, string> dir(_("User|#U#u"), user_lyxdir);

		parent_.browse(dialog_->input_lastfiles,
			       N_("Lastfiles"), string(), dir);
	} else if (ob == dialog_->button_backup_path_browse) {
		parent_.browse(dialog_->input_backup_path,
			       N_("Backup path"), string());
	} else if (ob == dialog_->button_serverpipe_browse) {
		parent_.browse(dialog_->input_serverpipe,
			       N_("LyX Server pipes"), string());
	}
	
	return activate;
}


void FormPreferences::Paths::update()
{
	fl_set_input(dialog_->input_default_path,
		     lyxrc.document_path.c_str());
	fl_set_input(dialog_->input_template_path,
		     lyxrc.template_path.c_str());

	string str;
	if (lyxrc.make_backup) str = lyxrc.backupdir_path;

	fl_set_button(dialog_->check_make_backups,
		      lyxrc.make_backup);
	fl_set_input(dialog_->input_backup_path, str.c_str());

	str.erase();
	if (lyxrc.use_tempdir) str = lyxrc.tempdir_path;

	fl_set_button(dialog_->check_use_temp_dir,
		      lyxrc.use_tempdir);
	fl_set_input(dialog_->input_temp_dir, str.c_str());

	str.erase();
	if (lyxrc.check_lastfiles) str = lyxrc.lastfiles;

	fl_set_button(dialog_->check_last_files,
		      lyxrc.check_lastfiles);		
	fl_set_input(dialog_->input_lastfiles, str.c_str());
	fl_set_counter_value(dialog_->counter_lastfiles,
			     lyxrc.num_lastfiles);

	fl_set_input(dialog_->input_serverpipe, lyxrc.lyxpipes.c_str());

	// Activate/Deactivate the input fields dependent on the state of the
	// buttons.
	input(0);
}


FormPreferences::Printer::Printer(FormPreferences &  p)
	: parent_(p)
{}


FD_form_printer const * FormPreferences::Printer::dialog()
{
	return dialog_.get();
}


void FormPreferences::Printer::apply() const
{
	lyxrc.print_adapt_output = fl_get_button(dialog_->check_adapt_output);
	lyxrc.print_command = fl_get_input(dialog_->input_command);
	lyxrc.print_pagerange_flag = fl_get_input(dialog_->input_page_range);
	lyxrc.print_copies_flag = fl_get_input(dialog_->input_copies);
	lyxrc.print_reverse_flag = fl_get_input(dialog_->input_reverse);
	lyxrc.print_to_printer = fl_get_input(dialog_->input_to_printer);
	lyxrc.print_file_extension =
		fl_get_input(dialog_->input_file_extension);
	lyxrc.print_spool_command =
		fl_get_input(dialog_->input_spool_command);
	lyxrc.print_paper_flag = fl_get_input(dialog_->input_paper_type);
	lyxrc.print_evenpage_flag = fl_get_input(dialog_->input_even_pages);
	lyxrc.print_oddpage_flag = fl_get_input(dialog_->input_odd_pages);
	lyxrc.print_collcopies_flag = fl_get_input(dialog_->input_collated);
	lyxrc.print_landscape_flag = fl_get_input(dialog_->input_landscape);
	lyxrc.print_to_file = fl_get_input(dialog_->input_to_file);
	lyxrc.print_extra_options =
		fl_get_input(dialog_->input_extra_options);
	lyxrc.print_spool_printerprefix =
		fl_get_input(dialog_->input_spool_prefix);
	lyxrc.print_paper_dimension_flag =
		fl_get_input(dialog_->input_paper_size);
	lyxrc.printer = fl_get_input(dialog_->input_name);
}


string const
FormPreferences::Printer::feedback(FL_OBJECT const * const ob) const
{
	string str;

	if (ob == dialog_->input_command)
		str = lyxrc.getDescription(LyXRC::RC_PRINT_COMMAND);
	else if (ob == dialog_->check_adapt_output)
		str = lyxrc.getDescription(LyXRC::RC_PRINT_ADAPTOUTPUT);
	else if (ob == dialog_->input_to_printer)
		str = lyxrc.getDescription(LyXRC::RC_PRINTTOPRINTER);
	else if (ob == dialog_->input_to_file)
		str = lyxrc.getDescription(LyXRC::RC_PRINTTOFILE);
	else if (ob == dialog_->input_file_extension)
		str = lyxrc.getDescription(LyXRC::RC_PRINTFILEEXTENSION);
	else if (ob == dialog_->input_extra_options)
		str = lyxrc.getDescription(LyXRC::RC_PRINTEXSTRAOPTIONS);
	else if (ob == dialog_->input_spool_command)
		str = lyxrc.getDescription(LyXRC::RC_PRINTSPOOL_COMMAND);
	else if (ob == dialog_->input_spool_prefix)
		str = lyxrc.getDescription(LyXRC::RC_PRINTSPOOL_PRINTERPREFIX);
	else if (ob == dialog_->input_name)
		str = lyxrc.getDescription(LyXRC::RC_PRINTER);
	else if (ob == dialog_->input_even_pages)
		str = lyxrc.getDescription(LyXRC::RC_PRINTEVENPAGEFLAG);
	else if (ob == dialog_->input_odd_pages)
		str = lyxrc.getDescription(LyXRC::RC_PRINTODDPAGEFLAG);
	else if (ob == dialog_->input_page_range)
		str = lyxrc.getDescription(LyXRC::RC_PRINTPAGERANGEFLAG);
	else if (ob == dialog_->input_reverse)
		str = lyxrc.getDescription(LyXRC::RC_PRINTREVERSEFLAG);
	else if (ob == dialog_->input_landscape)
		str = lyxrc.getDescription(LyXRC::RC_PRINTLANDSCAPEFLAG);
	else if (ob == dialog_->input_copies)
		str = lyxrc.getDescription(LyXRC::RC_PRINTCOPIESFLAG);
	else if (ob == dialog_->input_collated)
		str = lyxrc.getDescription(LyXRC::RC_PRINTCOLLCOPIESFLAG);
	else if (ob == dialog_->input_paper_type)
		str = lyxrc.getDescription(LyXRC::RC_PRINTPAPERFLAG);
	else if (ob == dialog_->input_paper_size)
		str = lyxrc.getDescription(LyXRC::RC_PRINTPAPERDIMENSIONFLAG);

	return str;
}


void FormPreferences::Printer::build()
{
	dialog_.reset(parent_.build_printer());

	fl_set_input_return(dialog_->input_command, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_page_range, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_copies, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_reverse, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_to_printer, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_file_extension, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_spool_command, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_paper_type, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_even_pages, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_odd_pages, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_collated, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_landscape, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_to_file, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_extra_options, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_spool_prefix, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_paper_size, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_name, FL_RETURN_CHANGED);

	// set up the feedback mechanism
	setPreHandler(dialog_->input_command);
	setPreHandler(dialog_->input_page_range);
	setPreHandler(dialog_->input_copies);
	setPreHandler(dialog_->input_reverse);
	setPreHandler(dialog_->input_to_printer);
	setPreHandler(dialog_->input_file_extension);
	setPreHandler(dialog_->input_spool_command);
	setPreHandler(dialog_->input_paper_type);
	setPreHandler(dialog_->input_even_pages);
	setPreHandler(dialog_->input_odd_pages);
	setPreHandler(dialog_->input_collated);
	setPreHandler(dialog_->input_landscape);
	setPreHandler(dialog_->input_to_file);
	setPreHandler(dialog_->input_extra_options);
	setPreHandler(dialog_->input_spool_prefix);
	setPreHandler(dialog_->input_paper_size);
	setPreHandler(dialog_->input_name);
	setPreHandler(dialog_->check_adapt_output);
}


void FormPreferences::Printer::update()
{
	fl_set_button(dialog_->check_adapt_output,
		      lyxrc.print_adapt_output);
	fl_set_input(dialog_->input_command,
		     lyxrc.print_command.c_str());
	fl_set_input(dialog_->input_page_range,
		     lyxrc.print_pagerange_flag.c_str());
	fl_set_input(dialog_->input_copies,
		     lyxrc.print_copies_flag.c_str());
	fl_set_input(dialog_->input_reverse,
		     lyxrc.print_reverse_flag.c_str());
	fl_set_input(dialog_->input_to_printer,
		     lyxrc.print_to_printer.c_str());
	fl_set_input(dialog_->input_file_extension,
		     lyxrc.print_file_extension.c_str());
	fl_set_input(dialog_->input_spool_command,
		     lyxrc.print_spool_command.c_str());
	fl_set_input(dialog_->input_paper_type,
		     lyxrc.print_paper_flag.c_str());
	fl_set_input(dialog_->input_even_pages,
		     lyxrc.print_evenpage_flag.c_str());
	fl_set_input(dialog_->input_odd_pages,
		     lyxrc.print_oddpage_flag.c_str());
	fl_set_input(dialog_->input_collated,
		     lyxrc.print_collcopies_flag.c_str());
	fl_set_input(dialog_->input_landscape,
		     lyxrc.print_landscape_flag.c_str());
	fl_set_input(dialog_->input_to_file,
		     lyxrc.print_to_file.c_str());
	fl_set_input(dialog_->input_extra_options,
		     lyxrc.print_extra_options.c_str());
	fl_set_input(dialog_->input_spool_prefix,
		     lyxrc.print_spool_printerprefix.c_str());
	fl_set_input(dialog_->input_paper_size,
		     lyxrc.print_paper_dimension_flag.c_str());
	fl_set_input(dialog_->input_name,
		     lyxrc.printer.c_str());
}


FormPreferences::ScreenFonts::ScreenFonts(FormPreferences &  p)
	: parent_(p)
{}


FD_form_screen_fonts const * FormPreferences::ScreenFonts::dialog()
{
	return dialog_.get();
}


void FormPreferences::ScreenFonts::apply() const
{
	bool changed = false;

	string str = fl_get_input(dialog_->input_roman);
	if (lyxrc.roman_font_name != str) {
		changed = true;
		lyxrc.roman_font_name = str;
	}

	str = fl_get_input(dialog_->input_sans);
	if (lyxrc.sans_font_name != str) {
		changed = true;
		lyxrc.sans_font_name = str;
	}

	str = fl_get_input(dialog_->input_typewriter);
	if (lyxrc.typewriter_font_name != str) {
		changed = true;
		lyxrc.typewriter_font_name = str;
	}

	str = fl_get_input(dialog_->input_screen_encoding);
	if (lyxrc.font_norm != str) {
		changed = true;
		lyxrc.font_norm = str;
	}

	bool button = fl_get_button(dialog_->check_scalable);
	if (lyxrc.use_scalable_fonts != button) {
		changed = true;
		lyxrc.use_scalable_fonts = button;
	}

	unsigned int ivalue = static_cast<unsigned int>
		(fl_get_counter_value(dialog_->counter_zoom));
	if (lyxrc.zoom != ivalue) {
		changed = true;
		lyxrc.zoom = ivalue;
	}

	ivalue = static_cast<unsigned int>
		(fl_get_counter_value(dialog_->counter_dpi));
	if (lyxrc.dpi != ivalue) {
		changed = true;
		lyxrc.dpi = ivalue;
	}
	
	double dvalue = strToDbl(fl_get_input(dialog_->input_tiny));
	if (lyxrc.font_sizes[LyXFont::SIZE_TINY] != dvalue) {
		changed = true;
		lyxrc.font_sizes[LyXFont::SIZE_TINY] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(dialog_->input_script));
	if (lyxrc.font_sizes[LyXFont::SIZE_SCRIPT] != dvalue) {
		changed = true;
		lyxrc.font_sizes[LyXFont::SIZE_SCRIPT] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(dialog_->input_footnote));
	if (lyxrc.font_sizes[LyXFont::SIZE_FOOTNOTE] != dvalue) {
		changed = true;
		lyxrc.font_sizes[LyXFont::SIZE_FOOTNOTE] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(dialog_->input_small));
	if (lyxrc.font_sizes[LyXFont::SIZE_SMALL] != dvalue) {
		changed = true;
		lyxrc.font_sizes[LyXFont::SIZE_SMALL] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(dialog_->input_normal));
	if (lyxrc.font_sizes[LyXFont::SIZE_NORMAL] != dvalue) {
		changed = true;
		lyxrc.font_sizes[LyXFont::SIZE_NORMAL] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(dialog_->input_large));
	if (lyxrc.font_sizes[LyXFont::SIZE_LARGE] != dvalue) {
		changed = true;
		lyxrc.font_sizes[LyXFont::SIZE_LARGE] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(dialog_->input_larger));
	if (lyxrc.font_sizes[LyXFont::SIZE_LARGER] != dvalue) {
		changed = true;
		lyxrc.font_sizes[LyXFont::SIZE_LARGER] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(dialog_->input_largest));
	if (lyxrc.font_sizes[LyXFont::SIZE_LARGEST] != dvalue) {
		changed = true;
		lyxrc.font_sizes[LyXFont::SIZE_LARGEST] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(dialog_->input_huge));
	if (lyxrc.font_sizes[LyXFont::SIZE_HUGE] != dvalue) {
		changed = true;
		lyxrc.font_sizes[LyXFont::SIZE_HUGE] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(dialog_->input_huger));
	if (lyxrc.font_sizes[LyXFont::SIZE_HUGER] != dvalue) {
		changed = true;
		lyxrc.font_sizes[LyXFont::SIZE_HUGER] = dvalue;
	}

	if (changed) {
		// Now update the buffers
		// Can anything below here affect the redraw process?
		parent_.lv_->getLyXFunc()->dispatch(LFUN_SCREEN_FONT_UPDATE);
	}
}


void FormPreferences::ScreenFonts::build()
{
	dialog_.reset(parent_.build_screen_fonts());

	fl_set_counter_step(dialog_->counter_zoom, 1, 10);
	fl_set_counter_step(dialog_->counter_dpi,  1, 10);

	fl_set_input_return(dialog_->input_roman,           FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_sans,            FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_typewriter,      FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_screen_encoding, FL_RETURN_CHANGED);
	fl_set_counter_return(dialog_->counter_zoom,        FL_RETURN_CHANGED);
	fl_set_counter_return(dialog_->counter_dpi,         FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_tiny,            FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_script,          FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_footnote,        FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_small,           FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_normal,          FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_large,           FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_larger,          FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_largest,         FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_huge,            FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_huger,           FL_RETURN_CHANGED);

	fl_set_input_filter(dialog_->input_tiny,     fl_unsigned_float_filter);
	fl_set_input_filter(dialog_->input_script,   fl_unsigned_float_filter);
	fl_set_input_filter(dialog_->input_footnote, fl_unsigned_float_filter);
	fl_set_input_filter(dialog_->input_small,    fl_unsigned_float_filter);
	fl_set_input_filter(dialog_->input_normal,   fl_unsigned_float_filter);
	fl_set_input_filter(dialog_->input_large,    fl_unsigned_float_filter);
	fl_set_input_filter(dialog_->input_larger,   fl_unsigned_float_filter);
	fl_set_input_filter(dialog_->input_largest,  fl_unsigned_float_filter);
	fl_set_input_filter(dialog_->input_huge,     fl_unsigned_float_filter);
	fl_set_input_filter(dialog_->input_huger,    fl_unsigned_float_filter);

	// set up the feedback mechanism
	setPreHandler(dialog_->input_roman);
	setPreHandler(dialog_->input_sans);
	setPreHandler(dialog_->input_typewriter);
	setPreHandler(dialog_->counter_zoom);
	setPreHandler(dialog_->counter_dpi);
	setPreHandler(dialog_->check_scalable);
	setPreHandler(dialog_->input_screen_encoding);
	setPreHandler(dialog_->input_tiny);
	setPreHandler(dialog_->input_script);
	setPreHandler(dialog_->input_footnote);
	setPreHandler(dialog_->input_small);
	setPreHandler(dialog_->input_large);
	setPreHandler(dialog_->input_larger);
	setPreHandler(dialog_->input_largest);
	setPreHandler(dialog_->input_normal);
	setPreHandler(dialog_->input_huge);
	setPreHandler(dialog_->input_huger);
}

	
string const
FormPreferences::ScreenFonts::feedback(FL_OBJECT const * const ob) const
{
	string str;

	if (ob == dialog_->input_roman)
		str = lyxrc.getDescription(LyXRC::RC_SCREEN_FONT_ROMAN);
	else if (ob == dialog_->input_sans)
		str = lyxrc.getDescription(LyXRC::RC_SCREEN_FONT_SANS);
	else if (ob == dialog_->input_typewriter)
		str = lyxrc.getDescription(LyXRC::RC_SCREEN_FONT_TYPEWRITER);
	else if (ob == dialog_->check_scalable)
		str = lyxrc.getDescription(LyXRC::RC_SCREEN_FONT_SCALABLE);
	else if (ob == dialog_->input_screen_encoding)
		str = lyxrc.getDescription(LyXRC::RC_SCREEN_FONT_ENCODING);
	else if (ob == dialog_->counter_zoom)
		str = lyxrc.getDescription(LyXRC::RC_SCREEN_ZOOM);
	else if (ob == dialog_->counter_dpi) 
		str = lyxrc.getDescription(LyXRC::RC_SCREEN_DPI);
	else if (ob == dialog_->input_tiny
		 || ob == dialog_->input_script
		 || ob == dialog_->input_footnote
		 || ob == dialog_->input_small
		 || ob == dialog_->input_large
		 || ob == dialog_->input_larger
		 || ob == dialog_->input_larger
		 || ob == dialog_->input_largest
		 || ob == dialog_->input_normal
		 || ob == dialog_->input_huge
		 || ob == dialog_->input_huger)
		str = lyxrc.getDescription(LyXRC::RC_SCREEN_FONT_SIZES);

	return str;
}


bool FormPreferences::ScreenFonts::input()
{
	bool activate = true;
	string str;

	// Make sure that all fonts all have positive entries
	// Also note that an empty entry is returned as 0.0 by strToDbl
	if (0.0 >= strToDbl(fl_get_input(dialog_->input_tiny))
	    || 0.0 >= strToDbl(fl_get_input(dialog_->input_script))
	    || 0.0 >= strToDbl(fl_get_input(dialog_->input_footnote))
	    || 0.0 >= strToDbl(fl_get_input(dialog_->input_small))
	    || 0.0 >= strToDbl(fl_get_input(dialog_->input_normal))
	    || 0.0 >= strToDbl(fl_get_input(dialog_->input_large))
	    || 0.0 >= strToDbl(fl_get_input(dialog_->input_larger))
	    || 0.0 >= strToDbl(fl_get_input(dialog_->input_largest))
	    || 0.0 >= strToDbl(fl_get_input(dialog_->input_huge))
	    || 0.0 >= strToDbl(fl_get_input(dialog_->input_huger))) {
		activate = false;
		str = N_("Fonts must be positive!");

	// Fontsizes -- tiny < script < footnote etc.
	} else if (strToDbl(fl_get_input(dialog_->input_tiny)) >
		   strToDbl(fl_get_input(dialog_->input_script)) ||
		   strToDbl(fl_get_input(dialog_->input_script)) >
		   strToDbl(fl_get_input(dialog_->input_footnote)) ||
		   strToDbl(fl_get_input(dialog_->input_footnote)) >
		   strToDbl(fl_get_input(dialog_->input_small)) ||
		   strToDbl(fl_get_input(dialog_->input_small)) >
		   strToDbl(fl_get_input(dialog_->input_normal)) ||
		   strToDbl(fl_get_input(dialog_->input_normal)) >
		   strToDbl(fl_get_input(dialog_->input_large)) ||
		   strToDbl(fl_get_input(dialog_->input_large)) >
		   strToDbl(fl_get_input(dialog_->input_larger)) ||
		   strToDbl(fl_get_input(dialog_->input_larger)) >
		   strToDbl(fl_get_input(dialog_->input_largest)) ||
		   strToDbl(fl_get_input(dialog_->input_largest)) >
		   strToDbl(fl_get_input(dialog_->input_huge)) ||
		   strToDbl(fl_get_input(dialog_->input_huge)) >
		   strToDbl(fl_get_input(dialog_->input_huger))) {
		activate = false;

		str = N_("Fonts must be input in the order tiny > script> footnote > small > normal > large > larger > largest > huge > huger.");
	}

	if (!activate)
		parent_.printWarning(str);
	
	return activate;
}


void FormPreferences::ScreenFonts::update()
{
	fl_set_input(dialog_->input_roman,
		     lyxrc.roman_font_name.c_str());
	fl_set_input(dialog_->input_sans,
		     lyxrc.sans_font_name.c_str());
	fl_set_input(dialog_->input_typewriter,
		     lyxrc.typewriter_font_name.c_str());
	fl_set_input(dialog_->input_screen_encoding,
		     lyxrc.font_norm.c_str());
	fl_set_button(dialog_->check_scalable,
		      lyxrc.use_scalable_fonts);
	fl_set_counter_value(dialog_->counter_zoom, lyxrc.zoom);
	fl_set_counter_value(dialog_->counter_dpi,  lyxrc.dpi);
	fl_set_input(dialog_->input_tiny,
		     tostr(lyxrc.font_sizes[LyXFont::SIZE_TINY]).c_str());
	fl_set_input(dialog_->input_script,
		     tostr(lyxrc.font_sizes[LyXFont::SIZE_SCRIPT]).c_str());
	fl_set_input(dialog_->input_footnote,
		     tostr(lyxrc.font_sizes[LyXFont::SIZE_FOOTNOTE]).c_str());
	fl_set_input(dialog_->input_small,
		     tostr(lyxrc.font_sizes[LyXFont::SIZE_SMALL]).c_str());
	fl_set_input(dialog_->input_normal,
		     tostr(lyxrc.font_sizes[LyXFont::SIZE_NORMAL]).c_str());
	fl_set_input(dialog_->input_large,
		     tostr(lyxrc.font_sizes[LyXFont::SIZE_LARGE]).c_str());
	fl_set_input(dialog_->input_larger,
		     tostr(lyxrc.font_sizes[LyXFont::SIZE_LARGER]).c_str());
	fl_set_input(dialog_->input_largest,
		     tostr(lyxrc.font_sizes[LyXFont::SIZE_LARGEST]).c_str());
	fl_set_input(dialog_->input_huge,
		     tostr(lyxrc.font_sizes[LyXFont::SIZE_HUGE]).c_str());
	fl_set_input(dialog_->input_huger,
		     tostr(lyxrc.font_sizes[LyXFont::SIZE_HUGER]).c_str());
}



FormPreferences::SpellOptions::SpellOptions(FormPreferences &  p)
	: parent_(p)
{}


FD_form_spelloptions const * FormPreferences::SpellOptions::dialog()
{
	return dialog_.get();
}


void FormPreferences::SpellOptions::apply()
{

	string choice = fl_get_choice_text(dialog_->choice_spell_command);
	choice = strip(frontStrip(choice));
	
	lyxrc.isp_command = choice;

#if 0
	// If spell checker == "none", all other input set to off.
	if (fl_get_choice(dialog_->choice_spell_command) == 1) {
		lyxrc.isp_use_alt_lang = false;
		lyxrc.isp_alt_lang.erase();

		lyxrc.isp_use_esc_chars = false;
		lyxrc.isp_esc_chars.erase();

		lyxrc.isp_use_pers_dict = false;
		lyxrc.isp_pers_dict.erase();

		lyxrc.isp_accept_compound = false;
		lyxrc.isp_use_input_encoding = false;
	} else {
#else
		int button = fl_get_button(dialog_->check_alt_lang);
		choice = fl_get_input(dialog_->input_alt_lang);
		if (button && choice.empty()) button = 0;
		if (!button) choice.erase();

		lyxrc.isp_use_alt_lang = static_cast<bool>(button);
		lyxrc.isp_alt_lang = choice;

		button = fl_get_button(dialog_->check_escape_chars);
		choice = fl_get_input(dialog_->input_escape_chars);
		if (button && choice.empty()) button = 0;
		if (!button) choice.erase();
	
		lyxrc.isp_use_esc_chars = static_cast<bool>(button);
		lyxrc.isp_esc_chars = choice;

		button = fl_get_button(dialog_->check_personal_dict);
		choice = fl_get_input(dialog_->input_personal_dict);
		if (button && choice.empty()) button = 0;
		if (!button) choice.erase();

		lyxrc.isp_use_pers_dict = static_cast<bool>(button);
		lyxrc.isp_pers_dict = choice;

		button = fl_get_button(dialog_->check_compound_words);
		lyxrc.isp_accept_compound = static_cast<bool>(button);

		button = fl_get_button(dialog_->check_input_enc);
		lyxrc.isp_use_input_encoding = static_cast<bool>(button);
#endif
#if 0
	}
#endif

	// Reset view
	update();
}


void FormPreferences::SpellOptions::build()
{
	dialog_.reset(parent_.build_spelloptions());

	fl_addto_choice(dialog_->choice_spell_command,
#if 0
			_(" none | ispell | aspell "));
#else
			_(" ispell | aspell "));
#endif
	fl_set_input_return(dialog_->input_alt_lang,      FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_escape_chars,  FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_personal_dict, FL_RETURN_CHANGED);

	// set up the feedback mechanism
	setPreHandler(dialog_->choice_spell_command);
	setPreHandler(dialog_->check_alt_lang);
	setPreHandler(dialog_->input_alt_lang);
	setPreHandler(dialog_->check_escape_chars);
	setPreHandler(dialog_->input_escape_chars);
	setPreHandler(dialog_->check_personal_dict);
	setPreHandler(dialog_->input_personal_dict);
	setPreHandler(dialog_->button_personal_dict);
	setPreHandler(dialog_->check_compound_words);
	setPreHandler(dialog_->check_input_enc);
}


string const
FormPreferences::SpellOptions::feedback(FL_OBJECT const * const ob) const
{
	string str;

	if (ob == dialog_->choice_spell_command)
		str = lyxrc.getDescription(LyXRC::RC_SPELL_COMMAND);
	else if (ob == dialog_->check_alt_lang)
		str = lyxrc.getDescription(LyXRC::RC_USE_ALT_LANG);
	else if (ob == dialog_->input_alt_lang)
		str = lyxrc.getDescription(LyXRC::RC_ALT_LANG);
	else if (ob == dialog_->check_escape_chars)
		str = lyxrc.getDescription(LyXRC::RC_USE_ESC_CHARS);
	else if (ob == dialog_->input_escape_chars)
		str = lyxrc.getDescription(LyXRC::RC_ESC_CHARS);
	else if (ob == dialog_->check_personal_dict)
		str = lyxrc.getDescription(LyXRC::RC_USE_PERS_DICT);
	else if (ob == dialog_->input_personal_dict)
		str = lyxrc.getDescription(LyXRC::RC_PERS_DICT);
	else if (ob == dialog_->check_compound_words)
		str = lyxrc.getDescription(LyXRC::RC_ACCEPT_COMPOUND);
	else if (ob == dialog_->check_input_enc)
		str = lyxrc.getDescription(LyXRC::RC_USE_INP_ENC);

	return str;
}


bool FormPreferences::SpellOptions::input(FL_OBJECT const * const ob)
{
	// !ob if function is called from updateSpellOptions() to de/activate
	// objects,
	// otherwise the function is called by an xforms CB via input().

#if 0
	// If spell checker == "none", disable all input.
	if (!ob || ob == dialog_->choice_spell_command) {
		if (fl_get_choice(dialog_->choice_spell_command) == 1) {
			fl_deactivate_object(dialog_->check_alt_lang);
			fl_deactivate_object(dialog_->input_alt_lang);
			fl_deactivate_object(dialog_->check_escape_chars);
			fl_deactivate_object(dialog_->input_escape_chars);
			fl_deactivate_object(dialog_->check_personal_dict);
			fl_deactivate_object(dialog_->input_personal_dict);
			fl_deactivate_object(dialog_->check_compound_words);
			fl_deactivate_object(dialog_->check_input_enc);
			return true;
		} else {
			fl_activate_object(dialog_->check_alt_lang);
			fl_activate_object(dialog_->check_escape_chars);
			fl_activate_object(dialog_->check_personal_dict);
			fl_activate_object(dialog_->check_compound_words);
			fl_activate_object(dialog_->check_input_enc);
		}
	}
#endif

	if (!ob || ob == dialog_->check_alt_lang) {
		bool const enable = fl_get_button(dialog_->check_alt_lang);
		setEnabled(dialog_->input_alt_lang, enable);
	}

	if (!ob || ob == dialog_->check_escape_chars) {
		bool const enable = fl_get_button(dialog_->check_escape_chars);
		setEnabled(dialog_->input_escape_chars, enable);
	}

	if (!ob || ob == dialog_->check_personal_dict) {
		bool const enable = fl_get_button(dialog_->check_personal_dict);
		setEnabled(dialog_->input_personal_dict, enable);
	}

	if (ob == dialog_->button_personal_dict) {
		parent_.browse(dialog_->input_personal_dict,
			       N_("Personal dictionary"), "*.ispell");
	}
	
	return true; // All input is valid!
}


void FormPreferences::SpellOptions::update()
{
	int choice = 1;
#if 0
	if (lyxrc.isp_command == "none")
		choice = 1;
	else if (lyxrc.isp_command == "ispell")
		choice = 2;
	else if (lyxrc.isp_command == "aspell")
		choice = 3;
#else
	if (lyxrc.isp_command == "ispell")
		choice = 1;
	else if (lyxrc.isp_command == "aspell")
		choice = 2;
#endif
	fl_set_choice(dialog_->choice_spell_command, choice);
	
	string str;
	if (lyxrc.isp_use_alt_lang) str = lyxrc.isp_alt_lang;

	fl_set_button(dialog_->check_alt_lang,
		      lyxrc.isp_use_alt_lang);
	fl_set_input(dialog_->input_alt_lang, str.c_str());
	
	str.erase();
	if (lyxrc.isp_use_esc_chars) str = lyxrc.isp_esc_chars;

	fl_set_button(dialog_->check_escape_chars,
		      lyxrc.isp_use_esc_chars);
	fl_set_input(dialog_->input_escape_chars, str.c_str());

	str.erase();
	if (lyxrc.isp_use_pers_dict) str = lyxrc.isp_pers_dict;

	fl_set_button(dialog_->check_personal_dict,
		      lyxrc.isp_use_pers_dict);
	fl_set_input(dialog_->input_personal_dict, str.c_str());

	fl_set_button(dialog_->check_compound_words,
		      lyxrc.isp_accept_compound);
	fl_set_button(dialog_->check_input_enc,
		      lyxrc.isp_use_input_encoding);

	// Activate/Deactivate the input fields dependent on the state of the
	// buttons.
	input(0);
}


void FormPreferences::printWarning(string const & warning)
{
	warningPosted = true;

	string str = _("WARNING!") + string(" ") + warning;
	str = formatted(str, dialog_->text_warning->w-10, FL_SMALL_SIZE);

	fl_set_object_label(dialog_->text_warning, str.c_str());
	fl_set_object_lsize(dialog_->text_warning, FL_SMALL_SIZE);
}


void FormPreferences::browse(FL_OBJECT * inpt,
			     string const & title,
			     string const & pattern, 
			     pair<string,string> const & dir1,
			     pair<string,string> const & dir2)
{
	// Get the filename from the dialog
	string const filename = fl_get_input(inpt);

	// Show the file browser dialog
	string const new_filename =
		browseFile(lv_, filename, title, pattern, dir1, dir2);

	// Save the filename to the dialog
	if (new_filename != filename && !new_filename.empty()) {
		fl_set_input(inpt, new_filename.c_str());
		input(inpt, 0);
	}
}


// C function wrapper, required by xforms.
extern "C" {
	
	static
	int C_FormPreferencesFeedbackCB(FL_OBJECT * ob, int event,
					FL_Coord mx, FL_Coord my,
					int key, void * xev)
	{
		return FormPreferences::FeedbackCB(ob, event,
						   mx, my, key, xev);
	}
	
}


int FormPreferences::FeedbackCB(FL_OBJECT * ob, int event,
				FL_Coord, FL_Coord, int, void *)
{
	// Note that the return value is important in the pre-emptive handler.
	// Don't return anything other than 0.

	lyx::Assert(ob);
	// Don't Assert this one, as it can happen quite reasonably when things
	// are being deleted in the d-tor.
	//Assert(ob->form);
	if (!ob->form) return 0;

	FormPreferences * pre =
		static_cast<FormPreferences*>(ob->form->u_vdata);
	pre->Feedback(ob, event);
	return 0;
}


// preemptive handler for feedback messages
void FormPreferences::Feedback(FL_OBJECT * ob, int event)
{
	lyx::Assert(ob);

	switch (event) {
	case FL_ENTER:
		warningPosted = false;
		feedback(ob);
		break;

	case FL_LEAVE:
		if (!warningPosted)
			fl_set_object_label(dialog_->text_warning, "");
		break;

	default:
		break;
	}
}


void FormPreferences::setPreHandler(FL_OBJECT * ob)
{
	lyx::Assert(ob);
	fl_set_object_prehandler(ob, C_FormPreferencesFeedbackCB);
}
