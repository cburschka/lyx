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

#include "FormBase.h"

#ifdef __GNUG_
#pragma interface
#endif

class Combox;
class Command;
class Dialogs;
class Format;
class LyXView;
struct FD_form_colours;
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
	static void ComboLanguageCB(int, void *, Combox *);
	///
	static int FeedbackCB(FL_OBJECT *, int,
			      FL_Coord, FL_Coord, int, void *);

private:
	/// helper struct for Colours
	struct RGB {
		int r;
		int g;
		int b;
		RGB() : r(0), g(0), b(0) {}
		RGB(int red, int green, int blue) : r(red), g(green), b(blue) {}
	};
	///
	friend bool operator==(RGB const &, RGB const &);
	///
	friend bool operator!=(RGB const &, RGB const &);
	///
	typedef std::pair<string, RGB> X11Colour;

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

	/** Folder specific apply functions.
	 */
	
	///
	void applyColours() const;
	///
	void applyConverters() const;
	///
	void applyFormats() const;
	///
	void applyInputsMisc() const;
	///
	void applyInterface() const;
	///
	void applyLanguage(); // not const because calls updateLanguage!
	///
	void applyLnFmisc() const;
	///
	void applyOutputsMisc() const;
	///
	void applyPaths(); // not const because calls updatePaths!
	///
	void applyPrinter() const;
	///
	void applyScreenFonts() const;
	///
	void applySpellChecker(); // not const because calls updateSpellChecker!

	/** Folder specific build functions.
	 */
	
	///
	void buildColours();
	///
	void buildConverters();
	///
	void buildFormats();
	///
	void buildInputsMisc();
	///
	void buildInterface();
	///
	void buildLanguage();
	///
	void buildLnFmisc();
	///
	void buildOutputsMisc();
	///
	void buildPaths();
	///
	void buildPrinter();
	///
	void buildScreenFonts();
	///
	void buildSpellchecker();

	/** Folder specific feedback functions.
	 */
	
	///
	string const feedbackColours(FL_OBJECT const * const) const;
	///
	string const feedbackConverters(FL_OBJECT const * const) const;
	///
	string const feedbackFormats( FL_OBJECT const * const ) const;
	///
	string const feedbackInputsMisc(FL_OBJECT const * const) const;
	///
	string const feedbackInterface(FL_OBJECT const * const) const;
	///
	string const feedbackLanguage(FL_OBJECT const * const) const;
	///
	string const feedbackLnFmisc(FL_OBJECT const * const) const;
	///
	string const feedbackOutputsMisc(FL_OBJECT const * const) const;
	///
	string const feedbackPaths(FL_OBJECT const * const) const;
	///
	string const feedbackPrinter(FL_OBJECT const * const) const;
	///
	string const feedbackScreenFonts(FL_OBJECT const * const) const;
	///
	string const feedbackSpellChecker(FL_OBJECT const * const) const;

	/** Folder specific input functions. Not all folders require one.
	 */
	
	///
	bool inputColours(FL_OBJECT const * const);
	///
	bool inputConverters( FL_OBJECT const * const );
	///
	bool inputFormats( FL_OBJECT const * const );
	///
	bool inputLanguage(FL_OBJECT const * const);
	///
	bool inputPaths(FL_OBJECT const * const);
	///
	bool inputScreenFonts();
	///
	bool inputSpellChecker(FL_OBJECT const * const);

	/** Folder specific update functions.
	 */
	
	///
	void updateColours();
	///
	void updateConverters();
	///
	void updateFormats();
	///
	void updateInputsMisc();
	///
	void updateInterface();
	///
	void updateLanguage();
	///
	void updateLnFmisc();
	///
	void updateOutputsMisc();
	///
	void updatePaths();
	///
	void updatePrinter();
	///
	void updateScreenFonts();
	///
	void updateSpellChecker();

	/** Some helper functions for specific folders.
	 */
	
	///
	bool ColoursLoadBrowser( string const & );
	///
	int  ColoursSearchEntry(RGB const & ) const;
	///
	void ColoursUpdateBrowser( int );
	///
	void ColoursUpdateRGB();

	///
	bool ConvertersAdd();
	///
	bool ConvertersBrowser();
	///
	void ConvertersClear() const;
	///
	bool ConvertersContainFormat( Format const &) const;
	///
	bool ConvertersDelete();
	///
	bool ConvertersInput();
	///
	bool ConvertersSetCommand( Command & ) const;
	///
	void ConvertersUpdateChoices();

	bool FormatsAdd();
	///
	bool FormatsBrowser();
	///
	void FormatsClear() const;
	///
	bool FormatsDelete();
	///
	bool FormatsInput();

	///
	bool WriteableDir( string const & );
	///
	bool ReadableDir( string const & );
	///
	bool WriteableFile( string const &, string const & = string() );

	///
	void setPreHandler( FL_OBJECT * ) const;
	///
	void printWarning( string const & );

	/// Type definitions from the fdesign produced header file.
	FD_form_preferences * build_preferences();
	///
	FD_form_outer_tab * build_outer_tab();
	///
	FD_form_colours * build_colours();
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
	///
	FD_form_colours * colours_;
	///
	FD_form_converters * converters_;
	///
	FD_form_formats * formats_;
	///
	FD_form_inputs_misc * inputs_misc_;
	///
	FD_form_interface * interface_;
	///
	FD_form_language * language_;
	///
	FD_form_lnf_misc * lnf_misc_;
	///
	FD_form_outputs_misc * outputs_misc_;
	///
	FD_form_paths * paths_;
	///
	FD_form_printer * printer_;
	///
	FD_form_screen_fonts * screen_fonts_;
	///
	FD_form_spellchecker * spellchecker_;
	///
	Combox * combo_default_lang;
	///
	Combox * combo_kbmap_1;
	///
	Combox * combo_kbmap_2;

	/// A vector of Formats, to be manipulated in the Format browser.
	std::vector<Format> formats_vec;
	/// A vector of Commands, to be manipulated in the Converter browser.
	std::vector<Command> commands_vec;
	/// A vector of RGB colours and associated name.
	static std::vector<X11Colour> colourDB;
	/** A collection of kmap files.
	    First entry is the file name, full path.
	    Second entry is the shorthand, as appears in the fl_choice.
	    Eg, system_lyxdir/kbd/american2.kmap, american2
	*/
	static std::pair<std::vector<string>, std::vector<string> > dirlist;
	///
	bool warningPosted;
};

inline
bool operator==(FormPreferences::RGB const & c1,
		FormPreferences::RGB const & c2)
{
	return (c1.r == c2.r && c1.g == c2.g && c1.b == c2.b);
}


inline
bool operator!=(FormPreferences::RGB const & c1,
		FormPreferences::RGB const & c2)
{
	return !(c1 == c2);
}

#endif
