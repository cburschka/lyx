// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2000 The LyX Team.
 *
 *======================================================*/
/* FormPreferences.C
 * FormPreferences Interface Class Implementation
 */

#include <utility>
#include <config.h>

#include FORMS_H_LOCATION

#ifdef __GNUG_
#pragma implementation
#endif

#include "Lsstream.h"
#include "FormPreferences.h"
#include "form_preferences.h"
#include "input_validators.h"
#include "LyXView.h"
#include "language.h"
#include "lyxfunc.h"
#include "Dialogs.h"
#include "lyxrc.h"
#include "combox.h"
#include "debug.h"
#include "support/FileInfo.h"
#include "support/filetools.h"
#include "lyx_gui_misc.h"
#include "lyxlex.h"
#include "input_validators.h"
#include "xform_helpers.h" // formatted()
#include "xform_macros.h"
#include "converter.h"
#include "support/lyxfunctional.h"


#ifdef SIGC_CXX_NAMESPACES
using SigC::slot;
#endif

using std::find;
using std::find_if;
using std::pair;
using std::max;
using std::sort;
using std::vector;
using std::endl;

extern string fmt(char const * fmtstr ...);
extern string system_lyxdir;
extern string user_lyxdir;
extern Languages languages;

static string const colorFile("/usr/lib/X11/rgb.txt");
vector<X11Color> FormPreferences::colorDB;
vector<XformColor> FormPreferences::xformColorDB;
pair<vector<string>, vector<string> > FormPreferences::dirlist;

Formats local_formats;
Converters local_converters;

FormPreferences::FormPreferences(LyXView * lv, Dialogs * d)
	: FormBaseBI(lv, d, _("Preferences"), new PreferencesPolicy),
	  dialog_(0),
	  converters_tab_(0), inputs_tab_(0), look_n_feel_tab_(0),
	  outputs_tab_(0),  usage_tab_(0),
	  colors_(0), converters_(0), formats_(0), inputs_misc_(0),
	  interface_(0), language_(0), lnf_misc_(0), outputs_misc_(0),
	  paths_(0), printer_(0), screen_fonts_(0), spellchecker_(0),
	  combo_default_lang(0), combo_kbmap_1(0), combo_kbmap_2(0),
	  warningPosted(false), modifiedXformPrefs(false)

{
	// let the dialog be shown
	// This is a permanent connection so we won't bother
	// storing a copy because we won't be disconnecting.
	d->showPreferences.connect(slot(this, &FormPreferences::show));
}


FormPreferences::~FormPreferences()
{
	delete combo_default_lang;
	delete combo_kbmap_1;
	delete combo_kbmap_2;

	delete colors_;
	delete converters_;
	delete formats_;
	delete inputs_misc_;
	delete interface_;
	delete language_;
	delete lnf_misc_;
	delete outputs_misc_;
	delete paths_;
	delete printer_;
	delete screen_fonts_;
	delete spellchecker_;

	delete converters_tab_;
	delete inputs_tab_;
	delete look_n_feel_tab_;
	delete outputs_tab_;
	delete usage_tab_;

	delete dialog_;
}


void FormPreferences::redraw()
{
	if( !(form() && form()->visible) )
		return;
	fl_redraw_form( form() );

	FL_FORM * form2 = fl_get_active_folder(dialog_->tabfolder_prefs);
	if( !(form2 && form2->visible) )
		return;
	fl_redraw_form( form2 );

	FL_FORM * form3 = 0;
	if( form2 == converters_tab_->form )
		form3 = fl_get_active_folder(converters_tab_->tabfolder_outer);

	else if( form2 == look_n_feel_tab_->form )
		form3 = fl_get_active_folder(look_n_feel_tab_->tabfolder_outer);

	else if( form2 == inputs_tab_->form )
		form3 = fl_get_active_folder(inputs_tab_->tabfolder_outer);

	else if( form2 == outputs_tab_->form )
		form3 = fl_get_active_folder(outputs_tab_->tabfolder_outer);

	else if( form2 == usage_tab_->form )
		form3 = fl_get_active_folder(usage_tab_->tabfolder_outer);

	if (form3 && form3->visible)
		fl_redraw_form( form3 );
}


FL_FORM * FormPreferences::form() const
{
	if (dialog_) return dialog_->form;
	return 0;
}


void FormPreferences::ok()
{
	FormBase::ok();

	if( modifiedXformPrefs ) {
		string filename = user_lyxdir + "/preferences.xform";
		modifiedXformPrefs = ! XformColor::write( filename );
	}
	
	lv_->getLyXFunc()->Dispatch(LFUN_SAVEPREFERENCES);
}


void FormPreferences::hide()
{
	// We need to hide the active tabfolder otherwise we get a
	// BadDrawable error from X windows and LyX crashes without saving.
	FL_FORM * outer_form = fl_get_active_folder(dialog_->tabfolder_prefs);
	if (outer_form
	    && outer_form->visible) {
		fl_hide_form(outer_form);
	}
	FormBase::hide();
}


void FormPreferences::build()
{
	dialog_ = build_preferences();

	// manage the restore, save, apply and cancel/close buttons
	bc_.setOK(dialog_->button_ok);
	bc_.setApply(dialog_->button_apply);
	bc_.setCancel(dialog_->button_cancel);
	bc_.setUndoAll(dialog_->button_restore);
	bc_.refresh();

	// Workaround dumb xforms sizing bug
	minw_ = form()->w;
	minh_ = form()->h;

	// build the tab folders
	converters_tab_ = build_outer_tab();
	look_n_feel_tab_ = build_outer_tab();
	inputs_tab_ = build_outer_tab();
	outputs_tab_ = build_outer_tab();
	usage_tab_ = build_outer_tab();

	// build actual tabfolder contents
	// these will become nested tabfolders
	buildColors();
	buildConverters();
	buildFormats();
	buildInputsMisc();
	buildInterface();
	buildLanguage();
	buildLnFmisc();
	buildOutputsMisc();
	buildPaths();
	buildPrinter();
	buildScreenFonts();
	buildSpellchecker();

	// Now add them to the tabfolder
	fl_addto_tabfolder(dialog_->tabfolder_prefs,
			   _("Look and Feel"),
			   look_n_feel_tab_->form);
	fl_addto_tabfolder(dialog_->tabfolder_prefs,
			   _("Usage"),
			   usage_tab_->form);
	fl_addto_tabfolder(dialog_->tabfolder_prefs,
			   _("Converters"),
			   converters_tab_->form);
	fl_addto_tabfolder(dialog_->tabfolder_prefs,
			   _("Inputs"),
			   inputs_tab_->form);
	fl_addto_tabfolder(dialog_->tabfolder_prefs,
			   _("Outputs"),
			   outputs_tab_->form);

	// now build the nested tabfolders
	// Starting with look and feel
	fl_addto_tabfolder(look_n_feel_tab_->tabfolder_outer,
			   _("Screen Fonts"),
			   screen_fonts_->form);
	fl_addto_tabfolder(look_n_feel_tab_->tabfolder_outer,
			   _("Interface"),
			   interface_->form);
	fl_addto_tabfolder(look_n_feel_tab_->tabfolder_outer,
			   _("Colors"),
			   colors_->form);
	fl_addto_tabfolder(look_n_feel_tab_->tabfolder_outer,
			   _("Misc"),
			   lnf_misc_->form);

	// then build converters
	fl_addto_tabfolder(converters_tab_->tabfolder_outer,
			   _("Formats"),
			   formats_->form);
	fl_addto_tabfolder(converters_tab_->tabfolder_outer,
			   _("Converters"),
			   converters_->form);

	// then build inputs
	// Paths should probably go in a few outer_tab called Files
	fl_addto_tabfolder(inputs_tab_->tabfolder_outer,
			   _("Paths"),
			   paths_->form);
	fl_addto_tabfolder(inputs_tab_->tabfolder_outer,
			   _("Misc"),
			   inputs_misc_->form);

	// then building outputs
	fl_addto_tabfolder(outputs_tab_->tabfolder_outer,
			   _("Printer"),
			   printer_->form);
	fl_addto_tabfolder(outputs_tab_->tabfolder_outer,
			   _("Misc"),
			   outputs_misc_->form);

	// then building usage
	fl_addto_tabfolder(usage_tab_->tabfolder_outer,
			   _("Spell checker"),
			   spellchecker_->form);
	fl_addto_tabfolder(usage_tab_->tabfolder_outer,
			   _("Language"),
			   language_->form);
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
	// and other stuff which may cost us a lot on slower/high-load machines.

	applyColors();
	applyFormats();    /// Must be before applyConverters()
	applyConverters();
	applyInputsMisc();
	applyInterface();
	applyLanguage();
	applyLnFmisc();
	applyOutputsMisc();
	applyPaths();
	applyPrinter();
	applyScreenFonts();
	applySpellChecker();
}


void FormPreferences::feedback( FL_OBJECT * ob )
{
	Assert(ob);

	string str;

	if (ob->form->fdui == colors_) {
		str = feedbackColors( ob );
	} else if (ob->form->fdui == converters_) {
		str = feedbackConverters( ob );
	} else if (ob->form->fdui == formats_) {
		str = feedbackFormats( ob );
	} else if (ob->form->fdui == inputs_misc_) {
		str = feedbackInputsMisc( ob );
	} else if (ob->form->fdui == interface_) {
		str = feedbackInterface( ob );
	} else if (ob->form->fdui == language_) {
		str = feedbackLanguage( ob );
	} else if (ob->form->fdui == lnf_misc_) {
		str = feedbackLnFmisc( ob );
	} else if (ob->form->fdui == outputs_misc_) {
		str = feedbackOutputsMisc( ob );
	} else if (ob->form->fdui == paths_) {
		str = feedbackPaths( ob );
	} else if (ob->form->fdui == printer_) {
		str = feedbackPrinter( ob );
	} else if (ob->form->fdui == screen_fonts_) {
		str = feedbackScreenFonts( ob );
	} else if (ob->form->fdui == spellchecker_) {
		str = feedbackSpellChecker( ob );
	}

	str = formatted( str, dialog_->text_warning->w-10,
			 FL_SMALL_SIZE, FL_NORMAL_STYLE );

	fl_set_object_label(dialog_->text_warning, str.c_str());
	fl_set_object_lsize(dialog_->text_warning, FL_SMALL_SIZE);
}


bool FormPreferences::input(FL_OBJECT * ob, long)
{
	Assert(ob);
	
	// whatever checks you need to ensure the user hasn't entered
	// some totally ridiculous value somewhere.  Change activate to suit.
	// comments before each test describe what is _valid_

	if (ob->form->fdui == colors_)
		return inputColors(ob);
	else if (ob->form->fdui == converters_)
		return inputConverters(ob);
	else if (ob->form->fdui == language_)
		return inputLanguage(ob);
	else if (ob->form->fdui == paths_)
		return inputPaths(ob);
	else if (ob->form->fdui == screen_fonts_)
		return inputScreenFonts();
	else if (ob->form->fdui == spellchecker_)
		return inputSpellChecker(ob);
	else if (ob->form->fdui == formats_)
		return inputFormats(ob);

	return true;
}


void FormPreferences::update()
{
	if (!dialog_) return;
    
	// read lyxrc entries
	updateColors();
	updateFormats();   /// Must be before updateConverters()
	updateConverters();
	updateInputsMisc();
	updateInterface();
	updateLanguage();
	updateLnFmisc();
	updateOutputsMisc();
	updatePaths();
	updatePrinter();
	updateScreenFonts();
	updateSpellChecker();
}


void FormPreferences::applyColors()
{
	bool modifiedText = false;
	bool modifiedBackground = false;

	for( vector<XformColor>::const_iterator cit = xformColorDB.begin();
	     cit != xformColorDB.end(); ++cit ) {
		RGBColor col;
		fl_getmcolor((*cit).colorID, &col.r, &col.g, &col.b);
		if( col != (*cit).col ) {
			modifiedXformPrefs = true;
			if( (*cit).colorID == FL_BLACK )
				modifiedText = true;
			if( (*cit).colorID == FL_COL1 )
				modifiedBackground = true;
		}
	}

	if( modifiedXformPrefs ) {
		vector<XformColor>::const_iterator cit;
		for( cit = xformColorDB.begin(); 
		     cit != xformColorDB.end(); ++cit ) {
			fl_mapcolor((*cit).colorID,
				    (*cit).col.r, (*cit).col.g, (*cit).col.b);

			if( modifiedText && (*cit).colorID == FL_BLACK ) {
				ColorsAdjustVal( FL_INACTIVE, FL_BLACK, 0.5 );
			}

			if( modifiedBackground && (*cit).colorID == FL_COL1 ) {
				ColorsAdjustVal( FL_MCOL,      FL_COL1, 0.1 );
				ColorsAdjustVal( FL_TOP_BCOL,  FL_COL1, 0.1 );
				ColorsAdjustVal( FL_LEFT_BCOL, FL_COL1, 0.1 );

				ColorsAdjustVal( FL_RIGHT_BCOL,  FL_COL1, -0.5);
				ColorsAdjustVal( FL_BOTTOM_BCOL, FL_COL1, -0.5);
			}		
		}
		Dialogs::redrawGUI();
	}
}


