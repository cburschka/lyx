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
#include "filedlg.h" // for LyXFileDlg
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

using std::endl;
using std::find;
using std::find_if;
using std::pair;
using std::max;
using std::sort;
using std::vector;

extern string fmt(char const * fmtstr ...);
extern string system_lyxdir;
extern string user_lyxdir;
extern Languages languages;

static string const colorFile("/usr/lib/X11/rgb.txt");
static Formats    local_formats;
static Converters local_converters;

vector<X11Color>   FormPreferences::Colors::colorDB;
vector<XformColor> FormPreferences::Colors::xformColorDB;

FormPreferences::FormPreferences(LyXView * lv, Dialogs * d)
	: FormBaseBI(lv, d, _("Preferences"), new PreferencesPolicy),
	  dialog_(0),
	  converters_tab_(0), inputs_tab_(0), look_n_feel_tab_(0),
	  outputs_tab_(0),  usage_tab_(0),
	  warningPosted(false),
	  colors_(*this), converters_(*this), inputs_misc_(*this),
	  formats_(*this), interface_(*this), language_(*this), 
	  lnf_misc_(*this), outputs_misc_(*this), paths_(*this),
	  printer_(*this), screen_fonts_(*this), spellchecker_(*this)
{
	// let the dialog be shown
	// This is a permanent connection so we won't bother
	// storing a copy because we won't be disconnecting.
	d->showPreferences.connect(slot(this, &FormPreferences::show));
}


FormPreferences::~FormPreferences()
{
	delete converters_tab_;
	delete inputs_tab_;
	delete look_n_feel_tab_;
	delete outputs_tab_;
	delete usage_tab_;

	delete dialog_;
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
		form3 = fl_get_active_folder(converters_tab_->tabfolder_outer);

	else if (form2 == look_n_feel_tab_->form)
		form3 = fl_get_active_folder(look_n_feel_tab_->tabfolder_outer);

	else if (form2 == inputs_tab_->form)
		form3 = fl_get_active_folder(inputs_tab_->tabfolder_outer);

	else if (form2 == outputs_tab_->form)
		form3 = fl_get_active_folder(outputs_tab_->tabfolder_outer);

	else if (form2 == usage_tab_->form)
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

	if (colors_.modifiedXformPrefs) {
		string filename = AddName(user_lyxdir, "preferences.xform");
		colors_.modifiedXformPrefs = ! XformColor::write(filename);
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
	spellchecker_.build();

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
			   screen_fonts_.dialog()->form);
	fl_addto_tabfolder(look_n_feel_tab_->tabfolder_outer,
			   _("Interface"),
			   interface_.dialog()->form);
	fl_addto_tabfolder(look_n_feel_tab_->tabfolder_outer,
			   _("Colors"),
			   colors_.dialog()->form);
	fl_addto_tabfolder(look_n_feel_tab_->tabfolder_outer,
			   _("Misc"),
			   lnf_misc_.dialog()->form);

	// then build converters
	fl_addto_tabfolder(converters_tab_->tabfolder_outer,
			   _("Formats"),
			   formats_.dialog()->form);
	fl_addto_tabfolder(converters_tab_->tabfolder_outer,
			   _("Converters"),
			   converters_.dialog()->form);

	// then build inputs
	// Paths should probably go in a few outer_tab called Files
	fl_addto_tabfolder(inputs_tab_->tabfolder_outer,
			   _("Paths"),
			   paths_.dialog()->form);
	fl_addto_tabfolder(inputs_tab_->tabfolder_outer,
			   _("Misc"),
			   inputs_misc_.dialog()->form);

	// then building outputs
	fl_addto_tabfolder(outputs_tab_->tabfolder_outer,
			   _("Printer"),
			   printer_.dialog()->form);
	fl_addto_tabfolder(outputs_tab_->tabfolder_outer,
			   _("Misc"),
			   outputs_misc_.dialog()->form);

	// then building usage
	fl_addto_tabfolder(usage_tab_->tabfolder_outer,
			   _("Spell checker"),
			   spellchecker_.dialog()->form);
	fl_addto_tabfolder(usage_tab_->tabfolder_outer,
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
	spellchecker_.apply();
}


void FormPreferences::feedback( FL_OBJECT * ob )
{
	Assert(ob);

	string str;

	if (ob->form->fdui == colors_.dialog()) {
		str = colors_.feedback( ob );
	} else if (ob->form->fdui == converters_.dialog()) {
		str = converters_.feedback( ob );
	} else if (ob->form->fdui == formats_.dialog()) {
		str = formats_.feedback( ob );
	} else if (ob->form->fdui == inputs_misc_.dialog()) {
		str = inputs_misc_.feedback( ob );
	} else if (ob->form->fdui == interface_.dialog()) {
		str = interface_.feedback( ob );
	} else if (ob->form->fdui == language_.dialog()) {
		str = language_.feedback( ob );
	} else if (ob->form->fdui == lnf_misc_.dialog()) {
		str = lnf_misc_.feedback( ob );
	} else if (ob->form->fdui == outputs_misc_.dialog()) {
		str = outputs_misc_.feedback( ob );
	} else if (ob->form->fdui == paths_.dialog()) {
		str = paths_.feedback( ob );
	} else if (ob->form->fdui == printer_.dialog()) {
		str = printer_.feedback( ob );
	} else if (ob->form->fdui == screen_fonts_.dialog()) {
		str = screen_fonts_.feedback( ob );
	} else if (ob->form->fdui == spellchecker_.dialog()) {
		str = spellchecker_.feedback( ob );
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

	if (ob->form->fdui == colors_.dialog())
		return colors_.input(ob);
	else if (ob->form->fdui == converters_.dialog())
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
	else if (ob->form->fdui == spellchecker_.dialog())
		return spellchecker_.input(ob);

	return true;
}


void FormPreferences::update()
{
	if (!dialog_) return;
    
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
	spellchecker_.update();
}


FormPreferences::Colors::~Colors()
{
	delete dialog_;
}


void FormPreferences::Colors::apply()
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
				AdjustVal( FL_INACTIVE, FL_BLACK, 0.5 );
			}

			if( modifiedBackground && (*cit).colorID == FL_COL1 ) {
				AdjustVal( FL_MCOL,      FL_COL1, 0.1 );
				AdjustVal( FL_TOP_BCOL,  FL_COL1, 0.1 );
				AdjustVal( FL_LEFT_BCOL, FL_COL1, 0.1 );

				AdjustVal( FL_RIGHT_BCOL,  FL_COL1, -0.5);
				AdjustVal( FL_BOTTOM_BCOL, FL_COL1, -0.5);
			}		
		}
		Dialogs::redrawGUI();
	}
}


void FormPreferences::Colors::build()
{
	dialog_ = parent_.build_colors();

	FL_OBJECT * obj = dialog_->valslider_red;
	fl_set_slider_bounds(obj, 0, 255);
	fl_set_slider_precision(obj, 0);
	fl_set_slider_return(obj, FL_RETURN_END_CHANGED);
	
	obj = dialog_->valslider_green;
	fl_set_slider_bounds(obj, 0, 255);
	fl_set_slider_precision(obj, 0);
	fl_set_slider_return(obj, FL_RETURN_END_CHANGED);
	
	obj = dialog_->valslider_blue;
	fl_set_slider_bounds(obj, 0, 255);
	fl_set_slider_precision(obj, 0);
	fl_set_slider_return(obj, FL_RETURN_END_CHANGED);

	fl_set_object_color(dialog_->button_color,
			    FL_FREE_COL4, FL_FREE_COL4);
	
	fl_set_input_return(dialog_->input_name, FL_RETURN_CHANGED);

	if (LoadBrowserX11(colorFile) )
		fl_set_input(dialog_->input_name, colorFile.c_str());
	else
		fl_set_input(dialog_->input_name, N_("No file found"));

	LoadBrowserLyX();

	// set up the feedback mechanism
	parent_.setPreHandler( dialog_->browser_x11 );
	parent_.setPreHandler( dialog_->input_name );
	parent_.setPreHandler( dialog_->button_browse );
	parent_.setPreHandler( dialog_->button_color );
	parent_.setPreHandler( dialog_->valslider_red );
	parent_.setPreHandler( dialog_->valslider_green );
	parent_.setPreHandler( dialog_->valslider_blue );
	parent_.setPreHandler( dialog_->browser_lyx_objs );
	parent_.setPreHandler( dialog_->button_modify );
	parent_.setPreHandler( dialog_->button_modify );
}


string const
FormPreferences::Colors::feedback(FL_OBJECT const * const ob) const
{
	string str;

	if (ob == dialog_->browser_x11 ) {
		str = N_("The colors listed in the X11 database.");
	} else if (ob == dialog_->browser_lyx_objs ) {
		str = N_("LyX objects that can be assigned a color.");
	} else if (ob == dialog_->input_name ) {
		str = N_("The file containing the X11 color database.");
	} else if (ob == dialog_->button_color ) {
		str = N_("You will only be able to modify the LyX object if the sliders and X11 browser agree. Force this by clicking on the highlighted browser name.");
	} else if (ob == dialog_->valslider_red
		   || ob == dialog_->valslider_green
		   || ob == dialog_->valslider_blue ) {
		str = N_("Find a new color. The name highlighted in the X11 database is the closest match to this.");
	} else if (ob == dialog_->button_modify ) {
		str = N_("Modify the color of the LyX object.");
	}

	return str;
}


