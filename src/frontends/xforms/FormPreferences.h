// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-2000 The LyX Team.
 *
 *	    This file copyright 1999-2000
 *	    Allan Rae
 *======================================================*/
/* FormPreferences.h
 * FormPreferences Interface Class
 */

#ifndef FORMPREFERENCES_H
#define FORMPREFERENCES_H

#include <utility> // for pair

#include "FormBase.h"
#include "Color.h"
#include "LString.h"

#ifdef __GNUG_
#pragma interface
#endif

class Combox;
class Command;
class Dialogs;
class Format;
class LyXView;
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
struct FD_form_spellchecker;


/** This class provides an XForms implementation of the FormPreferences Dialog.
    The preferences dialog allows users to set/save their preferences.
 */
class FormPreferences : public FormBaseBI {
public:
	/// #FormPreferences x(LyXFunc ..., Dialogs ...);#
	FormPreferences(LyXView *, Dialogs *);
	///
	~FormPreferences();
	///
	static int FeedbackCB(FL_OBJECT *, int,
			      FL_Coord, FL_Coord, int, void *);

private:
	/** Redraw the form (on receipt of a Signal indicating, for example,
	    that the xform colours have been re-mapped). */
	virtual void redraw();
	/// Update the dialog.
	virtual void update();
	///
	virtual void hide();
	/// OK from dialog
	virtual void ok();
	/// Apply from dialog
	virtual void apply();
	/// Filter the inputs -- return true if entries are valid
	virtual bool input(FL_OBJECT *, long);
	/// Build the dialog
	virtual void build();
	/// control which feedback message is output
	void feedback( FL_OBJECT * );
	/// The handler for the preemptive feedback
	void Feedback(FL_OBJECT *, int);
	///
	virtual FL_FORM * form() const;

	/** Helper functions available to the various tab folders.
	 */

	///
	bool WriteableDir( string const & );
	///
	bool ReadableDir( string const & );
	///
	bool WriteableFile( string const &, string const & = string() );
	///
	bool ReadableFile( string const &, string const & = string() );
	///
	void setPreHandler( FL_OBJECT * ) const;
	///
	void printWarning( string const & );

	/** title: filedlg title, pattern: *.ps etc
	    extra buttons on filedlg: dir1 = (name, dir), dir2 = (name, dir)
	 */
	
	///
	bool browse(FL_OBJECT * input,
		    string const & title, string const & pattern, 
		    std::pair<string,string> const & dir1 //=
		    //std::pair<string,string>()
		    ,
		    std::pair<string,string> const & dir2 //=
		    //std::pair<string,string>()
		);
	/// called from browse()
	string const browseFile( string const & filename,
				 string const & title, string const & pattern, 
				 std::pair<string,string> const & dir1,
				 std::pair<string,string> const & dir2 ) const;

	/// Type definitions from the fdesign produced header file.
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
	FD_form_spellchecker * build_spellchecker();

	/// Real GUI implementation.
	FD_form_preferences * dialog_;
	/// Converters tabfolder
	FD_form_outer_tab * converters_tab_;
	/// reLyX and other import/input stuff
	FD_form_outer_tab * inputs_tab_;
	/// HCI configuration
	FD_form_outer_tab * look_n_feel_tab_;
	/// Outputs tabfolder
	FD_form_outer_tab * outputs_tab_;
	/// Spellchecker, language stuff, etc
	FD_form_outer_tab * usage_tab_;

	/** Flag whether a warning has been posted to the text window.
	    If so, don't redraw the window when the mouse leaves an object. */
	bool warningPosted;
	
	/** Each tab folder is encapsulated in its own class.
	 */

	class Colors {
	public:
		///
		Colors( FormPreferences & p ) : parent_(p), dialog_(0) {}
		///
		~Colors();
		///
		FD_form_colors const * dialog() { return dialog_; }
		///
		void apply(); // not const as modifies modifiedXformPrefs.
		///
		void build();
		///
		string const feedback(FL_OBJECT const * const) const;
		///
		bool input(FL_OBJECT const * const);
		///
		void update();
		/// Flag whether Xforms colors have changed since last file save
		bool modifiedXformPrefs;

	private:
		///
		void AdjustVal( int, int, double ) const;
		///
		bool BrowserLyX() const;
		///
		bool BrowserX11() const;
		///
		bool Database();
		///
		void LoadBrowserLyX();
		///
		bool LoadBrowserX11(string const &) const;
		///
		bool Modify() const;
		///
		bool RGB() const;
		///
		int SearchEntry(RGBColor const &) const;

