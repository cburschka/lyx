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
using std::getline;
using std::istream;
using std::pair;
using std::sort;
using std::vector;

extern string fmt(char const * fmtstr ...);
extern string system_lyxdir;
extern Languages languages;

static string const colourFile("/usr/lib/X11/rgb.txt");
vector<FormPreferences::X11Colour> FormPreferences::colourDB;
pair<vector<string>, vector<string> > FormPreferences::dirlist;
    
FormPreferences::FormPreferences(LyXView * lv, Dialogs * d)
	: FormBaseBI(lv, d, _("Preferences"), new PreferencesPolicy),
	  dialog_(0),
	  converters_tab_(0), inputs_tab_(0), look_n_feel_tab_(0),
	  outputs_tab_(0),  usage_tab_(0),
	  colours_(0), converters_(0), formats_(0), inputs_misc_(0),
	  interface_(0), language_(0), lnf_misc_(0), outputs_misc_(0),
	  paths_(0), printer_(0), screen_fonts_(0), spellchecker_(0),
	  combo_default_lang(0), combo_kbmap_1(0), combo_kbmap_2(0),
	  feedbackObj(0)
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

	delete converters_tab_;
	delete inputs_tab_;
	delete look_n_feel_tab_;
	delete outputs_tab_;
	delete usage_tab_;

	delete colours_;
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

	// Must delete dialog last or we'll end up with a SIGSEGV trying to
	// access dialog_->timer_feedback in feedbackPost().
	delete dialog_;
}


FL_FORM * FormPreferences::form() const
{
	if (dialog_) return dialog_->form;
	return 0;
}


