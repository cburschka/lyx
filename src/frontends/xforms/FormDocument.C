/**
 * \file FormDocument.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 * \author Rob Lahaye
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "ControlDocument.h"
#include "FormDocument.h"
#include "forms/form_document.h"
#include "xformsBC.h"
#include "ButtonController.h"

#include "bmtable.h"
#include "checkedwidgets.h"
#include "input_validators.h" // fl_unsigned_float_filter
#include "xforms_helpers.h"

#include "CutAndPaste.h"
#include "debug.h"
#include "language.h"
#include "lyxrc.h"
#include "lyxtextclasslist.h"
#include "tex-strings.h"

#include "controllers/frnt_lang.h"
#include "controllers/helper_funcs.h"

#include "support/tostr.h"
#include "support/lstrings.h" // contains_functor, getStringFromVector
#include "support/filetools.h" // LibFileSearch

#include "lyx_xpm.h"
#include "lyx_forms.h"
#include "combox.h"

#include <boost/bind.hpp>

#include <functional>

using std::bind2nd;
using std::vector;


namespace {

#if FL_VERSION == 0 || (FL_REVISION == 0 && FL_FIXLEVEL < 2)
bool const scalableTabfolders = false;
#else
bool const scalableTabfolders = true;
#endif

} // namespace anon


typedef FormCB<ControlDocument, FormDB<FD_document> > base_class;

FormDocument::FormDocument()
	: base_class(_("Document Settings"), scalableTabfolders),
	  ActCell(0), Confirmed(0),
	  current_bullet_panel(0), current_bullet_depth(0), fbullet(0)
{}


void FormDocument::redraw()
{
	if (form() && form()->visible)
		fl_redraw_form(form());
	else
		return;

	FL_FORM * outer_form = fl_get_active_folder(dialog_->tabfolder);
	if (outer_form && outer_form->visible)
		fl_redraw_form(outer_form);
}


void FormDocument::build()
{
	// the tabbed folder
	dialog_.reset(build_document(this));

	// Allow the base class to control messages
	setMessageWidget(dialog_->text_warning);

	// Manage the restore, ok, apply, restore and cancel/close buttons
	bcview().setOK(dialog_->button_ok);
	bcview().setApply(dialog_->button_apply);
	bcview().setCancel(dialog_->button_close);
	bcview().setRestore(dialog_->button_restore);

	// the document class form
	class_.reset(build_document_class(this));

	// disable for read-only documents
	bcview().addReadOnly(class_->combox_class);
	bcview().addReadOnly(class_->radio_indent);
	bcview().addReadOnly(class_->radio_skip);
	bcview().addReadOnly(class_->choice_pagestyle);
	bcview().addReadOnly(class_->choice_fonts);
	bcview().addReadOnly(class_->choice_fontsize);
	bcview().addReadOnly(class_->radio_sides_one);
	bcview().addReadOnly(class_->radio_sides_two);
	bcview().addReadOnly(class_->radio_columns_one);
	bcview().addReadOnly(class_->radio_columns_two);
	bcview().addReadOnly(class_->input_extra);
	bcview().addReadOnly(class_->input_skip);
	bcview().addReadOnly(class_->choice_skip);
	bcview().addReadOnly(class_->choice_spacing);
	bcview().addReadOnly(class_->input_spacing);

	// check validity of "length + unit" input
	addCheckedGlueLength(bcview(), class_->input_skip,
			     class_->choice_skip);

	// trigger an input event for cut&paste with middle mouse button.
	setPrehandler(class_->input_extra);
	setPrehandler(class_->input_skip);
	setPrehandler(class_->input_spacing);

	fl_set_input_return(class_->input_extra, FL_RETURN_CHANGED);
	fl_set_input_return(class_->input_skip, FL_RETURN_CHANGED);
	fl_set_input_return(class_->input_spacing, FL_RETURN_CHANGED);

	FL_OBJECT * obj;

	// Fill the combox and choices.
	obj = class_->combox_class;
	LyXTextClassList::const_iterator tit  = textclasslist.begin();
	LyXTextClassList::const_iterator tend = textclasslist.end();
	for (; tit != tend; ++tit) {
		if (tit->isTeXClassAvailable()) {
			fl_addto_combox(obj, tit->description().c_str());
		} else {
			string item = bformat(_("Unavailable: %1$s"), tit->description());
			fl_addto_combox(obj, item.c_str());
		}
	}
	fl_set_combox_browser_height(obj, 400);

	fl_addto_choice(class_->choice_spacing,
			_(" Single | OneHalf | Double | Custom ").c_str());
	fl_addto_choice(class_->choice_fontsize, "default|10|11|12");
	for (int n = 0; tex_fonts[n][0]; ++n) {
		fl_addto_choice(class_->choice_fonts,tex_fonts[n]);
	}

	// Create the contents of the unit choices; don't include the
	// "%" terms...
	vector<string> units_vec = getLatexUnits();
	vector<string>::iterator ret =
		std::remove_if(units_vec.begin(),
			       units_vec.end(),
			       bind2nd(contains_functor(), "%"));
	units_vec.erase(ret, units_vec.end());

	string const units = getStringFromVector(units_vec, "|");

	fl_addto_choice(class_->choice_pagestyle,
			"default|empty|plain|headings|fancy");
	fl_addto_choice(class_->choice_skip,
			_(" SmallSkip | MedSkip | BigSkip | Length ").c_str());
	fl_addto_choice(class_->choice_skip_units,  units.c_str());

	// Set input filters on doc spacing to make it accept only
	// unsigned numbers.
	fl_set_input_filter(class_->input_spacing,
			    fl_unsigned_float_filter);

	// disable for read-only documents
	bcview().addReadOnly(dialog_->button_save_defaults);
	bcview().addReadOnly(dialog_->button_reset_defaults);

	// the document paper form
	paper_.reset(build_document_paper(this));

	// disable for read-only documents
	bcview().addReadOnly(paper_->choice_paperpackage);
	bcview().addReadOnly(paper_->radio_portrait);
	bcview().addReadOnly(paper_->radio_landscape);
	bcview().addReadOnly(paper_->choice_papersize);
	bcview().addReadOnly(paper_->check_use_geometry);
	bcview().addReadOnly(paper_->input_custom_width);
	bcview().addReadOnly(paper_->input_custom_height);
	bcview().addReadOnly(paper_->input_top_margin);
	bcview().addReadOnly(paper_->input_bottom_margin);
	bcview().addReadOnly(paper_->input_inner_margin);
	bcview().addReadOnly(paper_->input_outer_margin);
	bcview().addReadOnly(paper_->input_head_height);
	bcview().addReadOnly(paper_->input_head_sep);
	bcview().addReadOnly(paper_->input_foot_skip);

	// check validity of "length + unit" input
	addCheckedGlueLength(bcview(), paper_->input_custom_width);
	addCheckedGlueLength(bcview(), paper_->input_custom_height);
	addCheckedGlueLength(bcview(), paper_->input_top_margin);
	addCheckedGlueLength(bcview(), paper_->input_bottom_margin);
	addCheckedGlueLength(bcview(), paper_->input_inner_margin);
	addCheckedGlueLength(bcview(), paper_->input_outer_margin);
	addCheckedGlueLength(bcview(), paper_->input_head_height);
	addCheckedGlueLength(bcview(), paper_->input_head_sep);
	addCheckedGlueLength(bcview(), paper_->input_foot_skip);

	// trigger an input event for cut&paste with middle mouse button.
	setPrehandler(paper_->input_custom_width);
	setPrehandler(paper_->input_custom_height);
	setPrehandler(paper_->input_top_margin);
	setPrehandler(paper_->input_bottom_margin);
	setPrehandler(paper_->input_inner_margin);
	setPrehandler(paper_->input_outer_margin);
	setPrehandler(paper_->input_head_height);
	setPrehandler(paper_->input_head_sep);
	setPrehandler(paper_->input_foot_skip);

	fl_set_input_return(paper_->input_custom_width,  FL_RETURN_CHANGED);
	fl_set_input_return(paper_->input_custom_height, FL_RETURN_CHANGED);
	fl_set_input_return(paper_->input_top_margin,    FL_RETURN_CHANGED);
	fl_set_input_return(paper_->input_bottom_margin, FL_RETURN_CHANGED);
	fl_set_input_return(paper_->input_inner_margin,  FL_RETURN_CHANGED);
	fl_set_input_return(paper_->input_outer_margin,  FL_RETURN_CHANGED);
	fl_set_input_return(paper_->input_head_height,   FL_RETURN_CHANGED);
	fl_set_input_return(paper_->input_head_sep,      FL_RETURN_CHANGED);
	fl_set_input_return(paper_->input_foot_skip,     FL_RETURN_CHANGED);

	fl_addto_choice(paper_->choice_papersize,
			_(" Default | Custom | US letter | US legal "
			  "| US executive | A3 | A4 | A5 "
			  "| B3 | B4 | B5 ").c_str());
	fl_addto_choice(paper_->choice_paperpackage,
			_(" None "
			  "| Small Margins "
			  "| Very small Margins "
			  "| Very wide Margins ").c_str());

	fl_addto_choice(paper_->choice_custom_width_units,  units.c_str());
	fl_addto_choice(paper_->choice_custom_height_units, units.c_str());
	fl_addto_choice(paper_->choice_top_margin_units,    units.c_str());
	fl_addto_choice(paper_->choice_bottom_margin_units, units.c_str());
	fl_addto_choice(paper_->choice_inner_margin_units,  units.c_str());
	fl_addto_choice(paper_->choice_outer_margin_units,  units.c_str());
	fl_addto_choice(paper_->choice_head_height_units,   units.c_str());
	fl_addto_choice(paper_->choice_head_sep_units,      units.c_str());
	fl_addto_choice(paper_->choice_foot_skip_units,     units.c_str());

	// the document language form
	language_.reset(build_document_language(this));

	// disable for read-only documents
	bcview().addReadOnly(language_->combox_language);
	bcview().addReadOnly(language_->choice_inputenc);
	bcview().addReadOnly(language_->choice_quotes_language);
	bcview().addReadOnly(language_->radio_single);
	bcview().addReadOnly(language_->radio_double);

	fl_addto_choice(language_->choice_inputenc,
			"default|auto|latin1|latin2|latin3|latin4|latin5|latin9"
			"|koi8-r|koi8-u|cp866|cp1251|iso88595|pt154");

	vector<frnt::LanguagePair> const langs = frnt::getLanguageData(false);
	// Store the identifiers for later
	lang_ = getSecond(langs);

	vector<frnt::LanguagePair>::const_iterator lit  = langs.begin();
	vector<frnt::LanguagePair>::const_iterator lend = langs.end();
	for (; lit != lend; ++lit) {
		fl_addto_combox(language_->combox_language,
				lit->first.c_str());
	}
	fl_set_combox_browser_height(language_->combox_language, 400);

	fl_addto_choice(language_->choice_quotes_language,
			_(" ``text'' | ''text'' | ,,text`` | ,,text'' |"
			  " «text» | »text« ").c_str());

	// the document options form
	options_.reset(build_document_options(this));

	// disable for read-only documents
	bcview().addReadOnly(options_->counter_secnumdepth);
	bcview().addReadOnly(options_->counter_tocdepth);
	bcview().addReadOnly(options_->choice_ams_math);
	bcview().addReadOnly(options_->check_use_natbib);
	bcview().addReadOnly(options_->choice_citation_format);
	bcview().addReadOnly(options_->input_float_placement);
	bcview().addReadOnly(options_->choice_postscript_driver);

	// trigger an input event for cut&paste with middle mouse button.
	setPrehandler(options_->input_float_placement);

	fl_set_input_return(options_->input_float_placement, FL_RETURN_CHANGED);

	fl_addto_choice(options_->choice_ams_math,
			_("Never | Automatically | Yes ").c_str());

	for (int n = 0; tex_graphics[n][0]; ++n) {
		fl_addto_choice(options_->choice_postscript_driver,
				tex_graphics[n]);
	}
	fl_addto_choice(options_->choice_citation_format,
			_(" Author-year | Numerical ").c_str());

	// the document bullets form
	bullets_.reset(build_document_bullet(this));

	// disable for read-only documents
	bcview().addReadOnly(bullets_->radio_depth_1);
	bcview().addReadOnly(bullets_->radio_depth_2);
	bcview().addReadOnly(bullets_->radio_depth_3);
	bcview().addReadOnly(bullets_->radio_depth_4);
	bcview().addReadOnly(bullets_->radio_panel_standard);
	bcview().addReadOnly(bullets_->radio_panel_maths);
	bcview().addReadOnly(bullets_->radio_panel_ding1);
	bcview().addReadOnly(bullets_->radio_panel_ding2);
	bcview().addReadOnly(bullets_->radio_panel_ding3);
	bcview().addReadOnly(bullets_->radio_panel_ding4);
	bcview().addReadOnly(bullets_->bmtable_panel);
	bcview().addReadOnly(bullets_->choice_size);
	bcview().addReadOnly(bullets_->input_latex);

	// trigger an input event for cut&paste with middle mouse button.
	setPrehandler(bullets_->input_latex);

	fl_set_input_return(bullets_->input_latex, FL_RETURN_CHANGED);

	fl_addto_choice(bullets_->choice_size,
			_(" Default | Tiny | Smallest | Smaller "
			  "| Small | Normal | Large | Larger | Largest "
			  "| Huge | Huger ").c_str());
	fl_set_choice(bullets_->choice_size, 1);

	fl_set_input_maxchars(bullets_->input_latex, 80);

	string const bmtablefile = LibFileSearch("images", "standard", "xpm");
	fl_set_bmtable_pixmap_file(bullets_->bmtable_panel, 6, 6,
				   bmtablefile.c_str());

	// Enable the tabfolder to be rescaled correctly.
	if (scalableTabfolders)
		fl_set_tabfolder_autofit(dialog_->tabfolder, FL_FIT);

	// Stack tabs
	fl_addto_tabfolder(dialog_->tabfolder,_("Document").c_str(),
			   class_->form);
	fl_addto_tabfolder(dialog_->tabfolder,_("Paper").c_str(),
			   paper_->form);
	fl_addto_tabfolder(dialog_->tabfolder,_("Language").c_str(),
			   language_->form);
	fl_addto_tabfolder(dialog_->tabfolder,_("Extra").c_str(),
			   options_->form);
	fbullet = fl_addto_tabfolder(dialog_->tabfolder,_("Bullets").c_str(),
				     bullets_->form);

	if ((XpmVersion < 4) || (XpmVersion == 4 && XpmRevision < 7)) {
		lyxerr << _("Your version of libXpm is older than 4.7.\n"
			    "The `bullet' tab of the document dialog "
			    "has been disabled") << '\n';
		fl_deactivate_object(fbullet);
		fl_set_object_lcol(fbullet, FL_INACTIVE);
	}
}


void FormDocument::apply()
{
	BufferParams & params = controller().params();

	class_apply(params);
	paper_apply(params);
	language_apply(params);
	options_apply(params);
	bullets_apply(params);
}


void FormDocument::update()
{
	if (!dialog_.get())
		return;

	checkReadOnly();

	BufferParams const & params = controller().params();

	class_update(params);
	paper_update(params);
	language_update(params);
	options_update(params);
	bullets_update(params);
}


ButtonPolicy::SMInput FormDocument::input(FL_OBJECT * ob, long)
{
	if (ob == bullets_->choice_size) {
		ChoiceBulletSize(ob, 0);

	} else if (ob == bullets_->input_latex) {
		InputBulletLaTeX(ob, 0);

	} else if (ob == bullets_->radio_depth_1 ||
		   ob == bullets_->radio_depth_2 ||
		   ob == bullets_->radio_depth_3 ||
		   ob == bullets_->radio_depth_4) {
		BulletDepth(ob);

	} else if (ob == bullets_->radio_panel_standard ||
		   ob == bullets_->radio_panel_maths ||
		   ob == bullets_->radio_panel_ding1 ||
		   ob == bullets_->radio_panel_ding2 ||
		   ob == bullets_->radio_panel_ding3 ||
		   ob == bullets_->radio_panel_ding4) {
		BulletPanel(ob);

	} else if (ob == bullets_->bmtable_panel) {
		BulletBMTable(ob, 0);

	} else if (ob == class_->choice_spacing) {
		setEnabled(class_->input_spacing,
			   fl_get_choice(class_->choice_spacing) == 4);

	} else if (ob == class_->combox_class) {
		CheckChoiceClass();
	} else if (ob == class_->radio_skip ||
		   ob == class_->radio_indent ||
		   ob == class_->choice_skip) {
		bool const skip_used = fl_get_button(class_->radio_skip);
		setEnabled(class_->choice_skip, skip_used);

		bool const length_input =
			fl_get_choice(class_->choice_skip) == 4;
		setEnabled(class_->input_skip,
			   skip_used && length_input);
		setEnabled(class_->choice_skip_units,
			   skip_used && length_input);

		// Default unit choice is cm if metric, inches if US paper.
		// If papersize is default, check the lyxrc-settings
		int const paperchoice = fl_get_choice(paper_->choice_papersize);
		bool const metric = (paperchoice == 1 && lyxrc.default_papersize > BufferParams::PAPER_EXECUTIVEPAPER)
			|| paperchoice == 2 || paperchoice > 5;
		string const default_unit = metric ? "cm" : "in";
		if (getString(class_->input_skip).empty())
			fl_set_choice_text(class_->choice_skip_units,
					   default_unit.c_str());

	} else if (ob == options_->check_use_natbib) {
		setEnabled(options_->choice_citation_format,
			   fl_get_button(options_->check_use_natbib));

	} else if (ob == dialog_->button_save_defaults) {
		apply();
		controller().saveAsDefault();

	} else if (ob == dialog_->button_reset_defaults) {
		BufferParams & params = controller().params();
		params.textclass = fl_get_combox(class_->combox_class) - 1;
		params.useClassDefaults();
		UpdateLayoutDocument(params);

	} else if (ob == paper_->radio_landscape) {
		fl_set_choice(paper_->choice_paperpackage,
			      BufferParams::PACKAGE_NONE + 1);

	} else if (ob == paper_->choice_papersize) {
		int const paperchoice = fl_get_choice(paper_->choice_papersize);
		bool const defsize = paperchoice == 1;
		bool const custom = paperchoice == 2;
		bool const a3size = paperchoice == 6;
		bool const b3size = paperchoice == 9;
		bool const b4size = paperchoice == 10;

		if (custom)
			fl_set_button(paper_->radio_portrait, 1);

		bool const use_geom = (custom || a3size || b3size || b4size);

		fl_set_button(paper_->check_use_geometry, int(use_geom));

		setEnabled(paper_->input_top_margin,    use_geom);
		setEnabled(paper_->input_bottom_margin, use_geom);
		setEnabled(paper_->input_inner_margin,  use_geom);
		setEnabled(paper_->input_outer_margin,  use_geom);
		setEnabled(paper_->input_head_height,   use_geom);
		setEnabled(paper_->input_head_sep,      use_geom);
		setEnabled(paper_->input_foot_skip,     use_geom);
		setEnabled(paper_->choice_top_margin_units,    use_geom);
		setEnabled(paper_->choice_bottom_margin_units, use_geom);
		setEnabled(paper_->choice_inner_margin_units,  use_geom);
		setEnabled(paper_->choice_outer_margin_units,  use_geom);
		setEnabled(paper_->choice_head_height_units,   use_geom);
		setEnabled(paper_->choice_head_sep_units,      use_geom);
		setEnabled(paper_->choice_foot_skip_units,     use_geom);
		setEnabled(paper_->choice_custom_width_units,  use_geom);
		setEnabled(paper_->choice_custom_height_units, use_geom);

		setEnabled(paper_->input_custom_width,  custom);
		setEnabled(paper_->input_custom_height, custom);
		setEnabled(paper_->choice_custom_width_units,  custom);
		setEnabled(paper_->choice_custom_height_units, custom);
		setEnabled(paper_->radio_portrait,  !custom);
		setEnabled(paper_->radio_landscape, !custom);

		// Default unit choice is cm if metric, inches if US paper.
		// If papersize is default, use the lyxrc-settings
		bool const metric = (defsize && lyxrc.default_papersize > BufferParams::PAPER_EXECUTIVEPAPER)
			|| paperchoice == 2 || paperchoice > 5;
		string const default_unit = metric ? "cm" : "in";
		if (getString(paper_->input_custom_width).empty())
			fl_set_choice_text(paper_->choice_custom_width_units,
					   default_unit.c_str());
		if (getString(paper_->input_custom_height).empty())
			fl_set_choice_text(paper_->choice_custom_height_units,
					   default_unit.c_str());
		if (getString(paper_->input_top_margin).empty())
			fl_set_choice_text(paper_->choice_top_margin_units,
					   default_unit.c_str());
		if (getString(paper_->input_bottom_margin).empty())
			fl_set_choice_text(paper_->choice_bottom_margin_units,
					   default_unit.c_str());
		if (getString(paper_->input_inner_margin).empty())
			fl_set_choice_text(paper_->choice_inner_margin_units,
					   default_unit.c_str());
		if (getString(paper_->input_outer_margin).empty())
			fl_set_choice_text(paper_->choice_outer_margin_units,
					   default_unit.c_str());
		if (getString(paper_->input_head_height).empty())
			fl_set_choice_text(paper_->choice_head_height_units,
					   default_unit.c_str());
		if (getString(paper_->input_head_sep).empty())
			fl_set_choice_text(paper_->choice_head_sep_units,
					   default_unit.c_str());
		if (getString(paper_->input_foot_skip).empty())
			fl_set_choice_text(paper_->choice_foot_skip_units,
					   default_unit.c_str());

	} else if (ob == paper_->choice_paperpackage &&
		   fl_get_choice(paper_->choice_paperpackage) != 1) {

		fl_set_button(paper_->check_use_geometry, 0);
		setEnabled(paper_->input_top_margin,    false);
		setEnabled(paper_->input_bottom_margin, false);
		setEnabled(paper_->input_inner_margin,  false);
		setEnabled(paper_->input_outer_margin,  false);
		setEnabled(paper_->input_head_height,   false);
		setEnabled(paper_->input_head_sep,      false);
		setEnabled(paper_->input_foot_skip,     false);
		setEnabled(paper_->choice_top_margin_units,    false);
		setEnabled(paper_->choice_bottom_margin_units, false);
		setEnabled(paper_->choice_inner_margin_units,  false);
		setEnabled(paper_->choice_outer_margin_units,  false);
		setEnabled(paper_->choice_head_height_units,   false);
		setEnabled(paper_->choice_head_sep_units,      false);
		setEnabled(paper_->choice_foot_skip_units,     false);

	} else if (ob == paper_->check_use_geometry) {
		// don't allow switching geometry off in custom papersize
		// mode nor in A3, B3, and B4
		int const choice = fl_get_choice(paper_->choice_papersize);
		if (choice == 2 || choice == 6 || choice == 9 || choice == 10)
			fl_set_button(paper_->check_use_geometry, 1);

		fl_set_choice(paper_->choice_paperpackage,
			      BufferParams::PACKAGE_NONE + 1);

		bool const use_geom = fl_get_button(paper_->check_use_geometry);
		setEnabled(paper_->input_top_margin,    use_geom);
		setEnabled(paper_->input_bottom_margin, use_geom);
		setEnabled(paper_->input_inner_margin,  use_geom);
		setEnabled(paper_->input_outer_margin,  use_geom);
		setEnabled(paper_->input_head_height,   use_geom);
		setEnabled(paper_->input_head_sep,      use_geom);
		setEnabled(paper_->input_foot_skip,     use_geom);
		setEnabled(paper_->choice_top_margin_units,    use_geom);
		setEnabled(paper_->choice_bottom_margin_units, use_geom);
		setEnabled(paper_->choice_inner_margin_units,  use_geom);
		setEnabled(paper_->choice_outer_margin_units,  use_geom);
		setEnabled(paper_->choice_head_height_units,   use_geom);
		setEnabled(paper_->choice_head_sep_units,      use_geom);
		setEnabled(paper_->choice_foot_skip_units,     use_geom);
	}

	if (ob == paper_->choice_papersize || ob == paper_->radio_portrait
	    || ob == paper_->radio_landscape) {
		// either default papersize (preferences) or document
		// papersize has to be A4
		bool const enable = ( fl_get_choice(paper_->choice_papersize) == 1
				      && lyxrc.default_papersize == BufferParams::PAPER_A4PAPER )
			|| fl_get_choice(paper_->choice_papersize) == 7;
		if (!enable)
			fl_set_choice(paper_->choice_paperpackage,
				      BufferParams::PACKAGE_NONE + 1);
		setEnabled(paper_->choice_paperpackage,
			   enable && fl_get_button(paper_->radio_portrait));
	}

	return ButtonPolicy::SMI_VALID;
}


bool FormDocument::class_apply(BufferParams &params)
{
	bool redo = false;

	// If default skip is a "Length" but there's no text in the
	// input field, reset the kind to "MedSkip", which is the default.
	if (fl_get_choice(class_->choice_skip) == 4 &&
	    getString(class_->input_skip).empty()) {
		fl_set_choice(class_->choice_skip, 2);
	}
	params.fonts = getString(class_->choice_fonts);
	params.fontsize = getString(class_->choice_fontsize);
	params.pagestyle = getString(class_->choice_pagestyle);

	params.textclass = fl_get_combox(class_->combox_class) - 1;

	BufferParams::PARSEP tmpsep = params.paragraph_separation;
	if (fl_get_button(class_->radio_indent))
		params.paragraph_separation = BufferParams::PARSEP_INDENT;
	else
		params.paragraph_separation = BufferParams::PARSEP_SKIP;
	if (tmpsep != params.paragraph_separation)
		redo = true;

	VSpace tmpdefskip = params.getDefSkip();
	switch (fl_get_choice(class_->choice_skip)) {
	case 1:
		params.setDefSkip(VSpace(VSpace::SMALLSKIP));
		break;
	case 2:
		params.setDefSkip(VSpace(VSpace::MEDSKIP));
		break;
	case 3:
		params.setDefSkip(VSpace(VSpace::BIGSKIP));
		break;
	case 4:
	{
		string const length =
			getLengthFromWidgets(class_->input_skip,
					     class_->choice_skip_units);

		params.setDefSkip(VSpace(LyXGlueLength(length)));
		break;
	}
	default:
		// DocumentDefskipCB assures that this never happens
		params.setDefSkip(VSpace(VSpace::MEDSKIP));
		break;
	}
	if (!(tmpdefskip == params.getDefSkip()))
		redo = true;

	if (fl_get_button(class_->radio_columns_two))
		params.columns = 2;
	else
		params.columns = 1;
	if (fl_get_button(class_->radio_sides_two))
		params.sides = LyXTextClass::TwoSides;
	else
		params.sides = LyXTextClass::OneSide;

	Spacing tmpSpacing = params.spacing;
	switch (fl_get_choice(class_->choice_spacing)) {
	case 1:
		lyxerr[Debug::INFO] << "Spacing: SINGLE\n";
		params.spacing.set(Spacing::Single);
		break;
	case 2:
		lyxerr[Debug::INFO] << "Spacing: ONEHALF\n";
		params.spacing.set(Spacing::Onehalf);
		break;
	case 3:
		lyxerr[Debug::INFO] << "Spacing: DOUBLE\n";
		params.spacing.set(Spacing::Double);
		break;
	case 4:
		lyxerr[Debug::INFO] << "Spacing: OTHER\n";
		params.spacing.set(Spacing::Other,
				   getString(class_->input_spacing));
		break;
	}
	if (tmpSpacing != params.spacing)
		redo = true;

	params.options = getString(class_->input_extra);

	return redo;
}


void FormDocument::paper_apply(BufferParams & params)
{
	params.papersize2 = BufferParams::VMARGIN_PAPER_TYPE(fl_get_choice(paper_->choice_papersize) - 1);

	params.paperpackage =
		BufferParams::PAPER_PACKAGES(fl_get_choice(paper_->choice_paperpackage) - 1);

	// set params.papersize from params.papersize2 and params.paperpackage
	params.setPaperStuff();

	params.use_geometry = fl_get_button(paper_->check_use_geometry);

	if (fl_get_button(paper_->radio_landscape))
		params.orientation = BufferParams::ORIENTATION_LANDSCAPE;
	else
		params.orientation = BufferParams::ORIENTATION_PORTRAIT;

	params.paperwidth =
		getLengthFromWidgets(paper_->input_custom_width,
				     paper_->choice_custom_width_units);

	params.paperheight =
		getLengthFromWidgets(paper_->input_custom_height,
				     paper_->choice_custom_height_units);

	params.leftmargin =
		getLengthFromWidgets(paper_->input_inner_margin,
				     paper_->choice_inner_margin_units);

	params.topmargin =
		getLengthFromWidgets(paper_->input_top_margin,
				     paper_->choice_top_margin_units);

	params.rightmargin =
		getLengthFromWidgets(paper_->input_outer_margin,
				     paper_->choice_outer_margin_units);

	params.bottommargin =
		getLengthFromWidgets(paper_->input_bottom_margin,
				     paper_->choice_bottom_margin_units);

	params.headheight =
		getLengthFromWidgets(paper_->input_head_height,
				     paper_->choice_head_height_units);

	params.headsep =
		getLengthFromWidgets(paper_->input_head_sep,
				     paper_->choice_head_sep_units);

	params.footskip =
		getLengthFromWidgets(paper_->input_foot_skip,
				     paper_->choice_foot_skip_units);
}


bool FormDocument::language_apply(BufferParams & params)
{
	InsetQuotes::quote_language lga = InsetQuotes::EnglishQ;
	bool redo = false;

	switch (fl_get_choice(language_->choice_quotes_language) - 1) {
	case 0:
		lga = InsetQuotes::EnglishQ;
		break;
	case 1:
		lga = InsetQuotes::SwedishQ;
		break;
	case 2:
		lga = InsetQuotes::GermanQ;
		break;
	case 3:
		lga = InsetQuotes::PolishQ;
		break;
	case 4:
		lga = InsetQuotes::FrenchQ;
		break;
	case 5:
		lga = InsetQuotes::DanishQ;
		break;
	}
	params.quotes_language = lga;
	if (fl_get_button(language_->radio_single))
		params.quotes_times = InsetQuotes::SingleQ;
	else
		params.quotes_times = InsetQuotes::DoubleQ;

	int const pos = fl_get_combox(language_->combox_language);
	Language const * new_language = languages.getLanguage(lang_[pos-1]);
	if (!new_language)
		new_language = default_language;

	params.language = new_language;
	params.inputenc = getString(language_->choice_inputenc);

	return redo;
}


bool FormDocument::options_apply(BufferParams & params)
{
	bool redo = false;

	params.graphicsDriver = getString(options_->choice_postscript_driver);
	params.use_amsmath = static_cast<BufferParams::AMS>(
		fl_get_choice(options_->choice_ams_math) - 1);
	params.use_natbib  = fl_get_button(options_->check_use_natbib);
	params.use_numerical_citations  =
		fl_get_choice(options_->choice_citation_format) - 1;

	int tmpchar = int(fl_get_counter_value(options_->counter_secnumdepth));
	if (params.secnumdepth != tmpchar)
		redo = true;
	params.secnumdepth = tmpchar;

	params.tocdepth = int(fl_get_counter_value(options_->counter_tocdepth));

	params.float_placement =
		getString(options_->input_float_placement);

	return redo;
}


void FormDocument::bullets_apply(BufferParams & params)
{
	/* update the bullet settings */
	BufferParams & buf_params = controller().params();

	for (int i = 0; i < 4; ++i) {
		params.user_defined_bullets[i] = buf_params.temp_bullets[i];
	}
}