		///
		FormPreferences & parent_;
		///
		FD_form_colors * dialog_;
		/// A vector of RGB colors and associated name.
		static std::vector<X11Color> colorDB;
		/// A vector of xform RGB colors and associated name.
		static std::vector<XformColor> xformColorDB;
	};
	///
	friend class Colors;

	///
	class Converters {
	public:
		///
		Converters( FormPreferences &  p ) : parent_(p), dialog_(0) {}
		///
		~Converters();
		///
		FD_form_converters const * dialog() { return dialog_; }
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
		bool Delete();
		///
		bool Input();
		///
		string const GetFrom() const;
		///
		string const GetTo() const;

		///
		FormPreferences & parent_;
		///
		FD_form_converters * dialog_;
	};
	///
	friend class Converters;

	///
	class Formats {
	public:
		///
		Formats( FormPreferences &  p ) : parent_(p), dialog_(0) {}
		///
		~Formats();
		///
		FD_form_formats const * dialog() { return dialog_; }
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
		bool Delete();
		///
		bool Input();

		///
		FormPreferences & parent_;
		///
		FD_form_formats * dialog_;
	};
	///
	friend class Formats;

	///
	class InputsMisc {
	public:
		///
		InputsMisc( FormPreferences &  p ) : parent_(p), dialog_(0) {}
		///
		~InputsMisc();
		///
		FD_form_inputs_misc const * dialog() { return dialog_; }
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
		FD_form_inputs_misc * dialog_;
	};
	///
	friend class InputsMisc;
	
	///
	class Interface {
	public:
		///
		Interface( FormPreferences &  p ) : parent_(p), dialog_(0) {}
		///
		~Interface();
		///
		FD_form_interface const * dialog() { return dialog_; }
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
		FD_form_interface * dialog_;
	};
	///
	friend class Interface;

	///
	class Language {
	public:
		///
		Language( FormPreferences &  p )
			: parent_(p), dialog_(0), combo_default_lang(0) {}
		///
		~Language();
		///
		FD_form_language const * dialog() { return dialog_; }
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
		FD_form_language * dialog_;
		///
		Combox * combo_default_lang;
	};
	///
	friend class Language;
	
	///
	class LnFmisc {
	public:
		///
		LnFmisc( FormPreferences &  p ) : parent_(p), dialog_(0) {}
		///
		~LnFmisc();
		///
		FD_form_lnf_misc const * dialog() { return dialog_; }
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
		FD_form_lnf_misc * dialog_;
	};
	///
	friend class LnFmisc;

	///
	class OutputsMisc {
	public:
		///
		OutputsMisc( FormPreferences &  p ) : parent_(p), dialog_(0) {}
		///
		~OutputsMisc();
		///
		FD_form_outputs_misc const * dialog() { return dialog_; }
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
		FD_form_outputs_misc * dialog_;
	};
	///
	friend class OutputsMisc;

	///
	class Paths {
	public:
		///
		Paths( FormPreferences &  p ) : parent_(p), dialog_(0) {}
		///
		~Paths();
		///
		FD_form_paths const * dialog() { return dialog_; }
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
		FD_form_paths * dialog_;
	};
	///
	friend class Paths;

	///
	class Printer {
	public:
		///
		Printer( FormPreferences &  p ) : parent_(p), dialog_(0) {}
		///
		~Printer();
		///
		FD_form_printer const * dialog() { return dialog_; }
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
		FD_form_printer * dialog_;
	};
	///
	friend class Printer;

	///
	class ScreenFonts {
	public:
		///
		ScreenFonts( FormPreferences &  p ) : parent_(p), dialog_(0) {}
		///
		~ScreenFonts();
		///
		FD_form_screen_fonts const * dialog() { return dialog_; }
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
		FD_form_screen_fonts * dialog_;
	};
	///
	friend class ScreenFonts;

	///
	class SpellChecker {
	public:
		///
		SpellChecker( FormPreferences &  p ) : parent_(p), dialog_(0) {}
		///
		~SpellChecker();
		///
		FD_form_spellchecker const * dialog() { return dialog_; }
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
		FD_form_spellchecker * dialog_;
	};
	///
	friend class SpellChecker;

	
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
	SpellChecker spellchecker_;
};

#endif