void FormPreferences::buildColors()
{
	colors_ = build_colors();

	FL_OBJECT * obj = colors_->valslider_red;
	fl_set_slider_bounds(obj, 0, 255);
	fl_set_slider_precision(obj, 0);
	fl_set_slider_return(obj, FL_RETURN_END_CHANGED);
	
	obj = colors_->valslider_green;
	fl_set_slider_bounds(obj, 0, 255);
	fl_set_slider_precision(obj, 0);
	fl_set_slider_return(obj, FL_RETURN_END_CHANGED);
	
	obj = colors_->valslider_blue;
	fl_set_slider_bounds(obj, 0, 255);
	fl_set_slider_precision(obj, 0);
	fl_set_slider_return(obj, FL_RETURN_END_CHANGED);

	fl_set_object_color(colors_->button_color,
			    FL_FREE_COL4, FL_FREE_COL4);
	
	fl_set_input_return(colors_->input_name, FL_RETURN_END_CHANGED);

	if (ColorsLoadBrowserX11(colorFile) )
		fl_set_input(colors_->input_name, colorFile.c_str());
	else
		fl_set_input(colors_->input_name, N_("No file found"));

	ColorsLoadBrowserLyX();

	// set up the feedback mechanism
	setPreHandler( colors_->browser_x11 );
	setPreHandler( colors_->input_name );
	setPreHandler( colors_->button_browse );
	setPreHandler( colors_->button_color );
	setPreHandler( colors_->valslider_red );
	setPreHandler( colors_->valslider_green );
	setPreHandler( colors_->valslider_blue );
	setPreHandler( colors_->browser_lyx_objs );
	setPreHandler( colors_->button_modify );
}


string const
FormPreferences::feedbackColors(FL_OBJECT const * const ob) const
{
	string str;

	if (ob == colors_->browser_x11 ) {
		str = N_("The colors listed in the X11 database.");
	} else if (ob == colors_->browser_lyx_objs ) {
		str = N_("LyX objects that can be assigned a color.");
	} else if (ob == colors_->input_name ) {
		str = N_("The file containing the X11 color database.");
	} else if (ob == colors_->button_color ) {
		str = N_("You will only be able to modify the LyX object if the sliders and X11 browser agree. Force this by clicking on the highlighted browser name.");
	} else if (ob == colors_->valslider_red
		   || ob == colors_->valslider_green
		   || ob == colors_->valslider_blue ) {
		str = N_("Find a new color. The name highlighted in the X11 database is the closest match to this.");
	} else if (ob == colors_->button_modify ) {
		str = N_("Modify the color of the LyX object.");
	}

	return str;
}


bool FormPreferences::inputColors( FL_OBJECT const * const ob )
{
	if (ob == colors_->browser_x11) {
		return ColorsBrowserX11();

	} else if (ob == colors_->valslider_red
		   || ob == colors_->valslider_green
		   || ob == colors_->valslider_blue) {
		return ColorsRGB();

	} else if (ob == colors_->input_name) {
		return ColorsDatabase();

	} else if (ob == colors_->button_browse) {
		return ColorsBrowseDatabase();

	} else if (ob == colors_->browser_lyx_objs) {
		return ColorsBrowserLyX();
		
	} else if (ob == colors_->button_modify) {
		return ColorsModify();
	}
	
	return true;
}


void FormPreferences::ColorsAdjustVal( int colAdjust, int colParent,
				       double addVal ) const
{
	RGBColor rgb;
	fl_getmcolor( colParent, &rgb.r, &rgb.g, &rgb.b);

	HSVColor hsv = HSVColor( rgb );
	hsv.v += addVal;
	if( hsv.v > 1.0 )
		hsv.v = 1.0;
	else if( hsv.v < 0.0 )
		hsv.v = 0.0;

	rgb = RGBColor( hsv );
	fl_mapcolor( colAdjust, rgb.r, rgb.g, rgb.b );
}


bool FormPreferences::ColorsBrowserLyX() const
{
	int i = fl_get_browser( colors_->browser_lyx_objs );
	if (i < 1)
		return true;

	string name = fl_get_browser_line( colors_->browser_lyx_objs, i );

	vector<XformColor>::const_iterator cit =
		find_if(xformColorDB.begin(), xformColorDB.end(),
			compare_memfun(&XformColor::getname, name));

	if( cit != xformColorDB.end() ) {
		fl_freeze_form( colors_->form );

		fl_set_slider_value( colors_->valslider_red,   (*cit).col.r );
		fl_set_slider_value( colors_->valslider_green, (*cit).col.g );
		fl_set_slider_value( colors_->valslider_blue,  (*cit).col.b );
		ColorsRGB();

		fl_unfreeze_form( colors_->form );
	}

	fl_deactivate_object( colors_->button_modify );
	fl_set_object_lcol( colors_->button_modify, FL_INACTIVE );

	return true;
}


bool FormPreferences::ColorsBrowserX11() const
{
	int i = fl_get_browser(colors_->browser_x11);
	if (i < 1)
		return true;

	fl_freeze_form( colors_->form );

	RGBColor col = colorDB[i-1].second;
    
	fl_mapcolor( FL_FREE_COL4 + i, col.r, col.g, col.b );
	fl_mapcolor( FL_FREE_COL4, col.r, col.g, col.b );
	fl_set_slider_value( colors_->valslider_red,   col.r );
	fl_set_slider_value( colors_->valslider_green, col.g );
	fl_set_slider_value( colors_->valslider_blue,  col.b );
	fl_redraw_object( colors_->button_color );

	// Is it valid to activate the "Modify" button?
	int line = fl_get_browser(colors_->browser_lyx_objs);
	bool isSelected = ( line > 0 );
	if( isSelected )
		if( line > fl_get_browser_maxline(colors_->browser_lyx_objs) )
			isSelected = false;	

	if( isSelected && colorDB[i-1].second == col ) {
		fl_activate_object( colors_->button_modify );
		fl_set_object_lcol( colors_->button_modify, FL_BLACK );
	}
	
	fl_unfreeze_form( colors_->form );

	return true;
}


bool FormPreferences::ColorsBrowseDatabase() const
{
	return true;
}


bool FormPreferences::ColorsDatabase() const
{
	string file = fl_get_input(colors_->input_name);
	if (ColorsLoadBrowserX11(file) )
		fl_set_input(colors_->input_name, file.c_str());
	else if (ColorsLoadBrowserX11(colorFile) )
		fl_set_input(colors_->input_name, colorFile.c_str());
	else
		fl_set_input(colors_->input_name, N_("No file found"));
	return true;
}


void FormPreferences::ColorsLoadBrowserLyX()
{
	// First, define the modifiable xform colors
	XformColor xcol;

	xcol.name = "GUI background";
	xcol.colorID = FL_COL1;
	fl_getmcolor(FL_COL1, &xcol.col.r, &xcol.col.g, &xcol.col.b);

	xformColorDB.push_back( xcol );

	xcol.name = "GUI text";
	xcol.colorID = FL_BLACK;
	fl_getmcolor(FL_BLACK, &xcol.col.r, &xcol.col.g, &xcol.col.b);

	xformColorDB.push_back( xcol );

 	// FL_LIGHTER_COL1 does not exist in xforms 0.88
//  	xcol.name = "GUI active tab";
//  	xcol.colorID = FL_LIGHTER_COL1;
//  	fl_getmcolor(FL_LIGHTER_COL1, &xcol.col.r, &xcol.col.g, &xcol.col.b);

//  	xformColorDB.push_back( xcol );

	xcol.name = "GUI push button";
	xcol.colorID = FL_YELLOW;
	fl_getmcolor(FL_YELLOW, &xcol.col.r, &xcol.col.g, &xcol.col.b);

	xformColorDB.push_back( xcol );

	FL_OBJECT * colbr = colors_->browser_lyx_objs;
	fl_clear_browser( colbr );
	for( vector<XformColor>::const_iterator cit = xformColorDB.begin();
	     cit != xformColorDB.end(); ++cit ) {
		fl_addto_browser(colbr, (*cit).name.c_str() );
	}

	// Now load the LyX LColors

	// just to be safe...
	fl_deselect_browser( colors_->browser_lyx_objs );
	fl_deactivate_object( colors_->button_modify );
	fl_set_object_lcol( colors_->button_modify, FL_INACTIVE );
}


bool FormPreferences::ColorsLoadBrowserX11(string const & filename) const
{
	LyXLex lex(0, 0);
	lex.setCommentChar('!');
	
	if (!lex.setFile(filename))
		return true;

	vector<RGBColor> cols;
	vector<string> names;
	
	while (lex.next()) {
		RGBColor col;
		col.r = lex.GetInteger();
		lex.next();
		col.g = lex.GetInteger();
		lex.next();
		col.b = lex.GetInteger();
		lex.EatLine();
		string name = frontStrip(lex.GetString(), " \t");

		// remove redundant entries on the fly
		bool add = cols.empty();
		if (!add) {
			vector<RGBColor>::const_iterator cit = 
				find( cols.begin(), cols.end(), col );
			add = (cit == cols.end());
		}
		
		if (add) {
			if( col == RGBColor(0,0,0) )
				name = "black";
			else if( col == RGBColor(255,255,255) )
				name = "white";
			else
				name = lowercase( name );

			if ( col == RGBColor(0,0,0) ||
			     col == RGBColor(255,255,255) ) {
				cols.insert(cols.begin(), col);
				names.insert(names.begin(), name);
			} else {
				cols.push_back(col);
				names.push_back(name);
			}
		}
	}
	
	FL_OBJECT * colbr = colors_->browser_x11;
	fl_freeze_form(colors_->form);
	fl_clear_browser( colbr );

	vector<string>::const_iterator sit = names.begin();
	for (vector<RGBColor>::const_iterator iit = cols.begin();
	     iit != cols.end(); ++iit, ++sit) {
		colorDB.push_back( X11Color(*sit, *iit) );
		fl_addto_browser(colbr, (*sit).c_str());
	}
	
	fl_set_browser_topline(colbr, 1);
	fl_select_browser_line(colbr, 1);
	ColorsBrowserX11();
	fl_unfreeze_form(colors_->form);
	
	return true;
}


bool FormPreferences::ColorsModify() const
{
	int i = fl_get_browser( colors_->browser_lyx_objs );
	if (i < 1)
		return true;

	string name = fl_get_browser_line( colors_->browser_lyx_objs, i );

	vector<XformColor>::iterator it = // non-const; it's modified below
		find_if(xformColorDB.begin(), xformColorDB.end(),
			compare_memfun(&XformColor::getname, name));

	if( it == xformColorDB.end() )
		return true;

	int j = fl_get_browser( colors_->browser_x11 );
	if (j < 1)
		return true;

	(*it).col.r = colorDB[j-1].second.r;
	(*it).col.g = colorDB[j-1].second.g;
	(*it).col.b = colorDB[j-1].second.b;

	fl_deselect_browser(colors_->browser_x11);
	fl_deselect_browser(colors_->browser_lyx_objs);
	fl_deactivate_object( colors_->button_modify );
	fl_set_object_lcol( colors_->button_modify, FL_INACTIVE );

	return true;
}

bool FormPreferences::ColorsRGB() const
{
	fl_freeze_form(colors_->form);

	RGBColor col;
	col.r = int(fl_get_slider_value(colors_->valslider_red));
	col.g = int(fl_get_slider_value(colors_->valslider_green));
	col.b = int(fl_get_slider_value(colors_->valslider_blue));
    
	fl_mapcolor(FL_FREE_COL4, col.r, col.g, col.b);
	fl_redraw_object(colors_->button_color);

	int const i = ColorsSearchEntry( col );
	// change topline only if necessary
	// int top = fl_get_browser_topline(colors_->browser_x11);
	// if (i < top || i > (top+15))
	fl_set_browser_topline(colors_->browser_x11, i - 5);
	fl_select_browser_line(colors_->browser_x11, i + 1);

	// Only activate the "Modify" button if the browser and slider colors
	// are the same AND if a LyX object is selected.
	int line = fl_get_browser(colors_->browser_lyx_objs);
	bool isSelected = ( line > 0 );
	if( isSelected )
		if( line > fl_get_browser_maxline(colors_->browser_lyx_objs) )
			isSelected = false;	

	if( isSelected && colorDB[i].second == col ) {
		fl_activate_object( colors_->button_modify );
		fl_set_object_lcol( colors_->button_modify, FL_BLACK );
	} else {
		fl_deactivate_object( colors_->button_modify );
		fl_set_object_lcol( colors_->button_modify, FL_INACTIVE );
	}

	fl_unfreeze_form(colors_->form);

	return true;
}


int FormPreferences::ColorsSearchEntry(RGBColor const & col) const
{
	int mindiff = 0x7fffffff;
	vector<X11Color>::const_iterator mincit = colorDB.begin();

	for (vector<X11Color>::const_iterator cit = colorDB.begin();
	     cit != colorDB.end(); ++cit) {
		RGBColor colDB = (*cit).second;
		RGBColor diff;
		diff.r = col.r - colDB.r;
		diff.g = col.g - colDB.g;
		diff.b = col.b - colDB.b;

		int d = (2 * (diff.r * diff.r) +
		         3 * (diff.g * diff.g) +
		             (diff.b * diff.b));

		if (mindiff > d) {
			mindiff = d;
			mincit = cit;
		}
	}
	return static_cast<int>(mincit - colorDB.begin());
}


void FormPreferences::updateColors()
{}


void FormPreferences::applyConverters() const
{
	converters = local_converters;
	converters.Update(formats);
	converters.BuildGraph();
}


void FormPreferences::buildConverters()
{
	converters_ = build_converters();

	fl_set_input_return(converters_->input_converter, FL_RETURN_CHANGED);
	fl_set_input_return(converters_->input_flags, FL_RETURN_CHANGED);

	// set up the feedback mechanism
	setPreHandler( converters_->browser_all );
	setPreHandler( converters_->button_delete );
	setPreHandler( converters_->button_add );
	setPreHandler( converters_->input_converter );
	setPreHandler( converters_->choice_from );
	setPreHandler( converters_->choice_to );
	setPreHandler( converters_->input_flags );
}


