// -*- C++ -*-
/* FormGraphics.h
 * FormGraphics Interface Class
 *
 * This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 *           This file Copyright 2000
 *           Baruch Even
 * ======================================================
 */

#ifndef FORMGRAPHICS_H
#define FORMGRAPHICS_H

#include <config.h>
#include "LString.h"
#include "frontends/DialogBase.h"
//#include "form_graphics.h"
#include "RadioButtonGroup.h"

#ifdef __GNUG__
#pragma interface
#endif

class Dialogs;
// same arguement as in Dialogs.h s/LyX/UI/
class LyXView;
class InsetGraphics;

struct FD_form_graphics;

/** This class provides an XForms implementation of the FormGraphics Dialog.
 */
class FormGraphics: public DialogBase {
public:
	/// #FormGraphics x(LyXFunc ..., Dialogs ...);#
	FormGraphics(LyXView *, Dialogs *);
	///
	~FormGraphics();
	///
	static  int WMHideCB(FL_FORM *, void *);
	///
	static void OKCB(FL_OBJECT *, long);
	///
	static void ApplyCB(FL_OBJECT *, long);
	///
	static void CancelCB(FL_OBJECT *, long);
	///
	static void BrowseCB(FL_OBJECT *, long);
	///
	static void AdvancedOptionsCB(FL_OBJECT *, long);
	///
	static void InputCB(FL_OBJECT *, long);

private:
	///
	FormGraphics()
		: widthButtons(5), heightButtons(4), displayButtons(4) {}
	//
	FormGraphics(FormGraphics const &) : DialogBase() {}
	
	/// The maximum digits for the image width (cm, inch, percent)
	enum {
		///
		WIDTH_MAXDIGITS = 3
	}; 
	/// The maximum digits for the image height (cm, inch, percent)
	enum {
		///
		HEIGHT_MAXDIGITS = 3
	};
	/// The maximum characters in the rotation angle (minus sign and 3 digits)
	enum {
		///
		ROTATE_MAXCHARS = 4
	};
	/// The maximum characters in a filename.
	enum {
		///
		FILENAME_MAXCHARS = 1024
	};
    
	/**@name Slot Methods */
	//@{
	/// Save the active inset and show the dialog.
	void showDialog(InsetGraphics * inset);
	/// Create the dialog if necessary, update it and display it.
	void show();
	/// Hide the dialog.
	void hide();
	/// Update the dialog
	void update();
	//@}

	/**@name Callback methods */
	//@{
	/// Apply the changes to the inset.
	void apply();
	/// Verify that the input is correct. If not disable ok/apply buttons.
	void input();
	/// Open the file browse dialog to select an image file.
	void browse();
	//@}
	
	/// Build the dialog
	void build();
	///
	FD_form_graphics * build_graphics();
	/// Explicitly free the dialog.
	void free();
	/// Display a file browser dialog and return the file chosen.
	string browseFile(string const & filename);

	/**@name Data */
	//@{
	/// Real GUI implementation.
	FD_form_graphics * dialog_;
	/** Which LyXFunc do we use?
	    We could modify Dialogs to have a visible LyXFunc* instead and
	    save a couple of bytes per dialog.
	*/
	LyXView * lv_;
	/** Which Dialogs do we belong to?
	    Used so we can get at the signals we have to connect to.
	*/
	Dialogs * d_;
	/** Which Inset do we belong to?
	   Used to set and update data to/from the inset.
	*/
	InsetGraphics * inset_;
	/// The radio buttons groups
	RadioButtonGroup widthButtons;
	///
	RadioButtonGroup heightButtons;
	///
	RadioButtonGroup displayButtons;

	/// Inset Hide connection, connected to the calling inset hide signal.
	Connection ih_;
	/// Hide connection.
	Connection h_;
	/// Update connection.
	Connection u_;
	/// Last used figure path
	string last_image_path;
	//@}

};

#endif


