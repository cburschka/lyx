// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2001 The LyX Team.
 *
 *	    This file copyright 1999-2001
 *	    Allan Rae
 *======================================================*/
/* FormPreferences.h
 * FormPreferences Interface Class
 */

#ifndef FORMPREFERENCES_H
#define FORMPREFERENCES_H

#include <utility> // pair
#include <boost/smart_ptr.hpp>

#ifdef __GNUG_
#pragma interface
#endif

#include "FormBaseDeprecated.h"
#include "Color.h" // NamedColor
#include "xforms_helpers.h" // XformColor

class Combox;
class Dialogs;
class LyXView;
class RGBColor;
struct FD_form_colors;
struct FD_form_converters;
struct FD_form_formats;
struct FD_form_inputs_misc;
struct FD_form_interface;
struct FD_form_language;
struct FD_form_lnf_misc;
struct FD_form_outer_tab;
struct FD_form_outputs_misc;
struct FD_form_paths;
struct FD_form_preferences;
struct FD_form_printer;
struct FD_form_screen_fonts;
struct FD_form_spelloptions;


/** This class provides an XForms implementation of the FormPreferences Dialog.
    The preferences dialog allows users to set/save their preferences.
 */
class FormPreferences : public FormBaseBI {
public:
	/// #FormPreferences x(LyXFunc ..., Dialogs ...);#
	FormPreferences(LyXView *, Dialogs *);
	///
	static int FeedbackCB(FL_OBJECT *, int,
			      FL_Coord, FL_Coord, int, void *);

private:
	/// Pointer to the actual instantiation of the ButtonController.
	virtual xformsBC & bc();
	/** Redraw the form (on receipt of a Signal indicating, for example,
	    that the xforms colours have been re-mapped). */
	virtual void redraw();
	/// Update the dialog.
	virtual void update();
	/// show the spellchecker tab
	void showSpellPref();
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
	void feedback(FL_OBJECT *);

       	/// Set the preemptive handler for each FL_OBJECT.
	static void setPreHandler(FL_OBJECT *);
	/// The preemptive handler for feedback messages.
	void Feedback(FL_OBJECT *, int);
	/// Print a warning message and set warning flag.
	void printWarning( string const & );
	/** Launch a file dialog and modify input if it returns a new file.
	    For an explanation of the various parameters, see xforms_helpers.h.
	 */
	void browse( FL_OBJECT * input,
		     string const & title, string const & pattern, 
		     std::pair<string,string> const & dir1= std::make_pair(string(),string()), 
		     std::pair<string,string> const & dir2 = std::make_pair(string(),string()));
	
	/// Fdesign generated methods
	FD_form_preferences * build_preferences();
	///
	FD_form_outer_tab * build_outer_tab();
	///
	FD_form_colors * build_colors();
	///
	FD_form_converters * build_converters();
	///
	FD_form_formats * build_formats();
	///
	FD_form_inputs_misc * build_inputs_misc();
	///
	FD_form_interface * build_interface();
	///
	FD_form_language * build_language();
	///
	FD_form_lnf_misc * build_lnf_misc();
	///
	FD_form_outputs_misc * build_outputs_misc();
	///
	FD_form_paths * build_paths();
	///
	FD_form_printer * build_printer();
	///
	FD_form_screen_fonts * build_screen_fonts();
	///
	FD_form_spelloptions * build_spelloptions();

	/// Real GUI implementation.
	boost::scoped_ptr<FD_form_preferences> dialog_;
	/// Converters tabfolder
	boost::scoped_ptr<FD_form_outer_tab> converters_tab_;
	/// reLyX and other import/input stuff
	boost::scoped_ptr<FD_form_outer_tab> inputs_tab_;
	/// HCI configuration
	boost::scoped_ptr<FD_form_outer_tab> look_n_feel_tab_;
	/// Outputs tabfolder
	boost::scoped_ptr<FD_form_outer_tab> outputs_tab_;
	/// Spellchecker, language stuff, etc
	boost::scoped_ptr<FD_form_outer_tab> lang_opts_tab_;

	/** Flag whether a warning has been posted to the text window.
	    If so, don't redraw the window when the mouse leaves an object. */
	bool warningPosted;
	