bool FormPreferences::Colors::input( FL_OBJECT const * const ob )
{
	if (ob == dialog_->browser_x11) {
		return BrowserX11();

	} else if (ob == dialog_->valslider_red
		   || ob == dialog_->valslider_green
		   || ob == dialog_->valslider_blue) {
		return RGB();

	} else if (ob == dialog_->input_name) {
		return Database();

	} else if (ob == dialog_->button_browse) {
		return parent_.browse( dialog_->input_name,
				      _("X11 color database"), "*.txt" );

	} else if (ob == dialog_->browser_lyx_objs) {
		return BrowserLyX();
		
	} else if (ob == dialog_->button_modify) {
		return Modify();
	}
	
	return true;
}


void FormPreferences::Colors::update()
{}


void FormPreferences::Colors::AdjustVal( int colAdjust, int colParent,
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


bool FormPreferences::Colors::BrowserLyX() const
{
	int i = fl_get_browser( dialog_->browser_lyx_objs );
	if (i < 1)
		return true;

	string name = fl_get_browser_line( dialog_->browser_lyx_objs, i );

	vector<XformColor>::const_iterator cit =
		find_if(xformColorDB.begin(), xformColorDB.end(),
			compare_memfun(&XformColor::getname, name));

	if( cit != xformColorDB.end() ) {
		fl_freeze_form( dialog_->form );

		fl_set_slider_value( dialog_->valslider_red,   (*cit).col.r );
		fl_set_slider_value( dialog_->valslider_green, (*cit).col.g );
		fl_set_slider_value( dialog_->valslider_blue,  (*cit).col.b );
		RGB();

		fl_unfreeze_form( dialog_->form );
	}

	fl_deactivate_object( dialog_->button_modify );
	fl_set_object_lcol( dialog_->button_modify, FL_INACTIVE );

	return true;
}


bool FormPreferences::Colors::BrowserX11() const
{
	int i = fl_get_browser(dialog_->browser_x11);
	if (i < 1)
		return true;

	fl_freeze_form( dialog_->form );

	RGBColor col = colorDB[i-1].second;
    
	fl_mapcolor( FL_FREE_COL4 + i, col.r, col.g, col.b );
	fl_mapcolor( FL_FREE_COL4, col.r, col.g, col.b );
	fl_set_slider_value( dialog_->valslider_red,   col.r );
	fl_set_slider_value( dialog_->valslider_green, col.g );
	fl_set_slider_value( dialog_->valslider_blue,  col.b );
	fl_redraw_object( dialog_->button_color );

	// Is it valid to activate the "Modify" button?
	int line = fl_get_browser(dialog_->browser_lyx_objs);
	bool isSelected = ( line > 0 );
	if( isSelected )
		if( line > fl_get_browser_maxline(dialog_->browser_lyx_objs) )
			isSelected = false;	

	if( isSelected && colorDB[i-1].second == col ) {
		fl_activate_object( dialog_->button_modify );
		fl_set_object_lcol( dialog_->button_modify, FL_BLACK );
	}
	
	fl_unfreeze_form( dialog_->form );

	return true;
}


bool FormPreferences::Colors::Database()
{
	string file = fl_get_input(dialog_->input_name);
	if( !parent_.ReadableFile(file) )
		return false;

	if (LoadBrowserX11(file) )
		return true;

	return false;
}


void FormPreferences::Colors::LoadBrowserLyX()
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
	// xcol.name = "GUI active tab";
	// xcol.colorID = FL_LIGHTER_COL1;
	// fl_getmcolor(FL_LIGHTER_COL1, &xcol.col.r, &xcol.col.g, &xcol.col.b);
	// 
	// xformColorDB.push_back( xcol );

	xcol.name = "GUI push button";
	xcol.colorID = FL_YELLOW;
	fl_getmcolor(FL_YELLOW, &xcol.col.r, &xcol.col.g, &xcol.col.b);

	xformColorDB.push_back( xcol );

	FL_OBJECT * colbr = dialog_->browser_lyx_objs;
	fl_clear_browser( colbr );
	for( vector<XformColor>::const_iterator cit = xformColorDB.begin();
	     cit != xformColorDB.end(); ++cit ) {
		fl_addto_browser(colbr, (*cit).name.c_str() );
	}

	// Now load the LyX LColors

	// just to be safe...
	fl_deselect_browser( dialog_->browser_lyx_objs );
	fl_deactivate_object( dialog_->button_modify );
	fl_set_object_lcol( dialog_->button_modify, FL_INACTIVE );
}


bool FormPreferences::Colors::LoadBrowserX11(string const & filename) const
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
	
	FL_OBJECT * colbr = dialog_->browser_x11;
	fl_freeze_form(dialog_->form);
	fl_clear_browser( colbr );

	vector<string>::const_iterator sit = names.begin();
	for (vector<RGBColor>::const_iterator iit = cols.begin();
	     iit != cols.end(); ++iit, ++sit) {
		colorDB.push_back( X11Color(*sit, *iit) );
		fl_addto_browser(colbr, (*sit).c_str());
	}
	
	fl_set_browser_topline(colbr, 1);
	fl_select_browser_line(colbr, 1);
	BrowserX11();
	fl_unfreeze_form(dialog_->form);
	
	return true;
}


bool FormPreferences::Colors::Modify() const
{
	int i = fl_get_browser( dialog_->browser_lyx_objs );
	if (i < 1)
		return true;

	string name = fl_get_browser_line( dialog_->browser_lyx_objs, i );

	vector<XformColor>::iterator it = // non-const; it's modified below
		find_if(xformColorDB.begin(), xformColorDB.end(),
			compare_memfun(&XformColor::getname, name));

	if( it == xformColorDB.end() )
		return true;

	int j = fl_get_browser( dialog_->browser_x11 );
	if (j < 1)
		return true;

	(*it).col.r = colorDB[j-1].second.r;
	(*it).col.g = colorDB[j-1].second.g;
	(*it).col.b = colorDB[j-1].second.b;

	fl_deselect_browser(dialog_->browser_x11);
	fl_deselect_browser(dialog_->browser_lyx_objs);
	fl_deactivate_object( dialog_->button_modify );
	fl_set_object_lcol( dialog_->button_modify, FL_INACTIVE );

	return true;
}

bool FormPreferences::Colors::RGB() const
{
	fl_freeze_form(dialog_->form);

	RGBColor col;
	col.r = int(fl_get_slider_value(dialog_->valslider_red));
	col.g = int(fl_get_slider_value(dialog_->valslider_green));
	col.b = int(fl_get_slider_value(dialog_->valslider_blue));
    
	fl_mapcolor(FL_FREE_COL4, col.r, col.g, col.b);
	fl_redraw_object(dialog_->button_color);

	int const i = SearchEntry( col );
	// change topline only if necessary
	// int top = fl_get_browser_topline(dialog_->browser_x11);
	// if (i < top || i > (top+15))
	fl_set_browser_topline(dialog_->browser_x11, i - 5);
	fl_select_browser_line(dialog_->browser_x11, i + 1);

	// Only activate the "Modify" button if the browser and slider colors
	// are the same AND if a LyX object is selected.
	int line = fl_get_browser(dialog_->browser_lyx_objs);
	bool isSelected = ( line > 0 );
	if( isSelected )
		if( line > fl_get_browser_maxline(dialog_->browser_lyx_objs) )
			isSelected = false;	

	if( isSelected && colorDB[i].second == col ) {
		fl_activate_object( dialog_->button_modify );
		fl_set_object_lcol( dialog_->button_modify, FL_BLACK );
	} else {
		fl_deactivate_object( dialog_->button_modify );
		fl_set_object_lcol( dialog_->button_modify, FL_INACTIVE );
	}

	fl_unfreeze_form(dialog_->form);

	return true;
}


int FormPreferences::Colors::SearchEntry(RGBColor const & col) const
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


FormPreferences::Converters::~Converters()
{
	delete dialog_;
}


void FormPreferences::Converters::apply() const
{
	converters = local_converters;
	converters.Update(formats);
	converters.BuildGraph();
}


void FormPreferences::Converters::build()
{
	dialog_ = parent_.build_converters();

	fl_set_input_return(dialog_->input_converter, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_flags, FL_RETURN_CHANGED);

	// set up the feedback mechanism
	parent_.setPreHandler( dialog_->browser_all );
	parent_.setPreHandler( dialog_->button_delete );
	parent_.setPreHandler( dialog_->button_add );
	parent_.setPreHandler( dialog_->input_converter );
	parent_.setPreHandler( dialog_->choice_from );
	parent_.setPreHandler( dialog_->choice_to );
	parent_.setPreHandler( dialog_->input_flags );
}


string const
FormPreferences::Converters::feedback( FL_OBJECT const * const ob ) const
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
		str = N_("Remove the current converter from the list of available converters.");
	} else if (ob == dialog_->button_add) {
		str = N_("Add the current converter to the list of available converters.");
	}

	return str;
}


bool FormPreferences::Converters::input( FL_OBJECT const * const ob )
{
	if( ob == dialog_->browser_all ) {
		return Browser();

	} else if (ob == dialog_->choice_from
		   || ob == dialog_->choice_to
		   || ob == dialog_->input_converter
		   || ob == dialog_->input_flags ) {
		return Input();

	} else if( ob == dialog_->button_add ) {
		return Add();

	} else if( ob == dialog_->button_delete ) {
		return Delete();
	}

	return true;
}


