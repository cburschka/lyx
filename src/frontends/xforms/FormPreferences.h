// -*- C++ -*-
/**
 * \file FormPreferences.h
 * Copyright 1999-2001 Allan Rae
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Allan Rae
 * \author Angus Leeming 
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef FORMPREFERENCES_H
#define FORMPREFERENCES_H

#ifdef __GNUG__
#pragma interface
#endif

#include "FormBaseDeprecated.h"
#include "Color.h" // NamedColor
#include "xforms_helpers.h" // XformColor

#include <boost/scoped_ptr.hpp>

#include FORMS_H_LOCATION
#include <utility> // pair

class Combox;
class Dialogs;
class LyXView;
class RGBColor;
struct FD_preferences;
struct FD_preferences_colors;
struct FD_preferences_converters;
struct FD_preferences_formats;
struct FD_preferences_inputs_misc;
struct FD_preferences_interface;
struct FD_preferences_language;
struct FD_preferences_lnf_misc;
struct FD_preferences_inner_tab;
struct FD_preferences_outputs_misc;
struct FD_preferences_paths;
struct FD_preferences_printer;
struct FD_preferences_screen_fonts;
struct FD_preferences_spelloptions;


/** This class provides an XForms implementation of the FormPreferences Dialog.
    The preferences dialog allows users to set/save their preferences.
 */
class FormPreferences : public FormBaseBI {
public:
	///
	FormPreferences(LyXView &, Dialogs &);

private:
	/// Pointer to the actual instantiation of the ButtonController.
	virtual xformsBC & bc();
	/** Redraw the form (on receipt of a Signal indicating, for example,
	    that the xforms colours have been re-mapped). */
	virtual void redraw();
	/// Update the dialog.
	virtual void update();
	/// Hide the dialog.
	virtual void hide();
	/// OK (Save) from dialog
	virtual void ok();
	/// Apply from dialog
	virtual void apply();
	/// Filter the inputs -- return true if entries are valid
	virtual bool input(FL_OBJECT *, long);
	/// Build the dialog
	virtual void build();
	/// Pointer to the actual instantiation of the xforms form.
	virtual FL_FORM * form() const;
	/// control which feedback message is output
	string const getFeedback(FL_OBJECT *);
	///
	void browse(FL_OBJECT * input,
		    string const & title, string const & pattern,
		    std::pair<string,string> const & dir1= std::make_pair(string(),string()),
		    std::pair<string,string> const & dir2 = std::make_pair(string(),string()));

	/// Real GUI implementation.
	boost::scoped_ptr<FD_preferences> dialog_;
	/// Converters tabfolder
	boost::scoped_ptr<FD_preferences_inner_tab> converters_tab_;
	/// reLyX and other import/input stuff
	boost::scoped_ptr<FD_preferences_inner_tab> inputs_tab_;
	/// HCI configuration
	boost::scoped_ptr<FD_preferences_inner_tab> look_n_feel_tab_;
	/// Outputs tabfolder
	boost::scoped_ptr<FD_preferences_inner_tab> outputs_tab_;
	/// Spellchecker, language stuff, etc
	boost::scoped_ptr<FD_preferences_inner_tab> lang_opts_tab_;

	/** Each tab folder is encapsulated in its own class.
	 */

	class Colors {
	public:
		///
		enum GuiColors {
			GUI_COLOR_CHOICE   = FL_FREE_COL14,
			GUI_COLOR_HUE_DIAL = FL_FREE_COL15,
			GUI_COLOR_CURSOR   = FL_FREE_COL16
		};
		///
		Colors(FormPreferences & p);
		///
		FD_preferences_colors const * dialog();
		///
		void apply(); // not const as modifies modifiedXformsPrefs.
		///
		void build();
		///
		string const feedback(FL_OBJECT const * const) const;
		///
		void input(FL_OBJECT const * const);
		///
		void update() { LoadBrowserLyX(); }