void FormDocument::UpdateClassParams(BufferParams const & params)
{
	// These are the params that have to be updated on any class change
	// (even if the class defaults are not used) (JSpitzm 2002-04-08)

	LyXTextClass const & tclass = textclasslist[params.textclass];

	fl_set_combox(class_->combox_class, params.textclass + 1);
	fl_clear_choice(class_->choice_fontsize);
	fl_addto_choice(class_->choice_fontsize, "default");
	fl_addto_choice(class_->choice_fontsize,
			tclass.opt_fontsize().c_str());
	fl_set_choice_text(class_->choice_fontsize,
			   params.fontsize.c_str());
	fl_clear_choice(class_->choice_pagestyle);
	fl_addto_choice(class_->choice_pagestyle, "default");
	fl_addto_choice(class_->choice_pagestyle,
			tclass.opt_pagestyle().c_str());
	fl_set_choice_text(class_->choice_pagestyle,
			   params.pagestyle.c_str());

}

void FormDocument::class_update(BufferParams const & params)
{
	if (!class_.get())
		return;

	UpdateClassParams(params);

	fl_set_choice_text(class_->choice_fonts, params.fonts.c_str());

	bool const indent = params.paragraph_separation == BufferParams::PARSEP_INDENT;
	fl_set_button(class_->radio_indent, indent);
	fl_set_button(class_->radio_skip, !indent);

	int pos;
	if (indent) {
		pos = 2; // VSpace::MEDSKIP is default
	} else {
		switch (params.getDefSkip().kind()) {
		case VSpace::LENGTH:
			pos = 4;
			break;
		case VSpace::BIGSKIP:
			pos = 3;
			break;
		case VSpace::SMALLSKIP:
			pos = 1;
			break;
		case VSpace::MEDSKIP:
		default:
			pos = 2;
			break;
		}
	}
	fl_set_choice (class_->choice_skip, pos);

	bool const length_input = pos == 4;
	if (length_input) {
		int const paperchoice = fl_get_choice(paper_->choice_papersize);
		bool const metric = (paperchoice == 1 && lyxrc.default_papersize > BufferParams::PAPER_EXECUTIVEPAPER)
			|| paperchoice == 2 || paperchoice > 5;
		string const default_unit = metric ? "cm" : "in";
		string const length = params.getDefSkip().asLyXCommand();
		updateWidgetsFromLengthString(class_->input_skip,
					      class_->choice_skip_units,
					      length, default_unit);

	} else {
		fl_set_input(class_->input_skip, "");
	}
	setEnabled(class_->choice_skip, !indent);
	setEnabled(class_->input_skip, length_input);
	setEnabled(class_->choice_skip_units, length_input);

	bool const two_sides = params.sides == LyXTextClass::TwoSides;
	fl_set_button(class_->radio_sides_one, !two_sides);
	fl_set_button(class_->radio_sides_two, two_sides);

	bool const two_columns = params.columns == 2;
	fl_set_button(class_->radio_columns_one, !two_columns);
	fl_set_button(class_->radio_columns_two, two_columns);

	fl_set_input(class_->input_extra, params.options.c_str());

	switch (params.spacing.getSpace()) {
	case Spacing::Other:
		pos = 4;
		break;
	case Spacing::Double: // \doublespacing
		pos = 3;
		break;
	case Spacing::Onehalf: // \onehalfspacing
		pos = 2;
		break;
	case Spacing::Single: // \singlespacing
	case Spacing::Default: // nothing bad should happen with this
	default:
		pos = 1;
		break;
	}
	fl_set_choice(class_->choice_spacing, pos);

	bool const spacing_input = pos == 4;
	setEnabled(class_->input_spacing, spacing_input);
	string const input = spacing_input ? tostr(params.spacing.getValue()) : string();
	fl_set_input(class_->input_spacing, input.c_str());
}