void FormPreferences::Converters::update()
{
	local_converters = converters;
	local_converters.Update(local_formats);
	UpdateBrowser();
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


void FormPreferences::Converters::UpdateBrowser()
{
	local_converters.Sort();

	fl_freeze_form(dialog_->form);
	fl_clear_browser(dialog_->browser_all);
	for (::Converters::const_iterator cit = local_converters.begin();
	     cit != local_converters.end(); ++cit) {
		string name = (*cit).From->prettyname() + " -> "
			+ (*cit).To->prettyname();
		fl_addto_browser(dialog_->browser_all, name.c_str());
	}
	Input();
	fl_unfreeze_form(dialog_->form);
}


bool FormPreferences::Converters::Add()
{
	string from = GetFrom();
	string to = GetTo();
	string command = fl_get_input(dialog_->input_converter);
	string flags = fl_get_input(dialog_->input_flags);

	Converter const * old = local_converters.GetConverter(from, to);
	local_converters.Add(from, to, command, flags);
	if (!old) {
		local_converters.UpdateLast(local_formats);
		UpdateBrowser();
	}
	fl_deactivate_object(dialog_->button_add);
	fl_set_object_lcol(dialog_->button_add, FL_INACTIVE);

	return true;
}


bool FormPreferences::Converters::Browser() 
{
	int i = fl_get_browser( dialog_->browser_all );
	if( i <= 0 ) return false;

	fl_freeze_form( dialog_->form );

	Converter const & c = local_converters.Get(i-1);
	int j = local_formats.GetNumber(c.from);
	if (j >= 0)
		fl_set_choice(dialog_->choice_from, j+1);

	j = local_formats.GetNumber(c.to);
	if (j >= 0)
		fl_set_choice(dialog_->choice_to, j+1);

	fl_set_input(dialog_->input_converter, c.command.c_str());
	fl_set_input(dialog_->input_flags, c.flags.c_str());

	fl_set_object_label( dialog_->button_add, idex(_("Modify|#M")) );
	fl_set_button_shortcut( dialog_->button_add,
				scex(_("Modify|#M")), 1 );

	fl_deactivate_object( dialog_->button_add );
	fl_set_object_lcol( dialog_->button_add, FL_INACTIVE );

	fl_activate_object( dialog_->button_delete );
	fl_set_object_lcol( dialog_->button_delete, FL_BLACK );
				
	fl_unfreeze_form( dialog_->form);
	return false;
}


bool FormPreferences::Converters::Delete()
{
	string from = GetFrom();
	string to = GetTo();

	local_converters.Delete(from, to);
	UpdateBrowser();
	return true;
}


bool FormPreferences::Converters::Input()
{
	string from = GetFrom();
	string to = GetTo();
	int sel = local_converters.GetNumber(from, to);
	
	fl_freeze_form(dialog_->form);

	if (sel < 0) {
		fl_set_object_label( dialog_->button_add,
				     idex(_("Add|#A")) );
		fl_set_button_shortcut( dialog_->button_add,
					scex(_("Add|#A")), 1);

		fl_deselect_browser(dialog_->browser_all);

		fl_deactivate_object(dialog_->button_delete);
		fl_set_object_lcol(dialog_->button_delete, FL_INACTIVE);

	} else {
		fl_set_object_label( dialog_->button_add,
				     idex(_("Modify|#M")) );
		fl_set_button_shortcut( dialog_->button_add,
					scex(_("Modify|#M")), 1);
		
		int top = max(sel-5, 0);
		fl_set_browser_topline(dialog_->browser_all, top);
		fl_select_browser_line(dialog_->browser_all, sel+1);
		
		fl_activate_object(dialog_->button_delete);
		fl_set_object_lcol(dialog_->button_delete, FL_BLACK);
	}

	string command = fl_get_input(dialog_->input_converter);
	if (command.empty() || from == to) {
		fl_deactivate_object(dialog_->button_add);
		fl_set_object_lcol(dialog_->button_add, FL_INACTIVE);
	} else {
		fl_activate_object(dialog_->button_add);
		fl_set_object_lcol(dialog_->button_add, FL_BLACK);
	}

	fl_unfreeze_form(dialog_->form);
	return false;
}


string const FormPreferences::Converters::GetFrom() const
{
	int i = fl_get_choice(dialog_->choice_from);
	if (i > 0)
		return local_formats.Get(i-1).name();
	else {
		lyxerr << "FormPreferences::Converters::GetFrom: No choice!" << endl;
		return "???";
	}
}


string const FormPreferences::Converters::GetTo() const
{
	int i = fl_get_choice(dialog_->choice_to);
	if (i > 0)
		return local_formats.Get(i-1).name();
	else {
		lyxerr << "FormPreferences::Converters::GetTo: No choice!" << endl;
		return "???";
	}
}


void FormPreferences::Converters::UpdateChoices() const
{
	string choice;
	for (::Formats::const_iterator cit = local_formats.begin();
	     cit != local_formats.end(); ++cit ) {
		if (!choice.empty())
			choice += " | ";
		else
			choice += " ";
		choice += (*cit).prettyname();
	}
	choice += " ";

	fl_clear_choice( dialog_->choice_from );
	fl_addto_choice( dialog_->choice_from, choice.c_str() );

	fl_clear_choice( dialog_->choice_to );
	fl_addto_choice( dialog_->choice_to, choice.c_str() );
}


FormPreferences::Formats::~Formats()
{
	delete dialog_;
}


void FormPreferences::Formats::apply() const
{
	formats = local_formats;
}


void FormPreferences::Formats::build()
{
	dialog_ = parent_.build_formats();

	fl_set_input_return(dialog_->input_format, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_viewer, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_shrtcut, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_gui_name, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_extension, FL_RETURN_CHANGED);

	fl_set_input_filter(dialog_->input_format, fl_lowercase_filter);

	// set up the feedback mechanism
	parent_.setPreHandler( dialog_->browser_all );
	parent_.setPreHandler( dialog_->input_format );
	parent_.setPreHandler( dialog_->input_gui_name );
	parent_.setPreHandler( dialog_->button_delete );
	parent_.setPreHandler( dialog_->button_add );
	parent_.setPreHandler( dialog_->input_extension );
	parent_.setPreHandler( dialog_->input_viewer );
	parent_.setPreHandler( dialog_->input_shrtcut );
}


string const
FormPreferences::Formats::feedback( FL_OBJECT const * const ob ) const
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
		str = N_("Remove the current format from the list of available formats.");
	} else if (ob == dialog_->button_add) {
		str = N_("Add the current format to the list of available formats.");
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
		   || ob == dialog_->input_viewer ) {
		return Input();

	} else if (ob == dialog_->button_add) {
		return Add();

	} else if (ob == dialog_->button_delete) {
		return Delete();
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
	local_formats.Sort();

	fl_freeze_form(dialog_->form);
	fl_deselect_browser(dialog_->browser_all);
	fl_clear_browser(dialog_->browser_all);
	for (::Formats::const_iterator cit = local_formats.begin();
	     cit != local_formats.end(); ++cit)
		fl_addto_browser(dialog_->browser_all,
				 (*cit).prettyname().c_str());

	Input();
	fl_unfreeze_form(dialog_->form);

	// Mustn't forget to update the Formats available to the converters_
	parent_.converters_.UpdateChoices();
	local_converters.Update(local_formats);
}


bool FormPreferences::Formats::Add()
{
	string name = fl_get_input(dialog_->input_format);
	string prettyname = fl_get_input(dialog_->input_gui_name);
	string extension = fl_get_input(dialog_->input_extension);
	string shortcut =  fl_get_input(dialog_->input_shrtcut);
	string viewer =  fl_get_input(dialog_->input_viewer);

	Format const * old = local_formats.GetFormat(name);
	string old_prettyname = old ? old->prettyname() : string();
	local_formats.Add(name, extension, prettyname, shortcut);
	local_formats.SetViewer(name, viewer);
	if (!old || prettyname != old_prettyname) {
		UpdateBrowser();
		if (old)
			parent_.converters_.UpdateBrowser();
	}
	fl_deactivate_object(dialog_->button_add);
	fl_set_object_lcol(dialog_->button_add, FL_INACTIVE);

	return true;
}


bool FormPreferences::Formats::Browser() 
{
	int i = fl_get_browser(dialog_->browser_all);
	if( i <= 0 ) return false;

	fl_freeze_form(dialog_->form);

	Format const & f = local_formats.Get(i-1);

	fl_set_input(dialog_->input_format, f.name().c_str());
	fl_set_input(dialog_->input_gui_name, f.prettyname().c_str());
	fl_set_input(dialog_->input_shrtcut, f.shortcut().c_str());
	fl_set_input(dialog_->input_extension, f.extension().c_str());
	fl_set_input(dialog_->input_viewer, f.viewer().c_str());

	fl_set_object_label( dialog_->button_add, idex(_("Modify|#M")) );
	fl_set_button_shortcut( dialog_->button_add, scex(_("Modify|#M")), 1);

	fl_deactivate_object(dialog_->button_add);
	fl_set_object_lcol(dialog_->button_add, FL_INACTIVE);

	fl_activate_object(dialog_->button_delete);
	fl_set_object_lcol(dialog_->button_delete, FL_BLACK);
				
	fl_unfreeze_form(dialog_->form);
	return false;
}


bool FormPreferences::Formats::Delete()
{
	string name = fl_get_input(dialog_->input_format);

	if (local_converters.FormatIsUsed(name)) {
		parent_.printWarning(_("WARNING! Cannot remove a Format used by a Converter. Remove the converter first."));
		fl_deactivate_object(dialog_->button_delete);
		fl_set_object_lcol(dialog_->button_delete, FL_INACTIVE);
		return false;
	}

	local_formats.Delete(name);
	UpdateBrowser();
	return true;
}


