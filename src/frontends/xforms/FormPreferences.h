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
struct FD_form_lnf_general;
struct FD_form_screen_fonts;
struct FD_form_interface;
struct FD_form_printer;
struct FD_form_paths;
struct FD_form_outer_tab;
struct FD_form_outputs_general;

/** This class provides an XForms implementation of the FormPreferences Dialog.
    The preferences dialog allows users to set/save their preferences.
 */
class FormPreferences : public FormBaseBI {
public:
	/// #FormPreferences x(LyXFunc ..., Dialogs ...);#
	FormPreferences(LyXView *, Dialogs *);
	///
	~FormPreferences();
private:
	/// Update the dialog.
	virtual void update(bool = false);
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
	///
	virtual FL_FORM * form() const;
	///
	FD_form_preferences * build_preferences();
	///
	FD_form_outer_tab * build_outer_tab();
	///
	FD_form_lnf_general * build_lnf_general();
	///
	FD_form_screen_fonts * build_screen_fonts();
	///
	FD_form_interface * build_interface();
	///
	FD_form_printer * build_printer();
	///
	FD_form_paths * build_paths();
	///
	FD_form_outputs_general * build_outputs_general();

	/// Real GUI implementation.
	FD_form_preferences * dialog_;
	/// Outputs tabfolder
	FD_form_outer_tab * outputs_tab_;
	/// HCI configuration
	FD_form_outer_tab * look_n_feel_tab_;
	/// reLyX and other import/input stuff
	FD_form_outer_tab * inputs_tab_;
	///
	FD_form_lnf_general * lnf_general_;
	///
	FD_form_screen_fonts * screen_fonts_;
	///
	FD_form_interface * interface_;
	///
	FD_form_printer * printer_;
	///
	FD_form_paths * paths_;
	///
	FD_form_outputs_general * outputs_general_;
};

#endif