string const
FormPreferences::feedbackConverters( FL_OBJECT const * const ob ) const
{
	string str;

	if (ob == converters_->browser_all) {
		str = N_("All the currently defined converters known to LyX.");
	} else if (ob == converters_->choice_from) {
		str = N_("Convert \"from\" this format");
	} else if (ob == converters_->choice_to) {
		str = N_("Convert \"to\" this format");
	} else if (ob == converters_->input_converter) {
		str = N_("The conversion command. $$i is the input file name, $$b is the file name without its extension and $$o is the name of the output file.");
	} else if (ob == converters_->input_flags) {
		str = N_("Flags that control the converter behavior");
	} else if (ob == converters_->button_delete) {
		str = N_("Remove the current converter from the list of available converters.");
	} else if (ob == converters_->button_add) {
		str = N_("Add the current converter to the list of available converters.");
	}

	return str;
}


bool FormPreferences::inputConverters( FL_OBJECT const * const ob )
{
	if( ob == converters_->browser_all ) {
		return ConvertersBrowser();

	} else if (ob == converters_->choice_from
		   || ob == converters_->choice_to
		   || ob == converters_->input_converter
		   || ob == converters_->input_flags ) {
		return ConvertersInput();

	} else if( ob == converters_->button_add ) {
		return ConvertersAdd();

	} else if( ob == converters_->button_delete ) {
		return ConvertersDelete();
	}

	return true;
}


void FormPreferences::updateConverters()
{
	local_converters = converters;
	local_converters.Update(local_formats);
	ConvertersUpdateBrowser();
}


struct compare_converter {
	int operator()(Converter const & a, Converter const & b) {
		int i = compare_no_case(a.From->prettyname(),
					b.From->prettyname());
		if (i == 0)
			return compare_no_case(a.To->prettyname(),
					       b.To->prettyname()) < 0;
		else
			return i < 0;
	}
};


void FormPreferences::ConvertersUpdateBrowser()
{
	local_converters.Sort();

	fl_freeze_form(converters_->form);
	fl_clear_browser(converters_->browser_all);
	for (Converters::const_iterator cit = local_converters.begin();
	     cit != local_converters.end(); ++cit) {
		string name = (*cit).From->prettyname() + " -> "
			+ (*cit).To->prettyname();
		fl_addto_browser(converters_->browser_all, name.c_str());
	}
	ConvertersInput();
	fl_unfreeze_form(converters_->form);
}


bool FormPreferences::ConvertersAdd()
{
	string from = ConverterGetFrom();
	string to = ConverterGetTo();
	string command = fl_get_input(converters_->input_converter);
	string flags = fl_get_input(converters_->input_flags);

	Converter const * old = local_converters.GetConverter(from, to);
	local_converters.Add(from, to, command, flags);
	if (!old) {
		local_converters.UpdateLast(local_formats);
		ConvertersUpdateBrowser();
	}
	fl_deactivate_object(converters_->button_add);
	fl_set_object_lcol(converters_->button_add, FL_INACTIVE);

	return true;
}


bool FormPreferences::ConvertersBrowser() 
{
	int i = fl_get_browser( converters_->browser_all );
	if( i <= 0 ) return false;

	fl_freeze_form( converters_->form );

	Converter const & c = local_converters.Get(i-1);
	int j = local_formats.GetNumber(c.from);
	if (j >= 0)
		fl_set_choice(converters_->choice_from, j+1);

	j = local_formats.GetNumber(c.to);
	if (j >= 0)
		fl_set_choice(converters_->choice_to, j+1);

	fl_set_input(converters_->input_converter, c.command.c_str());
	fl_set_input(converters_->input_flags, c.flags.c_str());

	fl_set_object_label( converters_->button_add, idex(_("Modify|#M")) );
	fl_set_button_shortcut( converters_->button_add,
				scex(_("Modify|#M")), 1 );

	fl_deactivate_object( converters_->button_add );
	fl_set_object_lcol( converters_->button_add, FL_INACTIVE );

	fl_activate_object( converters_->button_delete );
	fl_set_object_lcol( converters_->button_delete, FL_BLACK );
				
	fl_unfreeze_form( converters_->form);
	return false;
}


bool FormPreferences::ConvertersDelete()
{
	string from = ConverterGetFrom();
	string to = ConverterGetTo();

	local_converters.Delete(from, to);
	ConvertersUpdateBrowser();
	return true;
}


bool FormPreferences::ConvertersInput()
{
	string from = ConverterGetFrom();
	string to = ConverterGetTo();
	int sel = local_converters.GetNumber(from, to);
	
	fl_freeze_form(converters_->form);

	if (sel < 0) {
		fl_set_object_label( converters_->button_add,
				     idex(_("Add|#A")) );
		fl_set_button_shortcut( converters_->button_add,
					scex(_("Add|#A")), 1);

		fl_deselect_browser(converters_->browser_all);

		fl_deactivate_object(converters_->button_delete);
		fl_set_object_lcol(converters_->button_delete, FL_INACTIVE);

	} else {
		fl_set_object_label( converters_->button_add,
				     idex(_("Modify|#M")) );
		fl_set_button_shortcut( converters_->button_add,
					scex(_("Modify|#M")), 1);
		
		int top = max(sel-5, 0);
		fl_set_browser_topline(converters_->browser_all, top);
		fl_select_browser_line(converters_->browser_all, sel+1);
		
		fl_activate_object(converters_->button_delete);
		fl_set_object_lcol(converters_->button_delete, FL_BLACK);
	}

	string command = fl_get_input(converters_->input_converter);
	if (command.empty() || from == to) {
		fl_deactivate_object(converters_->button_add);
		fl_set_object_lcol(converters_->button_add, FL_INACTIVE);
	} else {
		fl_activate_object(converters_->button_add);
		fl_set_object_lcol(converters_->button_add, FL_BLACK);
	}

	fl_unfreeze_form(converters_->form);
	return false;
}


string const FormPreferences::ConverterGetFrom() const
{
	int i = fl_get_choice(converters_->choice_from);
	if (i > 0)
		return local_formats.Get(i-1).name();
	else {
		lyxerr << "FormPreferences::ConvertersGetFrom: No choice!" << endl;
		return "???";
	}
}


string const FormPreferences::ConverterGetTo() const
{
	int i = fl_get_choice(converters_->choice_to);
	if (i > 0)
		return local_formats.Get(i-1).name();
	else {
		lyxerr << "FormPreferences::ConvertersGetTo: No choice!" << endl;
		return "???";
	}
}


void FormPreferences::ConvertersUpdateChoices() const
{
	string choice;
	for (Formats::const_iterator cit = local_formats.begin();
	     cit != local_formats.end(); ++cit ) {
		if (!choice.empty())
			choice += " | ";
		else
			choice += " ";
		choice += (*cit).prettyname();
	}
	choice += " ";

	fl_clear_choice( converters_->choice_from );
	fl_addto_choice( converters_->choice_from, choice.c_str() );

	fl_clear_choice( converters_->choice_to );
	fl_addto_choice( converters_->choice_to, choice.c_str() );
}


void FormPreferences::applyFormats() const
{
	formats = local_formats;
}


void FormPreferences::buildFormats()
{
	formats_ = build_formats();

	fl_set_input_return(formats_->input_format, FL_RETURN_CHANGED);
	fl_set_input_return(formats_->input_viewer, FL_RETURN_CHANGED);
	fl_set_input_return(formats_->input_shrtcut, FL_RETURN_CHANGED);
	fl_set_input_return(formats_->input_gui_name, FL_RETURN_CHANGED);
	fl_set_input_return(formats_->input_extension, FL_RETURN_CHANGED);

	fl_set_input_filter(formats_->input_format, fl_lowercase_filter);

	// set up the feedback mechanism
	setPreHandler( formats_->browser_all );
	setPreHandler( formats_->input_format );
	setPreHandler( formats_->input_gui_name );
	setPreHandler( formats_->button_delete );
	setPreHandler( formats_->button_add );
	setPreHandler( formats_->input_extension );
	setPreHandler( formats_->input_viewer );
	setPreHandler( formats_->input_shrtcut );
}


string const
FormPreferences::feedbackFormats( FL_OBJECT const * const ob ) const
{
	string str;

	if (ob == formats_->browser_all) {
		str = N_("All the currently defined formats known to LyX.");
	} else if (ob == formats_->input_format) {
		str = N_("The format identifier.");
	} else if (ob == formats_->input_gui_name) {
		str = N_("The format name as it will appear in the menus.");
	} else if (ob == formats_->input_shrtcut) {
		str = N_("The keyboard accelerator. Use a letter in the GUI name. Case sensitive.");
	} else if (ob == formats_->input_extension) {
		str = N_("Used to recognize the file. E.g., ps, pdf, tex.");
	} else if (ob == formats_->input_viewer) {
		str = N_("The command used to launch the viewer application.");
	} else if (ob == formats_->button_delete) {
		str = N_("Remove the current format from the list of available formats.");
	} else if (ob == formats_->button_add) {
		str = N_("Add the current format to the list of available formats.");
	}

	return str;
}


bool FormPreferences::inputFormats(FL_OBJECT const * const ob)
{
	if (ob == formats_->browser_all) {
		return FormatsBrowser();

	} else if (ob == formats_->input_format
		   || ob == formats_->input_gui_name
		   || ob == formats_->input_shrtcut
		   || ob == formats_->input_extension
		   || ob == formats_->input_viewer ) {
		return FormatsInput();

	} else if (ob == formats_->button_add) {
		return FormatsAdd();

	} else if (ob == formats_->button_delete) {
		return FormatsDelete();
	}

	return false;
}


void FormPreferences::updateFormats()
{
	local_formats = formats;
	FormatsUpdateBrowser();
}


void FormPreferences::FormatsUpdateBrowser()
{
	local_formats.Sort();

	fl_freeze_form(formats_->form);
	fl_deselect_browser(formats_->browser_all);
	fl_clear_browser(formats_->browser_all);
	for (Formats::const_iterator cit = local_formats.begin();
	     cit != local_formats.end(); ++cit)
		fl_addto_browser(formats_->browser_all,
				 (*cit).prettyname().c_str());

	FormatsInput();
	fl_unfreeze_form(formats_->form);

	// Mustn't forget to update the Formats available to the converters
	ConvertersUpdateChoices();
	local_converters.Update(local_formats);
}


bool FormPreferences::FormatsAdd()
{
	string name = fl_get_input(formats_->input_format);
	string prettyname = fl_get_input(formats_->input_gui_name);
	string extension = fl_get_input(formats_->input_extension);
	string shortcut =  fl_get_input(formats_->input_shrtcut);
	string viewer =  fl_get_input(formats_->input_viewer);

	Format const * old = local_formats.GetFormat(name);
	string old_prettyname = old ? old->prettyname() : string();
	local_formats.Add(name, extension, prettyname, shortcut);
	local_formats.SetViewer(name, viewer);
	if (!old || prettyname != old_prettyname) {
		FormatsUpdateBrowser();
		if (old)
			ConvertersUpdateBrowser();
	}
	fl_deactivate_object(formats_->button_add);
	fl_set_object_lcol(formats_->button_add, FL_INACTIVE);

	return true;
}


bool FormPreferences::FormatsBrowser() 
{
	int i = fl_get_browser(formats_->browser_all);
	if( i <= 0 ) return false;

	fl_freeze_form(formats_->form);

	Format const & f = local_formats.Get(i-1);

	fl_set_input(formats_->input_format, f.name().c_str());
	fl_set_input(formats_->input_gui_name, f.prettyname().c_str());
	fl_set_input(formats_->input_shrtcut, f.shortcut().c_str());
	fl_set_input(formats_->input_extension, f.extension().c_str());
	fl_set_input(formats_->input_viewer, f.viewer().c_str());

	fl_set_object_label( formats_->button_add, idex(_("Modify|#M")) );
	fl_set_button_shortcut( formats_->button_add, scex(_("Modify|#M")), 1);

	fl_deactivate_object(formats_->button_add);
	fl_set_object_lcol(formats_->button_add, FL_INACTIVE);

	fl_activate_object(formats_->button_delete);
	fl_set_object_lcol(formats_->button_delete, FL_BLACK);
				
	fl_unfreeze_form(formats_->form);
	return false;
}


bool FormPreferences::FormatsDelete()
{
	string name = fl_get_input(formats_->input_format);

	if (local_converters.FormatIsUsed(name)) {
		printWarning(_("WARNING! Cannot remove a Format used by a Converter. Remove the converter first."));
		fl_deactivate_object(formats_->button_delete);
		fl_set_object_lcol(formats_->button_delete, FL_INACTIVE);
		return false;
	}

	local_formats.Delete(name);
	FormatsUpdateBrowser();
	return true;
}