bool FormPreferences::Formats::Input()
{
	string name = fl_get_input(dialog_->input_format);
	int sel = local_formats.GetNumber(name);
	fl_freeze_form(dialog_->form);

	if (sel < 0) {
		fl_set_object_label( dialog_->button_add,
				     idex(_("Add|#A")) );
		fl_set_button_shortcut( dialog_->button_add,
					scex(_("Add|#A")), 1);

		fl_deselect_browser(dialog_->browser_all);

		fl_deactivate_object(dialog_->button_delete);
		fl_set_object_lcol(dialog_->button_delete, FL_INACTIVE);

	} else {
		fl_set_object_label( dialog_->button_add,
				     idex(_("Modify|#M")) );
		fl_set_button_shortcut( dialog_->button_add,
					scex(_("Modify|#M")), 1);

		int top = max(sel-5, 0);
		fl_set_browser_topline(dialog_->browser_all, top);
		fl_select_browser_line(dialog_->browser_all, sel+1);
		
		fl_activate_object(dialog_->button_add);
		fl_set_object_lcol(dialog_->button_add, FL_BLACK);

		fl_activate_object(dialog_->button_delete);
		fl_set_object_lcol(dialog_->button_delete, FL_BLACK);
	}

	string prettyname = fl_get_input(dialog_->input_gui_name);
	if (name.empty() || prettyname.empty()) {
		fl_deactivate_object(dialog_->button_add);
		fl_set_object_lcol(dialog_->button_add, FL_INACTIVE);
	} else {
		fl_activate_object(dialog_->button_add);
		fl_set_object_lcol(dialog_->button_add, FL_BLACK);
	}

	fl_unfreeze_form(dialog_->form);
	return false;
}


FormPreferences::InputsMisc::~InputsMisc()
{
	delete dialog_;
}


void FormPreferences::InputsMisc::apply() const
{
	lyxrc.date_insert_format =
		fl_get_input(dialog_->input_date_format);
}


void FormPreferences::InputsMisc::build()
{
	dialog_ = parent_.build_inputs_misc();

	fl_set_input_return(dialog_->input_date_format, FL_RETURN_CHANGED);

	// set up the feedback mechanism
	parent_.setPreHandler( dialog_->input_date_format );
}


string const
FormPreferences::InputsMisc::feedback(FL_OBJECT const * const ob) const
{
	string str;

	if (ob == dialog_->input_date_format )
		str = lyxrc.getDescription( LyXRC::RC_DATE_INSERT_FORMAT );

	return str;
}


void FormPreferences::InputsMisc::update()
{
	fl_set_input(dialog_->input_date_format,
		     lyxrc.date_insert_format.c_str());
}


FormPreferences::Interface::~Interface()
{
	delete dialog_;
}


void FormPreferences::Interface::apply() const
{
	lyxrc.popup_font_name =
		fl_get_input(dialog_->input_popup_font);
	lyxrc.menu_font_name = fl_get_input(dialog_->input_menu_font);
	lyxrc.font_norm_menu =
		fl_get_input(dialog_->input_popup_encoding);
	lyxrc.bind_file = fl_get_input(dialog_->input_bind_file);
	lyxrc.ui_file = fl_get_input(dialog_->input_ui_file);
	lyxrc.override_x_deadkeys =
		fl_get_button(dialog_->check_override_x_dead_keys);
}


void FormPreferences::Interface::build()
{
	dialog_ = parent_.build_interface();

	fl_set_input_return(dialog_->input_popup_font, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_menu_font, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_popup_encoding, 
			    FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_bind_file, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_ui_file, FL_RETURN_CHANGED);

	// set up the feedback mechanism
	parent_.setPreHandler( dialog_->input_popup_font );
	parent_.setPreHandler( dialog_->input_menu_font );
	parent_.setPreHandler( dialog_->input_popup_encoding );
	parent_.setPreHandler( dialog_->input_bind_file );
	parent_.setPreHandler( dialog_->button_bind_file_browse );
	parent_.setPreHandler( dialog_->input_ui_file );
	parent_.setPreHandler( dialog_->button_ui_file_browse );
	parent_.setPreHandler( dialog_->check_override_x_dead_keys );
}


string const
FormPreferences::Interface::feedback( FL_OBJECT const * const ob ) const
{
	string str;

	if (ob == dialog_->input_popup_font )
		str = lyxrc.getDescription( LyXRC::RC_SCREEN_FONT_POPUP );
	else if (ob == dialog_->input_menu_font )
		str = lyxrc.getDescription( LyXRC::RC_SCREEN_FONT_MENU );
	else if (ob == dialog_->input_popup_encoding )
		str = lyxrc.getDescription( LyXRC::RC_SCREEN_FONT_ENCODING_MENU );
	else if (ob == dialog_->input_bind_file )
		str = lyxrc.getDescription( LyXRC::RC_BINDFILE );
	else if (ob == dialog_->input_ui_file )
		str = lyxrc.getDescription( LyXRC::RC_UIFILE );
	else if (ob == dialog_->check_override_x_dead_keys )
		str = lyxrc.getDescription( LyXRC::RC_OVERRIDE_X_DEADKEYS );

	return str;
}


bool FormPreferences::Interface::input( FL_OBJECT const * const ob )
{
	if( ob == dialog_->button_bind_file_browse ) {
		string dir  = system_lyxdir + string("bind");
		string name = N_("Sys Bind");
		pair<string,string> dir1(name, dir);

		dir = user_lyxdir + string("bind");
		name = N_("User Bind");
		pair<string,string> dir2(name, dir);

		parent_.browse( dialog_->input_bind_file,
				_("Bind file"), "*.bind", dir1, dir2 );
		
	} else if ( ob == dialog_->button_ui_file_browse ) {
		string dir  = system_lyxdir + string("ui");
		string name = N_("Sys UI");
		pair<string,string> dir1(name, dir);

		dir = user_lyxdir + string("ui");
		name = N_("User UI");
		pair<string,string> dir2(name, dir);

		parent_.browse( dialog_->input_ui_file,
				_("UI file"), "*.ui", dir1, dir2 );
	}
	
	return true;
}


void FormPreferences::Interface::update()
{
	fl_set_input(dialog_->input_popup_font,
		     lyxrc.popup_font_name.c_str());
	fl_set_input(dialog_->input_menu_font,
		     lyxrc.menu_font_name.c_str());
	fl_set_input(dialog_->input_popup_encoding,
		     lyxrc.font_norm_menu.c_str());
	fl_set_input(dialog_->input_bind_file,
		     lyxrc.bind_file.c_str());
	fl_set_input(dialog_->input_ui_file,
		     lyxrc.ui_file.c_str());
	fl_set_button(dialog_->check_override_x_dead_keys,
		      lyxrc.override_x_deadkeys);
}


FormPreferences::Language::~Language()
{
	delete combo_default_lang;
	delete dialog_;
}


void FormPreferences::Language::apply()
{
	lyxrc.default_language = combo_default_lang->getline();

	int button = fl_get_button(dialog_->check_use_kbmap);
	string name_1 = fl_get_input(dialog_->input_kbmap1);
	string name_2 = fl_get_input(dialog_->input_kbmap2);
	if( button )
		button = ( !name_1.empty() && !name_2.empty() );
	lyxrc.use_kbmap = static_cast<bool>(button);

	if (lyxrc.use_kbmap) {
		lyxrc.primary_kbmap = name_1;
		lyxrc.secondary_kbmap = name_2;
	}
	
	button = fl_get_button(dialog_->check_rtl_support);
	lyxrc.rtl_support = static_cast<bool>(button);

	button = fl_get_button(dialog_->check_auto_begin);
	lyxrc.language_auto_begin = static_cast<bool>(button);

	button = fl_get_button(dialog_->check_auto_end);
	lyxrc.language_auto_end = static_cast<bool>(button);

	button = fl_get_button(dialog_->check_mark_foreign);
	lyxrc.mark_foreign_language = static_cast<bool>(button);

	lyxrc.language_package = fl_get_input(dialog_->input_package);
	lyxrc.language_command_begin = fl_get_input(dialog_->input_command_begin);
	lyxrc.language_command_end = fl_get_input(dialog_->input_command_end);

	// Ensure that all is self-consistent.
	update();
}


void FormPreferences::Language::build()
{
	dialog_ = parent_.build_language();

	fl_set_input_return(dialog_->input_package, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_command_begin, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_command_end, FL_RETURN_CHANGED);

	// The default_language is a combo-box and has to be inserted manually
	fl_freeze_form(dialog_->form);
	fl_addto_form(dialog_->form);

	FL_OBJECT * obj = dialog_->choice_default_lang;
	fl_deactivate_object(dialog_->choice_default_lang);
	combo_default_lang = new Combox(FL_COMBOX_DROPLIST);
	combo_default_lang->add(obj->x, obj->y, obj->w, obj->h, 400);
	combo_default_lang->shortcut("#L",1);
	combo_default_lang->setcallback(ComboCB, &parent_);

	for (Languages::const_iterator cit = languages.begin();
	    cit != languages.end(); ++cit) {
		combo_default_lang->addto((*cit).second.lang());
	}

	fl_end_form();
	fl_unfreeze_form(dialog_->form);

	// set up the feedback mechanism
	parent_.setPreHandler( dialog_->input_package );
	parent_.setPreHandler( dialog_->check_use_kbmap );

	// This is safe, as nothing is done to the pointer, other than
	// to use its address in a block-if statement.
	//setPreHandler( reinterpret_cast<FL_OBJECT *>(combo_default_lang) );

	parent_.setPreHandler( dialog_->input_kbmap1 );
	parent_.setPreHandler( dialog_->input_kbmap2 );
	parent_.setPreHandler( dialog_->check_rtl_support );
	parent_.setPreHandler( dialog_->check_mark_foreign );
	parent_.setPreHandler( dialog_->check_auto_begin );
	parent_.setPreHandler( dialog_->check_auto_end );
	parent_.setPreHandler( dialog_->input_command_begin );
	parent_.setPreHandler( dialog_->input_command_end );

	// Activate/Deactivate the input fields dependent on the state of the
	// buttons.
	input( 0 );
}