		/// Flag whether Xforms colors have changed since last file save
		bool modifiedXformsPrefs;

	private:
		///
		void AdjustVal(int, int, double) const;
		///
		void InputBrowserLyX() const;
		///
		void InputHSV();
		///
		void InputRGB();
		///
		void LoadBrowserLyX();
		///
		void Modify();
		///
		void SwitchColorSpace() const;
		///
		string const X11hexname(RGBColor const &) const;

		///
		FormPreferences & parent_;
		///
		boost::scoped_ptr<FD_preferences_colors> dialog_;

		/// A vector of LyX LColor GUI name and associated RGB color.
		std::vector<NamedColor> lyxColorDB;
		/// A vector of xforms color ID, RGB colors and associated name.
		std::vector<XformsColor> xformsColorDB;
	};
	///
	friend class Colors;

	///
	class Converters {
	public:
		///
		Converters(FormPreferences & p);
		///
		FD_preferences_converters const * dialog();
		///
		void apply() const;
		///
		void build();
		///
		string const feedback(FL_OBJECT const * const) const;
		///
		bool input(FL_OBJECT const * const);
		///
		void update();
		///
		void UpdateBrowser();
		///
		void UpdateChoices() const;

	private:
		///
		bool Add();
		///
		bool Browser();
		///
		bool erase();
		///
		bool Input();
		///
		string const GetFrom() const;
		///
		string const GetTo() const;

		///
		FormPreferences & parent_;
		///
		boost::scoped_ptr<FD_preferences_converters> dialog_;
	};
	///
	friend class Converters;

	///
	class Formats {
	public:
		///
		Formats(FormPreferences &  p);
		///
		FD_preferences_formats const * dialog();
		///
		void apply() const;
		///
		void build();
		///
		string const feedback(FL_OBJECT const * const) const;
		///
		bool input(FL_OBJECT const * const);
		///
		void update();

	private:
		///
		bool Add();
		///
		bool Browser();
		///
		void UpdateBrowser();
		///
		bool erase();
		///
		bool Input();

		///
		FormPreferences & parent_;
		///
		boost::scoped_ptr<FD_preferences_formats> dialog_;
	};
	///
	friend class Formats;

	///
	class InputsMisc {
	public:
		///
		InputsMisc(FormPreferences &  p);
		///
		FD_preferences_inputs_misc const * dialog();
		///
		void apply() const;
		///
		void build();
		///
		string const feedback(FL_OBJECT const * const) const;
		///
		void update();

	private:
		///
		FormPreferences & parent_;
		///
		boost::scoped_ptr<FD_preferences_inputs_misc> dialog_;
	};
	///
	friend class InputsMisc;

	///
	class Interface {
	public:
		///
		Interface(FormPreferences &  p);
		///
		FD_preferences_interface const * dialog();
		///
		void apply() const;
		///
		void build();
		///
		string const feedback(FL_OBJECT const * const) const;
		///
		bool input(FL_OBJECT const * const);
		///
		void update();

	private:
		///
		FormPreferences & parent_;
		///
		boost::scoped_ptr<FD_preferences_interface> dialog_;
	};
	///
	friend class Interface;

	///
	class Language {
	public:
		///
		Language(FormPreferences &  p);
		///
		FD_preferences_language const * dialog();
		///
		void apply(); // not const because calls update()
		///
		void build();
		///
		string const feedback(FL_OBJECT const * const) const;
		///
		bool input(FL_OBJECT const * const);
		///
		void update();
		///
		static void ComboCB(int, void *, Combox *);

	private:
		///
		FormPreferences & parent_;
		///
		boost::scoped_ptr<FD_preferences_language> dialog_;
		///
		boost::scoped_ptr<Combox> combo_default_lang;
		///
		std::vector<string> lang_;
	};
	///
	friend class Language;