void FormDocument::language_update(BufferParams const & params)
{
	if (!language_.get())
		return;

	int const pos = int(findPos(lang_, params.language->lang()));
	fl_set_combox(language_->combox_language, pos+1);

	fl_set_choice_text(language_->choice_inputenc, params.inputenc.c_str());
	fl_set_choice(language_->choice_quotes_language, params.quotes_language + 1);
	fl_set_button(language_->radio_single, 0);
	fl_set_button(language_->radio_double, 0);
	if (params.quotes_times == InsetQuotes::SingleQ)
		fl_set_button(language_->radio_single, 1);
	else
		fl_set_button(language_->radio_double, 1);
}


void FormDocument::options_update(BufferParams const & params)
{
	if (!options_.get())
		return;

	fl_set_choice_text(options_->choice_postscript_driver,
			   params.graphicsDriver.c_str());
	fl_set_choice(options_->choice_ams_math, params.use_amsmath + 1);
	fl_set_button(options_->check_use_natbib,  params.use_natbib);
	fl_set_choice(options_->choice_citation_format,
		      int(params.use_numerical_citations)+1);
	setEnabled(options_->choice_citation_format, params.use_natbib);
	fl_set_counter_value(options_->counter_secnumdepth, params.secnumdepth);
	fl_set_counter_value(options_->counter_tocdepth, params.tocdepth);
	if (!params.float_placement.empty())
		fl_set_input(options_->input_float_placement,
			     params.float_placement.c_str());
	else
		fl_set_input(options_->input_float_placement, "");
}


