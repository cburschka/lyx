/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2000-2001 The LyX Team.
 *
 * ======================================================
 *
 * \file FormGraphics.C
 * \author Baruch Even, baruch.even@writeme.com
 */

#include <config.h> 

#ifdef __GNUG__
#pragma implementation
#endif 

#include "xformsBC.h"
#include "ControlGraphics.h"
#include "FormGraphics.h"
#include "form_graphics.h"

#include "xforms_helpers.h"
#include "input_validators.h"
#include "debug.h" // for lyxerr
#include "support/lstrings.h"  // for strToDbl & tostr
#include "support/FileInfo.h"  // for FileInfo
#include "insets/insetgraphicsParams.h"
#include "lyxrc.h" // for lyxrc.display_graphics

using std::endl;

namespace {

// Zero test for double precision numbers
double const tol = 1.0e-08;

// The maximum digits for the image scale
int const SCALE_MAXDIGITS = 3;

// The maximum digits for the image width
int const WIDTH_MAXDIGITS = 3;

// The maximum digits for the image height
int const HEIGHT_MAXDIGITS = 3;

// The max characters in the rotation angle (minus sign and 3 digits)
int const ROTATE_MAXCHARS = 4;

// The maximum characters in a filename.
int const FILENAME_MAXCHARS = 1024;
 
} // namespace anon


typedef FormCB<ControlGraphics, FormDB<FD_form_graphics> > base_class;

FormGraphics::FormGraphics(ControlGraphics & c)
	: base_class(c, _("Graphics"))
{}


void FormGraphics::build()
{
	dialog_.reset(build_graphics());

	// This is the place to add settings of the dialog that did not go
	// to the .fd file.

	// Set the input widgets to issue a callback to input() whenever
	// they change, so we can verify their content.
	fl_set_input_return (dialog_->input_filename,     FL_RETURN_CHANGED);
	fl_set_input_return (dialog_->input_scale,        FL_RETURN_CHANGED);
	fl_set_input_return (dialog_->input_width,        FL_RETURN_CHANGED);
	fl_set_input_return (dialog_->input_height,       FL_RETURN_CHANGED);
	fl_set_input_return (dialog_->input_rotate_angle, FL_RETURN_CHANGED);
	fl_set_input_return (dialog_->input_subcaption,   FL_RETURN_CHANGED);

	// Set the maximum characters that can be written in the input texts.
	fl_set_input_maxchars(dialog_->input_scale,        SCALE_MAXDIGITS);
	fl_set_input_maxchars(dialog_->input_width,        WIDTH_MAXDIGITS);
	fl_set_input_maxchars(dialog_->input_height,       HEIGHT_MAXDIGITS);
	fl_set_input_maxchars(dialog_->input_filename,     FILENAME_MAXCHARS);
	fl_set_input_maxchars(dialog_->input_rotate_angle, ROTATE_MAXCHARS);

	// Set input filter on width and height to make them accept only
	// unsigned numbers.
	fl_set_input_filter(dialog_->input_scale,  fl_unsigned_float_filter);
	fl_set_input_filter(dialog_->input_width,  fl_unsigned_float_filter);
	fl_set_input_filter(dialog_->input_height, fl_unsigned_float_filter);

	// Set input filter on rotate_angle to make it accept only
	// floating point numbers.
	fl_set_input_filter(dialog_->input_rotate_angle, fl_float_filter);

	// Create the contents of the choices
	string const width = " cm | inch | page %% | column %% ";
	fl_addto_choice(dialog_->choice_width_units, width.c_str());

	string const height = " cm | inch | page %% ";
	fl_addto_choice(dialog_->choice_height_units, height.c_str());

        // Manage the ok, apply, restore and cancel/close buttons
	bc().setOK(dialog_->button_ok);
	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_cancel);
	bc().setRestore(dialog_->button_restore);

	bc().addReadOnly(dialog_->input_filename);
	bc().addReadOnly(dialog_->button_browse);
	bc().addReadOnly(dialog_->check_display);
	bc().addReadOnly(dialog_->input_scale);
	bc().addReadOnly(dialog_->input_width);
	bc().addReadOnly(dialog_->choice_width_units);
	bc().addReadOnly(dialog_->input_height);
	bc().addReadOnly(dialog_->choice_height_units);
	bc().addReadOnly(dialog_->input_rotate_angle);
	bc().addReadOnly(dialog_->input_subcaption);
	bc().addReadOnly(dialog_->check_subcaption);
}