void FormPreferences::ok()
{
	FormBase::ok();
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
	buildColours();
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
			   _("Colours"),
			   colours_->form);
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

	applyColours();
	applyConverters();
	applyFormats();
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
	if( !ob ) return;

	string str;

	if (ob->form->fdui == colours_) {
		str = feedbackColours( ob );
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

	if (ob->form->fdui == colours_)
		return inputColours(ob);
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
	updateColours();
	updateConverters();
	updateFormats();
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


void FormPreferences::applyColours() const
{
}


void FormPreferences::buildColours()
{
	colours_ = build_colours();

	FL_OBJECT * obj = colours_->valslider_red;
	fl_set_slider_bounds(obj, 0, 255);
	fl_set_slider_precision(obj, 0);
	fl_set_slider_return(obj, FL_RETURN_END_CHANGED);
	
	obj = colours_->valslider_green;
	fl_set_slider_bounds(obj, 0, 255);
	fl_set_slider_precision(obj, 0);
	fl_set_slider_return(obj, FL_RETURN_END_CHANGED);
	
	obj = colours_->valslider_blue;
	fl_set_slider_bounds(obj, 0, 255);
	fl_set_slider_precision(obj, 0);
	fl_set_slider_return(obj, FL_RETURN_END_CHANGED);

	fl_set_object_color(colours_->button_colour,
			    FL_FREE_COL4, FL_FREE_COL4);
	
	fl_set_input_return(colours_->input_name, FL_RETURN_END_CHANGED);

	if (ColoursLoadBrowser(colourFile) )
		fl_set_input(colours_->input_name, colourFile.c_str());
	else
		fl_set_input(colours_->input_name, N_("No file found"));

	// deactivate the browse button because it isn't implemented
	fl_deactivate_object(colours_->button_browse);
	fl_set_object_lcol(colours_->button_browse, FL_INACTIVE);
}


string const
FormPreferences::feedbackColours(FL_OBJECT const * const ob) const
{
	Assert(ob);
	return string();
}


bool FormPreferences::inputColours( FL_OBJECT const * const ob )
{
	Assert(ob);
	bool activate = true;
	
	if (ob == colours_->browser_x11) {
		int i = fl_get_browser(colours_->browser_x11);
		if (i > 0) {
			ColoursUpdateBrowser(i-1);
		}

	} else if (ob == colours_->valslider_red
		   || ob == colours_->valslider_green
		   || ob == colours_->valslider_blue) {
		ColoursUpdateRGB();

	} else if (ob == colours_->input_name) {
		string file = fl_get_input(colours_->input_name);
		if (ColoursLoadBrowser(file) )
			fl_set_input(colours_->input_name, file.c_str());
		else if (ColoursLoadBrowser(colourFile) )
			fl_set_input(colours_->input_name, colourFile.c_str());
		else
			fl_set_input(colours_->input_name, N_("No file found"));
	}

	return activate;
}


bool FormPreferences::ColoursLoadBrowser(string const & filename)
{
	LyXLex lex(0, 0);

	if (!lex.setFile(filename))
		return false;

	istream & is = lex.getStream();
	string line;

	vector<RGB> cols;
	vector<string> names;
	
	while (true) {
		getline( is, line );
		if (line.empty() )
			break;

		if (line[0] != '!') {
			RGB col;
			string name;
			
			istringstream iss(line.c_str());
			iss >> col.r >> col.g >> col.b;
			while (iss.good()) {
				string next;
				iss >> next;
				if (!name.empty() ) name += " ";
				name += next;
			}

			// remove redundant entries on the fly
			bool add = cols.empty();
			if (!add) {
				vector<RGB>::const_iterator it = 
					find( cols.begin(), cols.end(), col );
				add = (it == cols.end());
			}
			
			if (add) {
				name = lowercase( name );
				if (name == "gray0" )   name = "black";
				if (name == "gray100" ) name = "white";

				if (name == "black" || name == "white") {
					cols.insert(cols.begin(), col);
					names.insert(names.begin(), name);
				} else {
					cols.push_back(col);
					names.push_back(name);
  				}
  			}
  		}
 	}

	vector<string>::iterator sit = names.begin();
	for (vector<RGB>::const_iterator iit = cols.begin();
	     iit != cols.end(); ++iit, ++sit) {
		colourDB.push_back( X11Colour(*sit, *iit) );
	}
	
	FL_OBJECT * colbr = colours_->browser_x11;
	fl_freeze_form(colours_->form);
	fl_clear_browser( colbr );

	for (vector<X11Colour>::const_iterator cit = colourDB.begin();
	     cit != colourDB.end(); ++cit) {
		string name = (*cit).first;
		//RGB col     = (*cit).second;
		//name += "  (" + tostr(col.r) + ", " + tostr(col.g) +
		//	", " + tostr(col.b) + ")";
		fl_addto_browser(colbr, name.c_str());
	}

	fl_set_browser_topline(colbr, 1);
	fl_select_browser_line(colbr, 1);
	ColoursUpdateBrowser(0);
	fl_unfreeze_form(colours_->form);
	
	return true;
}


int FormPreferences::ColoursSearchEntry(RGB const & col) const
{
	int mindiff = 0x7fffffff;
	vector<X11Colour>::const_iterator mincit = colourDB.begin();

	for (vector<X11Colour>::const_iterator cit = colourDB.begin();
	     cit != colourDB.end(); ++cit) {
		RGB colDB = (*cit).second;
		RGB diff;
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
	return static_cast<int>(mincit - colourDB.begin());
}


void FormPreferences::ColoursUpdateBrowser(int i)
{
	fl_freeze_form(colours_->form);

	RGB col = colourDB[i].second;
    
	fl_mapcolor(FL_FREE_COL4 + i, col.r, col.g, col.b);
	fl_mapcolor(FL_FREE_COL4, col.r, col.g, col.b);
	fl_set_slider_value(colours_->valslider_red,   col.r);
	fl_set_slider_value(colours_->valslider_green, col.g);
	fl_set_slider_value(colours_->valslider_blue,  col.b);
	fl_redraw_object(colours_->button_colour);

	fl_unfreeze_form(colours_->form);
}


void FormPreferences::ColoursUpdateRGB()
{
	fl_freeze_form(colours_->form);

	RGB col;
	col.r = int(fl_get_slider_value(colours_->valslider_red));
	col.g = int(fl_get_slider_value(colours_->valslider_green));
	col.b = int(fl_get_slider_value(colours_->valslider_blue));
    
	fl_mapcolor(FL_FREE_COL4, col.r, col.g, col.b);
	fl_redraw_object(colours_->button_colour);

	int const i = ColoursSearchEntry( col );
	// change topline only if necessary
	// int top = fl_get_browser_topline(colours_->browser_x11);
	// if (i < top || i > (top+15))
	fl_set_browser_topline(colours_->browser_x11, i - 5);
	fl_select_browser_line(colours_->browser_x11, i + 1);

	fl_unfreeze_form(colours_->form);
}


void FormPreferences::updateColours()
{}


void FormPreferences::applyConverters() const
{}


void FormPreferences::buildConverters()
{
	converters_ = build_converters();
	
	fl_set_input_return(converters_->input_converter, FL_RETURN_CHANGED);
	fl_set_input_return(converters_->input_flags, FL_RETURN_CHANGED);
	
	updateConverters();     
}


string const
FormPreferences::feedbackConverters( FL_OBJECT const * const ob) const
{
	Assert(ob);
	return string();
}


void FormPreferences::updateConverters()
{
	vector<Command> commands = Converter::GetAllCommands();
	
	vector<string> names;
	for(vector<Command>::const_iterator cit = commands.begin();
	     cit != commands.end(); ++cit) {
		string from = cit->from->prettyname;
		string to   = cit->to->prettyname;
		string name = from + " -> " + to;
		names.push_back(name);
	}
	sort(names.begin(), names.end());
	
	fl_clear_browser(converters_->browser_converters);
	for (vector<string>::const_iterator cit = names.begin();
	     cit != names.end(); ++cit)
		fl_addto_browser(converters_->browser_converters,
				 cit->c_str());
	fl_deactivate_object(converters_->button_add);
	fl_set_object_lcol(converters_->button_add, FL_INACTIVE);
	
	fl_deactivate_object(converters_->button_delete);
	fl_set_object_lcol(converters_->button_delete, FL_INACTIVE);
}


void FormPreferences::applyFormats() const
{
	vector<Format> old = formats.GetAllFormats();
	for (vector<Format>::const_iterator it = old.begin();
	     it != old.end(); ++it)
		if (find_if(formats_vec.begin(),formats_vec.end(),
			    compare_memfun(&Format::getname, it->name))
		    == formats_vec.end()) {
			formats.Add(it->name, string(), string(), string());
		}

	for (vector<Format>::const_iterator it = formats_vec.begin();
	     it != formats_vec.end(); ++it) {
		formats.Add(it->name, it->extension, it->prettyname, 
			    it->shortcut);
		formats.SetViewer(it->name, it->viewer);
	}
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

	updateFormats();
}


string const
FormPreferences::feedbackFormats(FL_OBJECT const * const ob) const
{
	Assert(ob);
	
	//if( !ob ) return;

	string str;

	return str;
}


bool FormPreferences::inputFormats(FL_OBJECT const * const ob)
{
	Assert(ob);
	
	//if( !ob ) return false;

	if (ob == formats_->browser_formats) {
		FormatsInputBrowser();

	} else if (ob == formats_->input_format) {
		FormatsInputInput();

	} else if (ob == formats_->button_add) {
		FormatsInputAdd();

	} else if (ob == formats_->button_delete) {
		FormatsInputDelete();
	}
	return true;
}


void FormPreferences::updateFormats()
{
	formats_vec = formats.GetAllFormats();

	vector<string> names;
	for (vector<Format>::const_iterator it = formats_vec.begin();
	     it != formats_vec.end(); ++it)
		names.push_back( it->name.c_str() );
	sort(names.begin(), names.end());

	fl_clear_browser(formats_->browser_formats);
	for (vector<string>::const_iterator it = names.begin();
	     it != names.end(); ++it)
		fl_addto_browser(formats_->browser_formats, it->c_str());

	fl_deactivate_object(formats_->button_add);
	fl_set_object_lcol(formats_->button_add, FL_INACTIVE);

	fl_deactivate_object(formats_->button_delete);
	fl_set_object_lcol(formats_->button_delete, FL_INACTIVE);
}


bool FormPreferences::FormatsInputAdd()
{
	string name = fl_get_input(formats_->input_format);
	string prettyname = fl_get_input(formats_->input_gui_name);
	string extension = fl_get_input(formats_->input_extension);
	string viewer =  fl_get_input(formats_->input_viewer);
	string shortcut =  fl_get_input(formats_->input_shrtcut);
	if (prettyname.empty())
		return false;

	Format format(name, extension, prettyname, shortcut, viewer);
	vector<Format>::iterator it =
		find_if(formats_vec.begin(), formats_vec.end(),
			compare_memfun(&Format::getname, name));

	if (it == formats_vec.end()) {
		fl_freeze_form(formats_->form);

		formats_vec.push_back(format);

		vector<string> names;
		for (vector<Format>::const_iterator it = formats_vec.begin();
		     it != formats_vec.end(); ++it)
			names.push_back( it->name.c_str() );

		sort( names.begin(), names.end() );

		fl_clear_browser( formats_->browser_formats );
		for (vector<string>::const_iterator it = names.begin();
		     it != names.end(); ++it)
			fl_addto_browser(formats_->browser_formats, 
					 it->c_str()); 

		fl_set_input(formats_->input_format, "");
		fl_set_input(formats_->input_gui_name, "");
		fl_set_input(formats_->input_shrtcut, "");
		fl_set_input(formats_->input_extension, "");
		fl_set_input(formats_->input_viewer, "");

		fl_set_object_label( formats_->button_add,
				     idex(_("Add|#A")) );
		fl_set_button_shortcut( formats_->button_add,
					scex(_("Add|#A")), 1);

		fl_deactivate_object(formats_->button_add);
		fl_set_object_lcol(formats_->button_add, FL_INACTIVE);

		fl_deactivate_object(formats_->button_delete);
		fl_set_object_lcol(formats_->button_delete, FL_INACTIVE);
			
		fl_unfreeze_form(formats_->form);

	} else {
		//if (*it == format)
		//	return false;
		*it = format;
	}
	return true;
}


bool FormPreferences::FormatsInputDelete()
{
	string name = fl_get_input(formats_->input_format);
	vector<Format>::iterator it =
		find_if(formats_vec.begin(), formats_vec.end(),
			compare_memfun(&Format::getname, name));

	fl_freeze_form(formats_->form);

	if (it == formats_vec.end()) {
		fl_deactivate_object(formats_->button_delete);
		fl_set_object_lcol(formats_->button_delete, FL_INACTIVE);

	} else {
		int sel = 0;
		for( int i = 0;
		     i < fl_get_browser_maxline(formats_->browser_formats); 
		     ++i ) {
			string str = fl_get_browser_line( formats_->browser_formats, i+1 );
			if( str == name ) {
				sel = i+1;
				break;
			}
		}
		
		if( sel != 0 ) {
			fl_delete_browser_line(formats_->browser_formats, sel );

			fl_set_input(formats_->input_format, "");
			fl_set_input(formats_->input_gui_name, "");
			fl_set_input(formats_->input_shrtcut, "");
			fl_set_input(formats_->input_extension, "");
			fl_set_input(formats_->input_viewer, "");

			fl_set_object_label( formats_->button_add,
					     idex(_("Add|#A")) );
			fl_set_button_shortcut( formats_->button_add,
						scex(_("Add|#A")), 1);

			fl_deactivate_object(formats_->button_add);
			fl_set_object_lcol(formats_->button_add, FL_INACTIVE);

			fl_deactivate_object(formats_->button_delete);
			fl_set_object_lcol(formats_->button_delete,
					   FL_INACTIVE);
		}
	}
	
	fl_unfreeze_form(formats_->form);

	return true;
}


bool FormPreferences::FormatsInputBrowser() 
{
	int const i = fl_get_browser(formats_->browser_formats);
	if( i <= 0 ) return true;

	string name = fl_get_browser_line(formats_->browser_formats, i);
	vector<Format>::iterator it =
		find_if(formats_vec.begin(), formats_vec.end(),
			compare_memfun(&Format::getname, name));

	if (it != formats_vec.end()) {
		fl_freeze_form(formats_->form);

		fl_set_input(formats_->input_format, it->name.c_str());
		fl_set_input(formats_->input_gui_name, it->prettyname.c_str());
		fl_set_input(formats_->input_shrtcut, it->shortcut.c_str());
		fl_set_input(formats_->input_extension, it->extension.c_str());
		fl_set_input(formats_->input_viewer, it->viewer.c_str());

		fl_set_object_label( formats_->button_add,
				     idex(_("Modify|#M")) );
		fl_set_button_shortcut( formats_->button_add, 
					scex(_("Modify|#M")), 1);

		fl_activate_object(formats_->button_add);
		fl_set_object_lcol(formats_->button_add, FL_BLACK);

		fl_activate_object(formats_->button_delete);
		fl_set_object_lcol(formats_->button_delete, FL_BLACK);
				
		fl_unfreeze_form(formats_->form);
	}
	return true;
}


bool FormPreferences::FormatsInputInput()
{
	string const name = fl_get_input(formats_->input_format);
	vector<Format>::iterator it =
		find_if(formats_vec.begin(), formats_vec.end(),
			compare_memfun(&Format::getname, name));

	fl_freeze_form(formats_->form);

	if (it == formats_vec.end()) {
		fl_set_object_label( formats_->button_add,
				     idex(_("Add|#A")) );
		fl_set_button_shortcut( formats_->button_add,
					scex(_("Add|#A")), 1);

		if( name.empty() ) {
			fl_deactivate_object(formats_->button_add);
			fl_set_object_lcol(formats_->button_add, FL_INACTIVE);
		} else {
			fl_activate_object(formats_->button_add);
			fl_set_object_lcol(formats_->button_add, FL_BLACK);
		}

		fl_deselect_browser(formats_->browser_formats);

		fl_deactivate_object(formats_->button_delete);
		fl_set_object_lcol(formats_->button_delete, FL_INACTIVE);

	} else {
		fl_set_object_label( formats_->button_add,
				     idex(_("Modify|#M")) );
		fl_set_button_shortcut( formats_->button_add,
					scex(_("Modify|#M")), 1);

		int sel = 0;
		for( int i = 0;
		     i < fl_get_browser_maxline(formats_->browser_formats);
		     ++i ) {
			string str = fl_get_browser_line( formats_->browser_formats, i+1 );
			if( str == name ) {
				sel = i+1;
				break;
			}
		}

		int top = sel - 6;
		if( top < 1 ) top = 0;
		fl_set_browser_topline(formats_->browser_formats, top);
		fl_select_browser_line(formats_->browser_formats, sel);
		
		fl_activate_object(formats_->button_add);
		fl_set_object_lcol(formats_->button_add, FL_BLACK);

		fl_activate_object(formats_->button_delete);
		fl_set_object_lcol(formats_->button_delete, FL_BLACK);
	}

	fl_unfreeze_form(formats_->form);
	return true;
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
	fl_addto_form(inputs_misc_->form);

	setPostHandler( inputs_misc_->input_date_format );

	fl_end_form();
}


string const
FormPreferences::feedbackInputsMisc(FL_OBJECT const * const ob) const
{
	Assert(ob);
	
	//if( !ob ) return string();

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
	fl_addto_form(interface_->form);

	setPostHandler( interface_->input_popup_font );
	setPostHandler( interface_->input_menu_font );
	setPostHandler( interface_->input_popup_encoding );
	setPostHandler( interface_->input_bind_file );
	setPostHandler( interface_->button_bind_file_browse );
	setPostHandler( interface_->input_ui_file );
	setPostHandler( interface_->button_ui_file_browse );
	setPostHandler( interface_->check_override_x_dead_keys );

	fl_end_form();
}


string const
FormPreferences::feedbackInterface( FL_OBJECT const * const ob ) const
{
	Assert(ob);
	
	//if( !ob ) return string();

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
		vector<string>::iterator it =
			find( dirlist.second.begin(), dirlist.second.end(),
			      name_1 );
		vector<string>::size_type sel = it - dirlist.second.begin();
		lyxrc.primary_kbmap = dirlist.first[sel];

		it = find( dirlist.second.begin(), dirlist.second.end(),
			   name_2 );
		sel = it - dirlist.second.begin();
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
	    cit != languages.end(); cit++) {
		combo_default_lang->addto((*cit).second.lang());
	}

	// ditto kbmap_1
	string dir = AddPath(system_lyxdir, "kbd");
	vector<string> dirlist = DirList(dir , "kmap");

	obj = language_->choice_kbmap_1;
	fl_deactivate_object(language_->choice_kbmap_1);
	combo_kbmap_1 = new Combox(FL_COMBOX_DROPLIST);
	combo_kbmap_1->add(obj->x, obj->y, obj->w, obj->h, 400);
	combo_kbmap_1->shortcut("#1",1);
	combo_kbmap_1->setcallback(ComboLanguageCB, this);

	for (vector<string>::const_iterator cit = dirlist.begin();
	    cit != dirlist.end(); cit++) {
		combo_kbmap_1->addto(*cit);
	}

	// ditto kbmap_2
	obj = language_->choice_kbmap_2;
	fl_deactivate_object(language_->choice_kbmap_2);
	combo_kbmap_2 = new Combox(FL_COMBOX_DROPLIST);
	combo_kbmap_2->add(obj->x, obj->y, obj->w, obj->h, 400);
	combo_kbmap_2->shortcut("#2",1);
	combo_kbmap_2->setcallback(ComboLanguageCB, this);

	for (vector<string>::const_iterator cit = dirlist.begin();
	    cit != dirlist.end(); cit++) {
		combo_kbmap_2->addto(*cit);
	}

	fl_end_form();
	fl_unfreeze_form(language_->form);

	// set up the feedback mechanism
	fl_addto_form(language_->form);

	setPostHandler( language_->input_package );
	setPostHandler( language_->check_use_kbmap );

	// This is safe, as nothing is done to the pointer, other than
	// to use its address in a block-if statement.
	//setPostHandler( reinterpret_cast<FL_OBJECT *>(combo_default_lang) );
	//setPostHandler( reinterpret_cast<FL_OBJECT *>(combo_kbmap_1) );
	//setPostHandler( reinterpret_cast<FL_OBJECT *>(combo_kbmap_2) );

	setPostHandler( language_->check_rtl_support );
	setPostHandler( language_->check_mark_foreign );
	setPostHandler( language_->check_auto_begin );
	setPostHandler( language_->check_auto_end );
	setPostHandler( language_->input_command_begin );
	setPostHandler( language_->input_command_end );

	fl_end_form();

	// Activate/Deactivate the input fields dependent on the state of the
	// buttons.
	inputLanguage( 0 );
}


string const
FormPreferences::feedbackLanguage(FL_OBJECT const * const ob) const
{
	Assert(ob);
	
	//if( !ob ) return string();

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
		vector<string>::iterator it =
			find( dirlist.first.begin(), dirlist.first.end(),
			      fullpath );
		vector<string>::size_type sel = it - dirlist.first.begin();
		combo_kbmap_1->select_text( dirlist.second[sel] );

		fullpath = lyxrc.secondary_kbmap;
		it = find( dirlist.first.begin(), dirlist.first.end(),
			   fullpath );
		sel = it - dirlist.first.begin();
		combo_kbmap_2->select_text( dirlist.second[sel] );
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
	fl_addto_form(lnf_misc_->form);

	setPostHandler( lnf_misc_->check_banner );
	setPostHandler( lnf_misc_->check_auto_region_delete );
	setPostHandler( lnf_misc_->check_exit_confirm );
	setPostHandler( lnf_misc_->check_display_shrtcuts );
	setPostHandler( lnf_misc_->counter_autosave );
	setPostHandler( lnf_misc_->check_ask_new_file );
	setPostHandler( lnf_misc_->check_cursor_follows_scrollbar );
	setPostHandler( lnf_misc_->counter_wm_jump );

	fl_end_form();
}


string const FormPreferences::feedbackLnFmisc(FL_OBJECT const * const ob) const
{
	Assert(ob);
	
	//if( !ob ) return string();

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
	fl_addto_form(outputs_misc_->form);

	setPostHandler( outputs_misc_->counter_line_len );
	setPostHandler( outputs_misc_->input_tex_encoding );
	setPostHandler( outputs_misc_->choice_default_papersize );
	setPostHandler( outputs_misc_->input_ascii_roff );
	setPostHandler( outputs_misc_->input_checktex );

	fl_end_form();
}


string const
FormPreferences::feedbackOutputsMisc(FL_OBJECT const * const ob) const
{
	Assert(ob);
	
	//if( !ob ) return string();

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
	fl_addto_form(paths_->form);

	setPostHandler( paths_->input_default_path );
	setPostHandler( paths_->button_document_browse );
	setPostHandler( paths_->counter_lastfiles );
	setPostHandler( paths_->input_template_path );
	setPostHandler( paths_->button_template_browse );
	setPostHandler( paths_->check_last_files );
	setPostHandler( paths_->button_temp_dir_browse );
	setPostHandler( paths_->input_lastfiles );
	setPostHandler( paths_->button_lastfiles_browse );
	setPostHandler( paths_->check_make_backups );
	setPostHandler( paths_->input_backup_path );
	setPostHandler( paths_->button_backup_path_browse );
	setPostHandler( paths_->input_serverpipe );
	setPostHandler( paths_->button_serverpipe_browse );
	setPostHandler( paths_->input_temp_dir );
	setPostHandler( paths_->check_use_temp_dir );

	fl_end_form();
}


string const FormPreferences::feedbackPaths( FL_OBJECT const * const ob ) const
{
	Assert(ob);
	
	//if( !ob ) return string();

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
	// what kind of coding is this? Do you plan for a ob == NULL?
	// When is that allowed? (Lgb)
	//Assert(ob);
	
	bool activate = true;
	
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
	Assert(ob);
	
	//if( !ob ) return string();

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
	fl_addto_form(printer_->form);

	setPostHandler( printer_->input_command );
	setPostHandler( printer_->input_page_range );
	setPostHandler( printer_->input_copies );
	setPostHandler( printer_->input_reverse );
	setPostHandler( printer_->input_to_printer );
	setPostHandler( printer_->input_file_extension );
	setPostHandler( printer_->input_spool_command );
	setPostHandler( printer_->input_paper_type );
	setPostHandler( printer_->input_even_pages );
	setPostHandler( printer_->input_odd_pages );
	setPostHandler( printer_->input_collated );
	setPostHandler( printer_->input_landscape );
	setPostHandler( printer_->input_to_file );
	setPostHandler( printer_->input_extra_options );
	setPostHandler( printer_->input_spool_prefix );
	setPostHandler( printer_->input_paper_size );
	setPostHandler( printer_->input_name );
	setPostHandler( printer_->check_adapt_output );

	fl_end_form();
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
	fl_addto_form(screen_fonts_->form);

	setPostHandler( screen_fonts_->input_roman );
	setPostHandler( screen_fonts_->input_sans );
	setPostHandler( screen_fonts_->input_typewriter );
	setPostHandler( screen_fonts_->counter_zoom );
	setPostHandler( screen_fonts_->counter_dpi );
	setPostHandler( screen_fonts_->check_scalable );
	setPostHandler( screen_fonts_->input_screen_encoding );
	setPostHandler( screen_fonts_->input_tiny );
	setPostHandler( screen_fonts_->input_script );
	setPostHandler( screen_fonts_->input_footnote );
	setPostHandler( screen_fonts_->input_small );
	setPostHandler( screen_fonts_->input_large );
	setPostHandler( screen_fonts_->input_larger );
	setPostHandler( screen_fonts_->input_largest );
	setPostHandler( screen_fonts_->input_normal );
	setPostHandler( screen_fonts_->input_huge );
	setPostHandler( screen_fonts_->input_huger );

	fl_end_form();
}

	
string const
FormPreferences::feedbackScreenFonts(FL_OBJECT const * const ob ) const
{
	Assert(ob);
	
	//if( !ob ) return string();

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

	if (!activate) {
		fl_set_object_label(dialog_->text_warning, str.c_str());
		fl_set_object_lsize(dialog_->text_warning, FL_SMALL_SIZE);
	}
	
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

	string choice = "none";
	switch (fl_get_choice(spellchecker_->choice_spell_command)) {
	case 1:
		choice = "none";
		break;
	case 2:
		choice = "ispell";
		break;
	case 3:
		choice = "aspell";
		break;
	default:
		break;
	}
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
	fl_addto_form(spellchecker_->form);

	setPostHandler( spellchecker_->choice_spell_command );
	setPostHandler( spellchecker_->check_alt_lang );
	setPostHandler( spellchecker_->input_alt_lang );
	setPostHandler( spellchecker_->check_escape_chars );
	setPostHandler( spellchecker_->input_escape_chars );
	setPostHandler( spellchecker_->check_personal_dict );
	setPostHandler( spellchecker_->input_personal_dict );
	setPostHandler( spellchecker_->button_personal_dict );
	setPostHandler( spellchecker_->check_compound_words );
	setPostHandler( spellchecker_->check_input_enc );

	fl_end_form();
}


string const
FormPreferences::feedbackSpellChecker(FL_OBJECT const * const ob) const
{
	Assert(ob);
	
	//if( !ob ) return string();

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
	// ditto ob! (Lgb)
	
	// Allow/dissallow input

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


bool FormPreferences::WriteableDir(string const & name) const
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

	if (!success) {
		fl_set_object_label(dialog_->text_warning, str.c_str());
		fl_set_object_lsize(dialog_->text_warning, FL_SMALL_SIZE);
	}
	
	return success;
}


bool FormPreferences::ReadableDir(string const & name) const
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

	if (!success) {
		fl_set_object_label(dialog_->text_warning, str.c_str());
		fl_set_object_lsize(dialog_->text_warning, FL_SMALL_SIZE);
	}

	return success;
}