void FormDocument::paper_update(BufferParams const & params)
{
	if (!paper_.get())
		return;

	fl_set_choice(paper_->choice_papersize, params.papersize2 + 1);
	fl_set_choice(paper_->choice_paperpackage, params.paperpackage + 1);
	fl_set_button(paper_->check_use_geometry, params.use_geometry);

	int const paperchoice = fl_get_choice(paper_->choice_papersize);
	bool const useCustom = paperchoice == 2;
	bool const useGeom = fl_get_button(paper_->check_use_geometry);

	fl_set_button(paper_->radio_portrait, 0);
	setEnabled(paper_->radio_portrait, !useCustom);
	fl_set_button(paper_->radio_landscape, 0);
	setEnabled(paper_->radio_landscape, !useCustom);

	if (params.orientation == BufferParams::ORIENTATION_LANDSCAPE)
		fl_set_button(paper_->radio_landscape, 1);
	else
		fl_set_button(paper_->radio_portrait, 1);
	setEnabled(paper_->choice_paperpackage,
		   //either default papersize (preferences)
		   //or document papersize has to be A4
		   (paperchoice == 7
		    || paperchoice == 1 && lyxrc.default_papersize == BufferParams::PAPER_A4PAPER)
		   && fl_get_button(paper_->radio_portrait));

	// Default unit choice is cm if metric, inches if US paper.
	bool const metric = (paperchoice == 1 && lyxrc.default_papersize > BufferParams::PAPER_EXECUTIVEPAPER)
		|| paperchoice == 2 || paperchoice > 5;
	string const default_unit = metric ? "cm" : "in";
	updateWidgetsFromLengthString(paper_->input_custom_width,
				      paper_->choice_custom_width_units,
				      params.paperwidth, default_unit);
	setEnabled(paper_->input_custom_width, useCustom);
	setEnabled(paper_->choice_custom_width_units, useCustom);

	updateWidgetsFromLengthString(paper_->input_custom_height,
				      paper_->choice_custom_height_units,
				      params.paperheight, default_unit);
	setEnabled(paper_->input_custom_height, useCustom);
	setEnabled(paper_->choice_custom_height_units, useCustom);

	updateWidgetsFromLengthString(paper_->input_inner_margin,
				      paper_->choice_inner_margin_units,
				      params.leftmargin, default_unit);
	setEnabled(paper_->input_inner_margin, useGeom);
	setEnabled(paper_->choice_inner_margin_units, useGeom);

	updateWidgetsFromLengthString(paper_->input_top_margin,
				      paper_->choice_top_margin_units,
				      params.topmargin, default_unit);
	setEnabled(paper_->input_top_margin, useGeom);
	setEnabled(paper_->choice_top_margin_units, useGeom);

	updateWidgetsFromLengthString(paper_->input_outer_margin,
				      paper_->choice_outer_margin_units,
				      params.rightmargin, default_unit);
	setEnabled(paper_->input_outer_margin, useGeom);
	setEnabled(paper_->choice_outer_margin_units, useGeom);

	updateWidgetsFromLengthString(paper_->input_bottom_margin,
				      paper_->choice_bottom_margin_units,
				      params.bottommargin, default_unit);
	setEnabled(paper_->input_bottom_margin, useGeom);
	setEnabled(paper_->choice_bottom_margin_units, useGeom);

	updateWidgetsFromLengthString(paper_->input_head_height,
				      paper_->choice_head_height_units,
				      params.headheight, default_unit);
	setEnabled(paper_->input_head_height, useGeom);
	setEnabled(paper_->choice_head_height_units, useGeom);

	updateWidgetsFromLengthString(paper_->input_head_sep,
				      paper_->choice_head_sep_units,
				      params.headsep, default_unit);
	setEnabled(paper_->input_head_sep, useGeom);
	setEnabled(paper_->choice_head_sep_units, useGeom);

	updateWidgetsFromLengthString(paper_->input_foot_skip,
				      paper_->choice_foot_skip_units,
				      params.footskip, default_unit);
	setEnabled(paper_->input_foot_skip, useGeom);
	setEnabled(paper_->choice_foot_skip_units, useGeom);

	fl_set_focus_object(paper_->form, paper_->choice_papersize);
}