string const
FormPreferences::Language::feedback(FL_OBJECT const * const ob) const
{
	string str;

	if (reinterpret_cast<Combox const *>(ob) == combo_default_lang )
		str = lyxrc.getDescription( LyXRC::RC_DEFAULT_LANGUAGE );
	else if (ob == dialog_->check_use_kbmap )
		str = lyxrc.getDescription( LyXRC::RC_KBMAP );
	else if (ob == dialog_->input_kbmap1)
		str = lyxrc.getDescription( LyXRC::RC_KBMAP_PRIMARY );
	else if (ob == dialog_->input_kbmap2)
		str = lyxrc.getDescription( LyXRC::RC_KBMAP_SECONDARY );
	else if (ob == dialog_->check_rtl_support )
		str = lyxrc.getDescription( LyXRC::RC_RTL_SUPPORT );
	else if (ob == dialog_->check_auto_begin )
		str = lyxrc.getDescription( LyXRC::RC_LANGUAGE_AUTO_BEGIN );
	else if (ob == dialog_->check_auto_end )
		str = lyxrc.getDescription( LyXRC::RC_LANGUAGE_AUTO_END );
	else if (ob == dialog_->check_mark_foreign )
		str = lyxrc.getDescription( LyXRC::RC_MARK_FOREIGN_LANGUAGE );
	else if (ob == dialog_->input_package )
		str = lyxrc.getDescription( LyXRC::RC_LANGUAGE_PACKAGE );
	else if (ob == dialog_->input_command_begin )
		str = lyxrc.getDescription( LyXRC::RC_LANGUAGE_COMMAND_BEGIN );
	else if (ob == dialog_->input_command_end )
		str = lyxrc.getDescription( LyXRC::RC_LANGUAGE_COMMAND_END );

	return str;
}


bool FormPreferences::Language::input( FL_OBJECT const * const ob )
{
	bool activate = true;

	// !ob if function is called from Language::build() to de/activate
	// objects,
	// otherwise the function is called by an xforms CB via input().
	if (!ob || ob == dialog_->check_use_kbmap) {
		if (fl_get_button(dialog_->check_use_kbmap)) {
			fl_activate_object(dialog_->button_kbmap1_browse);
			fl_set_object_lcol(dialog_->button_kbmap1_browse,
					   FL_BLACK);

			fl_activate_object(dialog_->button_kbmap2_browse);
			fl_set_object_lcol(dialog_->button_kbmap2_browse,
					   FL_BLACK);

			fl_activate_object(dialog_->input_kbmap1);
			fl_set_object_lcol(dialog_->input_kbmap1, FL_BLACK);
			fl_activate_object(dialog_->input_kbmap2);
			fl_set_object_lcol(dialog_->input_kbmap2, FL_BLACK);
		} else {
			fl_deactivate_object(dialog_->button_kbmap1_browse);
			fl_set_object_lcol(dialog_->button_kbmap1_browse,
					   FL_INACTIVE);

			fl_deactivate_object(dialog_->button_kbmap2_browse);
			fl_set_object_lcol(dialog_->button_kbmap2_browse,
					   FL_INACTIVE);

			fl_deactivate_object(dialog_->input_kbmap1);
			fl_set_object_lcol(dialog_->input_kbmap1,
					   FL_INACTIVE);
			fl_deactivate_object(dialog_->input_kbmap2);
			fl_set_object_lcol(dialog_->input_kbmap2,
					   FL_INACTIVE);
		}
	}

	if( ob == dialog_->button_kbmap1_browse ) {
		string dir  = system_lyxdir + string("kbd");
		string name = N_("Key maps");
		pair<string,string> dir1(name, dir);

		parent_.browse( dialog_->input_kbmap1,
				_("Keyboard map"), "*.kmap", dir1 );
	} else if( ob == dialog_->button_kbmap2_browse ) {
		string dir  = system_lyxdir + string("kbd");
		string name = N_("Key maps");
		pair<string,string> dir1(name, dir);

		parent_.browse( dialog_->input_kbmap2,
				_("Keyboard map"), "*.kmap", dir1 );
	}

	return activate;
}


void FormPreferences::Language::update()
{
	fl_set_button(dialog_->check_use_kbmap,
		      lyxrc.use_kbmap);

	combo_default_lang->select_text( lyxrc.default_language );

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
	fl_set_button(dialog_->check_auto_begin,  lyxrc.language_auto_begin);
	fl_set_button(dialog_->check_auto_end,    lyxrc.language_auto_end);
	fl_set_button(dialog_->check_mark_foreign,
		      lyxrc.mark_foreign_language);

	fl_set_input(dialog_->input_package,
		     lyxrc.language_package.c_str());
	fl_set_input(dialog_->input_command_begin,
		     lyxrc.language_command_begin.c_str());
	fl_set_input(dialog_->input_command_end,
		     lyxrc.language_command_end.c_str());

	// Activate/Deactivate the input fields dependent on the state of the
	// buttons.
	input( 0 );
}


void FormPreferences::Language::ComboCB(int, void * v, Combox * combox)
{
    FormPreferences * pre = static_cast<FormPreferences*>(v);
    // This is safe, as nothing is done to the pointer, other than
    // to use its address in a block-if statement.
    pre->bc_.valid( pre->input( reinterpret_cast<FL_OBJECT *>(combox), 0 ));
}


FormPreferences::LnFmisc::~LnFmisc()
{
	delete dialog_;
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
	lyxrc.autosave = static_cast<unsigned int>
		(fl_get_counter_value(dialog_->counter_autosave));
	lyxrc.wheel_jump = static_cast<unsigned int>
		(fl_get_counter_value(dialog_->counter_wm_jump));
}


void FormPreferences::LnFmisc::build()
{
	dialog_ = parent_.build_lnf_misc();

	fl_set_counter_return(dialog_->counter_autosave, FL_RETURN_CHANGED);
	fl_set_counter_return(dialog_->counter_wm_jump, FL_RETURN_CHANGED);

	// set up the feedback mechanism
	parent_.setPreHandler( dialog_->check_banner );
	parent_.setPreHandler( dialog_->check_auto_region_delete );
	parent_.setPreHandler( dialog_->check_exit_confirm );
	parent_.setPreHandler( dialog_->check_display_shrtcuts );
	parent_.setPreHandler( dialog_->counter_autosave );
	parent_.setPreHandler( dialog_->check_ask_new_file );
	parent_.setPreHandler( dialog_->check_cursor_follows_scrollbar );
	parent_.setPreHandler( dialog_->counter_wm_jump );
}


string const
FormPreferences::LnFmisc::feedback(FL_OBJECT const * const ob) const
{
	string str;

	if (ob == dialog_->check_banner )
		str = lyxrc.getDescription( LyXRC::RC_SHOW_BANNER );
	else if (ob == dialog_->check_auto_region_delete )
		str = lyxrc.getDescription( LyXRC::RC_AUTOREGIONDELETE );
	else if (ob == dialog_->check_exit_confirm )
		str = lyxrc.getDescription( LyXRC::RC_EXIT_CONFIRMATION );
	else if (ob == dialog_->check_display_shrtcuts )
		str = lyxrc.getDescription( LyXRC::RC_DISPLAY_SHORTCUTS );
	else if (ob == dialog_->check_ask_new_file )
		str = lyxrc.getDescription( LyXRC::RC_NEW_ASK_FILENAME );
	else if (ob == dialog_->check_cursor_follows_scrollbar )
		str = lyxrc.getDescription( LyXRC::RC_CURSOR_FOLLOWS_SCROLLBAR );
	else if (ob == dialog_->counter_autosave )
		str = lyxrc.getDescription( LyXRC::RC_AUTOSAVE );
	else if (ob == dialog_->counter_wm_jump )
		str = lyxrc.getDescription( LyXRC::RC_WHEEL_JUMP );

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
	fl_set_counter_value(dialog_->counter_autosave, lyxrc.autosave);
	fl_set_counter_value(dialog_->counter_wm_jump, lyxrc.wheel_jump);
}


FormPreferences::OutputsMisc::~OutputsMisc()
{
	delete dialog_;
}


void FormPreferences::OutputsMisc::apply() const
{
	lyxrc.ascii_linelen = static_cast<unsigned int>
		(fl_get_counter_value(dialog_->counter_line_len));
	lyxrc.fontenc = fl_get_input(dialog_->input_tex_encoding);

	int choice =
		fl_get_choice(dialog_->choice_default_papersize) - 1;
	lyxrc.default_papersize = static_cast<BufferParams::PAPER_SIZE>(choice);

	lyxrc.ascii_roff_command = fl_get_input(dialog_->input_ascii_roff);
	lyxrc.chktex_command = fl_get_input(dialog_->input_checktex);
}