bool FormPreferences::FormatsInput()
{
	string name = fl_get_input(formats_->input_format);
	int sel = local_formats.GetNumber(name);
	fl_freeze_form(formats_->form);

	if (sel < 0) {
		fl_set_object_label( formats_->button_add,
				     idex(_("Add|#A")) );
		fl_set_button_shortcut( formats_->button_add,
					scex(_("Add|#A")), 1);

		fl_deselect_browser(formats_->browser_all);

		fl_deactivate_object(formats_->button_delete);
		fl_set_object_lcol(formats_->button_delete, FL_INACTIVE);

	} else {
		fl_set_object_label( formats_->button_add,
				     idex(_("Modify|#M")) );
		fl_set_button_shortcut( formats_->button_add,
					scex(_("Modify|#M")), 1);

		int top = max(sel-5, 0);
		fl_set_browser_topline(formats_->browser_all, top);
		fl_select_browser_line(formats_->browser_all, sel+1);
		
		fl_activate_object(formats_->button_add);
		fl_set_object_lcol(formats_->button_add, FL_BLACK);

		fl_activate_object(formats_->button_delete);
		fl_set_object_lcol(formats_->button_delete, FL_BLACK);
	}

	string prettyname = fl_get_input(formats_->input_gui_name);
	if (name.empty() || prettyname.empty()) {
		fl_deactivate_object(formats_->button_add);
		fl_set_object_lcol(formats_->button_add, FL_INACTIVE);
	} else {
		fl_activate_object(formats_->button_add);
		fl_set_object_lcol(formats_->button_add, FL_BLACK);
	}

	fl_unfreeze_form(formats_->form);
	return false;
}


void FormPreferences::applyInputsMisc() const
{
	lyxrc.date_insert_format =
		fl_get_input(inputs_misc_->input_date_format);
}


void FormPreferences::buildInputsMisc()
{
	inputs_misc_ = build_inputs_misc();

	fl_set_input_return(inputs_misc_->input_date_format,
			    FL_RETURN_CHANGED);

	// set up the feedback mechanism
	setPreHandler( inputs_misc_->input_date_format );
}


string const
FormPreferences::feedbackInputsMisc(FL_OBJECT const * const ob) const
{
	string str;

	if (ob == inputs_misc_->input_date_format )
		str = lyxrc.getDescription( LyXRC::RC_DATE_INSERT_FORMAT );

	return str;
}


void FormPreferences::updateInputsMisc()
{
	fl_set_input(inputs_misc_->input_date_format,
		     lyxrc.date_insert_format.c_str());
}


void FormPreferences::applyInterface() const
{
	lyxrc.popup_font_name =
		fl_get_input(interface_->input_popup_font);
	lyxrc.menu_font_name = fl_get_input(interface_->input_menu_font);
	lyxrc.font_norm_menu =
		fl_get_input(interface_->input_popup_encoding);
	lyxrc.bind_file = fl_get_input(interface_->input_bind_file);
	lyxrc.ui_file = fl_get_input(interface_->input_ui_file);
	lyxrc.override_x_deadkeys =
		fl_get_button(interface_->check_override_x_dead_keys);
}


void FormPreferences::buildInterface()
{
	interface_ = build_interface();

	fl_set_input_return(interface_->input_popup_font, FL_RETURN_CHANGED);
	fl_set_input_return(interface_->input_menu_font, FL_RETURN_CHANGED);
	fl_set_input_return(interface_->input_popup_encoding, 
			    FL_RETURN_CHANGED);
	fl_set_input_return(interface_->input_bind_file, FL_RETURN_CHANGED);
	fl_set_input_return(interface_->input_ui_file, FL_RETURN_CHANGED);

	// deactivate the browse buttons because they aren't implemented
	fl_deactivate_object(interface_->button_bind_file_browse);
	fl_deactivate_object(interface_->button_ui_file_browse);
	fl_set_object_lcol(interface_->button_bind_file_browse, FL_INACTIVE);
	fl_set_object_lcol(interface_->button_ui_file_browse, FL_INACTIVE);

	// set up the feedback mechanism
	setPreHandler( interface_->input_popup_font );
	setPreHandler( interface_->input_menu_font );
	setPreHandler( interface_->input_popup_encoding );
	setPreHandler( interface_->input_bind_file );
	setPreHandler( interface_->button_bind_file_browse );
	setPreHandler( interface_->input_ui_file );
	setPreHandler( interface_->button_ui_file_browse );
	setPreHandler( interface_->check_override_x_dead_keys );
}


string const
FormPreferences::feedbackInterface( FL_OBJECT const * const ob ) const
{
	string str;

	if (ob == interface_->input_popup_font )
		str = lyxrc.getDescription( LyXRC::RC_SCREEN_FONT_POPUP );
	else if (ob == interface_->input_menu_font )
		str = lyxrc.getDescription( LyXRC::RC_SCREEN_FONT_MENU );
	else if (ob == interface_->input_popup_encoding )
		str = lyxrc.getDescription( LyXRC::RC_SCREEN_FONT_ENCODING_MENU );
	else if (ob == interface_->input_bind_file )
		str = lyxrc.getDescription( LyXRC::RC_BINDFILE );
	else if (ob == interface_->input_ui_file )
		str = lyxrc.getDescription( LyXRC::RC_UIFILE );
	else if (ob == interface_->check_override_x_dead_keys )
		str = lyxrc.getDescription( LyXRC::RC_OVERRIDE_X_DEADKEYS );

	return str;
}


void FormPreferences::updateInterface()
{
	fl_set_input(interface_->input_popup_font,
		     lyxrc.popup_font_name.c_str());
	fl_set_input(interface_->input_menu_font,
		     lyxrc.menu_font_name.c_str());
	fl_set_input(interface_->input_popup_encoding,
		     lyxrc.font_norm_menu.c_str());
	fl_set_input(interface_->input_bind_file,
		     lyxrc.bind_file.c_str());
	fl_set_input(interface_->input_ui_file,
		     lyxrc.ui_file.c_str());
	fl_set_button(interface_->check_override_x_dead_keys,
		      lyxrc.override_x_deadkeys);
}


void FormPreferences::applyLanguage()
{
	lyxrc.default_language = combo_default_lang->getline();

	int button = fl_get_button(language_->check_use_kbmap);
	string name_1 = combo_kbmap_1->getline();
	string name_2 = combo_kbmap_2->getline();
	if( button )
		button = ( !name_1.empty() && !name_2.empty() );
	lyxrc.use_kbmap = static_cast<bool>(button);

	if (button) {
		vector<string>::const_iterator cit =
			find( dirlist.second.begin(), dirlist.second.end(),
			      name_1 );
		vector<string>::size_type sel = cit - dirlist.second.begin();
		lyxrc.primary_kbmap = dirlist.first[sel];

		cit = find( dirlist.second.begin(), dirlist.second.end(),
			   name_2 );
		sel = cit - dirlist.second.begin();
		lyxrc.secondary_kbmap = dirlist.first[sel];
	}
	
	button = fl_get_button(language_->check_rtl_support);
	lyxrc.rtl_support = static_cast<bool>(button);

	button = fl_get_button(language_->check_auto_begin);
	lyxrc.language_auto_begin = static_cast<bool>(button);

	button = fl_get_button(language_->check_auto_end);
	lyxrc.language_auto_end = static_cast<bool>(button);

	button = fl_get_button(language_->check_mark_foreign);
	lyxrc.mark_foreign_language = static_cast<bool>(button);

	lyxrc.language_package = fl_get_input(language_->input_package);
	lyxrc.language_command_begin = fl_get_input(language_->input_command_begin);
	lyxrc.language_command_end = fl_get_input(language_->input_command_end);

	// Ensure that all is self-consistent.
	updateLanguage();
}


void FormPreferences::buildLanguage()
{
	language_ = build_language();

	fl_set_input_return(language_->input_package, FL_RETURN_CHANGED);
	fl_set_input_return(language_->input_command_begin, FL_RETURN_CHANGED);
	fl_set_input_return(language_->input_command_end, FL_RETURN_CHANGED);

	// The default_language is a combo-box and has to be inserted manually
	fl_freeze_form(language_->form);
	fl_addto_form(language_->form);

	FL_OBJECT * obj = language_->choice_default_lang;
	fl_deactivate_object(language_->choice_default_lang);
	combo_default_lang = new Combox(FL_COMBOX_DROPLIST);
	combo_default_lang->add(obj->x, obj->y, obj->w, obj->h, 400);
	combo_default_lang->shortcut("#L",1);
	combo_default_lang->setcallback(ComboLanguageCB, this);

	for (Languages::const_iterator cit = languages.begin();
	    cit != languages.end(); ++cit) {
		combo_default_lang->addto((*cit).second.lang());
	}

	// ditto kbmap_1
	string dir = AddPath(system_lyxdir, "kbd");
	dirlist.first = DirList(dir , "kmap");

	for (vector<string>::const_iterator cit = dirlist.first.begin();
	    cit != dirlist.first.end(); ++cit) {
		string name = ChangeExtension( OnlyFilename(*cit), "" );
		dirlist.second.push_back( name );
	}

	obj = language_->choice_kbmap_1;
	fl_deactivate_object(language_->choice_kbmap_1);
	combo_kbmap_1 = new Combox(FL_COMBOX_DROPLIST);
	combo_kbmap_1->add(obj->x, obj->y, obj->w, obj->h, 400);
	combo_kbmap_1->shortcut("#1",1);
	combo_kbmap_1->setcallback(ComboLanguageCB, this);

	for (vector<string>::const_iterator cit = dirlist.second.begin();
	    cit != dirlist.second.end(); ++cit) {
		combo_kbmap_1->addto(*cit);
	}

	// ditto kbmap_2
	obj = language_->choice_kbmap_2;
	fl_deactivate_object(language_->choice_kbmap_2);
	combo_kbmap_2 = new Combox(FL_COMBOX_DROPLIST);
	combo_kbmap_2->add(obj->x, obj->y, obj->w, obj->h, 400);
	combo_kbmap_2->shortcut("#2",1);
	combo_kbmap_2->setcallback(ComboLanguageCB, this);

	for (vector<string>::const_iterator cit = dirlist.second.begin();
	    cit != dirlist.second.end(); ++cit) {
		combo_kbmap_2->addto(*cit);
	}

	fl_end_form();
	fl_unfreeze_form(language_->form);

	// set up the feedback mechanism
	setPreHandler( language_->input_package );
	setPreHandler( language_->check_use_kbmap );

	// This is safe, as nothing is done to the pointer, other than
	// to use its address in a block-if statement.
	//setPreHandler( reinterpret_cast<FL_OBJECT *>(combo_default_lang) );
	//setPreHandler( reinterpret_cast<FL_OBJECT *>(combo_kbmap_1) );
	//setPreHandler( reinterpret_cast<FL_OBJECT *>(combo_kbmap_2) );

	setPreHandler( language_->check_rtl_support );
	setPreHandler( language_->check_mark_foreign );
	setPreHandler( language_->check_auto_begin );
	setPreHandler( language_->check_auto_end );
	setPreHandler( language_->input_command_begin );
	setPreHandler( language_->input_command_end );

	// Activate/Deactivate the input fields dependent on the state of the
	// buttons.
	inputLanguage( 0 );
}


string const
FormPreferences::feedbackLanguage(FL_OBJECT const * const ob) const
{
	string str;

	if (reinterpret_cast<Combox const *>(ob) == combo_default_lang )
		str = lyxrc.getDescription( LyXRC::RC_DEFAULT_LANGUAGE );
	else if (ob == language_->check_use_kbmap )
		str = lyxrc.getDescription( LyXRC::RC_KBMAP );
	else if (reinterpret_cast<Combox const *>(ob) == combo_kbmap_1)
		str = lyxrc.getDescription( LyXRC::RC_KBMAP_PRIMARY );
	else if (reinterpret_cast<Combox const *>(ob) == combo_kbmap_2 )
		str = lyxrc.getDescription( LyXRC::RC_KBMAP_SECONDARY );
	else if (ob == language_->check_rtl_support )
		str = lyxrc.getDescription( LyXRC::RC_RTL_SUPPORT );
	else if (ob == language_->check_auto_begin )
		str = lyxrc.getDescription( LyXRC::RC_LANGUAGE_AUTO_BEGIN );
	else if (ob == language_->check_auto_end )
		str = lyxrc.getDescription( LyXRC::RC_LANGUAGE_AUTO_END );
	else if (ob == language_->check_mark_foreign )
		str = lyxrc.getDescription( LyXRC::RC_MARK_FOREIGN_LANGUAGE );
	else if (ob == language_->input_package )
		str = lyxrc.getDescription( LyXRC::RC_LANGUAGE_PACKAGE );
	else if (ob == language_->input_command_begin )
		str = lyxrc.getDescription( LyXRC::RC_LANGUAGE_COMMAND_BEGIN );
	else if (ob == language_->input_command_end )
		str = lyxrc.getDescription( LyXRC::RC_LANGUAGE_COMMAND_END );

	return str;
}


bool FormPreferences::inputLanguage( FL_OBJECT const * const ob )
{
	bool activate = true;

	// !ob if function is called from buildLanguage() to de/activate
	// objects,
	// otherwise the function is called by an xforms CB via input().
	if (!ob || ob == language_->check_use_kbmap) {
		if (fl_get_button(language_->check_use_kbmap)) {
			combo_kbmap_1->activate();
			combo_kbmap_2->activate();
		} else {
			combo_kbmap_1->deactivate();
			combo_kbmap_2->deactivate();
		}
	}

	return activate;
}