void FormGraphics::apply()
{
	// Create the parameters structure and fill the data from the dialog.
	InsetGraphicsParams & igp = controller().params();

	igp.filename = fl_get_input(dialog_->input_filename);

	if (lyxrc.display_graphics == "no") {
		igp.display = InsetGraphicsParams::NONE;

	} else {
		if (fl_get_button(dialog_->check_display)) {
			if (lyxrc.display_graphics == "mono") {
				igp.display = InsetGraphicsParams::MONOCHROME;
			} else if (lyxrc.display_graphics == "gray") {
				igp.display = InsetGraphicsParams::GRAYSCALE;
			} else if (lyxrc.display_graphics == "color") {
				igp.display = InsetGraphicsParams::COLOR;
			}
			
		} else {
			igp.display = InsetGraphicsParams::NONE;
		}
	}

	double const scale =
		strToDbl(strip(fl_get_input(dialog_->input_scale)));
	if (scale < tol) {
		double const width =
			strToDbl(strip(fl_get_input(dialog_->input_width)));

		if (width < tol) {
			igp.widthResize = InsetGraphicsParams::DEFAULT_SIZE;
			igp.widthSize   = 0.0;
		} else {
			switch (fl_get_choice(dialog_->choice_width_units)) {
			case 2:
				igp.widthResize = InsetGraphicsParams::INCH;
				break;
			case 3:
				igp.widthResize =
					InsetGraphicsParams::PERCENT_PAGE;
				break;
			case 4:
				igp.widthResize =
					InsetGraphicsParams::PERCENT_COLUMN;
				break;
			default:
				igp.widthResize = InsetGraphicsParams::CM;
				break;
			}
			igp.widthSize = width;
		}
		
		double const height =
			strToDbl(strip(fl_get_input(dialog_->input_height)));
		
		if (height < tol) {
			igp.heightResize = InsetGraphicsParams::DEFAULT_SIZE;
			igp.heightSize   = 0.0;
		} else {
			switch (fl_get_choice(dialog_->choice_height_units)) {
			case 2:
				igp.heightResize = InsetGraphicsParams::INCH;
				break;
			case 3:
				igp.heightResize =
					InsetGraphicsParams::PERCENT_PAGE;
				break;
			default:
				igp.heightResize = InsetGraphicsParams::CM;
				break;
			}
			igp.heightSize = height;
		}
		
	} else {
		igp.widthResize  = InsetGraphicsParams::DEFAULT_SIZE;
		igp.widthSize    = 0.0;
		igp.heightResize = InsetGraphicsParams::SCALE;
		igp.heightSize   = scale;
	}
	
	igp.rotateAngle =
		strToDbl(strip(fl_get_input(dialog_->input_rotate_angle)));
	while (igp.rotateAngle < 0.0 || igp.rotateAngle > 360.0) {
		if (igp.rotateAngle < 0.0) {
			igp.rotateAngle += 360.0;
		} else if (igp.rotateAngle > 360.0) {
			igp.rotateAngle -= 360.0;
		}
	}

	igp.subcaption = fl_get_button(dialog_->check_subcaption);
	igp.subcaptionText = fl_get_input(dialog_->input_subcaption);

	igp.testInvariant();
}