void FormPreferences::OutputsMisc::build()
{
	dialog_ = parent_.build_outputs_misc();

	fl_set_counter_return(dialog_->counter_line_len,
			      FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_tex_encoding,
			    FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_ascii_roff,
			    FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_checktex,
			    FL_RETURN_CHANGED);
	fl_addto_choice(dialog_->choice_default_papersize,
			_(" default | US letter | legal | executive | A3 | A4 | A5 | B5 "));

	// set up the feedback mechanism
	parent_.setPreHandler( dialog_->counter_line_len );
	parent_.setPreHandler( dialog_->input_tex_encoding );
	parent_.setPreHandler( dialog_->choice_default_papersize );
	parent_.setPreHandler( dialog_->input_ascii_roff );
	parent_.setPreHandler( dialog_->input_checktex );
}


string const
FormPreferences::OutputsMisc::feedback(FL_OBJECT const * const ob) const
{
	string str;

	if (ob == dialog_->counter_line_len )
		str = lyxrc.getDescription( LyXRC::RC_ASCII_LINELEN );
	else if (ob == dialog_->input_tex_encoding )
		str = lyxrc.getDescription( LyXRC::RC_FONT_ENCODING );
	else if (ob == dialog_->input_ascii_roff )
		str = lyxrc.getDescription( LyXRC::RC_ASCIIROFF_COMMAND );
	else if (ob == dialog_->input_checktex )
		str = lyxrc.getDescription( LyXRC::RC_CHKTEX_COMMAND );
	else if (ob == dialog_->choice_default_papersize )
		str = lyxrc.getDescription( LyXRC::RC_DEFAULT_PAPERSIZE );

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
}


FormPreferences::Paths::~Paths()
{
	delete dialog_;
}


void FormPreferences::Paths::apply()
{
	lyxrc.document_path = fl_get_input(dialog_->input_default_path);
	lyxrc.template_path = fl_get_input(dialog_->input_template_path);

	int button = fl_get_button(dialog_->check_use_temp_dir);
	string str  = fl_get_input(dialog_->input_temp_dir);
	if (!button ) str.erase();

	lyxrc.use_tempdir = button;
	lyxrc.tempdir_path = str;

	button = fl_get_button(dialog_->check_last_files);
	str = fl_get_input(dialog_->input_lastfiles);
	if (!button ) str.erase();
	
	lyxrc.check_lastfiles = button;
	lyxrc.lastfiles = str;
	lyxrc.num_lastfiles = static_cast<unsigned int>
		(fl_get_counter_value(dialog_->counter_lastfiles));

	button = fl_get_button(dialog_->check_make_backups);
	str = fl_get_input(dialog_->input_backup_path);
	if (!button ) str.erase();

	lyxrc.make_backup = button;
	lyxrc.backupdir_path = str;

	lyxrc.lyxpipes = fl_get_input(dialog_->input_serverpipe);

	// update view
	update();
}


void FormPreferences::Paths::build()
{
	dialog_ = parent_.build_paths();

	fl_set_input_return(dialog_->input_default_path, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_template_path, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_temp_dir, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_lastfiles, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_backup_path, FL_RETURN_CHANGED);
	fl_set_counter_return(dialog_->counter_lastfiles, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_serverpipe, FL_RETURN_CHANGED);

	// set up the feedback mechanism
	parent_.setPreHandler( dialog_->input_default_path );
	parent_.setPreHandler( dialog_->counter_lastfiles );
	parent_.setPreHandler( dialog_->input_template_path );
	parent_.setPreHandler( dialog_->check_last_files );
	parent_.setPreHandler( dialog_->input_lastfiles );
	parent_.setPreHandler( dialog_->check_make_backups );
	parent_.setPreHandler( dialog_->input_backup_path );
	parent_.setPreHandler( dialog_->input_serverpipe );
	parent_.setPreHandler( dialog_->input_temp_dir );
	parent_.setPreHandler( dialog_->check_use_temp_dir );
}


string const
FormPreferences::Paths::feedback( FL_OBJECT const * const ob ) const
{
	string str;

	if (ob == dialog_->input_default_path )
		str = lyxrc.getDescription( LyXRC::RC_DOCUMENTPATH );
	else if (ob == dialog_->input_template_path )
		str = lyxrc.getDescription( LyXRC::RC_TEMPLATEPATH );
	else if (ob == dialog_->check_use_temp_dir )
		str = lyxrc.getDescription( LyXRC::RC_USETEMPDIR );
	else if (ob == dialog_->input_temp_dir )
		str = lyxrc.getDescription( LyXRC::RC_TEMPDIRPATH );
	else if (ob == dialog_->check_last_files )
		str = lyxrc.getDescription( LyXRC::RC_CHECKLASTFILES );
	else if (ob == dialog_->input_lastfiles )
		str = lyxrc.getDescription( LyXRC::RC_LASTFILES );
	else if (ob == dialog_->counter_lastfiles )
		str = lyxrc.getDescription( LyXRC::RC_NUMLASTFILES );
	else if (ob == dialog_->check_make_backups )
		str = lyxrc.getDescription( LyXRC::RC_MAKE_BACKUP );
	else if (ob == dialog_->input_backup_path )
		str = lyxrc.getDescription( LyXRC::RC_BACKUPDIR_PATH );
	else if (ob == dialog_->input_serverpipe )
		str = lyxrc.getDescription( LyXRC::RC_SERVERPIPE );

	return str;
}


