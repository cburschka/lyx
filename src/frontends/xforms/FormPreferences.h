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

class LyXView;
class Dialogs;

struct FD_form_preferences;
struct FD_form_lnf_misc;
struct FD_form_screen_fonts;
struct FD_form_interface;
struct FD_form_printer;
struct FD_form_paths;
struct FD_form_outer_tab;
struct FD_form_inputs_misc;
struct FD_form_outputs_misc;
struct FD_form_spellchecker;
struct FD_form_language;
struct FD_form_colours;
class  Combox;

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
	static void FeedbackCB(FL_OBJECT *, long);
	///
	static int FeedbackPost(FL_OBJECT *ob, int, FL_Coord, FL_Coord,
				int, void *);

private:
	///
	enum State {
		///
		COLOURS,
		///
		INPUTSMISC,
		///
		INTERFACE,
		///
		LANGUAGE,
		///
		LOOKNFEELMISC,
		///
		OUTPUTSMISC,
		///
		PATHS,
		///
		PRINTER,
		///
		SCREENFONTS,
		///
		SPELLCHECKER
	};

	/// Update the dialog.
	virtual void update();
	///
	virtual void hide();
	/// OK from dialog
	virtual void ok();
	/// Apply from dialog
	virtual void apply();
	/// Restore from dialog
	virtual void restore();
	/// Filter the inputs -- return true if entries are valid
	virtual bool input(FL_OBJECT *, long);
	/// Build the dialog
	virtual void build();
	/// control which feedback message is output
	void feedback( FL_OBJECT * ob );
	///
	virtual FL_FORM * form() const;
	///
	void applyColours();
	///
	void applyInputsMisc();
	///
	void applyInterface();
	///
	void applyLanguage();
	///
	void applyLnFmisc();
	///
	void applyOutputsMisc();
	///
	void applyPaths();
	///
	void applyPrinter();
	///
	void applyScreenFonts();
	///
	void applySpellChecker();
	///
	void buildColours();
	///
	void buildInputsMisc();
	///
	void buildInterface();
	///
	void buildLanguage();
	///
	void addLanguages( Combox & );
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
	///
	void feedbackColours(FL_OBJECT const * const);
	///
	void feedbackInputsMisc(FL_OBJECT const * const);
	///
	void feedbackInterface(FL_OBJECT const * const);
	///
	void feedbackLanguage(FL_OBJECT const * const);
	///
	void feedbackLnFmisc(FL_OBJECT const * const);
	///
	void feedbackOutputsMisc(FL_OBJECT const * const);
	///
	void feedbackPaths(FL_OBJECT const * const);
	///
	void feedbackPrinter(FL_OBJECT const * const);
	///
	void feedbackScreenFonts(FL_OBJECT const * const);
	///
	void feedbackSpellChecker(FL_OBJECT const * const);
	///
	bool inputLanguage(FL_OBJECT const * const);
	///
	bool inputPaths(FL_OBJECT const * const);
	///
	bool inputScreenFonts();
	///
	bool inputSpellChecker(FL_OBJECT const * const);
	///
	void updateColours();
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

	///
	bool WriteableDir( string const & ) const;
	///
	bool ReadableDir( string const & ) const;
	///
	bool WriteableFile( string const &, string const & = string() ) const;
	///
	void setPostHandler( FL_OBJECT * ) const;

	///
	FD_form_preferences * build_preferences();
	///
	FD_form_outer_tab * build_outer_tab();
	///
	FD_form_colours * build_colours();
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
	/// Outputs tabfolder
	FD_form_outer_tab * outputs_tab_;
	/// HCI configuration
	FD_form_outer_tab * look_n_feel_tab_;
	/// reLyX and other import/input stuff
	FD_form_outer_tab * inputs_tab_;
	/// Spellchecker, language stuff, etc
	FD_form_outer_tab * usage_tab_;
	///
	FD_form_colours * colours_;
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
};

#endif
