/**
 * \file FormPreferences.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlPrefs.h"
#include "FormPreferences.h"
#include "forms/form_preferences.h"
#include "xformsBC.h"

#include "combox.h"
#include "Color.h"
#include "input_validators.h"
#include "forms_gettext.h"
#include "xforms_helpers.h"
#include "helper_funcs.h" // getSecond

#include "buffer.h"
#include "converter.h"
#include "debug.h"
#include "language.h"
#include "frnt_lang.h"
#include "lyxlex.h"
#include "lyxrc.h"
#include "LColor.h"
#include "Lsstream.h"
#include "funcrequest.h"
#include "author.h"

#include "support/lyxfunctional.h"
#include "support/lyxmanip.h"
#include "support/filetools.h"
#include "support/LAssert.h"

#include "graphics/GraphicsCache.h"
#include "graphics/GraphicsTypes.h"

#include <boost/bind.hpp>

#include FORMS_H_LOCATION
#include <utility>
#include <iomanip>
#include <X11/Xlib.h>

using std::endl;
using std::pair;
using std::make_pair;
using std::max;
using std::min;
using std::vector;
using std::setw;
using std::setfill;

extern string system_lyxdir;
extern string user_lyxdir;

namespace {

// These should probably go inside the class definition...
Formats    local_formats;
Converters local_converters;

string makeFontName(string const & family, string const & foundry)
{
	if (foundry.empty())
		return family;
	return family + ',' + foundry;
}


pair<string,string> parseFontName(string const & name)
{
	string::size_type const idx = name.find(',');
	if (idx == string::npos)
		return make_pair(name, string());
	return make_pair(name.substr(0, idx),
			 name.substr(idx+1));
}


} // namespace anon


typedef FormCB<ControlPrefs, FormDB<FD_preferences> > base_class;

FormPreferences::FormPreferences()
	: base_class(_("Preferences"), false),
	  colors_(*this), converters_(*this), inputs_misc_(*this),
	  formats_(*this), interface_(*this), language_(*this),
	  lnf_misc_(*this), identity_(*this), outputs_misc_(*this),
	  paths_(*this), printer_(*this), screen_fonts_(*this),
	  spelloptions_(*this)
{
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


void FormPreferences::hide()
{
	// We need to hide the active tabfolder otherwise we get a
	// BadDrawable error from X window and LyX crashes without saving.
	FL_FORM * inner_form = fl_get_active_folder(dialog_->tabfolder_prefs);
	if (inner_form && inner_form->visible)
		fl_hide_form(inner_form);
	FormBase::hide();
}


void FormPreferences::build()
{
	dialog_.reset(build_preferences(this));

	// Manage the restore, save, apply and cancel/close buttons
	bc().setOK(dialog_->button_ok);
	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_close);
	bc().setRestore(dialog_->button_restore);

	// Allow the base class to control messages
	setMessageWidget(dialog_->text_warning);

	// build the tab folders
	converters_tab_.reset(build_preferences_inner_tab(this));
	look_n_feel_tab_.reset(build_preferences_inner_tab(this));
	inputs_tab_.reset(build_preferences_inner_tab(this));
	outputs_tab_.reset(build_preferences_inner_tab(this));
	lang_opts_tab_.reset(build_preferences_inner_tab(this));

	// build actual tabfolder contents
	// these will become nested tabfolders
	colors_.build();
	converters_.build();
	formats_.build();
	inputs_misc_.build();
	interface_.build();
	language_.build();
	lnf_misc_.build();
	identity_.build();
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
	fl_addto_tabfolder(look_n_feel_tab_->tabfolder_inner,
			   _("Identity"),
			   identity_.dialog()->form);

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

	// work-around xforms bug re update of folder->x, folder->y coords.
	setPrehandler(look_n_feel_tab_->tabfolder_inner);
	setPrehandler(converters_tab_->tabfolder_inner);
	setPrehandler(inputs_tab_->tabfolder_inner);
	setPrehandler(outputs_tab_->tabfolder_inner);
	setPrehandler(lang_opts_tab_->tabfolder_inner);
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

	LyXRC & rc(controller().rc());

	colors_.apply();
	formats_.apply();    // Must be before converters_.apply()
	converters_.apply();
	inputs_misc_.apply(rc);
	interface_.apply(rc);
	language_.apply(rc);
	lnf_misc_.apply(rc);
	identity_.apply(rc);
	outputs_misc_.apply(rc);
	paths_.apply(rc);
	printer_.apply(rc);
	screen_fonts_.apply(rc);
	spelloptions_.apply(rc);

	// The "Save" button has been pressed.
	if (controller().isClosing() && colors_.modifiedXformsPrefs) {
		string const filename =
			AddName(user_lyxdir, "preferences.xform");
		colors_.modifiedXformsPrefs = !XformsColor::write(filename);
	}
}


string const FormPreferences::getFeedback(FL_OBJECT * ob)
{
	lyx::Assert(ob);

	if (ob->form->fdui == colors_.dialog())
		return colors_.feedback(ob);
	if (ob->form->fdui == converters_.dialog())
		return converters_.feedback(ob);
	if (ob->form->fdui == formats_.dialog())
		return formats_.feedback(ob);
	if (ob->form->fdui == inputs_misc_.dialog())
		return inputs_misc_.feedback(ob);
	if (ob->form->fdui == interface_.dialog())
		return interface_.feedback(ob);
	if (ob->form->fdui == language_.dialog())
		return language_.feedback(ob);
	if (ob->form->fdui == lnf_misc_.dialog())
		return lnf_misc_.feedback(ob);
	if (ob->form->fdui == outputs_misc_.dialog())
		return outputs_misc_.feedback(ob);
	if (ob->form->fdui == paths_.dialog())
		return paths_.feedback(ob);
	if (ob->form->fdui == printer_.dialog())
		return printer_.feedback(ob);
	if (ob->form->fdui == screen_fonts_.dialog())
		return screen_fonts_.feedback(ob);
	if (ob->form->fdui == spelloptions_.dialog())
		return spelloptions_.feedback(ob);

	return string();
}


ButtonPolicy::SMInput FormPreferences::input(FL_OBJECT * ob, long)
{
	lyx::Assert(ob);

	bool valid = true;

	// whatever checks you need to ensure the user hasn't entered
	// some totally ridiculous value somewhere.  Change activate to suit.
	// comments before each test describe what is _valid_

	if (ob->form->fdui == colors_.dialog()) {
		colors_.input(ob);
	} else if (ob->form->fdui == converters_.dialog()) {
		valid = converters_.input(ob);
	} else if (ob->form->fdui == formats_.dialog()) {
		valid = formats_.input(ob);
	} else if  (ob->form->fdui == interface_.dialog()) {
		valid = interface_.input(ob);
	} else if  (ob->form->fdui == language_.dialog()) {
		valid = language_.input(ob);
	} else if  (ob->form->fdui == paths_.dialog()) {
		valid = paths_.input(ob);
	} else if  (ob->form->fdui == screen_fonts_.dialog()) {
		valid = screen_fonts_.input();
	} else if  (ob->form->fdui == spelloptions_.dialog()) {
		valid = spelloptions_.input(ob);
	}

	return valid ? ButtonPolicy::SMI_VALID : ButtonPolicy::SMI_INVALID;
}


void FormPreferences::update()
{
	if (!dialog_.get()) return;

	LyXRC const & rc(controller().rc());

	// read lyxrc entries
	colors_.update();
	formats_.update();   // Must be before converters_.update()
	converters_.update();
	inputs_misc_.update(rc);
	interface_.update(rc);
	language_.update(rc);
	lnf_misc_.update(rc);
	identity_.update(rc);
	outputs_misc_.update(rc);
	paths_.update(rc);
	printer_.update(rc);
	screen_fonts_.update(rc);
	spelloptions_.update(rc);
}


FormPreferences::Colors::Colors(FormPreferences & p)
	: modifiedXformsPrefs(false), parent_(p)
{}


FD_preferences_colors const * FormPreferences::Colors::dialog()
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
				setCursorColor(GUI_COLOR_CURSOR);
			}
		}
		parent_.controller().redrawGUI();
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

			parent_.controller().setColor(lc, hexname);
		}
	}
}


void FormPreferences::Colors::build()
{
	dialog_.reset(build_preferences_colors(&parent_));

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
	setPrehandler(dialog_->browser_lyx_objs);
	setPrehandler(dialog_->button_color);
	setPrehandler(dialog_->button_modify);
	setPrehandler(dialog_->dial_hue);
	setPrehandler(dialog_->slider_saturation);
	setPrehandler(dialog_->slider_value);
	setPrehandler(dialog_->slider_red);
	setPrehandler(dialog_->slider_green);
	setPrehandler(dialog_->slider_blue);
	setPrehandler(dialog_->radio_rgb);
	setPrehandler(dialog_->radio_hsv);
}


string const
FormPreferences::Colors::feedback(FL_OBJECT const * const ob) const
{
	if (ob == dialog_->browser_lyx_objs)
		return _("LyX objects that can be assigned a color.");

	if (ob == dialog_->button_modify)
		return _("Modify the LyX object's color. Note: you must then \"Apply\" the change.");

	if (ob == dialog_->dial_hue ||
		  ob == dialog_->slider_saturation ||
		  ob == dialog_->slider_value ||
		  ob == dialog_->slider_red ||
		  ob == dialog_->slider_green ||
		  ob == dialog_->slider_blue)
		return  _("Find a new color.");

	if (ob == dialog_->radio_rgb || ob == dialog_->radio_hsv)
		return _("Toggle between RGB and HSV color spaces.");

	return string();
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
	if (!dialog_->browser_lyx_objs->visible)
		return;

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
			parent_.controller().setColor(lc, "black");
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
	if (selLyX < 1)
		return;

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
		col.r = max(col.r, 0);
		fl_mapcolor(GUI_COLOR_HUE_DIAL, col.r, col.g, col.b);
		fl_redraw_object(dialog_->dial_hue);

		// Adjust the label a bit, but not the actual values.
		// Means that toggling from one space to the other has no
		// effect on the final color.
		int const h = int(hsv.h);
		int const s = int(100 * hsv.s);
		int const v = int(100 * hsv.v);
		string const label = tostr(h) + ", " + tostr(s) +
			", " + tostr(v);
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
		string const label = tostr(r) + ", " + tostr(g) +
			", " + tostr(b);
		fl_set_object_label(dialog_->text_color_values, label.c_str());
	}

	fl_unfreeze_form(dialog_->form);
}

string const FormPreferences::Colors::X11hexname(RGBColor const & col) const
{
	ostringstream ostr;

	ostr << '#' << std::setbase(16) << setfill('0')
	     << setw(2) << col.r
	     << setw(2) << col.g
	     << setw(2) << col.b;

	return STRCONV(ostr.str());
}


FormPreferences::Converters::Converters(FormPreferences & p)
	: parent_(p)
{}


FD_preferences_converters const * FormPreferences::Converters::dialog()
{
	return dialog_.get();
}


void FormPreferences::Converters::apply() const
{
	parent_.controller().setConverters(local_converters);
}


void FormPreferences::Converters::build()
{
	dialog_.reset(build_preferences_converters(&parent_));

	fl_set_input_return(dialog_->input_converter, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_flags, FL_RETURN_CHANGED);

	// set up the feedback mechanism
	setPrehandler(dialog_->browser_all);
	setPrehandler(dialog_->button_delete);
	setPrehandler(dialog_->button_add);
	setPrehandler(dialog_->input_converter);
	setPrehandler(dialog_->choice_from);
	setPrehandler(dialog_->choice_to);
	setPrehandler(dialog_->input_flags);
}


string const
FormPreferences::Converters::feedback(FL_OBJECT const * const ob) const
{
	if (ob == dialog_->browser_all)
		return _("All the currently defined converters known to LyX.");

	if (ob == dialog_->choice_from)
		return _("Convert \"from\" this format");

	if (ob == dialog_->choice_to)
		return _("Convert \"to\" this format");

	if (ob == dialog_->input_converter)
		return _("The conversion command. $$i is the input file name, "
			 "$$b is the file name without its extension and $$o is "
			 "the name of the output file. $$s can be used as path to "
			 "LyX's own collection of conversion scripts.");

	if (ob == dialog_->input_flags)
		return _("Extra information for the Converter class, whether and "
			 "how to parse the result, and various other things.");

	if (ob == dialog_->button_delete)
		return _("Remove the current converter from the list of available "
			 "converters. Note: you must then \"Apply\" the change.");

	if (ob == dialog_->button_add) {
		if (string(ob->label) == _("Add"))
			return _("Add the current converter to the list of available "
				 "converters. Note: you must then \"Apply\" the change.");
		else
			return _("Modify the contents of the current converter. "
				 "Note: you must then \"Apply\" the change.");
	}

	return string();
}


bool FormPreferences::Converters::input(FL_OBJECT const * const ob)
{
	if (ob == dialog_->browser_all)
		return Browser();

	if (ob == dialog_->choice_from
		   || ob == dialog_->choice_to
		   || ob == dialog_->input_converter
		   || ob == dialog_->input_flags)
		return Input();

	if (ob == dialog_->button_add)
		return Add();

	if (ob == dialog_->button_delete)
		return erase();

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

	fl_set_object_label(dialog_->button_add, idex(_("Modify|#M")).c_str());
	fl_set_button_shortcut(dialog_->button_add,
			       scex(_("Modify|#M")).c_str(), 1);

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
		fl_set_object_label(dialog_->button_add,
				    idex(_("Add|#A")).c_str());
		fl_set_button_shortcut(dialog_->button_add,
				       scex(_("Add|#A")).c_str(), 1);

		fl_deselect_browser(dialog_->browser_all);
		setEnabled(dialog_->button_delete, false);

	} else {
		fl_set_object_label(dialog_->button_add,
				    idex(_("Modify|#M")).c_str());
		fl_set_button_shortcut(dialog_->button_add,
				       scex(_("Modify|#M")).c_str(), 1);

		int top = max(sel-5, 0);
		fl_set_browser_topline(dialog_->browser_all, top);
		fl_select_browser_line(dialog_->browser_all, sel+1);
		setEnabled(dialog_->button_delete, true);
	}

	string const command = rtrim(fl_get_input(dialog_->input_converter));
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

	lyxerr << "FormPreferences::Converters::GetFrom: No choice!" << endl;
	return "???";
}


string const FormPreferences::Converters::GetTo() const
{
	::Formats::FormatList::size_type const i =
		fl_get_choice(dialog_->choice_to);

	if (i > 0 && i <= local_formats.size())
		return local_formats.get(i - 1).name();

	lyxerr << "FormPreferences::Converters::GetTo: No choice!" << endl;
	return "???";
}


void FormPreferences::Converters::UpdateChoices() const
{
	string choice;
	for (::Formats::const_iterator cit = local_formats.begin();
	     cit != local_formats.end(); ++cit) {
		if (!choice.empty())
			choice += " | ";
		else
			choice += ' ';
		choice += cit->prettyname();
	}
	choice += ' ';

	fl_clear_choice(dialog_->choice_from);
	fl_addto_choice(dialog_->choice_from, choice.c_str());

	fl_clear_choice(dialog_->choice_to);
	fl_addto_choice(dialog_->choice_to, choice.c_str());
}


FormPreferences::Formats::Formats(FormPreferences &  p)
	: parent_(p)
{}


FD_preferences_formats const * FormPreferences::Formats::dialog()
{
	return dialog_.get();
}


void FormPreferences::Formats::apply() const
{
	parent_.controller().setFormats(local_formats);
}


void FormPreferences::Formats::build()
{
	dialog_.reset(build_preferences_formats(&parent_));

	fl_set_input_return(dialog_->input_format, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_viewer, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_shrtcut, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_gui_name, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_extension, FL_RETURN_CHANGED);

	fl_set_input_filter(dialog_->input_format, fl_lowercase_filter);

	// set up the feedback mechanism
	setPrehandler(dialog_->browser_all);
	setPrehandler(dialog_->input_format);
	setPrehandler(dialog_->input_gui_name);
	setPrehandler(dialog_->button_delete);
	setPrehandler(dialog_->button_add);
	setPrehandler(dialog_->input_extension);
	setPrehandler(dialog_->input_viewer);
	setPrehandler(dialog_->input_shrtcut);
}


string const
FormPreferences::Formats::feedback(FL_OBJECT const * const ob) const
{
	if (ob == dialog_->browser_all)
		return  _("All the currently defined formats known to LyX.");

	if (ob == dialog_->input_format)
		return  _("The format identifier.");

	if (ob == dialog_->input_gui_name)
		return  _("The format name as it will appear in the menus.");

	if (ob == dialog_->input_shrtcut)
		return  _("The keyboard accelerator. Use a letter in the GUI name. "
			  "Case sensitive.");

	if (ob == dialog_->input_extension)
		return  _("Used to recognize the file. E.g., ps, pdf, tex.");

	if (ob == dialog_->input_viewer)
		return  _("The command used to launch the viewer application.");

	if (ob == dialog_->button_delete)
		return  _("Remove the current format from the list of available "
			  "formats. Note: you must then \"Apply\" the change.");

	if (ob == dialog_->button_add) {
		if (string(ob->label) == _("Add"))
			return  _("Add the current format to the list of available "
				  "formats. Note: you must then \"Apply\" the change.");
		else
			return  _("Modify the contents of the current format. Note: "
				  "you must then \"Apply\" the change.");
	}

	return string();
}


bool FormPreferences::Formats::input(FL_OBJECT const * const ob)
{
	if (ob == dialog_->browser_all)
		return Browser();

	if (ob == dialog_->input_format
	 || ob == dialog_->input_gui_name
	 || ob == dialog_->input_shrtcut
	 || ob == dialog_->input_extension
	 || ob == dialog_->input_viewer)
		return Input();

	if (ob == dialog_->button_add)
		return Add();

	if (ob == dialog_->button_delete)
		return erase();

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
	if (i <= 0)
		return false;

	fl_freeze_form(dialog_->form);

	Format const & f = local_formats.get(i - 1);

	fl_set_input(dialog_->input_format, f.name().c_str());
	fl_set_input(dialog_->input_gui_name, f.prettyname().c_str());
	fl_set_input(dialog_->input_shrtcut, f.shortcut().c_str());
	fl_set_input(dialog_->input_extension, f.extension().c_str());
	fl_set_input(dialog_->input_viewer, f.viewer().c_str());

	fl_set_object_label(dialog_->button_add,
			    idex(_("Modify|#M")).c_str());
	fl_set_button_shortcut(dialog_->button_add,
			       scex(_("Modify|#M")).c_str(), 1);

	setEnabled(dialog_->button_add,    false);
	setEnabled(dialog_->button_delete, true);

	fl_unfreeze_form(dialog_->form);
	return false;
}


bool FormPreferences::Formats::erase()
{
	string const name = fl_get_input(dialog_->input_format);

	if (local_converters.formatIsUsed(name)) {
		parent_.postWarning(_("Cannot remove a Format used by a Converter. "
				      "Remove the converter first."));
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
				    idex(_("Add|#A")).c_str());
		fl_set_button_shortcut(dialog_->button_add,
				       scex(_("Add|#A")).c_str(), 1);

		fl_deselect_browser(dialog_->browser_all);
		setEnabled(dialog_->button_delete, false);

	} else {
		fl_set_object_label(dialog_->button_add,
				    idex(_("Modify|#M")).c_str());
		fl_set_button_shortcut(dialog_->button_add,
				       scex(_("Modify|#M")).c_str(), 1);

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


FormPreferences::Identity::Identity(FormPreferences & p)
	: parent_(p)
{}


FD_preferences_identity const * FormPreferences::Identity::dialog()
{
	return dialog_.get();
}


void FormPreferences::Identity::apply(LyXRC & rc) const
{
	rc.user_name = fl_get_input(dialog_->input_user_name);
	rc.user_email = fl_get_input(dialog_->input_user_email);
	parent_.controller().setCurrentAuthor();
}


void FormPreferences::Identity::build()
{
	dialog_.reset(build_preferences_identity(&parent_));
	fl_set_input_return(dialog_->input_user_name, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_user_email, FL_RETURN_CHANGED);
}


void FormPreferences::Identity::update(LyXRC const & rc)
{
	fl_set_input(dialog_->input_user_name, rc.user_name.c_str());
	fl_set_input(dialog_->input_user_email, rc.user_email.c_str());
}


FormPreferences::InputsMisc::InputsMisc(FormPreferences &  p)
	: parent_(p)
{}


FD_preferences_inputs_misc const * FormPreferences::InputsMisc::dialog()
{
	return dialog_.get();
}


void FormPreferences::InputsMisc::apply(LyXRC & rc) const
{
	rc.date_insert_format =
		fl_get_input(dialog_->input_date_format);
}


void FormPreferences::InputsMisc::build()
{
	dialog_.reset(build_preferences_inputs_misc(&parent_));

	fl_set_input_return(dialog_->input_date_format, FL_RETURN_CHANGED);

	// set up the feedback mechanism
	setPrehandler(dialog_->input_date_format);
}


string const
FormPreferences::InputsMisc::feedback(FL_OBJECT const * const ob) const
{
	if (ob == dialog_->input_date_format)
		return LyXRC::getDescription(LyXRC::RC_DATE_INSERT_FORMAT);
	return string();
}


void FormPreferences::InputsMisc::update(LyXRC const & rc)
{
	fl_set_input(dialog_->input_date_format,
		     rc.date_insert_format.c_str());
}


FormPreferences::Interface::Interface(FormPreferences &  p)
	: parent_(p)
{}


FD_preferences_interface const * FormPreferences::Interface::dialog()
{
	return dialog_.get();
}


void FormPreferences::Interface::apply(LyXRC & rc) const
{
	rc.popup_normal_font =
		fl_get_input(dialog_->input_popup_normal_font);
	rc.popup_bold_font = fl_get_input(dialog_->input_popup_bold_font);
	rc.popup_font_encoding =
		fl_get_input(dialog_->input_popup_font_encoding);
	rc.bind_file = fl_get_input(dialog_->input_bind_file);
	rc.ui_file = fl_get_input(dialog_->input_ui_file);
}


void FormPreferences::Interface::build()
{
	dialog_.reset(build_preferences_interface(&parent_));

	fl_set_input_return(dialog_->input_popup_normal_font, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_popup_bold_font, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_popup_font_encoding, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_bind_file, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_ui_file, FL_RETURN_CHANGED);

	// set up the feedback mechanism
	setPrehandler(dialog_->input_popup_normal_font);
	setPrehandler(dialog_->input_popup_bold_font);
	setPrehandler(dialog_->input_popup_font_encoding);
	setPrehandler(dialog_->input_bind_file);
	setPrehandler(dialog_->button_bind_file_browse);
	setPrehandler(dialog_->input_ui_file);
	setPrehandler(dialog_->button_ui_file_browse);
}


string const
FormPreferences::Interface::feedback(FL_OBJECT const * const ob) const
{
	if (ob == dialog_->input_popup_normal_font)
		return LyXRC::getDescription(LyXRC::RC_POPUP_NORMAL_FONT);
	if (ob == dialog_->input_popup_bold_font)
		return LyXRC::getDescription(LyXRC::RC_POPUP_BOLD_FONT);
	if (ob == dialog_->input_popup_font_encoding)
		return LyXRC::getDescription(LyXRC::RC_POPUP_FONT_ENCODING);
	if (ob == dialog_->input_bind_file)
		return LyXRC::getDescription(LyXRC::RC_BINDFILE);
	if (ob == dialog_->input_ui_file)
		return LyXRC::getDescription(LyXRC::RC_UIFILE);
	return string();
}


bool FormPreferences::Interface::input(FL_OBJECT const * const ob)
{
	if (ob == dialog_->button_bind_file_browse) {
		string f = parent_.controller().browsebind(
			fl_get_input(dialog_->input_bind_file));

		fl_set_input(dialog_->input_bind_file, f.c_str());
	} else if (ob == dialog_->button_ui_file_browse) {
		string f = parent_.controller().browseUI(
			fl_get_input(dialog_->input_ui_file));

		fl_set_input(dialog_->input_ui_file, f.c_str());
	}

	return true;
}


void FormPreferences::Interface::update(LyXRC const & rc)
{
	fl_set_input(dialog_->input_popup_normal_font,
		     rc.popup_normal_font.c_str());
	fl_set_input(dialog_->input_popup_bold_font,
		     rc.popup_bold_font.c_str());
	fl_set_input(dialog_->input_popup_font_encoding,
		     rc.popup_font_encoding.c_str());
	fl_set_input(dialog_->input_bind_file,
		     rc.bind_file.c_str());
	fl_set_input(dialog_->input_ui_file,
		     rc.ui_file.c_str());
}


FormPreferences::Language::Language(FormPreferences &  p)
	: parent_(p)
{}


FD_preferences_language const * FormPreferences::Language::dialog()
{
	return dialog_.get();
}


void FormPreferences::Language::apply(LyXRC & rc)
{
	int const pos = combo_default_lang->get();
	rc.default_language = lang_[pos-1];

	int button = fl_get_button(dialog_->check_use_kbmap);
	string const name_1 = fl_get_input(dialog_->input_kbmap1);
	string const name_2 = fl_get_input(dialog_->input_kbmap2);
	if (button)
		button = !(name_1.empty() && name_2.empty());
	rc.use_kbmap = static_cast<bool>(button);

	if (rc.use_kbmap) {
		rc.primary_kbmap = name_1;
		rc.secondary_kbmap = name_2;
	}

	button = fl_get_button(dialog_->check_rtl_support);
	rc.rtl_support = static_cast<bool>(button);

	button = fl_get_button(dialog_->check_mark_foreign);
	rc.mark_foreign_language = static_cast<bool>(button);

	button = fl_get_button(dialog_->check_auto_begin);
	rc.language_auto_begin = static_cast<bool>(button);

	button = fl_get_button(dialog_->check_auto_end);
	rc.language_auto_end = static_cast<bool>(button);

	button = fl_get_button(dialog_->check_use_babel);
	rc.language_use_babel = static_cast<bool>(button);

	button = fl_get_button(dialog_->check_global_options);
	rc.language_global_options = static_cast<bool>(button);

	rc.language_package = fl_get_input(dialog_->input_package);
	rc.language_command_begin = fl_get_input(dialog_->input_command_begin);
	rc.language_command_end = fl_get_input(dialog_->input_command_end);

	// Ensure that all is self-consistent.
	update(rc);
}


void FormPreferences::Language::build()
{
	dialog_.reset(build_preferences_language(&parent_));

	fl_set_input_return(dialog_->input_package, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_command_begin, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_command_end, FL_RETURN_CHANGED);

	// Store the lang identifiers for later
	vector<frnt::LanguagePair> const langs = frnt::getLanguageData(false);
	lang_ = getSecond(langs);

	// The default_language is a combo-box and has to be inserted manually
	fl_freeze_form(dialog_->form);
	fl_addto_form(dialog_->form);

	FL_OBJECT * obj = dialog_->choice_default_lang;
	fl_deactivate_object(dialog_->choice_default_lang);
	combo_default_lang.reset(new Combox(FL_COMBOX_DROPLIST));
	combo_default_lang->add(obj->x, obj->y, obj->w, obj->h, 400);
	combo_default_lang->shortcut("#L",1);
	combo_default_lang->setcallback(ComboCB, &parent_);

	vector<frnt::LanguagePair>::const_iterator lit  = langs.begin();
	vector<frnt::LanguagePair>::const_iterator lend = langs.end();
	for (; lit != lend; ++lit) {
		combo_default_lang->addto(lit->first);
	}
	combo_default_lang->select(1);

	fl_end_form();
	fl_unfreeze_form(dialog_->form);

	// set up the feedback mechanism
	setPrehandler(dialog_->input_package);
	setPrehandler(dialog_->check_use_kbmap);

	// This is safe, as nothing is done to the pointer, other than
	// to use its address in a block-if statement.
	// No it's not! Leads to crash.
	// setPrehandler(
	//		reinterpret_cast<FL_OBJECT *>(combo_default_lang),
	//		C_FormPreferencesFeedbackCB);

	setPrehandler(dialog_->input_kbmap1);
	setPrehandler(dialog_->input_kbmap2);
	setPrehandler(dialog_->check_rtl_support);
	setPrehandler(dialog_->check_mark_foreign);
	setPrehandler(dialog_->check_auto_begin);
	setPrehandler(dialog_->check_auto_end);
	setPrehandler(dialog_->check_use_babel);
	setPrehandler(dialog_->check_global_options);
	setPrehandler(dialog_->input_command_begin);
	setPrehandler(dialog_->input_command_end);

	// Activate/Deactivate the input fields dependent on the state of the
	// buttons.
	input(0);
}


string const
FormPreferences::Language::feedback(FL_OBJECT const * const ob) const
{
	if (reinterpret_cast<Combox const *>(ob) == combo_default_lang.get())
		return LyXRC::getDescription(LyXRC::RC_DEFAULT_LANGUAGE);
	if (ob == dialog_->check_use_kbmap)
		return LyXRC::getDescription(LyXRC::RC_KBMAP);
	if (ob == dialog_->input_kbmap1)
		return LyXRC::getDescription(LyXRC::RC_KBMAP_PRIMARY);
	if (ob == dialog_->input_kbmap2)
		return LyXRC::getDescription(LyXRC::RC_KBMAP_SECONDARY);
	if (ob == dialog_->check_rtl_support)
		return LyXRC::getDescription(LyXRC::RC_RTL_SUPPORT);
	if (ob == dialog_->check_mark_foreign)
		return LyXRC::getDescription(LyXRC::RC_MARK_FOREIGN_LANGUAGE);
	if (ob == dialog_->check_auto_begin)
		return LyXRC::getDescription(LyXRC::RC_LANGUAGE_AUTO_BEGIN);
	if (ob == dialog_->check_auto_end)
		return LyXRC::getDescription(LyXRC::RC_LANGUAGE_AUTO_END);
	if (ob == dialog_->check_use_babel)
		return LyXRC::getDescription(LyXRC::RC_LANGUAGE_USE_BABEL);
	if (ob == dialog_->check_global_options)
		return LyXRC::getDescription(LyXRC::RC_LANGUAGE_GLOBAL_OPTIONS);
	if (ob == dialog_->input_package)
		return LyXRC::getDescription(LyXRC::RC_LANGUAGE_PACKAGE);
	if (ob == dialog_->input_command_begin)
		return LyXRC::getDescription(LyXRC::RC_LANGUAGE_COMMAND_BEGIN);
	if (ob == dialog_->input_command_end)
		return LyXRC::getDescription(LyXRC::RC_LANGUAGE_COMMAND_END);
	return string();
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
		string f = parent_.controller().browsekbmap(
			fl_get_input(dialog_->input_kbmap1));

		fl_set_input(dialog_->input_kbmap1, f.c_str());
	} else if (ob == dialog_->button_kbmap2_browse) {
		string f = parent_.controller().browsekbmap(
			fl_get_input(dialog_->input_kbmap2));

		fl_set_input(dialog_->input_kbmap2, f.c_str());
	}

	return activate;
}


void FormPreferences::Language::update(LyXRC const & rc)
{
	fl_set_button(dialog_->check_use_kbmap,
		      rc.use_kbmap);

	int const pos = int(findPos(lang_, rc.default_language));
	combo_default_lang->select(pos + 1);

	if (rc.use_kbmap) {
		fl_set_input(dialog_->input_kbmap1,
			     rc.primary_kbmap.c_str());
		fl_set_input(dialog_->input_kbmap2,
			     rc.secondary_kbmap.c_str());
	} else {
		fl_set_input(dialog_->input_kbmap1, "");
		fl_set_input(dialog_->input_kbmap2, "");
	}

	fl_set_button(dialog_->check_rtl_support, rc.rtl_support);
	fl_set_button(dialog_->check_mark_foreign,
		      rc.mark_foreign_language);
	fl_set_button(dialog_->check_auto_begin, rc.language_auto_begin);
	fl_set_button(dialog_->check_auto_end, rc.language_auto_end);
	fl_set_button(dialog_->check_use_babel, rc.language_use_babel);
	fl_set_button(dialog_->check_global_options,
		      rc.language_global_options);

	fl_set_input(dialog_->input_package,
		     rc.language_package.c_str());
	fl_set_input(dialog_->input_command_begin,
		     rc.language_command_begin.c_str());
	fl_set_input(dialog_->input_command_end,
		     rc.language_command_end.c_str());

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


FD_preferences_lnf_misc const * FormPreferences::LnFmisc::dialog()
{
	return dialog_.get();
}


void FormPreferences::LnFmisc::apply(LyXRC & rc) const
{
	rc.auto_region_delete =
		fl_get_button(dialog_->check_auto_region_delete);
	rc.cursor_follows_scrollbar =
		fl_get_button(dialog_->check_cursor_follows_scrollbar);
	rc.dialogs_iconify_with_main =
		fl_get_button(dialog_->check_dialogs_iconify_with_main);
	rc.preview = fl_get_button(dialog_->check_preview_latex);
	rc.autosave = static_cast<unsigned int>
		(fl_get_counter_value(dialog_->counter_autosave));
	rc.wheel_jump = static_cast<unsigned int>
		(fl_get_counter_value(dialog_->counter_wm_jump));

	// See FIXME below
	// grfx::DisplayType old_value = rc.display_graphics;
	switch (fl_get_choice(dialog_->choice_display)) {
		case 4: rc.display_graphics = grfx::NoDisplay; break;
		case 3: rc.display_graphics = grfx::ColorDisplay; break;
		case 2: rc.display_graphics = grfx::GrayscaleDisplay; break;
		case 1: rc.display_graphics = grfx::MonochromeDisplay; break;
		default: rc.display_graphics = grfx::ColorDisplay; break;
	}

#ifdef WITH_WARNINGS
#warning FIXME!! The graphics cache no longer has a changeDisplay method.
#endif
#if 0
	if (old_value != rc.display_graphics) {
		grfx::GCache & gc = grfx::GCache::get();
		gc.changeDisplay();
	}
#endif
}


void FormPreferences::LnFmisc::build()
{
	dialog_.reset(build_preferences_lnf_misc(&parent_));

	fl_set_counter_step(dialog_->counter_autosave, 1, 10);
	fl_set_counter_step(dialog_->counter_wm_jump, 1, 10);

	fl_set_counter_return(dialog_->counter_autosave, FL_RETURN_CHANGED);
	fl_set_counter_return(dialog_->counter_wm_jump, FL_RETURN_CHANGED);

	// set up the feedback mechanism
	setPrehandler(dialog_->check_auto_region_delete);
	setPrehandler(dialog_->counter_autosave);
	setPrehandler(dialog_->check_cursor_follows_scrollbar);
	setPrehandler(dialog_->check_dialogs_iconify_with_main);
	setPrehandler(dialog_->check_preview_latex);
	setPrehandler(dialog_->counter_wm_jump);

	fl_addto_choice(dialog_->choice_display, _("Monochrome|Grayscale|Color|Do not display"));
}


string const
FormPreferences::LnFmisc::feedback(FL_OBJECT const * const ob) const
{
	if (ob == dialog_->check_auto_region_delete)
		return LyXRC::getDescription(LyXRC::RC_AUTOREGIONDELETE);
	if (ob == dialog_->check_cursor_follows_scrollbar)
		return LyXRC::getDescription(LyXRC::RC_CURSOR_FOLLOWS_SCROLLBAR);
	if (ob == dialog_->check_dialogs_iconify_with_main)
		return LyXRC::getDescription(LyXRC::RC_DIALOGS_ICONIFY_WITH_MAIN);
	if (ob == dialog_->check_preview_latex)
		return LyXRC::getDescription(LyXRC::RC_PREVIEW);
	if (ob == dialog_->counter_autosave)
		return LyXRC::getDescription(LyXRC::RC_AUTOSAVE);
	if (ob == dialog_->counter_wm_jump)
		return LyXRC::getDescription(LyXRC::RC_WHEEL_JUMP);
	if (ob == dialog_->choice_display)
		return LyXRC::getDescription(LyXRC::RC_DISPLAY_GRAPHICS);
	return string();
}


void FormPreferences::LnFmisc::update(LyXRC const & rc)
{
	fl_set_button(dialog_->check_auto_region_delete,
		      rc.auto_region_delete);
	fl_set_button(dialog_->check_cursor_follows_scrollbar,
		      rc.cursor_follows_scrollbar);
	fl_set_button(dialog_->check_dialogs_iconify_with_main,
		      rc.dialogs_iconify_with_main);
	fl_set_button(dialog_->check_preview_latex,
		      rc.preview);
	fl_set_counter_value(dialog_->counter_autosave, rc.autosave);
	fl_set_counter_value(dialog_->counter_wm_jump, rc.wheel_jump);

	switch (rc.display_graphics) {
		case grfx::NoDisplay:		fl_set_choice(dialog_->choice_display, 4); break;
		case grfx::ColorDisplay:	fl_set_choice(dialog_->choice_display, 3); break;
		case grfx::GrayscaleDisplay:	fl_set_choice(dialog_->choice_display, 2); break;
		case grfx::MonochromeDisplay:	fl_set_choice(dialog_->choice_display, 1); break;
		default:			fl_set_choice(dialog_->choice_display, 3); break;
	}
}


FormPreferences::OutputsMisc::OutputsMisc(FormPreferences &  p)
	: parent_(p)
{}


FD_preferences_outputs_misc const * FormPreferences::OutputsMisc::dialog()
{
	return dialog_.get();
}


void FormPreferences::OutputsMisc::apply(LyXRC & rc) const
{
	rc.ascii_linelen = static_cast<unsigned int>
		(fl_get_counter_value(dialog_->counter_line_len));
	rc.fontenc = fl_get_input(dialog_->input_tex_encoding);

	int const choice =
		fl_get_choice(dialog_->choice_default_papersize) - 1;
	rc.default_papersize = static_cast<BufferParams::PAPER_SIZE>(choice);

	rc.ascii_roff_command = fl_get_input(dialog_->input_ascii_roff);
	rc.chktex_command = fl_get_input(dialog_->input_checktex);
	rc.view_dvi_paper_option = fl_get_input(dialog_->input_paperoption);
	rc.auto_reset_options = fl_get_button(dialog_->check_autoreset_classopt);
}


void FormPreferences::OutputsMisc::build()
{
	dialog_.reset(build_preferences_outputs_misc(&parent_));

	fl_set_counter_step(dialog_->counter_line_len, 1, 10);

	fl_set_counter_return(dialog_->counter_line_len, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_tex_encoding, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_ascii_roff,   FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_checktex,     FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_paperoption,  FL_RETURN_CHANGED);

	fl_addto_choice(dialog_->choice_default_papersize,
			_(" default | US letter | US legal | US executive | A3 | A4 | A5 | B5 "));

	// set up the feedback mechanism
	setPrehandler(dialog_->counter_line_len);
	setPrehandler(dialog_->input_tex_encoding);
	setPrehandler(dialog_->choice_default_papersize);
	setPrehandler(dialog_->input_ascii_roff);
	setPrehandler(dialog_->input_checktex);
	setPrehandler(dialog_->input_paperoption);
	setPrehandler(dialog_->check_autoreset_classopt);
}


string const
FormPreferences::OutputsMisc::feedback(FL_OBJECT const * const ob) const
{
	if (ob == dialog_->counter_line_len)
		return LyXRC::getDescription(LyXRC::RC_ASCII_LINELEN);
	if (ob == dialog_->input_tex_encoding)
		return LyXRC::getDescription(LyXRC::RC_FONT_ENCODING);
	if (ob == dialog_->input_ascii_roff)
		return LyXRC::getDescription(LyXRC::RC_ASCIIROFF_COMMAND);
	if (ob == dialog_->input_checktex)
		return LyXRC::getDescription(LyXRC::RC_CHKTEX_COMMAND);
	if (ob == dialog_->choice_default_papersize)
		return LyXRC::getDescription(LyXRC::RC_DEFAULT_PAPERSIZE);
	if (ob == dialog_->input_paperoption)
		return LyXRC::getDescription(LyXRC::RC_VIEWDVI_PAPEROPTION);
	if (ob == dialog_->check_autoreset_classopt)
		return LyXRC::getDescription(LyXRC::RC_AUTORESET_OPTIONS);
	return string();
}


void FormPreferences::OutputsMisc::update(LyXRC const & rc)
{
	fl_set_counter_value(dialog_->counter_line_len,
			     rc.ascii_linelen);
	fl_set_input(dialog_->input_tex_encoding,
		     rc.fontenc.c_str());
	fl_set_choice(dialog_->choice_default_papersize,
		      rc.default_papersize + 1);
	fl_set_input(dialog_->input_ascii_roff,
		     rc.ascii_roff_command.c_str());
	fl_set_input(dialog_->input_checktex,
		     rc.chktex_command.c_str());
	fl_set_input(dialog_->input_paperoption,
		     rc.view_dvi_paper_option.c_str());
	fl_set_button(dialog_->check_autoreset_classopt,
		      rc.auto_reset_options);

}


FormPreferences::Paths::Paths(FormPreferences &  p)
	: parent_(p)
{}


FD_preferences_paths const * FormPreferences::Paths::dialog()
{
	return dialog_.get();
}


void FormPreferences::Paths::apply(LyXRC & rc)
{
	rc.document_path = fl_get_input(dialog_->input_default_path);
	rc.template_path = fl_get_input(dialog_->input_template_path);

	int button = fl_get_button(dialog_->check_use_temp_dir);
	string str  = fl_get_input(dialog_->input_temp_dir);
	if (!button)
		str.erase();

	rc.use_tempdir = button;
	rc.tempdir_path = str;

	button = fl_get_button(dialog_->check_last_files);
	str = fl_get_input(dialog_->input_lastfiles);
	if (!button) str.erase();

	rc.check_lastfiles = button;
	rc.lastfiles = str;
	rc.num_lastfiles = static_cast<unsigned int>
		(fl_get_counter_value(dialog_->counter_lastfiles));

	button = fl_get_button(dialog_->check_make_backups);
	str = fl_get_input(dialog_->input_backup_path);
	if (!button)
		str.erase();

	rc.make_backup = button;
	rc.backupdir_path = str;

	rc.lyxpipes = fl_get_input(dialog_->input_serverpipe);

	// update view
	update(rc);
}


void FormPreferences::Paths::build()
{
	dialog_.reset(build_preferences_paths(&parent_));

	fl_set_input_return(dialog_->input_default_path, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_template_path, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_temp_dir, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_lastfiles, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_backup_path, FL_RETURN_CHANGED);
	fl_set_counter_return(dialog_->counter_lastfiles, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_serverpipe, FL_RETURN_CHANGED);

	// set up the feedback mechanism
	setPrehandler(dialog_->input_default_path);
	setPrehandler(dialog_->counter_lastfiles);
	setPrehandler(dialog_->input_template_path);
	setPrehandler(dialog_->check_last_files);
	setPrehandler(dialog_->input_lastfiles);
	setPrehandler(dialog_->check_make_backups);
	setPrehandler(dialog_->input_backup_path);
	setPrehandler(dialog_->input_serverpipe);
	setPrehandler(dialog_->input_temp_dir);
	setPrehandler(dialog_->check_use_temp_dir);
}


string const
FormPreferences::Paths::feedback(FL_OBJECT const * const ob) const
{
	if (ob == dialog_->input_default_path)
		return LyXRC::getDescription(LyXRC::RC_DOCUMENTPATH);
	if (ob == dialog_->input_template_path)
		return LyXRC::getDescription(LyXRC::RC_TEMPLATEPATH);
	if (ob == dialog_->check_use_temp_dir)
		return LyXRC::getDescription(LyXRC::RC_USETEMPDIR);
	if (ob == dialog_->input_temp_dir)
		return LyXRC::getDescription(LyXRC::RC_TEMPDIRPATH);
	if (ob == dialog_->check_last_files)
		return LyXRC::getDescription(LyXRC::RC_CHECKLASTFILES);
	if (ob == dialog_->input_lastfiles)
		return LyXRC::getDescription(LyXRC::RC_LASTFILES);
	if (ob == dialog_->counter_lastfiles)
		return LyXRC::getDescription(LyXRC::RC_NUMLASTFILES);
	if (ob == dialog_->check_make_backups)
		return LyXRC::getDescription(LyXRC::RC_MAKE_BACKUP);
	if (ob == dialog_->input_backup_path)
		return LyXRC::getDescription(LyXRC::RC_BACKUPDIR_PATH);
	if (ob == dialog_->input_serverpipe)
		return LyXRC::getDescription(LyXRC::RC_SERVERPIPE);
	return string();
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
			parent_.postWarning(RWInfo::ErrorMessage());
			return false;
		}
	}

	if (!ob || ob == dialog_->input_template_path) {
		string const name = fl_get_input(dialog_->input_template_path);
		if (!name.empty() && !RWInfo::ReadableDir(name)) {
			parent_.postWarning(RWInfo::ErrorMessage());
			return false;
		}
	}

	if (!ob || ob == dialog_->input_temp_dir) {
		string const name = fl_get_input(dialog_->input_temp_dir);
		if (fl_get_button(dialog_->check_make_backups)
		    && !name.empty()
		    && !RWInfo::WriteableDir(name)) {
			parent_.postWarning(RWInfo::ErrorMessage());
			return false;
		}
	}

	if (!ob || ob == dialog_->input_backup_path) {
		string const name = fl_get_input(dialog_->input_backup_path);
		if (fl_get_button(dialog_->check_make_backups)
		    && !name.empty()
		    && !RWInfo::WriteableDir(name)) {
			parent_.postWarning(RWInfo::ErrorMessage());
			return false;
		}
	}

	if (!ob || ob == dialog_->input_lastfiles) {
		string const name = fl_get_input(dialog_->input_lastfiles);
		if (fl_get_button(dialog_->check_last_files)
		    && !name.empty()
		    && !RWInfo::WriteableFile(name)) {
			parent_.postWarning(RWInfo::ErrorMessage());
			return false;
		}
	}

	if (!ob || ob == dialog_->input_serverpipe) {
		string const name = fl_get_input(dialog_->input_serverpipe);
		if (!name.empty()) {
			// strip off the extension
			string const str = ChangeExtension(name, "");
			if (!RWInfo::WriteableFile(str + ".in")) {
				parent_.postWarning(RWInfo::ErrorMessage());
				return false;
			}
			if (!RWInfo::WriteableFile(str + ".out")) {
				parent_.postWarning(RWInfo::ErrorMessage());
				return false;
			}
		}
	}

	if (ob == dialog_->button_default_path_browse) {
		string f = parent_.controller().browsedir(
			fl_get_input(dialog_->input_default_path), _("Default path"));
		if (!f.empty())
			fl_set_input(dialog_->input_default_path, f.c_str());
	} else if (ob == dialog_->button_template_path_browse) {
		string f = parent_.controller().browsedir(
			fl_get_input(dialog_->input_template_path), _("Template path"));
		if (!f.empty())
			fl_set_input(dialog_->input_template_path, f.c_str());
	} else if (ob == dialog_->button_temp_dir_browse) {
		string f = parent_.controller().browsedir(
			fl_get_input(dialog_->input_temp_dir), _("Temporary dir"));
		if (!f.empty())
			fl_set_input(dialog_->input_temp_dir, f.c_str());
	} else if (ob == dialog_->button_lastfiles_browse) {
		string f = parent_.controller().browse(
			fl_get_input(dialog_->input_lastfiles), _("Last files"));
		if (!f.empty())
			fl_set_input(dialog_->input_lastfiles, f.c_str());
	} else if (ob == dialog_->button_backup_path_browse) {
		string f = parent_.controller().browsedir(
			fl_get_input(dialog_->input_backup_path), _("Backup path"));
		if (!f.empty())
			fl_set_input(dialog_->input_backup_path, f.c_str());
	} else if (ob == dialog_->button_serverpipe_browse) {
		string f = parent_.controller().browse(
			fl_get_input(dialog_->input_serverpipe), _("LyX server pipes"));
		if (!f.empty())
			fl_set_input(dialog_->input_serverpipe, f.c_str());
	}

	return activate;
}


void FormPreferences::Paths::update(LyXRC const & rc)
{
	fl_set_input(dialog_->input_default_path,
		     rc.document_path.c_str());
	fl_set_input(dialog_->input_template_path,
		     rc.template_path.c_str());

	string str;
	if (rc.make_backup)
		str = rc.backupdir_path;

	fl_set_button(dialog_->check_make_backups,
		      rc.make_backup);
	fl_set_input(dialog_->input_backup_path, str.c_str());

	str.erase();
	if (rc.use_tempdir)
		str = rc.tempdir_path;

	fl_set_button(dialog_->check_use_temp_dir,
		      rc.use_tempdir);
	fl_set_input(dialog_->input_temp_dir, str.c_str());

	str.erase();
	if (rc.check_lastfiles)
		str = rc.lastfiles;

	fl_set_button(dialog_->check_last_files,
		      rc.check_lastfiles);
	fl_set_input(dialog_->input_lastfiles, str.c_str());
	fl_set_counter_value(dialog_->counter_lastfiles,
			     rc.num_lastfiles);

	fl_set_input(dialog_->input_serverpipe, rc.lyxpipes.c_str());

	// Activate/Deactivate the input fields dependent on the state of the
	// buttons.
	input(0);
}


FormPreferences::Printer::Printer(FormPreferences &  p)
	: parent_(p)
{}


FD_preferences_printer const * FormPreferences::Printer::dialog()
{
	return dialog_.get();
}


void FormPreferences::Printer::apply(LyXRC & rc) const
{
	rc.print_adapt_output = fl_get_button(dialog_->check_adapt_output);
	rc.print_command = fl_get_input(dialog_->input_command);
	rc.print_pagerange_flag = fl_get_input(dialog_->input_page_range);
	rc.print_copies_flag = fl_get_input(dialog_->input_copies);
	rc.print_reverse_flag = fl_get_input(dialog_->input_reverse);
	rc.print_to_printer = fl_get_input(dialog_->input_to_printer);
	rc.print_file_extension =
		fl_get_input(dialog_->input_file_extension);
	rc.print_spool_command =
		fl_get_input(dialog_->input_spool_command);
	rc.print_paper_flag = fl_get_input(dialog_->input_paper_type);
	rc.print_evenpage_flag = fl_get_input(dialog_->input_even_pages);
	rc.print_oddpage_flag = fl_get_input(dialog_->input_odd_pages);
	rc.print_collcopies_flag = fl_get_input(dialog_->input_collated);
	rc.print_landscape_flag = fl_get_input(dialog_->input_landscape);
	rc.print_to_file = fl_get_input(dialog_->input_to_file);
	rc.print_extra_options =
		fl_get_input(dialog_->input_extra_options);
	rc.print_spool_printerprefix =
		fl_get_input(dialog_->input_spool_prefix);
	rc.print_paper_dimension_flag =
		fl_get_input(dialog_->input_paper_size);
	rc.printer = fl_get_input(dialog_->input_name);
}


string const
FormPreferences::Printer::feedback(FL_OBJECT const * const ob) const
{
	if (ob == dialog_->input_command)
		return LyXRC::getDescription(LyXRC::RC_PRINT_COMMAND);
	if (ob == dialog_->check_adapt_output)
		return LyXRC::getDescription(LyXRC::RC_PRINT_ADAPTOUTPUT);
	if (ob == dialog_->input_to_printer)
		return LyXRC::getDescription(LyXRC::RC_PRINTTOPRINTER);
	if (ob == dialog_->input_to_file)
		return LyXRC::getDescription(LyXRC::RC_PRINTTOFILE);
	if (ob == dialog_->input_file_extension)
		return LyXRC::getDescription(LyXRC::RC_PRINTFILEEXTENSION);
	if (ob == dialog_->input_extra_options)
		return LyXRC::getDescription(LyXRC::RC_PRINTEXSTRAOPTIONS);
	if (ob == dialog_->input_spool_command)
		return LyXRC::getDescription(LyXRC::RC_PRINTSPOOL_COMMAND);
	if (ob == dialog_->input_spool_prefix)
		return LyXRC::getDescription(LyXRC::RC_PRINTSPOOL_PRINTERPREFIX);
	if (ob == dialog_->input_name)
		return LyXRC::getDescription(LyXRC::RC_PRINTER);
	if (ob == dialog_->input_even_pages)
		return LyXRC::getDescription(LyXRC::RC_PRINTEVENPAGEFLAG);
	if (ob == dialog_->input_odd_pages)
		return LyXRC::getDescription(LyXRC::RC_PRINTODDPAGEFLAG);
	if (ob == dialog_->input_page_range)
		return LyXRC::getDescription(LyXRC::RC_PRINTPAGERANGEFLAG);
	if (ob == dialog_->input_reverse)
		return LyXRC::getDescription(LyXRC::RC_PRINTREVERSEFLAG);
	if (ob == dialog_->input_landscape)
		return LyXRC::getDescription(LyXRC::RC_PRINTLANDSCAPEFLAG);
	if (ob == dialog_->input_copies)
		return LyXRC::getDescription(LyXRC::RC_PRINTCOPIESFLAG);
	if (ob == dialog_->input_collated)
		return LyXRC::getDescription(LyXRC::RC_PRINTCOLLCOPIESFLAG);
	if (ob == dialog_->input_paper_type)
		return LyXRC::getDescription(LyXRC::RC_PRINTPAPERFLAG);
	if (ob == dialog_->input_paper_size)
		return LyXRC::getDescription(LyXRC::RC_PRINTPAPERDIMENSIONFLAG);
	return string();
}


void FormPreferences::Printer::build()
{
	dialog_.reset(build_preferences_printer(&parent_));

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
	setPrehandler(dialog_->input_command);
	setPrehandler(dialog_->input_page_range);
	setPrehandler(dialog_->input_copies);
	setPrehandler(dialog_->input_reverse);
	setPrehandler(dialog_->input_to_printer);
	setPrehandler(dialog_->input_file_extension);
	setPrehandler(dialog_->input_spool_command);
	setPrehandler(dialog_->input_paper_type);
	setPrehandler(dialog_->input_even_pages);
	setPrehandler(dialog_->input_odd_pages);
	setPrehandler(dialog_->input_collated);
	setPrehandler(dialog_->input_landscape);
	setPrehandler(dialog_->input_to_file);
	setPrehandler(dialog_->input_extra_options);
	setPrehandler(dialog_->input_spool_prefix);
	setPrehandler(dialog_->input_paper_size);
	setPrehandler(dialog_->input_name);
	setPrehandler(dialog_->check_adapt_output);
}


void FormPreferences::Printer::update(LyXRC const & rc)
{
	fl_set_button(dialog_->check_adapt_output,
		      rc.print_adapt_output);
	fl_set_input(dialog_->input_command,
		     rc.print_command.c_str());
	fl_set_input(dialog_->input_page_range,
		     rc.print_pagerange_flag.c_str());
	fl_set_input(dialog_->input_copies,
		     rc.print_copies_flag.c_str());
	fl_set_input(dialog_->input_reverse,
		     rc.print_reverse_flag.c_str());
	fl_set_input(dialog_->input_to_printer,
		     rc.print_to_printer.c_str());
	fl_set_input(dialog_->input_file_extension,
		     rc.print_file_extension.c_str());
	fl_set_input(dialog_->input_spool_command,
		     rc.print_spool_command.c_str());
	fl_set_input(dialog_->input_paper_type,
		     rc.print_paper_flag.c_str());
	fl_set_input(dialog_->input_even_pages,
		     rc.print_evenpage_flag.c_str());
	fl_set_input(dialog_->input_odd_pages,
		     rc.print_oddpage_flag.c_str());
	fl_set_input(dialog_->input_collated,
		     rc.print_collcopies_flag.c_str());
	fl_set_input(dialog_->input_landscape,
		     rc.print_landscape_flag.c_str());
	fl_set_input(dialog_->input_to_file,
		     rc.print_to_file.c_str());
	fl_set_input(dialog_->input_extra_options,
		     rc.print_extra_options.c_str());
	fl_set_input(dialog_->input_spool_prefix,
		     rc.print_spool_printerprefix.c_str());
	fl_set_input(dialog_->input_paper_size,
		     rc.print_paper_dimension_flag.c_str());
	fl_set_input(dialog_->input_name,
		     rc.printer.c_str());
}


FormPreferences::ScreenFonts::ScreenFonts(FormPreferences &  p)
	: parent_(p)
{}


FD_preferences_screen_fonts const * FormPreferences::ScreenFonts::dialog()
{
	return dialog_.get();
}


void FormPreferences::ScreenFonts::apply(LyXRC & rc) const
{
	bool changed = false;

	pair<string, string> tmp =
		parseFontName(fl_get_input(dialog_->input_roman));
	if (rc.roman_font_name != tmp.first ||
	    rc.roman_font_foundry != tmp.second) {
		changed = true;
		rc.roman_font_name = tmp.first;
		rc.roman_font_foundry = tmp.second;
	}

	tmp = parseFontName(fl_get_input(dialog_->input_sans));
	if (rc.sans_font_name != tmp.first ||
	    rc.sans_font_foundry != tmp.second) {
		changed = true;
		rc.sans_font_name = tmp.first;
		rc.sans_font_foundry = tmp.second;
	}

	tmp = parseFontName(fl_get_input(dialog_->input_typewriter));
	if (rc.typewriter_font_name != tmp.first ||
	    rc.typewriter_font_foundry != tmp.second) {
		changed = true;
		rc.typewriter_font_name = tmp.first;
		rc.typewriter_font_foundry = tmp.second;
	}

	string str = fl_get_input(dialog_->input_screen_encoding);
	if (rc.font_norm != str) {
		changed = true;
		rc.font_norm = str;
	}

	bool button = fl_get_button(dialog_->check_scalable);
	if (rc.use_scalable_fonts != button) {
		changed = true;
		rc.use_scalable_fonts = button;
	}

	unsigned int ivalue = static_cast<unsigned int>
		(fl_get_counter_value(dialog_->counter_zoom));
	if (rc.zoom != ivalue) {
		changed = true;
		rc.zoom = ivalue;
	}

	ivalue = static_cast<unsigned int>
		(fl_get_counter_value(dialog_->counter_dpi));
	if (rc.dpi != ivalue) {
		changed = true;
		rc.dpi = ivalue;
	}

	double dvalue = strToDbl(fl_get_input(dialog_->input_tiny));
	if (rc.font_sizes[LyXFont::SIZE_TINY] != dvalue) {
		changed = true;
		rc.font_sizes[LyXFont::SIZE_TINY] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(dialog_->input_script));
	if (rc.font_sizes[LyXFont::SIZE_SCRIPT] != dvalue) {
		changed = true;
		rc.font_sizes[LyXFont::SIZE_SCRIPT] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(dialog_->input_footnote));
	if (rc.font_sizes[LyXFont::SIZE_FOOTNOTE] != dvalue) {
		changed = true;
		rc.font_sizes[LyXFont::SIZE_FOOTNOTE] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(dialog_->input_small));
	if (rc.font_sizes[LyXFont::SIZE_SMALL] != dvalue) {
		changed = true;
		rc.font_sizes[LyXFont::SIZE_SMALL] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(dialog_->input_normal));
	if (rc.font_sizes[LyXFont::SIZE_NORMAL] != dvalue) {
		changed = true;
		rc.font_sizes[LyXFont::SIZE_NORMAL] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(dialog_->input_large));
	if (rc.font_sizes[LyXFont::SIZE_LARGE] != dvalue) {
		changed = true;
		rc.font_sizes[LyXFont::SIZE_LARGE] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(dialog_->input_larger));
	if (rc.font_sizes[LyXFont::SIZE_LARGER] != dvalue) {
		changed = true;
		rc.font_sizes[LyXFont::SIZE_LARGER] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(dialog_->input_largest));
	if (rc.font_sizes[LyXFont::SIZE_LARGEST] != dvalue) {
		changed = true;
		rc.font_sizes[LyXFont::SIZE_LARGEST] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(dialog_->input_huge));
	if (rc.font_sizes[LyXFont::SIZE_HUGE] != dvalue) {
		changed = true;
		rc.font_sizes[LyXFont::SIZE_HUGE] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(dialog_->input_huger));
	if (rc.font_sizes[LyXFont::SIZE_HUGER] != dvalue) {
		changed = true;
		rc.font_sizes[LyXFont::SIZE_HUGER] = dvalue;
	}

	if (changed) {
		// Now update the buffers
		// Can anything below here affect the redraw process?
		parent_.controller().updateScreenFonts();
	}
}


void FormPreferences::ScreenFonts::build()
{
	dialog_.reset(build_preferences_screen_fonts(&parent_));

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
	setPrehandler(dialog_->input_roman);
	setPrehandler(dialog_->input_sans);
	setPrehandler(dialog_->input_typewriter);
	setPrehandler(dialog_->counter_zoom);
	setPrehandler(dialog_->counter_dpi);
	setPrehandler(dialog_->check_scalable);
	setPrehandler(dialog_->input_screen_encoding);
	setPrehandler(dialog_->input_tiny);
	setPrehandler(dialog_->input_script);
	setPrehandler(dialog_->input_footnote);
	setPrehandler(dialog_->input_small);
	setPrehandler(dialog_->input_large);
	setPrehandler(dialog_->input_larger);
	setPrehandler(dialog_->input_largest);
	setPrehandler(dialog_->input_normal);
	setPrehandler(dialog_->input_huge);
	setPrehandler(dialog_->input_huger);
}


string const
FormPreferences::ScreenFonts::feedback(FL_OBJECT const * const ob) const
{
	if (ob == dialog_->input_roman)
		return LyXRC::getDescription(LyXRC::RC_SCREEN_FONT_ROMAN);
	if (ob == dialog_->input_sans)
		return LyXRC::getDescription(LyXRC::RC_SCREEN_FONT_SANS);
	if (ob == dialog_->input_typewriter)
		return LyXRC::getDescription(LyXRC::RC_SCREEN_FONT_TYPEWRITER);
	if (ob == dialog_->check_scalable)
		return LyXRC::getDescription(LyXRC::RC_SCREEN_FONT_SCALABLE);
	if (ob == dialog_->input_screen_encoding)
		return LyXRC::getDescription(LyXRC::RC_SCREEN_FONT_ENCODING);
	if (ob == dialog_->counter_zoom)
		return LyXRC::getDescription(LyXRC::RC_SCREEN_ZOOM);
	if (ob == dialog_->counter_dpi)
		return LyXRC::getDescription(LyXRC::RC_SCREEN_DPI);
	if (ob == dialog_->input_tiny
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
		return LyXRC::getDescription(LyXRC::RC_SCREEN_FONT_SIZES);
	return string();
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
		str = _("Fonts must be positive!");

	} else if (strToDbl(fl_get_input(dialog_->input_tiny)) >
		   // Fontsizes -- tiny < script < footnote etc.
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

		str = _("Fonts must be input in the order Tiny > Smallest > Smaller > Small > Normal > Large > Larger > Largest > Huge > Huger.");
	}

	if (!activate)
		parent_.postWarning(str);

	return activate;
}


void FormPreferences::ScreenFonts::update(LyXRC const & rc)
{
	fl_set_input(dialog_->input_roman,
		     makeFontName(rc.roman_font_name,
				  rc.roman_font_foundry).c_str());
	fl_set_input(dialog_->input_sans,
		     makeFontName(rc.sans_font_name,
				  rc.sans_font_foundry).c_str());
	fl_set_input(dialog_->input_typewriter,
		     makeFontName(rc.typewriter_font_name,
				  rc.typewriter_font_foundry).c_str());
	fl_set_input(dialog_->input_screen_encoding,
		     rc.font_norm.c_str());
	fl_set_button(dialog_->check_scalable,
		      rc.use_scalable_fonts);
	fl_set_counter_value(dialog_->counter_zoom, rc.zoom);
	fl_set_counter_value(dialog_->counter_dpi,  rc.dpi);
	fl_set_input(dialog_->input_tiny,
		     tostr(rc.font_sizes[LyXFont::SIZE_TINY]).c_str());
	fl_set_input(dialog_->input_script,
		     tostr(rc.font_sizes[LyXFont::SIZE_SCRIPT]).c_str());
	fl_set_input(dialog_->input_footnote,
		     tostr(rc.font_sizes[LyXFont::SIZE_FOOTNOTE]).c_str());
	fl_set_input(dialog_->input_small,
		     tostr(rc.font_sizes[LyXFont::SIZE_SMALL]).c_str());
	fl_set_input(dialog_->input_normal,
		     tostr(rc.font_sizes[LyXFont::SIZE_NORMAL]).c_str());
	fl_set_input(dialog_->input_large,
		     tostr(rc.font_sizes[LyXFont::SIZE_LARGE]).c_str());
	fl_set_input(dialog_->input_larger,
		     tostr(rc.font_sizes[LyXFont::SIZE_LARGER]).c_str());
	fl_set_input(dialog_->input_largest,
		     tostr(rc.font_sizes[LyXFont::SIZE_LARGEST]).c_str());
	fl_set_input(dialog_->input_huge,
		     tostr(rc.font_sizes[LyXFont::SIZE_HUGE]).c_str());
	fl_set_input(dialog_->input_huger,
		     tostr(rc.font_sizes[LyXFont::SIZE_HUGER]).c_str());
}



FormPreferences::SpellOptions::SpellOptions(FormPreferences &  p)
	: parent_(p)
{}


FD_preferences_spelloptions const * FormPreferences::SpellOptions::dialog()
{
	return dialog_.get();
}


void FormPreferences::SpellOptions::apply(LyXRC & rc)
{
	string choice = fl_get_choice_text(dialog_->choice_spell_command);
	choice = trim(choice);

	rc.isp_command = choice;

#if 0
	// If spell checker == "none", all other input set to off.
	if (fl_get_choice(dialog_->choice_spell_command) == 1) {
		rc.isp_use_alt_lang = false;
		rc.isp_alt_lang.erase();

		rc.isp_use_esc_chars = false;
		rc.isp_esc_chars.erase();

		rc.isp_use_pers_dict = false;
		rc.isp_pers_dict.erase();

		rc.isp_accept_compound = false;
		rc.isp_use_input_encoding = false;
	} else {
#else
		int button = fl_get_button(dialog_->check_alt_lang);
		choice = fl_get_input(dialog_->input_alt_lang);
		if (button && choice.empty()) button = 0;
		if (!button) choice.erase();

		rc.isp_use_alt_lang = static_cast<bool>(button);
		rc.isp_alt_lang = choice;

		button = fl_get_button(dialog_->check_escape_chars);
		choice = fl_get_input(dialog_->input_escape_chars);
		if (button && choice.empty()) button = 0;
		if (!button) choice.erase();

		rc.isp_use_esc_chars = static_cast<bool>(button);
		rc.isp_esc_chars = choice;

		button = fl_get_button(dialog_->check_personal_dict);
		choice = fl_get_input(dialog_->input_personal_dict);
		if (button && choice.empty()) button = 0;
		if (!button) choice.erase();

		rc.isp_use_pers_dict = static_cast<bool>(button);
		rc.isp_pers_dict = choice;

		button = fl_get_button(dialog_->check_compound_words);
		rc.isp_accept_compound = static_cast<bool>(button);

		button = fl_get_button(dialog_->check_input_enc);
		rc.isp_use_input_encoding = static_cast<bool>(button);
#endif
#if 0
	}
#endif

	// Reset view
	update(rc);
}


void FormPreferences::SpellOptions::build()
{
	dialog_.reset(build_preferences_spelloptions(&parent_));

	fl_addto_choice(dialog_->choice_spell_command,
			_(" ispell | aspell "));
	fl_set_input_return(dialog_->input_alt_lang,      FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_escape_chars,  FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_personal_dict, FL_RETURN_CHANGED);

	// set up the feedback mechanism
	setPrehandler(dialog_->choice_spell_command);
	setPrehandler(dialog_->check_alt_lang);
	setPrehandler(dialog_->input_alt_lang);
	setPrehandler(dialog_->check_escape_chars);
	setPrehandler(dialog_->input_escape_chars);
	setPrehandler(dialog_->check_personal_dict);
	setPrehandler(dialog_->input_personal_dict);
	setPrehandler(dialog_->button_personal_dict);
	setPrehandler(dialog_->check_compound_words);
	setPrehandler(dialog_->check_input_enc);
}


string const
FormPreferences::SpellOptions::feedback(FL_OBJECT const * const ob) const
{
	if (ob == dialog_->choice_spell_command)
		return LyXRC::getDescription(LyXRC::RC_SPELL_COMMAND);
	if (ob == dialog_->check_alt_lang)
		return LyXRC::getDescription(LyXRC::RC_USE_ALT_LANG);
	if (ob == dialog_->input_alt_lang)
		return LyXRC::getDescription(LyXRC::RC_ALT_LANG);
	if (ob == dialog_->check_escape_chars)
		return LyXRC::getDescription(LyXRC::RC_USE_ESC_CHARS);
	if (ob == dialog_->input_escape_chars)
		return LyXRC::getDescription(LyXRC::RC_ESC_CHARS);
	if (ob == dialog_->check_personal_dict)
		return LyXRC::getDescription(LyXRC::RC_USE_PERS_DICT);
	if (ob == dialog_->input_personal_dict)
		return LyXRC::getDescription(LyXRC::RC_PERS_DICT);
	if (ob == dialog_->check_compound_words)
		return LyXRC::getDescription(LyXRC::RC_ACCEPT_COMPOUND);
	if (ob == dialog_->check_input_enc)
		return LyXRC::getDescription(LyXRC::RC_USE_INP_ENC);
	return string();
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
		string f = parent_.controller().browsedict(
			fl_get_input(dialog_->input_personal_dict));
		fl_set_input(dialog_->input_personal_dict, f.c_str());
	}

	return true; // All input is valid!
}


void FormPreferences::SpellOptions::update(LyXRC const & rc)
{
	int choice = 1;
#if 0
	if (rc.isp_command == "none")
		choice = 1;
	else if (rc.isp_command == "ispell")
		choice = 2;
	else if (rc.isp_command == "aspell")
		choice = 3;
#else
	if (rc.isp_command == "ispell")
		choice = 1;
	else if (rc.isp_command == "aspell")
		choice = 2;
#endif
	fl_set_choice(dialog_->choice_spell_command, choice);

	string str;
	if (rc.isp_use_alt_lang)
		str = rc.isp_alt_lang;

	fl_set_button(dialog_->check_alt_lang,
		      rc.isp_use_alt_lang);
	fl_set_input(dialog_->input_alt_lang, str.c_str());

	str.erase();
	if (rc.isp_use_esc_chars)
		str = rc.isp_esc_chars;

	fl_set_button(dialog_->check_escape_chars,
		      rc.isp_use_esc_chars);
	fl_set_input(dialog_->input_escape_chars, str.c_str());

	str.erase();
	if (rc.isp_use_pers_dict)
		str = rc.isp_pers_dict;

	fl_set_button(dialog_->check_personal_dict,
		      rc.isp_use_pers_dict);
	fl_set_input(dialog_->input_personal_dict, str.c_str());

	fl_set_button(dialog_->check_compound_words,
		      rc.isp_accept_compound);
	fl_set_button(dialog_->check_input_enc,
		      rc.isp_use_input_encoding);

	// Activate/Deactivate the input fields dependent on the state of the
	// buttons.
	input(0);
}