	///
	class LnFmisc {
	public:
		///
		LnFmisc(FormPreferences &  p);
		///
		FD_preferences_lnf_misc const * dialog();
		///
		void apply() const;
		///
		void build();
		///
		string const feedback(FL_OBJECT const * const) const;
		///
		void update();

	private:
		///
		FormPreferences & parent_;
		///
		boost::scoped_ptr<FD_preferences_lnf_misc> dialog_;
	};
	///
	friend class LnFmisc;

	///
	class OutputsMisc {
	public:
		///
		OutputsMisc(FormPreferences &  p);
		///
		FD_preferences_outputs_misc const * dialog();
		///
		void apply() const;
		///
		void build();
		///
		string const feedback(FL_OBJECT const * const) const;
		///
		void update();

	private:
		///
		FormPreferences & parent_;
		///
		boost::scoped_ptr<FD_preferences_outputs_misc> dialog_;
	};
	///
	friend class OutputsMisc;

	///
	class Paths {
	public:
		///
		Paths(FormPreferences &  p);
		///
		FD_preferences_paths const * dialog();
		///
		void apply();
		///
		void build();
		///
		string const feedback(FL_OBJECT const * const) const;
		///
		bool input(FL_OBJECT const * const);
		///
		void update();

	private:
		///
		FormPreferences & parent_;
		///
		boost::scoped_ptr<FD_preferences_paths> dialog_;
	};
	///
	friend class Paths;

	///
	class Printer {
	public:
		///
		Printer(FormPreferences &  p);
		///
		FD_preferences_printer const * dialog();
		///
		void apply() const;
		///
		void build();
		///
		string const feedback(FL_OBJECT const * const) const;
		///
		void update();

	private:
		///
		FormPreferences & parent_;
		///
		boost::scoped_ptr<FD_preferences_printer> dialog_;
	};
	///
	friend class Printer;

	///
	class ScreenFonts {
	public:
		///
		ScreenFonts(FormPreferences &  p);
		///
		FD_preferences_screen_fonts const * dialog();
		///
		void apply() const;
		///
		void build();
		///
		string const feedback(FL_OBJECT const * const) const;
		///
		bool input();
		///
		void update();

	private:
		///
		FormPreferences & parent_;
		///
		boost::scoped_ptr<FD_preferences_screen_fonts> dialog_;
	};
	///
	friend class ScreenFonts;

	///
	class SpellOptions {
	public:
		///
		SpellOptions(FormPreferences &  p);
		///
		FD_preferences_spelloptions const * dialog();
		///
		void apply(); // not const because calls update()!
		///
		void build();
		///
		string const feedback(FL_OBJECT const * const) const;
		///
		bool input(FL_OBJECT const * const);
		///
		void update();

	private:
		///
		FormPreferences & parent_;
		///
		boost::scoped_ptr<FD_preferences_spelloptions> dialog_;
	};
	///
	friend class SpellOptions;

	/** The tab folders.
	 */

	///
	Colors colors_;
	///
	Converters converters_;
	///
	InputsMisc inputs_misc_;
	///
	Formats formats_;
	///
	Interface interface_;
	///
	Language language_;
	///
	LnFmisc lnf_misc_;
	///
	OutputsMisc outputs_misc_;
	///
	Paths paths_;
	///
	Printer printer_;
	///
	ScreenFonts screen_fonts_;
	///
	SpellOptions spelloptions_;

	/** A couple of helper structs to enable colors to be sorted by name
	    and by color */
	///
	struct SortColorsByName {
		///
		int operator()(NamedColor const & a, NamedColor const & b) const
			{ return (a.getname() < b.getname()); }
	};
	///
	struct SortColorsByColor {
		///
		SortColorsByColor(RGBColor c) : col(c) {}
		///
		int operator()(RGBColor const &, RGBColor const &) const;
		///
		RGBColor col;
	};
	/// The ButtonController
	ButtonController<PreferencesPolicy, xformsBC> bc_;
};


inline
xformsBC & FormPreferences::bc()
{
	return bc_;
}

#endif
