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
 * ======================================================
 */

#ifndef FORMGRAPHICS_H
#define FORMGRAPHICS_H

#include <boost/smart_ptr.hpp>

#ifdef __GNUG__
#pragma interface
#endif 

#include "LString.h"
#include "RadioButtonGroup.h"
#include "FormInset.h"

// Forward declarations for classes we use only as pointers.
class InsetGraphics;

struct FD_form_graphics;

/** This class provides an XForms implementation of the FormGraphics Dialog.
 *
 *  @Author Baruch Even <baruch.even@writeme.com>
 */
class FormGraphics : public FormInset {
public:
	/// #FormGraphics x(LyXFunc ..., Dialogs ...);#
	FormGraphics(LyXView *, Dialogs *);
	///
	~FormGraphics();

private:
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

	/// Pointer to the actual instantiation of the ButtonController.
	virtual xformsBC & bc();
	/// Build the dialog
	virtual void build();
	/// Filter the inputs
	virtual bool input( FL_OBJECT *, long );
	/// Update the popup.
	virtual void update();
	/// Apply from popup
	virtual void apply();

	/// Disconnect signals. Also perform any necessary housekeeping.
	virtual void disconnect();

	/// Save the active inset and show the dialog.
	void showDialog(InsetGraphics *);
	/// Verify that the input is correct. If not disable ok/apply buttons.
	bool checkInput();
	/// Open the file browse dialog to select an image file.
	void browse();

	/// Pointer to the actual instantiation of the xforms form
	virtual FL_FORM * form() const;
	/// Fdesign generated method
	FD_form_graphics * build_graphics();

	/// Real GUI implementation.
	boost::scoped_ptr<FD_form_graphics> dialog_;
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
	/// Last used figure path
	string last_image_path;
	/// The ButtonController
	ButtonController<NoRepeatedApplyReadOnlyPolicy, xformsBC> bc_;
};


inline
xformsBC & FormGraphics::bc()
{
	return bc_;
}
#endif 