void FormGraphics::update()
{
	// Update dialog with details from inset
	InsetGraphicsParams & igp = controller().params();

	// Update the filename input field
	fl_set_input(dialog_->input_filename,
	             igp.filename.c_str());

	// To display or not to display
	if (lyxrc.display_graphics == "no") {
		fl_set_button(dialog_->check_display, 0);
	} else {
		if (igp.display == InsetGraphicsParams::NONE) {
			fl_set_button(dialog_->check_display, 0);
		} else {
			fl_set_button(dialog_->check_display, 1);
		}
	}

	setEnabled(dialog_->check_display, (lyxrc.display_graphics != "no"));

	if (igp.heightResize == InsetGraphicsParams::SCALE) {
		string number = tostr(igp.heightSize);
		fl_set_input(dialog_->input_scale, number.c_str());
		fl_set_input(dialog_->input_width, "");
		fl_set_choice(dialog_->choice_width_units, 1);
		fl_set_input(dialog_->input_height, "");
		fl_set_choice(dialog_->choice_height_units, 1);

	} else {
		fl_set_input(dialog_->input_scale, "");

		string number;
		if (igp.widthResize != InsetGraphicsParams::DEFAULT_SIZE) {
			number = tostr(igp.widthSize);
		}
		fl_set_input(dialog_->input_width, number.c_str());

		int pos = 1;
		switch (igp.widthResize) {
		case InsetGraphicsParams::INCH:
			pos = 2; break;

		case InsetGraphicsParams::PERCENT_PAGE:
			pos = 3; break;

		case InsetGraphicsParams::PERCENT_COLUMN:
			pos = 4; break;

		default:
			break;
		}
		fl_set_choice(dialog_->choice_width_units, pos);

		number.erase();
		if (igp.heightResize != InsetGraphicsParams::DEFAULT_SIZE) {
			number = tostr(igp.heightSize);
		}
		fl_set_input(dialog_->input_height, number.c_str());

		pos = 1;
		switch (igp.heightResize) {
		case InsetGraphicsParams::INCH:
			pos = 2; break;

		case InsetGraphicsParams::PERCENT_PAGE:
			pos = 3; break;

		default:
			break;
		}
		fl_set_choice(dialog_->choice_height_units, pos);
	}
		
	// Update the rotate angle
	fl_set_input(dialog_->input_rotate_angle,
	             tostr(igp.rotateAngle).c_str());

	// Update the subcaption check button and input field
	fl_set_button(dialog_->check_subcaption,
	              igp.subcaption);
	fl_set_input(dialog_->input_subcaption,
        	     igp.subcaptionText.c_str());

	setEnabled(dialog_->input_subcaption,
		   fl_get_button(dialog_->check_subcaption));
}


ButtonPolicy::SMInput FormGraphics::input(FL_OBJECT * ob, long)
{
	if (ob == dialog_->button_browse) {
		// Get the filename from the dialog
		string const in_name = fl_get_input(dialog_->input_filename);
		string const out_name = controller().Browse(in_name);

		if (out_name != in_name && !out_name.empty()) {
			fl_set_input(dialog_->input_filename, out_name.c_str());
		}
	}

	if (ob == dialog_->input_scale) {
		double const scale =
			strToDbl(strip(fl_get_input(dialog_->input_scale)));
		if (scale > tol) {
			fl_set_input(dialog_->input_width, "");
			fl_set_choice(dialog_->choice_width_units, 1);
			fl_set_input(dialog_->input_height, "");
			fl_set_choice(dialog_->choice_height_units, 1);
		}
	}

	if (ob == dialog_->input_width || ob == dialog_->input_height) {
		double const width =
			strToDbl(strip(fl_get_input(dialog_->input_width)));
		double const height =
			strToDbl(strip(fl_get_input(dialog_->input_height)));

		if (width > tol || height > tol) {
			fl_set_input(dialog_->input_scale, "");
		}
	}

	if (ob == dialog_->check_subcaption) {
	    	setEnabled(dialog_->input_subcaption,
			   fl_get_button(dialog_->check_subcaption));
	}

	return checkInput();
}


ButtonPolicy::SMInput FormGraphics::checkInput()
{
	// Put verifications that the dialog shows some sane values,
	// if not disallow clicking on ok/apply.
	// Possibly use a label in the bottom of the dialog to give the reason.

	ButtonPolicy::SMInput activate = ButtonPolicy::SMI_VALID;

	// We verify now that there is a filename, it exists, it's a file
	// and it's readable.
	string filename = fl_get_input(dialog_->input_filename);
	FileInfo file(filename);
	if (filename.empty()
	        || !file.isOK()
	        || !file.exist()
	        || !file.isRegular()
	        || !file.readable()
	   )
		activate = ButtonPolicy::SMI_INVALID;

	return activate;
}