void FormDocument::bullets_update(BufferParams const & params)
{
	if (!bullets_.get() ||
	    ((XpmVersion<4) ||
	     (XpmVersion==4 && XpmRevision<7)))
		return;

	bool const isLinuxDoc =
		controller().docType() == ControlDocument::LINUXDOC;
	setEnabled(fbullet, !isLinuxDoc);

	if (isLinuxDoc) return;

	fl_set_button(bullets_->radio_depth_1, 1);
	fl_set_input(bullets_->input_latex,
		     params.user_defined_bullets[0].getText().c_str());
	fl_set_choice(bullets_->choice_size,
		      params.user_defined_bullets[0].getSize() + 2);
}


void FormDocument::checkReadOnly()
{
	if (bc().readOnly(controller().bufferIsReadonly())) {
		postWarning(_("Document is read-only."
			      " No changes to layout permitted."));
	} else {
		clearMessage();
	}
}


void FormDocument::ChoiceBulletSize(FL_OBJECT * ob, long /*data*/)
{
	BufferParams & param = controller().params();

	// convert from 1-6 range to -1-4
	param.temp_bullets[current_bullet_depth].setSize(fl_get_choice(ob) - 2);
	fl_set_input(bullets_->input_latex,
		     param.temp_bullets[current_bullet_depth].getText().c_str());
}


