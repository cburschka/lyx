// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2000-2001 The LyX Team.
 *
 * ======================================================
 *
 * \file FormGraphics.h
 * \author Baruch Even, baruch.even@writeme.com
 */

#ifndef FORMGRAPHICS_H
#define FORMGRAPHICS_H

#ifdef __GNUG__
#pragma interface
#endif 

#include "FormBase.h"
#include "RadioButtonGroup.h"

class ControlGraphics;
struct FD_form_graphics;

/** This class provides an XForms implementation of the Graphics Dialog.
 */
class FormGraphics : public FormCB<ControlGraphics, FormDB<FD_form_graphics> > {
public:
	///
	FormGraphics(ControlGraphics &);

private:
	/// Set the Params variable for the Controller.
	virtual void apply();
	/// Build the dialog.
	virtual void build();
	/// Update dialog before/whilst showing it.
	virtual void update();
	/// Filter the inputs on callback from xforms
	virtual ButtonPolicy::SMInput input(FL_OBJECT *, long);

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
	///
	enum State {
		///
		CHECKINPUT,
		///
		BROWSE,
		///
		ADVANCEDINPUT
	};

	/// Verify that the input is correct. If not disable ok/apply buttons.
	ButtonPolicy::SMInput checkInput();

	/// Fdesign generated method
	FD_form_graphics * build_graphics();

	/// The radio buttons groups
	RadioButtonGroup widthButtons;
	///
	RadioButtonGroup heightButtons;
	///
	RadioButtonGroup displayButtons;
	/// Last used figure path
	string last_image_path;
};


#endif  // FORMGRAPHICS_H