void FormPreferences::updateLanguage()
{
	fl_set_button(language_->check_use_kbmap,
		      lyxrc.use_kbmap);

	combo_default_lang->select_text( lyxrc.default_language );

	if (lyxrc.use_kbmap) {
		string fullpath = lyxrc.primary_kbmap;
		vector<string>::const_iterator cit =
			find( dirlist.first.begin(), dirlist.first.end(),
			      fullpath );
		if (cit != dirlist.first.end()) {
			vector<string>::size_type sel =
				cit - dirlist.first.begin();
			combo_kbmap_1->select_text(dirlist.second[sel]);
		} else
			combo_kbmap_1->select_text("");

		fullpath = lyxrc.secondary_kbmap;
		cit = find( dirlist.first.begin(), dirlist.first.end(),
			   fullpath );
		if (cit != dirlist.first.end()) {
			vector<string>::size_type sel =
				cit - dirlist.first.begin();
			combo_kbmap_2->select_text(dirlist.second[sel]);
		} else
			combo_kbmap_2->select_text("");
	} else {
		combo_kbmap_1->select_text( "" );
		combo_kbmap_2->select_text( "" );
	}
	
	fl_set_button(language_->check_rtl_support, lyxrc.rtl_support);
	fl_set_button(language_->check_auto_begin,  lyxrc.language_auto_begin);
	fl_set_button(language_->check_auto_end,    lyxrc.language_auto_end);
	fl_set_button(language_->check_mark_foreign,
		      lyxrc.mark_foreign_language);

	fl_set_input(language_->input_package,
		     lyxrc.language_package.c_str());
	fl_set_input(language_->input_command_begin,
		     lyxrc.language_command_begin.c_str());
	fl_set_input(language_->input_command_end,
		     lyxrc.language_command_end.c_str());

	// Activate/Deactivate the input fields dependent on the state of the
	// buttons.
	inputLanguage( 0 );
}


void FormPreferences::applyLnFmisc() const
{
	lyxrc.show_banner = fl_get_button(lnf_misc_->check_banner);
	lyxrc.auto_region_delete = fl_get_button(lnf_misc_->
						 check_auto_region_delete);
	lyxrc.exit_confirmation = fl_get_button(lnf_misc_->check_exit_confirm);
	lyxrc.display_shortcuts =
		fl_get_button(lnf_misc_->check_display_shrtcuts);
	lyxrc.new_ask_filename = fl_get_button(lnf_misc_->check_ask_new_file);
	lyxrc.cursor_follows_scrollbar =
		fl_get_button(lnf_misc_->check_cursor_follows_scrollbar);
	lyxrc.autosave = static_cast<unsigned int>
		(fl_get_counter_value(lnf_misc_->counter_autosave));
	lyxrc.wheel_jump = static_cast<unsigned int>
		(fl_get_counter_value(lnf_misc_->counter_wm_jump));
}


void FormPreferences::buildLnFmisc()
{
	lnf_misc_ = build_lnf_misc();

	fl_set_counter_return(lnf_misc_->counter_autosave,
			      FL_RETURN_CHANGED);
	fl_set_counter_return(lnf_misc_->counter_wm_jump,
			      FL_RETURN_CHANGED);

	// set up the feedback mechanism
	setPreHandler( lnf_misc_->check_banner );
	setPreHandler( lnf_misc_->check_auto_region_delete );
	setPreHandler( lnf_misc_->check_exit_confirm );
	setPreHandler( lnf_misc_->check_display_shrtcuts );
	setPreHandler( lnf_misc_->counter_autosave );
	setPreHandler( lnf_misc_->check_ask_new_file );
	setPreHandler( lnf_misc_->check_cursor_follows_scrollbar );
	setPreHandler( lnf_misc_->counter_wm_jump );
}


string const FormPreferences::feedbackLnFmisc(FL_OBJECT const * const ob) const
{
	string str;

	if (ob == lnf_misc_->check_banner )
		str = lyxrc.getDescription( LyXRC::RC_SHOW_BANNER );
	else if (ob == lnf_misc_->check_auto_region_delete )
		str = lyxrc.getDescription( LyXRC::RC_AUTOREGIONDELETE );
	else if (ob == lnf_misc_->check_exit_confirm )
		str = lyxrc.getDescription( LyXRC::RC_EXIT_CONFIRMATION );
	else if (ob == lnf_misc_->check_display_shrtcuts )
		str = lyxrc.getDescription( LyXRC::RC_DISPLAY_SHORTCUTS );
	else if (ob == lnf_misc_->check_ask_new_file )
		str = lyxrc.getDescription( LyXRC::RC_NEW_ASK_FILENAME );
	else if (ob == lnf_misc_->check_cursor_follows_scrollbar )
		str = lyxrc.getDescription( LyXRC::RC_CURSOR_FOLLOWS_SCROLLBAR );
	else if (ob == lnf_misc_->counter_autosave )
		str = lyxrc.getDescription( LyXRC::RC_AUTOSAVE );
	else if (ob == lnf_misc_->counter_wm_jump )
		str = lyxrc.getDescription( LyXRC::RC_WHEEL_JUMP );

	return str;
}


void FormPreferences::updateLnFmisc()
{
	fl_set_button(lnf_misc_->check_banner,
		      lyxrc.show_banner);
	fl_set_button(lnf_misc_->check_auto_region_delete,
		      lyxrc.auto_region_delete);
	fl_set_button(lnf_misc_->check_exit_confirm,
		      lyxrc.exit_confirmation);
	fl_set_button(lnf_misc_->check_display_shrtcuts,
		      lyxrc.display_shortcuts);
	fl_set_button(lnf_misc_->check_ask_new_file,
		      lyxrc.new_ask_filename);
	fl_set_button(lnf_misc_->check_cursor_follows_scrollbar,
		      lyxrc.cursor_follows_scrollbar);
	fl_set_counter_value(lnf_misc_->counter_autosave,
			     lyxrc.autosave);
	fl_set_counter_value(lnf_misc_->counter_wm_jump,
			     lyxrc.wheel_jump);
}


void FormPreferences::applyOutputsMisc() const
{
	lyxrc.ascii_linelen = static_cast<unsigned int>
		(fl_get_counter_value(outputs_misc_->counter_line_len));
	lyxrc.fontenc = fl_get_input(outputs_misc_->input_tex_encoding);

	int choice =
		fl_get_choice(outputs_misc_->choice_default_papersize) - 1;
	lyxrc.default_papersize = static_cast<BufferParams::PAPER_SIZE>(choice);

	lyxrc.ascii_roff_command = fl_get_input(outputs_misc_->input_ascii_roff);
	lyxrc.chktex_command = fl_get_input(outputs_misc_->input_checktex);
}


void FormPreferences::buildOutputsMisc()
{
	outputs_misc_ = build_outputs_misc();

	fl_set_counter_return(outputs_misc_->counter_line_len,
			      FL_RETURN_CHANGED);
	fl_set_input_return(outputs_misc_->input_tex_encoding,
			    FL_RETURN_CHANGED);
	fl_set_input_return(outputs_misc_->input_ascii_roff,
			    FL_RETURN_CHANGED);
	fl_set_input_return(outputs_misc_->input_checktex,
			    FL_RETURN_CHANGED);
	fl_addto_choice(outputs_misc_->choice_default_papersize,
			_(" default | US letter | legal | executive | A3 | A4 | A5 | B5 "));

	// set up the feedback mechanism
	setPreHandler( outputs_misc_->counter_line_len );
	setPreHandler( outputs_misc_->input_tex_encoding );
	setPreHandler( outputs_misc_->choice_default_papersize );
	setPreHandler( outputs_misc_->input_ascii_roff );
	setPreHandler( outputs_misc_->input_checktex );
}


string const
FormPreferences::feedbackOutputsMisc(FL_OBJECT const * const ob) const
{
	string str;

	if (ob == outputs_misc_->counter_line_len )
		str = lyxrc.getDescription( LyXRC::RC_ASCII_LINELEN );
	else if (ob == outputs_misc_->input_tex_encoding )
		str = lyxrc.getDescription( LyXRC::RC_FONT_ENCODING );
	else if (ob == outputs_misc_->input_ascii_roff )
		str = lyxrc.getDescription( LyXRC::RC_ASCIIROFF_COMMAND );
	else if (ob == outputs_misc_->input_checktex )
		str = lyxrc.getDescription( LyXRC::RC_CHKTEX_COMMAND );
	else if (ob == outputs_misc_->choice_default_papersize )
		str = lyxrc.getDescription( LyXRC::RC_DEFAULT_PAPERSIZE );

	return str;
}


void FormPreferences::updateOutputsMisc()
{
	fl_set_counter_value(outputs_misc_->counter_line_len,
			     lyxrc.ascii_linelen);
	fl_set_input(outputs_misc_->input_tex_encoding,
		     lyxrc.fontenc.c_str());
	fl_set_choice(outputs_misc_->choice_default_papersize,
		      lyxrc.default_papersize+1);
	fl_set_input(outputs_misc_->input_ascii_roff,
		     lyxrc.ascii_roff_command.c_str());
	fl_set_input(outputs_misc_->input_checktex,
		     lyxrc.chktex_command.c_str());
}


void FormPreferences::applyPaths()
{
	lyxrc.document_path = fl_get_input(paths_->input_default_path);
	lyxrc.template_path = fl_get_input(paths_->input_template_path);

	int button = fl_get_button(paths_->check_use_temp_dir);
	string str  = fl_get_input(paths_->input_temp_dir);
	if (!button ) str.erase();

	lyxrc.use_tempdir = button;
	lyxrc.tempdir_path = str;

	button = fl_get_button(paths_->check_last_files);
	str = fl_get_input(paths_->input_lastfiles);
	if (!button ) str.erase();
	
	lyxrc.check_lastfiles = button;
	lyxrc.lastfiles = str;
	lyxrc.num_lastfiles = static_cast<unsigned int>
		(fl_get_counter_value(paths_->counter_lastfiles));

	button = fl_get_button(paths_->check_make_backups);
	str = fl_get_input(paths_->input_backup_path);
	if (!button ) str.erase();

	lyxrc.make_backup = button;
	lyxrc.backupdir_path = str;

	lyxrc.lyxpipes = fl_get_input(paths_->input_serverpipe);

	// update view
	updatePaths();
}


void FormPreferences::buildPaths()
{
	paths_ = build_paths();

	fl_set_input_return(paths_->input_default_path, FL_RETURN_CHANGED);
	fl_set_input_return(paths_->input_template_path, FL_RETURN_CHANGED);
	fl_set_input_return(paths_->input_temp_dir, FL_RETURN_CHANGED);
	fl_set_input_return(paths_->input_lastfiles, FL_RETURN_CHANGED);
	fl_set_input_return(paths_->input_backup_path, FL_RETURN_CHANGED);
	fl_set_counter_return(paths_->counter_lastfiles, FL_RETURN_CHANGED);
	fl_set_input_return(paths_->input_serverpipe, FL_RETURN_CHANGED);

	// deactivate the browse buttons because they aren't implemented
	fl_deactivate_object(paths_->button_document_browse);
	fl_deactivate_object(paths_->button_template_browse);
	fl_deactivate_object(paths_->button_temp_dir_browse);
	fl_deactivate_object(paths_->button_lastfiles_browse);
	fl_deactivate_object(paths_->button_backup_path_browse);
	fl_deactivate_object(paths_->button_serverpipe_browse);
	fl_set_object_lcol(paths_->button_document_browse, FL_INACTIVE);
	fl_set_object_lcol(paths_->button_template_browse, FL_INACTIVE);
	fl_set_object_lcol(paths_->button_temp_dir_browse, FL_INACTIVE);
	fl_set_object_lcol(paths_->button_lastfiles_browse, FL_INACTIVE);
	fl_set_object_lcol(paths_->button_backup_path_browse, FL_INACTIVE);
	fl_set_object_lcol(paths_->button_serverpipe_browse, FL_INACTIVE);

	// set up the feedback mechanism
	setPreHandler( paths_->input_default_path );
	setPreHandler( paths_->button_document_browse );
	setPreHandler( paths_->counter_lastfiles );
	setPreHandler( paths_->input_template_path );
	setPreHandler( paths_->button_template_browse );
	setPreHandler( paths_->check_last_files );
	setPreHandler( paths_->button_temp_dir_browse );
	setPreHandler( paths_->input_lastfiles );
	setPreHandler( paths_->button_lastfiles_browse );
	setPreHandler( paths_->check_make_backups );
	setPreHandler( paths_->input_backup_path );
	setPreHandler( paths_->button_backup_path_browse );
	setPreHandler( paths_->input_serverpipe );
	setPreHandler( paths_->button_serverpipe_browse );
	setPreHandler( paths_->input_temp_dir );
	setPreHandler( paths_->check_use_temp_dir );
}


string const FormPreferences::feedbackPaths( FL_OBJECT const * const ob ) const
{
	string str;

	if (ob == paths_->input_default_path )
		str = lyxrc.getDescription( LyXRC::RC_DOCUMENTPATH );
	else if (ob == paths_->input_template_path )
		str = lyxrc.getDescription( LyXRC::RC_TEMPLATEPATH );
	else if (ob == paths_->check_use_temp_dir )
		str = lyxrc.getDescription( LyXRC::RC_USETEMPDIR );
	else if (ob == paths_->input_temp_dir )
		str = lyxrc.getDescription( LyXRC::RC_TEMPDIRPATH );
	else if (ob == paths_->check_last_files )
		str = lyxrc.getDescription( LyXRC::RC_CHECKLASTFILES );
	else if (ob == paths_->input_lastfiles )
		str = lyxrc.getDescription( LyXRC::RC_LASTFILES );
	else if (ob == paths_->counter_lastfiles )
		str = lyxrc.getDescription( LyXRC::RC_NUMLASTFILES );
	else if (ob == paths_->check_make_backups )
		str = lyxrc.getDescription( LyXRC::RC_MAKE_BACKUP );
	else if (ob == paths_->input_backup_path )
		str = lyxrc.getDescription( LyXRC::RC_BACKUPDIR_PATH );
	else if (ob == paths_->input_serverpipe )
		str = lyxrc.getDescription( LyXRC::RC_SERVERPIPE );

	return str;
}