	/** Each tab folder is encapsulated in its own class.
	 */

	class Colors {
	public:
		///
		enum GuiColors {
			GUI_COLOR_CHOICE = FL_FREE_COL1,
			GUI_COLOR_HUE_DIAL = FL_FREE_COL2,
			GUI_COLOR_CURSOR = FL_FREE_COL3
		};
		///
		Colors( FormPreferences & p );
		///
		FD_form_colors const * dialog();
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
		void AdjustVal( int, int, double ) const;
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
		boost::scoped_ptr<FD_form_colors> dialog_;

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
		Converters( FormPreferences & p );
		///
		FD_form_converters const * dialog();
		///
		void apply() const;
		///
		void build();
		///
		string const feedback(FL_OBJECT const * const) const;
		///
		bool input( FL_OBJECT const * const );
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
		boost::scoped_ptr<FD_form_converters> dialog_;
	};
	///
	friend class Converters;

	///
	class Formats {
	public:
		///
		Formats( FormPreferences &  p );
		///
		FD_form_formats const * dialog();
		///
		void apply() const;
		///
		void build();
		///
		string const feedback(FL_OBJECT const * const) const;
		///
		bool input( FL_OBJECT const * const );
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
		boost::scoped_ptr<FD_form_formats> dialog_;
	};
	///
	friend class Formats;

	///
	class InputsMisc {
	public:
		///
		InputsMisc( FormPreferences &  p );
		///
		FD_form_inputs_misc const * dialog();
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
		boost::scoped_ptr<FD_form_inputs_misc> dialog_;
	};
	///
	friend class InputsMisc;
	
	///
	class Interface {
	public:
		///
		Interface( FormPreferences &  p );
		///
		FD_form_interface const * dialog();
		///
		void apply() const;
		///
		void build();
		///
		string const feedback(FL_OBJECT const * const) const;
		///
		bool input( FL_OBJECT const * const );
		///
		void update();

	private:
		///
		FormPreferences & parent_;
		///
		boost::scoped_ptr<FD_form_interface> dialog_;
	};
	///
	friend class Interface;

	///
	class Language {
	public:
		///
		Language( FormPreferences &  p );
		///
		FD_form_language const * dialog();
		///
		void apply(); // not const because calls update()
		///
		void build();
		///
		string const feedback(FL_OBJECT const * const) const;
		///
		bool input( FL_OBJECT const * const );
		///
		void update();
		///
		static void ComboCB(int, void *, Combox *);

	private:
		///
		FormPreferences & parent_;
		///
		boost::scoped_ptr<FD_form_language> dialog_;
		///
		boost::scoped_ptr<Combox> combo_default_lang;
	};
	///
	friend class Language;
	
	///
	class LnFmisc {
	public:
		///
		LnFmisc( FormPreferences &  p );
		///
		FD_form_lnf_misc const * dialog();
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
		boost::scoped_ptr<FD_form_lnf_misc> dialog_;
	};
	///
	friend class LnFmisc;

	///
	class OutputsMisc {
	public:
		///
		OutputsMisc( FormPreferences &  p );
		///
		FD_form_outputs_misc const * dialog();
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
		boost::scoped_ptr<FD_form_outputs_misc> dialog_;
	};
	///
	friend class OutputsMisc;

	///
	class Paths {
	public:
		///
		Paths( FormPreferences &  p );
		///
		FD_form_paths const * dialog();
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
		boost::scoped_ptr<FD_form_paths> dialog_;
	};
	///
	friend class Paths;

	///
	class Printer {
	public:
		///
		Printer( FormPreferences &  p );
		///
		FD_form_printer const * dialog();
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
		boost::scoped_ptr<FD_form_printer> dialog_;
	};
	///
	friend class Printer;

	///
	class ScreenFonts {
	public:
		///
		ScreenFonts( FormPreferences &  p );
		///
		FD_form_screen_fonts const * dialog();
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
		boost::scoped_ptr<FD_form_screen_fonts> dialog_;
	};
	///
	friend class ScreenFonts;

	///
	class SpellOptions {
	public:
		///
		SpellOptions( FormPreferences &  p );
		///
		FD_form_spelloptions const * dialog();
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
		boost::scoped_ptr<FD_form_spelloptions> dialog_;
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