void FormDocument::InputBulletLaTeX(FL_OBJECT *, long)
{
	BufferParams & param = controller().params();

	param.temp_bullets[current_bullet_depth].
		setText(getString(bullets_->input_latex));
}


void FormDocument::BulletDepth(FL_OBJECT * ob)
{
	/* Should I do the following:                                 */
	/*  1. change to the panel that the current bullet belongs in */
	/*  2. show that bullet as selected                           */
	/*  3. change the size setting to the size of the bullet in Q.*/
	/*  4. display the latex equivalent in the latex box          */
	/*                                                            */
	/* I'm inclined to just go with 3 and 4 at the moment and     */
	/* maybe try to support the others later                      */
	BufferParams & param = controller().params();

	int data = 0;
	if (ob == bullets_->radio_depth_1)
		data = 0;
	else if (ob == bullets_->radio_depth_2)
		data = 1;
	else if (ob == bullets_->radio_depth_3)
		data = 2;
	else if (ob == bullets_->radio_depth_4)
		data = 3;

	switch (fl_get_button_numb(ob)) {
	case 3:
		// right mouse button resets to default
		param.temp_bullets[data] = ITEMIZE_DEFAULTS[data];
	default:
		current_bullet_depth = data;
		fl_set_input(bullets_->input_latex,
			     param.temp_bullets[data].getText().c_str());
		fl_set_choice(bullets_->choice_size,
			      param.temp_bullets[data].getSize() + 2);
	}
}


