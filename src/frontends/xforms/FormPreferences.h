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

#include "DialogBase.h"
#include "support/utility.hpp"

#ifdef __GNUG_
#pragma interface
#endif

class LyXView;
class Dialogs;
class PreferencesPolicy;
template <class x> class ButtonController;

struct FD_form_preferences;
struct FD_form_bind;
struct FD_form_misc;
struct FD_form_screen_fonts;
struct FD_form_interface_fonts;
struct FD_form_printer;
struct FD_form_paths;

#ifdef SIGC_CXX_NAMESPACES
using SigC::Connection;
#endif

/** This class provides an XForms implementation of the FormPreferences Dialog.
    The preferences dialog allows users to set/save their preferences.
 */
class FormPreferences : public DialogBase, public noncopyable {
public:
	/**@name Constructors and Destructors */
	//@{
	/// #FormPreferences x(LyXFunc ..., Dialogs ...);#
	FormPreferences(LyXView *, Dialogs *);
	///
	~FormPreferences();
	//@}

	/**@name Real per-instance Callback Methods */
	//@{
	static  int WMHideCB(FL_FORM *, void *);
	static void OKCB(FL_OBJECT *, long);
	static void ApplyCB(FL_OBJECT *, long);
	static void CancelCB(FL_OBJECT *, long);
	static void InputCB(FL_OBJECT *, long);
	static void RestoreCB(FL_OBJECT *, long);
	//@}

private:
	/**@name Slot Methods */
	//@{
	/// Create the dialog if necessary, update it and display it.
	void show();
	/// Hide the dialog.
	void hide();
	/// Update the dialog.
	void update();
	//@}

	/**@name Dialog internal methods */
	//@{
	/// Apply from dialog
	void apply();
	/// Filter the inputs -- return true if entries are valid
	bool input();
	/// Build the dialog
	void build();
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
	//@}

	/**@name Private Data */
	//@{
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
	/// Which LyXView do we belong to?
	LyXView * lv_;
	///
	Dialogs * d_;
	/// Update connection.
	Connection u_;
	/// Hide connection.
	Connection h_;
	/// Overcome a dumb xforms sizing bug
	int minw_, minh_;
	///
	ButtonController<PreferencesPolicy> * bc_;
	//@}
};

#endif