bool FormPreferences::Paths::input( FL_OBJECT const * const ob )
{
	bool activate = true;
	
	// !ob if function is called from Paths::update() to de/activate
	// objects,
	// otherwise the function is called by an xforms CB via input().
	if (!ob || ob == dialog_->check_use_temp_dir) {
		if (fl_get_button(dialog_->check_use_temp_dir)) {
			fl_activate_object(dialog_->input_temp_dir);
			fl_set_object_lcol(dialog_->input_temp_dir,
					   FL_BLACK);
		} else {
			fl_deactivate_object(dialog_->input_temp_dir);
			fl_set_object_lcol(dialog_->input_temp_dir,
					   FL_INACTIVE);
		}
	}

	if (!ob || ob == dialog_->check_last_files) {
		if (fl_get_button(dialog_->check_last_files)) {
			fl_activate_object(dialog_->input_lastfiles);
			fl_set_object_lcol(dialog_->input_lastfiles,
					   FL_BLACK);
		} else {
			fl_deactivate_object(dialog_->input_lastfiles);
			fl_set_object_lcol(dialog_->input_lastfiles,
					   FL_INACTIVE);
		}
	}

	if (!ob || ob == dialog_->check_make_backups) {
		if (fl_get_button(dialog_->check_make_backups)) {
			fl_activate_object(dialog_->input_backup_path);
			fl_set_object_lcol(dialog_->input_backup_path,
					   FL_BLACK);
		} else {
			fl_deactivate_object(dialog_->input_backup_path);
			fl_set_object_lcol(dialog_->input_backup_path,
					   FL_INACTIVE);
		}
	}

	if (!ob || ob == dialog_->input_default_path) {
		string name = fl_get_input(dialog_->input_default_path);
		if (!parent_.WriteableDir(name) )
			return false;
	}

	if (!ob || ob == dialog_->input_template_path) {
		string name = fl_get_input(dialog_->input_template_path);
		if (!parent_.ReadableDir(name) )
		    return false;
	}

	if (!ob || ob == dialog_->input_temp_dir) {
		string name = fl_get_input(dialog_->input_temp_dir);
		if (fl_get_button(dialog_->check_make_backups)
		    && !name.empty()
		    && !parent_.WriteableDir(name) )
			return false;
	}

	if (!ob || ob == dialog_->input_backup_path) {
		string name = fl_get_input(dialog_->input_backup_path);
		if (fl_get_button(dialog_->check_make_backups)
		    && !name.empty()
		    && !parent_.WriteableDir(name) )
			return false;
	}

	if (!ob || ob == dialog_->input_lastfiles) {
		string name = fl_get_input(dialog_->input_lastfiles);
		if (fl_get_button(dialog_->check_last_files)
		    && !name.empty()
		    && !parent_.WriteableFile(name) )
			return false;
	}

	if (!ob || ob == dialog_->input_serverpipe) {
		string name = fl_get_input(dialog_->input_serverpipe);
		if (!name.empty()) {
			if (!parent_.WriteableFile(name, ".in"))
				return false;
			if (!parent_.WriteableFile(name, ".out"))
				return false;
		}
	}

	if( ob == dialog_->button_default_path_browse ) {
		parent_.browse( dialog_->input_default_path,
				_("Default path"), string() );
	} else if( ob == dialog_->button_template_path_browse ) {
		parent_.browse( dialog_->input_template_path,
				_("Template path"), string() );
	} else if( ob == dialog_->button_temp_dir_browse ) {
		parent_.browse( dialog_->input_temp_dir,
				_("Temp dir"), string() );
	} else if( ob == dialog_->button_lastfiles_browse ) {
		pair<string,string> dir(_("User"), user_lyxdir);

		parent_.browse( dialog_->input_lastfiles,
				_("Lastfiles"), string(), dir );
	} else if( ob == dialog_->button_backup_path_browse ) {
		parent_.browse( dialog_->input_backup_path,
				_("Backup path"), string() );
	} else if( ob == dialog_->button_serverpipe_browse ) {
		// Not sure how to do this!!!
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
	input( 0 );
}


FormPreferences::Printer::~Printer()
{
	delete dialog_;
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

	if (ob == dialog_->input_command )
		str = lyxrc.getDescription( LyXRC::RC_PRINT_COMMAND );
	else if (ob == dialog_->check_adapt_output )
		str = lyxrc.getDescription( LyXRC::RC_PRINT_ADAPTOUTPUT );
	else if (ob == dialog_->input_to_printer )
		str = lyxrc.getDescription( LyXRC::RC_PRINTTOPRINTER );
	else if (ob == dialog_->input_to_file )
		str = lyxrc.getDescription( LyXRC::RC_PRINTTOFILE );
	else if (ob == dialog_->input_file_extension )
		str = lyxrc.getDescription( LyXRC::RC_PRINTFILEEXTENSION );
	else if (ob == dialog_->input_extra_options )
		str = lyxrc.getDescription( LyXRC::RC_PRINTEXSTRAOPTIONS );
	else if (ob == dialog_->input_spool_command )
		str = lyxrc.getDescription( LyXRC::RC_PRINTSPOOL_COMMAND );
	else if (ob == dialog_->input_spool_prefix )
		str = lyxrc.getDescription( LyXRC::RC_PRINTSPOOL_PRINTERPREFIX );
	else if (ob == dialog_->input_name )
		str = lyxrc.getDescription( LyXRC::RC_PRINTER );
	else if (ob == dialog_->input_even_pages )
		str = lyxrc.getDescription( LyXRC::RC_PRINTEVENPAGEFLAG );
	else if (ob == dialog_->input_odd_pages )
		str = lyxrc.getDescription( LyXRC::RC_PRINTODDPAGEFLAG );
	else if (ob == dialog_->input_page_range )
		str = lyxrc.getDescription( LyXRC::RC_PRINTPAGERANGEFLAG );
	else if (ob == dialog_->input_reverse )
		str = lyxrc.getDescription( LyXRC::RC_PRINTREVERSEFLAG );
	else if (ob == dialog_->input_landscape )
		str = lyxrc.getDescription( LyXRC::RC_PRINTLANDSCAPEFLAG );
	else if (ob == dialog_->input_copies )
		str = lyxrc.getDescription( LyXRC::RC_PRINTCOLLCOPIESFLAG );
	else if (ob == dialog_->input_collated )
		str = lyxrc.getDescription( LyXRC::RC_PRINTCOPIESFLAG );
	else if (ob == dialog_->input_paper_type )
		str = lyxrc.getDescription( LyXRC::RC_PRINTPAPERFLAG );
	else if (ob == dialog_->input_paper_size )
		str = lyxrc.getDescription( LyXRC::RC_PRINTPAPERDIMENSIONFLAG );

	return str;
}


void FormPreferences::Printer::build()
{
	dialog_ = parent_.build_printer();

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
	parent_.setPreHandler( dialog_->input_command );
	parent_.setPreHandler( dialog_->input_page_range );
	parent_.setPreHandler( dialog_->input_copies );
	parent_.setPreHandler( dialog_->input_reverse );
	parent_.setPreHandler( dialog_->input_to_printer );
	parent_.setPreHandler( dialog_->input_file_extension );
	parent_.setPreHandler( dialog_->input_spool_command );
	parent_.setPreHandler( dialog_->input_paper_type );
	parent_.setPreHandler( dialog_->input_even_pages );
	parent_.setPreHandler( dialog_->input_odd_pages );
	parent_.setPreHandler( dialog_->input_collated );
	parent_.setPreHandler( dialog_->input_landscape );
	parent_.setPreHandler( dialog_->input_to_file );
	parent_.setPreHandler( dialog_->input_extra_options );
	parent_.setPreHandler( dialog_->input_spool_prefix );
	parent_.setPreHandler( dialog_->input_paper_size );
	parent_.setPreHandler( dialog_->input_name );
	parent_.setPreHandler( dialog_->check_adapt_output );
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


FormPreferences::ScreenFonts::~ScreenFonts()
{
	delete dialog_;
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
		parent_.lv_->getLyXFunc()->Dispatch(LFUN_SCREEN_FONT_UPDATE);
	}
}


void FormPreferences::ScreenFonts::build()
{
	dialog_ = parent_.build_screen_fonts();

	fl_set_input_return(dialog_->input_roman, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_sans, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_typewriter,
			    FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_screen_encoding,
			    FL_RETURN_CHANGED);
	fl_set_counter_return(dialog_->counter_zoom, FL_RETURN_CHANGED);
	fl_set_counter_return(dialog_->counter_dpi, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_tiny, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_script, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_footnote, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_small, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_normal, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_large, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_larger, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_largest, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_huge, FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_huger, FL_RETURN_CHANGED);

	fl_set_input_filter(dialog_->input_tiny,
			    fl_unsigned_int_filter);
	fl_set_input_filter(dialog_->input_script,
			    fl_unsigned_int_filter);
	fl_set_input_filter(dialog_->input_footnote,
			    fl_unsigned_int_filter);
	fl_set_input_filter(dialog_->input_small,
			    fl_unsigned_int_filter);
	fl_set_input_filter(dialog_->input_normal,
			    fl_unsigned_int_filter);
	fl_set_input_filter(dialog_->input_large,
			    fl_unsigned_int_filter);
	fl_set_input_filter(dialog_->input_larger,
			    fl_unsigned_int_filter);
	fl_set_input_filter(dialog_->input_largest,
			    fl_unsigned_int_filter);
	fl_set_input_filter(dialog_->input_huge,
			    fl_unsigned_int_filter);
	fl_set_input_filter(dialog_->input_huger,
			    fl_unsigned_int_filter);

	// set up the feedback mechanism
	parent_.setPreHandler( dialog_->input_roman );
	parent_.setPreHandler( dialog_->input_sans );
	parent_.setPreHandler( dialog_->input_typewriter );
	parent_.setPreHandler( dialog_->counter_zoom );
	parent_.setPreHandler( dialog_->counter_dpi );
	parent_.setPreHandler( dialog_->check_scalable );
	parent_.setPreHandler( dialog_->input_screen_encoding );
	parent_.setPreHandler( dialog_->input_tiny );
	parent_.setPreHandler( dialog_->input_script );
	parent_.setPreHandler( dialog_->input_footnote );
	parent_.setPreHandler( dialog_->input_small );
	parent_.setPreHandler( dialog_->input_large );
	parent_.setPreHandler( dialog_->input_larger );
	parent_.setPreHandler( dialog_->input_largest );
	parent_.setPreHandler( dialog_->input_normal );
	parent_.setPreHandler( dialog_->input_huge );
	parent_.setPreHandler( dialog_->input_huger );
}

	
string const
FormPreferences::ScreenFonts::feedback(FL_OBJECT const * const ob ) const
{
	string str;

	if (ob == dialog_->input_roman )
		str = lyxrc.getDescription( LyXRC::RC_SCREEN_FONT_ROMAN );
	else if (ob == dialog_->input_sans )
		str = lyxrc.getDescription( LyXRC::RC_SCREEN_FONT_SANS );
	else if (ob == dialog_->input_typewriter )
		str = lyxrc.getDescription( LyXRC::RC_SCREEN_FONT_TYPEWRITER );
	else if (ob == dialog_->check_scalable )
		str = lyxrc.getDescription( LyXRC::RC_SCREEN_FONT_SCALABLE );
	else if (ob == dialog_->input_screen_encoding )
		str = lyxrc.getDescription( LyXRC::RC_SCREEN_FONT_ENCODING );
	else if (ob == dialog_->counter_zoom )
		str = lyxrc.getDescription( LyXRC::RC_SCREEN_ZOOM );
	else if (ob == dialog_->counter_dpi ) 
		str = lyxrc.getDescription( LyXRC::RC_SCREEN_DPI );
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
		 || ob == dialog_->input_huger )
		str = lyxrc.getDescription( LyXRC::RC_SCREEN_FONT_SIZES );

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
		str = N_("WARNING! Fonts must be positive!");

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

		str = N_("WARNING! Fonts must be input in the order tiny > script> footnote > small > normal > large > larger > largest > huge > huger.");
	}

	if (!activate)
		parent_.printWarning( str );
	
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


FormPreferences::SpellChecker::~SpellChecker()
{
	delete dialog_;
}


void FormPreferences::SpellChecker::apply()
{

	string choice = fl_get_choice_text(dialog_->choice_spell_command);
	choice = strip( frontStrip( choice ) );
	
	lyxrc.isp_command = choice;

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
		int button = fl_get_button(dialog_->check_alt_lang);
		choice = fl_get_input(dialog_->input_alt_lang);
		if (button && choice.empty() ) button = 0;
		if (!button ) choice.erase();

		lyxrc.isp_use_alt_lang = static_cast<bool>(button);
		lyxrc.isp_alt_lang = choice;

		button = fl_get_button(dialog_->check_escape_chars);
		choice = fl_get_input(dialog_->input_escape_chars);
		if (button && choice.empty() ) button = 0;
		if (!button ) choice.erase();
	
		lyxrc.isp_use_esc_chars = static_cast<bool>(button);
		lyxrc.isp_esc_chars = choice;

		button = fl_get_button(dialog_->check_personal_dict);
		choice = fl_get_input(dialog_->input_personal_dict);
		if (button && choice.empty() ) button = 0;
		if (!button ) choice.erase();

		lyxrc.isp_use_pers_dict = static_cast<bool>(button);
		lyxrc.isp_pers_dict = choice;

		button = fl_get_button(dialog_->check_compound_words);
		lyxrc.isp_accept_compound = static_cast<bool>(button);

		button = fl_get_button(dialog_->check_input_enc);
		lyxrc.isp_use_input_encoding = static_cast<bool>(button);
	}

	// Reset view
	update();
}