void FormDocument::BulletPanel(FL_OBJECT * ob)
{
	/* Here we have to change the background pixmap to that selected */
	/* by the user. (eg. standard.xpm, psnfss1.xpm etc...)           */

	int data = 0;
	if (ob == bullets_->radio_panel_standard)
		data = 0;
	else if (ob == bullets_->radio_panel_maths)
		data = 1;
	else if (ob == bullets_->radio_panel_ding2)
		data = 2;
	else if (ob == bullets_->radio_panel_ding3)
		data = 3;
	else if (ob == bullets_->radio_panel_ding4)
		data = 4;
	else if (ob == bullets_->radio_panel_ding1)
		data = 5;

	if (data != current_bullet_panel) {
		fl_freeze_form(bullets_->form);
		current_bullet_panel = data;

		/* free the current pixmap */
		fl_free_bmtable_pixmap(bullets_->bmtable_panel);
		string new_panel;
		if (ob == bullets_->radio_panel_standard) {
			new_panel = "standard";
		} else if (ob == bullets_->radio_panel_maths ) {
			new_panel = "amssymb";
		} else if (ob == bullets_->radio_panel_ding2) {
			new_panel = "psnfss1";
		} else if (ob == bullets_->radio_panel_ding3) {
			new_panel = "psnfss2";
		} else if (ob == bullets_->radio_panel_ding4) {
			new_panel = "psnfss3";
		} else if (ob == bullets_->radio_panel_ding1) {
			new_panel = "psnfss4";
		} else {
			/* something very wrong happened */
			// play it safe for now but should be an exception
			current_bullet_panel = 0;  // standard panel
			new_panel = "standard";
		}
		new_panel += ".xpm";
		fl_set_bmtable_pixmap_file(bullets_->bmtable_panel, 6, 6,
					   LibFileSearch("images", new_panel).c_str());
		fl_redraw_object(bullets_->bmtable_panel);
		fl_unfreeze_form(bullets_->form);
	}
}


