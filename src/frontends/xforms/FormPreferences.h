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
struct FD_form_bind;
struct FD_form_misc;
struct FD_form_screen_fonts;
struct FD_form_interface_fonts;
struct FD_form_printer;
struct FD_form_paths;

/** This class provides an XForms implementation of the FormPreferences Dialog.
    The preferences dialog allows users to set/save their preferences.
 */
class FormPreferences : public FormBase {
public:
	/// #FormPreferences x(LyXFunc ..., Dialogs ...);#
	FormPreferences(LyXView *, Dialogs *);
	///
	~FormPreferences();
private:
	///
	virtual void connect();
	/// Update the dialog.
	virtual void update();
	/// OK from dialog
	virtual void ok();
	/// Apply from dialog
	virtual void apply();
	/// Filter the inputs -- return true if entries are valid
	virtual bool input(long);
	/// Build the dialog
	virtual void build();
	///
	virtual FL_FORM * const form() const;
	///
	FD_form_preferences * build_preferences();
	///
	FD_form_bind * build_bind();
	///
	FD_form_misc * build_misc();
	///
	FD_form_screen_fonts * build_screen_fonts();
	///
	FD_form_interface_fonts * build_interface_fonts();
	///
	FD_form_printer * build_printer();
	///
	FD_form_paths * build_paths();

	/// Real GUI implementation.
	FD_form_preferences * dialog_;
	///
	FD_form_bind * bind_;
	///
	FD_form_misc * misc_;
	///
	FD_form_screen_fonts * screen_fonts_;
	///
	FD_form_interface_fonts * interface_fonts_;
	///
	FD_form_printer * printer_;
	///
	FD_form_paths * paths_;
	/// Overcome a dumb xforms sizing bug
	int minw_;
	///
	int minh_;
};

#endif