bool FormPreferences::WriteableFile(string const & name,
				    string const & suffix) const
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
	
	if (!success) {
		fl_set_object_label(dialog_->text_warning, str.c_str());
		fl_set_object_lsize(dialog_->text_warning, FL_SMALL_SIZE);
	}

	return success;
}


void FormPreferences::ComboLanguageCB(int, void * v, Combox * combox)
{
	Assert(v && combox);
	FormPreferences * pre = static_cast<FormPreferences*>(v);
	// This is safe, as nothing is done to the pointer, other than
	// to use its address in a block-if statement.
	pre->bc_.valid(pre->input(reinterpret_cast<FL_OBJECT *>(combox), 0));
}


// C functions for the timer callback used to give the user feedback
C_GENERICCB(FormPreferences, FeedbackCB)

	
void FormPreferences::FeedbackCB(FL_OBJECT * ob, long)
{
	Assert(ob);
	
	//if( !ob ) return;

	FormPreferences * pre =
		static_cast<FormPreferences*>(ob->form->u_vdata);
	Assert(pre);
	
	pre->feedback( pre->feedbackObj );
}


extern "C"
int C_FormPreferencesFeedbackPost(FL_OBJECT * ob, int event,
				  FL_Coord, FL_Coord, int, void *)
{
	// can occur when form is being deleted. This seems an easier fix than
	// a call "fl_set_object_posthandler(ob, 0)" for each and every object
	// in the destructor.
	if (!ob ) return 0;
	if (!ob->form ) return 0;

	FormPreferences * pre =
		static_cast<FormPreferences*>(ob->form->u_vdata);
	pre->feedbackPost(ob, event);
	return 0;
}


// post_handler for feedback messages
void FormPreferences::feedbackPost(FL_OBJECT * ob, int event)
{
	Assert(ob);
	
	//if( !ob ) return;
	
	// We do not test for empty help here, since this can never happen
	if (event == FL_ENTER) {
		// Used as a placeholder for ob, so that we don't have to
		// a horrible reinterpret_cast to long and pass it as an
		// argument in fl_set_object_callback.
		feedbackObj = ob;
		fl_set_object_callback(dialog_->timer_feedback,
				       C_FormPreferencesFeedbackCB,
				       0);
		fl_set_timer(dialog_->timer_feedback, 0.5);
	} else if (event != FL_MOTION) {
		fl_set_timer(dialog_->timer_feedback, 0);
		feedbackObj = 0;
		fl_set_object_label(dialog_->text_warning, "");
	}
}


void FormPreferences::setPostHandler(FL_OBJECT * ob) const
{
	Assert(ob);
	
	//if( !ob ) return;

	fl_set_object_posthandler(ob, C_FormPreferencesFeedbackPost);
}