void FormDocument::BulletBMTable(FL_OBJECT * ob, long /*data*/)
{
	/* handle the user input by setting the current bullet depth's pixmap */
	/* to that extracted from the current chosen position of the BMTable  */
	/* Don't forget to free the button's old pixmap first.                */

	BufferParams & param = controller().params();
	int bmtable_button = fl_get_bmtable(ob);

	/* try to keep the button held down till another is pushed */
	/*  fl_set_bmtable(ob, 1, bmtable_button); */
	param.temp_bullets[current_bullet_depth].setFont(current_bullet_panel);
	param.temp_bullets[current_bullet_depth].setCharacter(bmtable_button);
	fl_set_input(bullets_->input_latex,
		     param.temp_bullets[current_bullet_depth].getText().c_str());
}


void FormDocument::CheckChoiceClass()
{
	BufferParams & params = controller().params();

	lyx::textclass_type const tc =
		fl_get_combox(class_->combox_class) - 1;

	if (controller().loadTextclass(tc)) {
		params.textclass = tc;

		if (lyxrc.auto_reset_options) {
			params.useClassDefaults();
			UpdateLayoutDocument(params);
		} else {
			// update the params which are needed in any case
			// (fontsizes, pagestyle)
			UpdateClassParams(params);
		}

	} else {
		int const revert = int(params.textclass);
		fl_set_combox(class_->combox_class, revert + 1);
	}
}


void FormDocument::UpdateLayoutDocument(BufferParams const & params)
{
	if (!dialog_.get())
		return;

	checkReadOnly();
	class_update(params);
	paper_update(params);
	language_update(params);
	options_update(params);
	bullets_update(params);
}