bool FormPreferences::inputPaths( FL_OBJECT const * const ob )
{
	bool activate = true;
	
	// !ob if function is called from updatePaths() to de/activate objects,
	// otherwise the function is called by an xforms CB via input().
	if (!ob || ob == paths_->check_use_temp_dir) {
		if (fl_get_button(paths_->check_use_temp_dir)) {
			fl_activate_object(paths_->input_temp_dir);
			fl_set_object_lcol(paths_->input_temp_dir,
					   FL_BLACK);
		} else {
			fl_deactivate_object(paths_->input_temp_dir);
			fl_set_object_lcol(paths_->input_temp_dir,
					   FL_INACTIVE);
		}
	}

	if (!ob || ob == paths_->check_last_files) {
		if (fl_get_button(paths_->check_last_files)) {
			fl_activate_object(paths_->input_lastfiles);
			fl_set_object_lcol(paths_->input_lastfiles,
					   FL_BLACK);
		} else {
			fl_deactivate_object(paths_->input_lastfiles);
			fl_set_object_lcol(paths_->input_lastfiles,
					   FL_INACTIVE);
		}
	}

	if (!ob || ob == paths_->check_make_backups) {
		if (fl_get_button(paths_->check_make_backups)) {
			fl_activate_object(paths_->input_backup_path);
			fl_set_object_lcol(paths_->input_backup_path,
					   FL_BLACK);
		} else {
			fl_deactivate_object(paths_->input_backup_path);
			fl_set_object_lcol(paths_->input_backup_path,
					   FL_INACTIVE);
		}
	}

	if (!ob || ob == paths_->input_default_path) {
		string name = fl_get_input(paths_->input_default_path);
		if (!WriteableDir(name) )
			return false;
	}

	if (!ob || ob == paths_->input_template_path) {
		string name = fl_get_input(paths_->input_template_path);
		if (!ReadableDir(name) )
		    return false;
	}

	if (!ob || ob == paths_->input_temp_dir) {
		string name = fl_get_input(paths_->input_temp_dir);
		if (fl_get_button(paths_->check_make_backups)
		    && !name.empty()
		    && !WriteableDir(name) )
			return false;
	}

	if (!ob || ob == paths_->input_backup_path) {
		string name = fl_get_input(paths_->input_backup_path);
		if (fl_get_button(paths_->check_make_backups)
		    && !name.empty()
		    && !WriteableDir(name) )
			return false;
	}

	if (!ob || ob == paths_->input_lastfiles) {
		string name = fl_get_input(paths_->input_lastfiles);
		if (fl_get_button(paths_->check_last_files)
		    && !name.empty()
		    && !WriteableFile(name) )
			return false;
	}

	if (!ob || ob == paths_->input_serverpipe) {
		string name = fl_get_input(paths_->input_serverpipe);
		if (!name.empty()) {
			if (!WriteableFile(name, ".in"))
				return false;
			if (!WriteableFile(name, ".out"))
				return false;
		}
	}

	return activate;
}


void FormPreferences::updatePaths()
{
	fl_set_input(paths_->input_default_path,
		     lyxrc.document_path.c_str());
	fl_set_input(paths_->input_template_path,
		     lyxrc.template_path.c_str());

	string str;
	if (lyxrc.make_backup) str = lyxrc.backupdir_path;

	fl_set_button(paths_->check_make_backups,
		      lyxrc.make_backup);
	fl_set_input(paths_->input_backup_path, str.c_str());

	str.erase();
	if (lyxrc.use_tempdir) str = lyxrc.tempdir_path;

	fl_set_button(paths_->check_use_temp_dir,
		      lyxrc.use_tempdir);
	fl_set_input(paths_->input_temp_dir, str.c_str());

	str.erase();
	if (lyxrc.check_lastfiles) str = lyxrc.lastfiles;

	fl_set_button(paths_->check_last_files,
		      lyxrc.check_lastfiles);		
	fl_set_input(paths_->input_lastfiles, str.c_str());
	fl_set_counter_value(paths_->counter_lastfiles,
			     lyxrc.num_lastfiles);

	fl_set_input(paths_->input_serverpipe, lyxrc.lyxpipes.c_str());

	// Activate/Deactivate the input fields dependent on the state of the
	// buttons.
	inputPaths( 0 );
}


void FormPreferences::applyPrinter() const
{
	lyxrc.print_adapt_output = fl_get_button(printer_->check_adapt_output);
	lyxrc.print_command = fl_get_input(printer_->input_command);
	lyxrc.print_pagerange_flag = fl_get_input(printer_->input_page_range);
	lyxrc.print_copies_flag = fl_get_input(printer_->input_copies);
	lyxrc.print_reverse_flag = fl_get_input(printer_->input_reverse);
	lyxrc.print_to_printer = fl_get_input(printer_->input_to_printer);
	lyxrc.print_file_extension =
		fl_get_input(printer_->input_file_extension);
	lyxrc.print_spool_command =
		fl_get_input(printer_->input_spool_command);
	lyxrc.print_paper_flag = fl_get_input(printer_->input_paper_type);
	lyxrc.print_evenpage_flag = fl_get_input(printer_->input_even_pages);
	lyxrc.print_oddpage_flag = fl_get_input(printer_->input_odd_pages);
	lyxrc.print_collcopies_flag = fl_get_input(printer_->input_collated);
	lyxrc.print_landscape_flag = fl_get_input(printer_->input_landscape);
	lyxrc.print_to_file = fl_get_input(printer_->input_to_file);
	lyxrc.print_extra_options =
		fl_get_input(printer_->input_extra_options);
	lyxrc.print_spool_printerprefix =
		fl_get_input(printer_->input_spool_prefix);
	lyxrc.print_paper_dimension_flag =
		fl_get_input(printer_->input_paper_size);
	lyxrc.printer = fl_get_input(printer_->input_name);
}


string const
FormPreferences::feedbackPrinter(FL_OBJECT const * const ob) const
{
	string str;

	if (ob == printer_->input_command )
		str = lyxrc.getDescription( LyXRC::RC_PRINT_COMMAND );
	else if (ob == printer_->check_adapt_output )
		str = lyxrc.getDescription( LyXRC::RC_PRINT_ADAPTOUTPUT );
	else if (ob == printer_->input_to_printer )
		str = lyxrc.getDescription( LyXRC::RC_PRINTTOPRINTER );
	else if (ob == printer_->input_to_file )
		str = lyxrc.getDescription( LyXRC::RC_PRINTTOFILE );
	else if (ob == printer_->input_file_extension )
		str = lyxrc.getDescription( LyXRC::RC_PRINTFILEEXTENSION );
	else if (ob == printer_->input_extra_options )
		str = lyxrc.getDescription( LyXRC::RC_PRINTEXSTRAOPTIONS );
	else if (ob == printer_->input_spool_command )
		str = lyxrc.getDescription( LyXRC::RC_PRINTSPOOL_COMMAND );
	else if (ob == printer_->input_spool_prefix )
		str = lyxrc.getDescription( LyXRC::RC_PRINTSPOOL_PRINTERPREFIX );
	else if (ob == printer_->input_name )
		str = lyxrc.getDescription( LyXRC::RC_PRINTER );
	else if (ob == printer_->input_even_pages )
		str = lyxrc.getDescription( LyXRC::RC_PRINTEVENPAGEFLAG );
	else if (ob == printer_->input_odd_pages )
		str = lyxrc.getDescription( LyXRC::RC_PRINTODDPAGEFLAG );
	else if (ob == printer_->input_page_range )
		str = lyxrc.getDescription( LyXRC::RC_PRINTPAGERANGEFLAG );
	else if (ob == printer_->input_reverse )
		str = lyxrc.getDescription( LyXRC::RC_PRINTREVERSEFLAG );
	else if (ob == printer_->input_landscape )
		str = lyxrc.getDescription( LyXRC::RC_PRINTLANDSCAPEFLAG );
	else if (ob == printer_->input_copies )
		str = lyxrc.getDescription( LyXRC::RC_PRINTCOLLCOPIESFLAG );
	else if (ob == printer_->input_collated )
		str = lyxrc.getDescription( LyXRC::RC_PRINTCOPIESFLAG );
	else if (ob == printer_->input_paper_type )
		str = lyxrc.getDescription( LyXRC::RC_PRINTPAPERFLAG );
	else if (ob == printer_->input_paper_size )
		str = lyxrc.getDescription( LyXRC::RC_PRINTPAPERDIMENSIONFLAG );

	return str;
}


void FormPreferences::buildPrinter()
{
	printer_ = build_printer();

	fl_set_input_return(printer_->input_command, FL_RETURN_CHANGED);
	fl_set_input_return(printer_->input_page_range, FL_RETURN_CHANGED);
	fl_set_input_return(printer_->input_copies, FL_RETURN_CHANGED);
	fl_set_input_return(printer_->input_reverse, FL_RETURN_CHANGED);
	fl_set_input_return(printer_->input_to_printer, FL_RETURN_CHANGED);
	fl_set_input_return(printer_->input_file_extension, FL_RETURN_CHANGED);
	fl_set_input_return(printer_->input_spool_command, FL_RETURN_CHANGED);
	fl_set_input_return(printer_->input_paper_type, FL_RETURN_CHANGED);
	fl_set_input_return(printer_->input_even_pages, FL_RETURN_CHANGED);
	fl_set_input_return(printer_->input_odd_pages, FL_RETURN_CHANGED);
	fl_set_input_return(printer_->input_collated, FL_RETURN_CHANGED);
	fl_set_input_return(printer_->input_landscape, FL_RETURN_CHANGED);
	fl_set_input_return(printer_->input_to_file, FL_RETURN_CHANGED);
	fl_set_input_return(printer_->input_extra_options, FL_RETURN_CHANGED);
	fl_set_input_return(printer_->input_spool_prefix, FL_RETURN_CHANGED);
	fl_set_input_return(printer_->input_paper_size, FL_RETURN_CHANGED);
	fl_set_input_return(printer_->input_name, FL_RETURN_CHANGED);

	// set up the feedback mechanism
	setPreHandler( printer_->input_command );
	setPreHandler( printer_->input_page_range );
	setPreHandler( printer_->input_copies );
	setPreHandler( printer_->input_reverse );
	setPreHandler( printer_->input_to_printer );
	setPreHandler( printer_->input_file_extension );
	setPreHandler( printer_->input_spool_command );
	setPreHandler( printer_->input_paper_type );
	setPreHandler( printer_->input_even_pages );
	setPreHandler( printer_->input_odd_pages );
	setPreHandler( printer_->input_collated );
	setPreHandler( printer_->input_landscape );
	setPreHandler( printer_->input_to_file );
	setPreHandler( printer_->input_extra_options );
	setPreHandler( printer_->input_spool_prefix );
	setPreHandler( printer_->input_paper_size );
	setPreHandler( printer_->input_name );
	setPreHandler( printer_->check_adapt_output );
}


void FormPreferences::updatePrinter()
{
	fl_set_button(printer_->check_adapt_output,
		      lyxrc.print_adapt_output);
	fl_set_input(printer_->input_command,
		     lyxrc.print_command.c_str());
	fl_set_input(printer_->input_page_range,
		     lyxrc.print_pagerange_flag.c_str());
	fl_set_input(printer_->input_copies,
		     lyxrc.print_copies_flag.c_str());
	fl_set_input(printer_->input_reverse,
		     lyxrc.print_reverse_flag.c_str());
	fl_set_input(printer_->input_to_printer,
		     lyxrc.print_to_printer.c_str());
	fl_set_input(printer_->input_file_extension,
		     lyxrc.print_file_extension.c_str());
	fl_set_input(printer_->input_spool_command,
		     lyxrc.print_spool_command.c_str());
	fl_set_input(printer_->input_paper_type,
		     lyxrc.print_paper_flag.c_str());
	fl_set_input(printer_->input_even_pages,
		     lyxrc.print_evenpage_flag.c_str());
	fl_set_input(printer_->input_odd_pages,
		     lyxrc.print_oddpage_flag.c_str());
	fl_set_input(printer_->input_collated,
		     lyxrc.print_collcopies_flag.c_str());
	fl_set_input(printer_->input_landscape,
		     lyxrc.print_landscape_flag.c_str());
	fl_set_input(printer_->input_to_file,
		     lyxrc.print_to_file.c_str());
	fl_set_input(printer_->input_extra_options,
		     lyxrc.print_extra_options.c_str());
	fl_set_input(printer_->input_spool_prefix,
		     lyxrc.print_spool_printerprefix.c_str());
	fl_set_input(printer_->input_paper_size,
		     lyxrc.print_paper_dimension_flag.c_str());
	fl_set_input(printer_->input_name,
		     lyxrc.printer.c_str());
}