void FormPreferences::SpellChecker::build()
{
	dialog_ = parent_.build_spellchecker();

	fl_addto_choice(dialog_->choice_spell_command,
			_(" none | ispell | aspell "));
	fl_set_input_return(dialog_->input_alt_lang,
			    FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_escape_chars,
			    FL_RETURN_CHANGED);
	fl_set_input_return(dialog_->input_personal_dict,
			    FL_RETURN_CHANGED);

	// set up the feedback mechanism
	parent_.setPreHandler( dialog_->choice_spell_command );
	parent_.setPreHandler( dialog_->check_alt_lang );
	parent_.setPreHandler( dialog_->input_alt_lang );
	parent_.setPreHandler( dialog_->check_escape_chars );
	parent_.setPreHandler( dialog_->input_escape_chars );
	parent_.setPreHandler( dialog_->check_personal_dict );
	parent_.setPreHandler( dialog_->input_personal_dict );
	parent_.setPreHandler( dialog_->button_personal_dict );
	parent_.setPreHandler( dialog_->check_compound_words );
	parent_.setPreHandler( dialog_->check_input_enc );
}


string const
FormPreferences::SpellChecker::feedback(FL_OBJECT const * const ob) const
{
	string str;

	if (ob == dialog_->choice_spell_command )
		str = lyxrc.getDescription( LyXRC::RC_SPELL_COMMAND );
	else if (ob == dialog_->check_alt_lang )
		str = lyxrc.getDescription( LyXRC::RC_USE_ALT_LANG );
	else if (ob == dialog_->input_alt_lang )
		str = lyxrc.getDescription( LyXRC::RC_ALT_LANG );
	else if (ob == dialog_->check_escape_chars )
		str = lyxrc.getDescription( LyXRC::RC_USE_ESC_CHARS );
	else if (ob == dialog_->input_escape_chars )
		str = lyxrc.getDescription( LyXRC::RC_ESC_CHARS );
	else if (ob == dialog_->check_personal_dict )
		str = lyxrc.getDescription( LyXRC::RC_USE_PERS_DICT );
	else if (ob == dialog_->input_personal_dict )
		str = lyxrc.getDescription( LyXRC::RC_PERS_DICT );
	else if (ob == dialog_->check_compound_words )
		str = lyxrc.getDescription( LyXRC::RC_ACCEPT_COMPOUND );
	else if (ob == dialog_->check_input_enc )
		str = lyxrc.getDescription( LyXRC::RC_USE_INP_ENC );

	return str;
}


bool FormPreferences::SpellChecker::input( FL_OBJECT const * const ob )
{
	// !ob if function is called from updateSpellChecker() to de/activate
	// objects,
	// otherwise the function is called by an xforms CB via input().

	// If spell checker == "none", disable all input.
	if (!ob || ob == dialog_->choice_spell_command) {
		if (fl_get_choice(dialog_->choice_spell_command) == 1) {
			fl_deactivate_object( dialog_->check_alt_lang );
			fl_deactivate_object( dialog_->input_alt_lang );
			fl_deactivate_object( dialog_->check_escape_chars );
			fl_deactivate_object( dialog_->input_escape_chars );
			fl_deactivate_object( dialog_->check_personal_dict );
			fl_deactivate_object( dialog_->input_personal_dict );
			fl_deactivate_object( dialog_->check_compound_words );
			fl_deactivate_object( dialog_->check_input_enc );
			return true;
		} else {
			fl_activate_object( dialog_->check_alt_lang );
			fl_activate_object( dialog_->check_escape_chars );
			fl_activate_object( dialog_->check_personal_dict );
			fl_activate_object( dialog_->check_compound_words );
			fl_activate_object( dialog_->check_input_enc );
		}
	}

	if (!ob || ob == dialog_->check_alt_lang) {
		if (fl_get_button(dialog_->check_alt_lang)) {
			fl_activate_object(dialog_->input_alt_lang);
			fl_set_object_lcol(dialog_->input_alt_lang,
					   FL_BLACK);
		} else {
			fl_deactivate_object(dialog_->input_alt_lang);
			fl_set_object_lcol(dialog_->input_alt_lang,
					   FL_INACTIVE);
		}
	}

	if (!ob || ob == dialog_->check_escape_chars) {
		if (fl_get_button(dialog_->check_escape_chars)) {
			fl_activate_object(dialog_->input_escape_chars);
			fl_set_object_lcol(dialog_->input_escape_chars,
					   FL_BLACK);
		} else {
			fl_deactivate_object(dialog_->input_escape_chars);
			fl_set_object_lcol(dialog_->input_escape_chars,
					   FL_INACTIVE);
		}
	}

	if (!ob || ob == dialog_->check_personal_dict) {
		if (fl_get_button(dialog_->check_personal_dict)) {
			fl_activate_object(dialog_->input_personal_dict);
			fl_set_object_lcol(dialog_->input_personal_dict,
					   FL_BLACK);
		} else {
			fl_deactivate_object(dialog_->input_personal_dict);
			fl_set_object_lcol(dialog_->input_personal_dict,
					   FL_INACTIVE);
		}
	}

	if( ob == dialog_->button_personal_dict) {
		parent_.browse( dialog_->input_personal_dict,
				_("Personal dictionary"), "*.ispell" );
	}
	
	return true; // All input is valid!
}


void FormPreferences::SpellChecker::update()
{
	int choice = 1;
	if (lyxrc.isp_command == "none" )
		choice = 1;
	else if (lyxrc.isp_command == "ispell" )
		choice = 2;
	else if (lyxrc.isp_command == "aspell" )
		choice = 3;
	fl_set_choice(dialog_->choice_spell_command, choice);
	
	string str;
	if (lyxrc.isp_use_alt_lang ) str = lyxrc.isp_alt_lang;

	fl_set_button(dialog_->check_alt_lang,
		      lyxrc.isp_use_alt_lang);
	fl_set_input(dialog_->input_alt_lang, str.c_str());
	
	str.erase();
	if (lyxrc.isp_use_esc_chars ) str = lyxrc.isp_esc_chars;

	fl_set_button(dialog_->check_escape_chars,
		      lyxrc.isp_use_esc_chars);
	fl_set_input(dialog_->input_escape_chars, str.c_str());

	str.erase();
	if (lyxrc.isp_use_pers_dict ) str = lyxrc.isp_pers_dict;

	fl_set_button(dialog_->check_personal_dict,
		      lyxrc.isp_use_pers_dict);
	fl_set_input(dialog_->input_personal_dict, str.c_str());

	fl_set_button(dialog_->check_compound_words,
		      lyxrc.isp_accept_compound);
	fl_set_button(dialog_->check_input_enc,
		      lyxrc.isp_use_input_encoding);

	// Activate/Deactivate the input fields dependent on the state of the
	// buttons.
	input( 0 );
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


bool FormPreferences::ReadableFile(string const & name,
				   string const & suffix)
{
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

	FileInfo d(name);
	if (!d.isDir()) {
		d.newFile(dir);
	}

	if (success && !d.isDir()) {
		success = false;
		str = N_("WARNING! Directory does not exist.");
	}
	
	if (success && !d.readable()) {
		success = false;
		str = N_("WARNING! Cannot read from this directory.");
	}

	FileInfo f(name+suffix);
	if (success && (dir == name || f.isDir())) {
		success = false;
		str = N_("WARNING! A file is required, not a directory.");
	}

	if (success && !f.exist()) {
		success = false;
		str = N_("WARNING! File does not exist.");
	}
	
	if (success && !f.readable()) {
		success = false;
		str = N_("WARNING! Cannot read from this file.");
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


bool FormPreferences::browse( FL_OBJECT * inpt,
			      string const & title,
			      string const & pattern, 
			      pair<string,string> const & dir1,
			      pair<string,string> const & dir2 )
{
	// Get the filename from the dialog
	string const filename = fl_get_input(inpt);

	// Show the file browser dialog
	string const new_filename =
		browseFile(filename, title, pattern, dir1, dir2 );

	// Save the filename to the dialog
	if (new_filename != filename && ! new_filename.empty()) {
		fl_set_input(inpt, new_filename.c_str());
		input( inpt, 0 );
	}
	
	return true;
}


string const
FormPreferences::browseFile( string const & filename,
			     string const & title,
			     string const & pattern, 
			     pair<string,string> const & dir1,
			     pair<string,string> const & dir2 ) const
{
	string lastPath = ".";
	if( !filename.empty() ) lastPath = OnlyPath(filename);

	LyXFileDlg fileDlg;

	if( !dir1.second.empty() ) {
		FileInfo fileInfo( dir1.second );
		if( fileInfo.isOK() && fileInfo.isDir() )
			fileDlg.SetButton( 0, dir1.first, dir1.second );
	}

	if( !dir2.second.empty() ) {
		FileInfo fileInfo( dir2.second );
		if( fileInfo.isOK() && fileInfo.isDir() )
		    fileDlg.SetButton( 1, dir2.first, dir2.second );
	}

	bool error = false;
	string buf;
	do {
		string p = fileDlg.Select(title,
		                          lastPath,
		                          pattern, filename );

		if (p.empty()) return p;

		lastPath = OnlyPath(p);

		if (p.find_first_of("#~$% ") != string::npos) {
			WriteAlert(_("Filename can't contain any "
			             "of these characters:"),
			           _("space, '#', '~', '$' or '%'."));
			error = true;
		} else {
			error = false;
			buf = p;
		}
	} while (error);

	return buf;
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