void FormPreferences::applyScreenFonts() const
{
	bool changed = false;

	string str = fl_get_input(screen_fonts_->input_roman);
	if (lyxrc.roman_font_name != str) {
		changed = true;
		lyxrc.roman_font_name = str;
	}

	str = fl_get_input(screen_fonts_->input_sans);
	if (lyxrc.sans_font_name != str) {
		changed = true;
		lyxrc.sans_font_name = str;
	}

	str = fl_get_input(screen_fonts_->input_typewriter);
	if (lyxrc.typewriter_font_name != str) {
		changed = true;
		lyxrc.typewriter_font_name = str;
	}

	str = fl_get_input(screen_fonts_->input_screen_encoding);
	if (lyxrc.font_norm != str) {
		changed = true;
		lyxrc.font_norm = str;
	}

	bool button = fl_get_button(screen_fonts_->check_scalable);
	if (lyxrc.use_scalable_fonts != button) {
		changed = true;
		lyxrc.use_scalable_fonts = button;
	}

	unsigned int ivalue = static_cast<unsigned int>
		(fl_get_counter_value(screen_fonts_->counter_zoom));
	if (lyxrc.zoom != ivalue) {
		changed = true;
		lyxrc.zoom = ivalue;
	}

	ivalue = static_cast<unsigned int>
		(fl_get_counter_value(screen_fonts_->counter_dpi));
	if (lyxrc.dpi != ivalue) {
		changed = true;
		lyxrc.dpi = ivalue;
	}
	
	double dvalue = strToDbl(fl_get_input(screen_fonts_->input_tiny));
	if (lyxrc.font_sizes[LyXFont::SIZE_TINY] != dvalue) {
		changed = true;
		lyxrc.font_sizes[LyXFont::SIZE_TINY] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(screen_fonts_->input_script));
	if (lyxrc.font_sizes[LyXFont::SIZE_SCRIPT] != dvalue) {
		changed = true;
		lyxrc.font_sizes[LyXFont::SIZE_SCRIPT] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(screen_fonts_->input_footnote));
	if (lyxrc.font_sizes[LyXFont::SIZE_FOOTNOTE] != dvalue) {
		changed = true;
		lyxrc.font_sizes[LyXFont::SIZE_FOOTNOTE] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(screen_fonts_->input_small));
	if (lyxrc.font_sizes[LyXFont::SIZE_SMALL] != dvalue) {
		changed = true;
		lyxrc.font_sizes[LyXFont::SIZE_SMALL] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(screen_fonts_->input_normal));
	if (lyxrc.font_sizes[LyXFont::SIZE_NORMAL] != dvalue) {
		changed = true;
		lyxrc.font_sizes[LyXFont::SIZE_NORMAL] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(screen_fonts_->input_large));
	if (lyxrc.font_sizes[LyXFont::SIZE_LARGE] != dvalue) {
		changed = true;
		lyxrc.font_sizes[LyXFont::SIZE_LARGE] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(screen_fonts_->input_larger));
	if (lyxrc.font_sizes[LyXFont::SIZE_LARGER] != dvalue) {
		changed = true;
		lyxrc.font_sizes[LyXFont::SIZE_LARGER] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(screen_fonts_->input_largest));
	if (lyxrc.font_sizes[LyXFont::SIZE_LARGEST] != dvalue) {
		changed = true;
		lyxrc.font_sizes[LyXFont::SIZE_LARGEST] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(screen_fonts_->input_huge));
	if (lyxrc.font_sizes[LyXFont::SIZE_HUGE] != dvalue) {
		changed = true;
		lyxrc.font_sizes[LyXFont::SIZE_HUGE] = dvalue;
	}

	dvalue = strToDbl(fl_get_input(screen_fonts_->input_huger));
	if (lyxrc.font_sizes[LyXFont::SIZE_HUGER] != dvalue) {
		changed = true;
		lyxrc.font_sizes[LyXFont::SIZE_HUGER] = dvalue;
	}

	if (changed) {
		// Now update the buffers
		// Can anything below here affect the redraw process?
		lv_->getLyXFunc()->Dispatch(LFUN_SCREEN_FONT_UPDATE);
	}
}


void FormPreferences::buildScreenFonts()
{
	screen_fonts_ = build_screen_fonts();

	fl_set_input_return(screen_fonts_->input_roman, FL_RETURN_CHANGED);
	fl_set_input_return(screen_fonts_->input_sans, FL_RETURN_CHANGED);
	fl_set_input_return(screen_fonts_->input_typewriter,
			    FL_RETURN_CHANGED);
	fl_set_input_return(screen_fonts_->input_screen_encoding,
			    FL_RETURN_CHANGED);
	fl_set_counter_return(screen_fonts_->counter_zoom, FL_RETURN_CHANGED);
	fl_set_counter_return(screen_fonts_->counter_dpi, FL_RETURN_CHANGED);
	fl_set_input_return(screen_fonts_->input_tiny, FL_RETURN_CHANGED);
	fl_set_input_return(screen_fonts_->input_script, FL_RETURN_CHANGED);
	fl_set_input_return(screen_fonts_->input_footnote, FL_RETURN_CHANGED);
	fl_set_input_return(screen_fonts_->input_small, FL_RETURN_CHANGED);
	fl_set_input_return(screen_fonts_->input_normal, FL_RETURN_CHANGED);
	fl_set_input_return(screen_fonts_->input_large, FL_RETURN_CHANGED);
	fl_set_input_return(screen_fonts_->input_larger, FL_RETURN_CHANGED);
	fl_set_input_return(screen_fonts_->input_largest, FL_RETURN_CHANGED);
	fl_set_input_return(screen_fonts_->input_huge, FL_RETURN_CHANGED);
	fl_set_input_return(screen_fonts_->input_huger, FL_RETURN_CHANGED);

	fl_set_input_filter(screen_fonts_->input_tiny,
			    fl_unsigned_int_filter);
	fl_set_input_filter(screen_fonts_->input_script,
			    fl_unsigned_int_filter);
	fl_set_input_filter(screen_fonts_->input_footnote,
			    fl_unsigned_int_filter);
	fl_set_input_filter(screen_fonts_->input_small,
			    fl_unsigned_int_filter);
	fl_set_input_filter(screen_fonts_->input_normal,
			    fl_unsigned_int_filter);
	fl_set_input_filter(screen_fonts_->input_large,
			    fl_unsigned_int_filter);
	fl_set_input_filter(screen_fonts_->input_larger,
			    fl_unsigned_int_filter);
	fl_set_input_filter(screen_fonts_->input_largest,
			    fl_unsigned_int_filter);
	fl_set_input_filter(screen_fonts_->input_huge,
			    fl_unsigned_int_filter);
	fl_set_input_filter(screen_fonts_->input_huger,
			    fl_unsigned_int_filter);

	// set up the feedback mechanism
	setPreHandler( screen_fonts_->input_roman );
	setPreHandler( screen_fonts_->input_sans );
	setPreHandler( screen_fonts_->input_typewriter );
	setPreHandler( screen_fonts_->counter_zoom );
	setPreHandler( screen_fonts_->counter_dpi );
	setPreHandler( screen_fonts_->check_scalable );
	setPreHandler( screen_fonts_->input_screen_encoding );
	setPreHandler( screen_fonts_->input_tiny );
	setPreHandler( screen_fonts_->input_script );
	setPreHandler( screen_fonts_->input_footnote );
	setPreHandler( screen_fonts_->input_small );
	setPreHandler( screen_fonts_->input_large );
	setPreHandler( screen_fonts_->input_larger );
	setPreHandler( screen_fonts_->input_largest );
	setPreHandler( screen_fonts_->input_normal );
	setPreHandler( screen_fonts_->input_huge );
	setPreHandler( screen_fonts_->input_huger );
}

	
string const
FormPreferences::feedbackScreenFonts(FL_OBJECT const * const ob ) const
{
	string str;

	if (ob == screen_fonts_->input_roman )
		str = lyxrc.getDescription( LyXRC::RC_SCREEN_FONT_ROMAN );
	else if (ob == screen_fonts_->input_sans )
		str = lyxrc.getDescription( LyXRC::RC_SCREEN_FONT_SANS );
	else if (ob == screen_fonts_->input_typewriter )
		str = lyxrc.getDescription( LyXRC::RC_SCREEN_FONT_TYPEWRITER );
	else if (ob == screen_fonts_->check_scalable )
		str = lyxrc.getDescription( LyXRC::RC_SCREEN_FONT_SCALABLE );
	else if (ob == screen_fonts_->input_screen_encoding )
		str = lyxrc.getDescription( LyXRC::RC_SCREEN_FONT_ENCODING );
	else if (ob == screen_fonts_->counter_zoom )
		str = lyxrc.getDescription( LyXRC::RC_SCREEN_ZOOM );
	else if (ob == screen_fonts_->counter_dpi ) 
		str = lyxrc.getDescription( LyXRC::RC_SCREEN_DPI );
	else if (ob == screen_fonts_->input_tiny
		 || ob == screen_fonts_->input_script
		 || ob == screen_fonts_->input_footnote
		 || ob == screen_fonts_->input_small
		 || ob == screen_fonts_->input_large
		 || ob == screen_fonts_->input_larger
		 || ob == screen_fonts_->input_larger
		 || ob == screen_fonts_->input_largest
		 || ob == screen_fonts_->input_normal
		 || ob == screen_fonts_->input_huge
		 || ob == screen_fonts_->input_huger )
		str = lyxrc.getDescription( LyXRC::RC_SCREEN_FONT_SIZES );

	return str;
}


bool FormPreferences::inputScreenFonts()
{
	bool activate = true;
	string str;

	// Make sure that all fonts all have positive entries
	// Also note that an empty entry is returned as 0.0 by strToDbl
	if (0.0 >= strToDbl(fl_get_input(screen_fonts_->input_tiny))
	    || 0.0 >= strToDbl(fl_get_input(screen_fonts_->input_script))
	    || 0.0 >= strToDbl(fl_get_input(screen_fonts_->input_footnote))
	    || 0.0 >= strToDbl(fl_get_input(screen_fonts_->input_small))
	    || 0.0 >= strToDbl(fl_get_input(screen_fonts_->input_normal))
	    || 0.0 >= strToDbl(fl_get_input(screen_fonts_->input_large))
	    || 0.0 >= strToDbl(fl_get_input(screen_fonts_->input_larger))
	    || 0.0 >= strToDbl(fl_get_input(screen_fonts_->input_largest))
	    || 0.0 >= strToDbl(fl_get_input(screen_fonts_->input_huge))
	    || 0.0 >= strToDbl(fl_get_input(screen_fonts_->input_huger))) {
		activate = false;
		str = N_("WARNING! Fonts must be positive!");

	// Fontsizes -- tiny < script < footnote etc.
	} else if (strToDbl(fl_get_input(screen_fonts_->input_tiny)) >
		   strToDbl(fl_get_input(screen_fonts_->input_script)) ||
		   strToDbl(fl_get_input(screen_fonts_->input_script)) >
		   strToDbl(fl_get_input(screen_fonts_->input_footnote)) ||
		   strToDbl(fl_get_input(screen_fonts_->input_footnote)) >
		   strToDbl(fl_get_input(screen_fonts_->input_small)) ||
		   strToDbl(fl_get_input(screen_fonts_->input_small)) >
		   strToDbl(fl_get_input(screen_fonts_->input_normal)) ||
		   strToDbl(fl_get_input(screen_fonts_->input_normal)) >
		   strToDbl(fl_get_input(screen_fonts_->input_large)) ||
		   strToDbl(fl_get_input(screen_fonts_->input_large)) >
		   strToDbl(fl_get_input(screen_fonts_->input_larger)) ||
		   strToDbl(fl_get_input(screen_fonts_->input_larger)) >
		   strToDbl(fl_get_input(screen_fonts_->input_largest)) ||
		   strToDbl(fl_get_input(screen_fonts_->input_largest)) >
		   strToDbl(fl_get_input(screen_fonts_->input_huge)) ||
		   strToDbl(fl_get_input(screen_fonts_->input_huge)) >
		   strToDbl(fl_get_input(screen_fonts_->input_huger))) {
		activate = false;

		str = N_("WARNING! Fonts must be input in the order tiny > script>\nfootnote > small > normal > large > larger > largest > huge > huger.");
	}

	if (!activate)
		printWarning( str );
	
	return activate;
}


void FormPreferences::updateScreenFonts()
{
	fl_set_input(screen_fonts_->input_roman,
		     lyxrc.roman_font_name.c_str());
	fl_set_input(screen_fonts_->input_sans,
		     lyxrc.sans_font_name.c_str());
	fl_set_input(screen_fonts_->input_typewriter,
		     lyxrc.typewriter_font_name.c_str());
	fl_set_input(screen_fonts_->input_screen_encoding,
		     lyxrc.font_norm.c_str());
	fl_set_button(screen_fonts_->check_scalable,
		      lyxrc.use_scalable_fonts);
	fl_set_counter_value(screen_fonts_->counter_zoom, lyxrc.zoom);
	fl_set_counter_value(screen_fonts_->counter_dpi,  lyxrc.dpi);
	fl_set_input(screen_fonts_->input_tiny,
		     tostr(lyxrc.font_sizes[LyXFont::SIZE_TINY]).c_str());
	fl_set_input(screen_fonts_->input_script,
		     tostr(lyxrc.font_sizes[LyXFont::SIZE_SCRIPT]).c_str());
	fl_set_input(screen_fonts_->input_footnote,
		     tostr(lyxrc.font_sizes[LyXFont::SIZE_FOOTNOTE]).c_str());
	fl_set_input(screen_fonts_->input_small,
		     tostr(lyxrc.font_sizes[LyXFont::SIZE_SMALL]).c_str());
	fl_set_input(screen_fonts_->input_normal,
		     tostr(lyxrc.font_sizes[LyXFont::SIZE_NORMAL]).c_str());
	fl_set_input(screen_fonts_->input_large,
		     tostr(lyxrc.font_sizes[LyXFont::SIZE_LARGE]).c_str());
	fl_set_input(screen_fonts_->input_larger,
		     tostr(lyxrc.font_sizes[LyXFont::SIZE_LARGER]).c_str());
	fl_set_input(screen_fonts_->input_largest,
		     tostr(lyxrc.font_sizes[LyXFont::SIZE_LARGEST]).c_str());
	fl_set_input(screen_fonts_->input_huge,
		     tostr(lyxrc.font_sizes[LyXFont::SIZE_HUGE]).c_str());
	fl_set_input(screen_fonts_->input_huger,
		     tostr(lyxrc.font_sizes[LyXFont::SIZE_HUGER]).c_str());
}


void FormPreferences::applySpellChecker()
{

	string choice = fl_get_choice_text(spellchecker_->choice_spell_command);
	choice = strip( frontStrip( choice ) );
	
	lyxrc.isp_command = choice;

	// If spell checker == "none", all other input set to off.
	if (fl_get_choice(spellchecker_->choice_spell_command) == 1) {
		lyxrc.isp_use_alt_lang = false;
		lyxrc.isp_alt_lang.erase();

		lyxrc.isp_use_esc_chars = false;
		lyxrc.isp_esc_chars.erase();

		lyxrc.isp_use_pers_dict = false;
		lyxrc.isp_pers_dict.erase();

		lyxrc.isp_accept_compound = false;
		lyxrc.isp_use_input_encoding = false;
	} else {
		int button = fl_get_button(spellchecker_->check_alt_lang);
		choice = fl_get_input(spellchecker_->input_alt_lang);
		if (button && choice.empty() ) button = 0;
		if (!button ) choice.erase();

		lyxrc.isp_use_alt_lang = static_cast<bool>(button);
		lyxrc.isp_alt_lang = choice;

		button = fl_get_button(spellchecker_->check_escape_chars);
		choice = fl_get_input(spellchecker_->input_escape_chars);
		if (button && choice.empty() ) button = 0;
		if (!button ) choice.erase();
	
		lyxrc.isp_use_esc_chars = static_cast<bool>(button);
		lyxrc.isp_esc_chars = choice;

		button = fl_get_button(spellchecker_->check_personal_dict);
		choice = fl_get_input(spellchecker_->input_personal_dict);
		if (button && choice.empty() ) button = 0;
		if (!button ) choice.erase();

		lyxrc.isp_use_pers_dict = static_cast<bool>(button);
		lyxrc.isp_pers_dict = choice;

		button = fl_get_button(spellchecker_->check_compound_words);
		lyxrc.isp_accept_compound = static_cast<bool>(button);

		button = fl_get_button(spellchecker_->check_input_enc);
		lyxrc.isp_use_input_encoding = static_cast<bool>(button);
	}

	// Reset view
	updateSpellChecker();
}


void FormPreferences::buildSpellchecker()
{
	spellchecker_ = build_spellchecker();

	fl_addto_choice(spellchecker_->choice_spell_command,
			_(" none | ispell | aspell "));
	fl_set_input_return(spellchecker_->input_alt_lang,
			    FL_RETURN_CHANGED);
	fl_set_input_return(spellchecker_->input_escape_chars,
			    FL_RETURN_CHANGED);
	fl_set_input_return(spellchecker_->input_personal_dict,
			    FL_RETURN_CHANGED);

	// deactivate the browse button because it isn't implemented
	fl_deactivate_object(spellchecker_->button_personal_dict);
	fl_set_object_lcol(spellchecker_->button_personal_dict,
			   FL_INACTIVE);

	// set up the feedback mechanism
	setPreHandler( spellchecker_->choice_spell_command );
	setPreHandler( spellchecker_->check_alt_lang );
	setPreHandler( spellchecker_->input_alt_lang );
	setPreHandler( spellchecker_->check_escape_chars );
	setPreHandler( spellchecker_->input_escape_chars );
	setPreHandler( spellchecker_->check_personal_dict );
	setPreHandler( spellchecker_->input_personal_dict );
	setPreHandler( spellchecker_->button_personal_dict );
	setPreHandler( spellchecker_->check_compound_words );
	setPreHandler( spellchecker_->check_input_enc );
}


string const
FormPreferences::feedbackSpellChecker(FL_OBJECT const * const ob) const
{
	string str;

	if (ob == spellchecker_->choice_spell_command )
		str = lyxrc.getDescription( LyXRC::RC_SPELL_COMMAND );
	else if (ob == spellchecker_->check_alt_lang )
		str = lyxrc.getDescription( LyXRC::RC_USE_ALT_LANG );
	else if (ob == spellchecker_->input_alt_lang )
		str = lyxrc.getDescription( LyXRC::RC_ALT_LANG );
	else if (ob == spellchecker_->check_escape_chars )
		str = lyxrc.getDescription( LyXRC::RC_USE_ESC_CHARS );
	else if (ob == spellchecker_->input_escape_chars )
		str = lyxrc.getDescription( LyXRC::RC_ESC_CHARS );
	else if (ob == spellchecker_->check_personal_dict )
		str = lyxrc.getDescription( LyXRC::RC_USE_PERS_DICT );
	else if (ob == spellchecker_->input_personal_dict )
		str = lyxrc.getDescription( LyXRC::RC_PERS_DICT );
	else if (ob == spellchecker_->check_compound_words )
		str = lyxrc.getDescription( LyXRC::RC_ACCEPT_COMPOUND );
	else if (ob == spellchecker_->check_input_enc )
		str = lyxrc.getDescription( LyXRC::RC_USE_INP_ENC );

	return str;
}


bool FormPreferences::inputSpellChecker( FL_OBJECT const * const ob )
{
	// !ob if function is called from updateSpellChecker() to de/activate
	// objects,
	// otherwise the function is called by an xforms CB via input().

	// If spell checker == "none", disable all input.
	if (!ob || ob == spellchecker_->choice_spell_command) {
		if (fl_get_choice(spellchecker_->choice_spell_command) == 1) {
			fl_deactivate_object( spellchecker_->check_alt_lang );
			fl_deactivate_object( spellchecker_->input_alt_lang );
			fl_deactivate_object( spellchecker_->check_escape_chars );
			fl_deactivate_object( spellchecker_->input_escape_chars );
			fl_deactivate_object( spellchecker_->check_personal_dict );
			fl_deactivate_object( spellchecker_->input_personal_dict );
			fl_deactivate_object( spellchecker_->check_compound_words );
			fl_deactivate_object( spellchecker_->check_input_enc );
			return true;
		} else {
			fl_activate_object( spellchecker_->check_alt_lang );
			fl_activate_object( spellchecker_->check_escape_chars );
			fl_activate_object( spellchecker_->check_personal_dict );
			fl_activate_object( spellchecker_->check_compound_words );
			fl_activate_object( spellchecker_->check_input_enc );
		}
	}

	if (!ob || ob == spellchecker_->check_alt_lang) {
		if (fl_get_button(spellchecker_->check_alt_lang)) {
			fl_activate_object(spellchecker_->input_alt_lang);
			fl_set_object_lcol(spellchecker_->input_alt_lang,
					   FL_BLACK);
		} else {
			fl_deactivate_object(spellchecker_->input_alt_lang);
			fl_set_object_lcol(spellchecker_->input_alt_lang,
					   FL_INACTIVE);
		}
	}

	if (!ob || ob == spellchecker_->check_escape_chars) {
		if (fl_get_button(spellchecker_->check_escape_chars)) {
			fl_activate_object(spellchecker_->input_escape_chars);
			fl_set_object_lcol(spellchecker_->input_escape_chars,
					   FL_BLACK);
		} else {
			fl_deactivate_object(spellchecker_->input_escape_chars);
			fl_set_object_lcol(spellchecker_->input_escape_chars,
					   FL_INACTIVE);
		}
	}

	if (!ob || ob == spellchecker_->check_personal_dict) {
		if (fl_get_button(spellchecker_->check_personal_dict)) {
			fl_activate_object(spellchecker_->input_personal_dict);
			fl_set_object_lcol(spellchecker_->input_personal_dict,
					   FL_BLACK);
		} else {
			fl_deactivate_object(spellchecker_->input_personal_dict);
			fl_set_object_lcol(spellchecker_->input_personal_dict,
					   FL_INACTIVE);
		}
	}
	
	return true; // All input is valid!
}


void FormPreferences::updateSpellChecker()
{
	int choice = 1;
	if (lyxrc.isp_command == "none" )
		choice = 1;
	else if (lyxrc.isp_command == "ispell" )
		choice = 2;
	else if (lyxrc.isp_command == "aspell" )
		choice = 3;
	fl_set_choice(spellchecker_->choice_spell_command, choice);
	
	string str;
	if (lyxrc.isp_use_alt_lang ) str = lyxrc.isp_alt_lang;

	fl_set_button(spellchecker_->check_alt_lang,
		      lyxrc.isp_use_alt_lang);
	fl_set_input(spellchecker_->input_alt_lang, str.c_str());
	
	str.erase();
	if (lyxrc.isp_use_esc_chars ) str = lyxrc.isp_esc_chars;

	fl_set_button(spellchecker_->check_escape_chars,
		      lyxrc.isp_use_esc_chars);
	fl_set_input(spellchecker_->input_escape_chars, str.c_str());

	str.erase();
	if (lyxrc.isp_use_pers_dict ) str = lyxrc.isp_pers_dict;

	fl_set_button(spellchecker_->check_personal_dict,
		      lyxrc.isp_use_pers_dict);
	fl_set_input(spellchecker_->input_personal_dict, str.c_str());

	fl_set_button(spellchecker_->check_compound_words,
		      lyxrc.isp_accept_compound);
	fl_set_button(spellchecker_->check_input_enc,
		      lyxrc.isp_use_input_encoding);

	// Activate/Deactivate the input fields dependent on the state of the
	// buttons.
	inputSpellChecker( 0 );
}


bool FormPreferences::WriteableDir( string const & name )
{
	bool success = true;
	string str;

	if (!AbsolutePath(name)) {
		success = false;
		str = N_("WARNING! The absolute path is required.");
	}

	FileInfo const tp(name);
	if (success && !tp.isDir()) {
		success = false;
		str = N_("WARNING! Directory does not exist.");
	}

	if (success && !tp.writable()) {
		success = false;
		str = N_("WARNING! Cannot write to this directory.");
	}

	if (!success)
		printWarning( str );
	
	return success;
}


bool FormPreferences::ReadableDir( string const & name )
{
	bool success = true;
	string str;

	if (!AbsolutePath(name)) {
		success = false;
		str = N_("WARNING! The absolute path is required.");
	}

	FileInfo const tp(name);
	if (success && !tp.isDir()) {
		success = false;
		str = N_("WARNING! Directory does not exist.");
	}

	if (success && !tp.readable()) {
		success = false;
		str = N_("WARNING! Cannot read this directory.");
	}

	if (!success)
		printWarning( str );

	return success;
}


bool FormPreferences::WriteableFile(string const & name,
				    string const & suffix)
{
	// A writeable file is either:
	// * An existing file to which we have write access, or
	// * A file that doesn't yet exist but that would exist in a writeable
	//   directory.

	bool success = true;
	string str;

	if (name.empty()) {
		success = false;
		str = N_("WARNING! No file input.");
	}

	string const dir = OnlyPath(name);
	if (success && !AbsolutePath(dir)) {
		success = false;
		str = N_("WARNING! The absolute path is required.");
	}

#if 0
	// This is not a nice way to use FileInfo (Lgb)
	FileInfo d;
	
	{
		FileInfo d1(dir);
		FileInfo d2(name);
		if (d2.isDir() )
			d = d2;
		else
			d = d1;
	}
#else
	// This should be equivalent (Lgb)
	FileInfo d(name);
	if (!d.isDir()) {
		d.newFile(dir);
	}
#endif
	if (success && !d.isDir()) {
		success = false;
		str = N_("WARNING! Directory does not exist.");
	}
	
	if (success && !d.writable()) {
		success = false;
		str = N_("WARNING! Cannot write to this directory.");
	}

	FileInfo f(name+suffix);
	if (success && (dir == name || f.isDir())) {
		success = false;
		str = N_("WARNING! A file is required, not a directory.");
	}

	if (success && (f.exist() && !f.writable())) {
		success = false;
		str = N_("WARNING! Cannot write to this file.");
	}
	
	if (!success)
		printWarning( str );

	return success;
}


void FormPreferences::printWarning( string const & warning )
{
	warningPosted = true;

	string str = formatted( warning, dialog_->text_warning->w-10,
				FL_SMALL_SIZE, FL_NORMAL_STYLE );

	fl_set_object_label(dialog_->text_warning, str.c_str());
	fl_set_object_lsize(dialog_->text_warning, FL_SMALL_SIZE);
}


void FormPreferences::ComboLanguageCB(int, void * v, Combox * combox)
{
    FormPreferences * pre = static_cast<FormPreferences*>(v);
    // This is safe, as nothing is done to the pointer, other than
    // to use its address in a block-if statement.
    pre->bc_.valid( pre->input( reinterpret_cast<FL_OBJECT *>(combox), 0 ));
}


// C function wrapper, required by xforms.
C_PREPOSTHANDLER(FormPreferences, FeedbackCB)

int FormPreferences::FeedbackCB(FL_OBJECT * ob, int event,
				FL_Coord, FL_Coord, int, void *)
{
	// Note that the return value is important in the pre-emptive handler.
	// Don't return anything other than 0.

	Assert(ob);
	// Don't Assert this one, as it can happen quite reasonably when things
	// are being deleted in the d-tor.
	//Assert(ob->form);
	if( !ob->form ) return 0;

	FormPreferences * pre =
		static_cast<FormPreferences*>(ob->form->u_vdata);
	pre->Feedback(ob, event);
	return 0;
}


// preemptive handler for feedback messages
void FormPreferences::Feedback(FL_OBJECT * ob, int event)
{
	Assert(ob);

	switch( event ) {
	case FL_ENTER:
		warningPosted = false;
		feedback( ob );
		break;

	case FL_LEAVE:
		if( !warningPosted )
			fl_set_object_label( dialog_->text_warning, "" );
		break;

	default:
		break;
	}
}


void FormPreferences::setPreHandler(FL_OBJECT * ob) const
{
	Assert(ob);
	fl_set_object_prehandler( ob, C_FormPreferencesFeedbackCB );
}
