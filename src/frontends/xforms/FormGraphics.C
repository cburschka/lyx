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

#include "input_validators.h"
#include "debug.h" // for lyxerr
#include "support/lstrings.h"  // for strToDbl & tostr
#include "support/FileInfo.h"  // for FileInfo
#include "insets/insetgraphicsParams.h"

using std::endl;

typedef FormCB<ControlGraphics, FormDB<FD_form_graphics> > base_class;

FormGraphics::FormGraphics(ControlGraphics & c)
	: base_class(c, _("Graphics")),
	  // The buttons c-tor values are the number of buttons we use
	  // This is only to reduce memory waste.
	  widthButtons(5), heightButtons(4), displayButtons(4),
	  last_image_path(".")
{}


void FormGraphics::hide()
{
	// Remove all associations for the radio buttons
	widthButtons.reset();
	heightButtons.reset();
	displayButtons.reset();

	FormBase::hide();
}


void FormGraphics::build()
{
	dialog_.reset(build_graphics());

	// This is the place to add settings of the dialog that did not go
	// to the .fd file.

	// Set the input widgets to issue a callback to input() whenever
	// they change, so we can verify their content.
	fl_set_input_return (dialog_->input_width,
	                      FL_RETURN_CHANGED);
	fl_set_input_return (dialog_->input_height,
	                      FL_RETURN_CHANGED);
	fl_set_input_return (dialog_->input_filename,
	                      FL_RETURN_CHANGED);
	//    fl_set_input_return(dialog_->input_rotate_angle,
	//            FL_RETURN_CHANGED);

	// Set the maximum characters that can be written in the input texts.
	fl_set_input_maxchars(dialog_->input_width, WIDTH_MAXDIGITS);
	fl_set_input_maxchars(dialog_->input_height, HEIGHT_MAXDIGITS);
	fl_set_input_maxchars(dialog_->input_filename, FILENAME_MAXCHARS);
	fl_set_input_maxchars(dialog_->input_rotate_angle, ROTATE_MAXCHARS);

	// Set input filter on width and height to make them accept only
	// unsigned numbers.
	fl_set_input_filter(dialog_->input_width,
	                    fl_unsigned_int_filter);
	fl_set_input_filter(dialog_->input_height,
	                    fl_unsigned_int_filter);


	// Add the widgets of the width radio buttons to their group
	widthButtons.reset();
	widthButtons.registerRadioButton(dialog_->radio_width_default,
	                                 InsetGraphicsParams::DEFAULT_SIZE);
	widthButtons.registerRadioButton(dialog_->radio_width_cm,
	                                 InsetGraphicsParams::CM);
	widthButtons.registerRadioButton(dialog_->radio_width_inch,
	                                 InsetGraphicsParams::INCH);
	widthButtons.registerRadioButton(dialog_->radio_width_percent_page,
	                                 InsetGraphicsParams::PERCENT_PAGE);
	widthButtons.registerRadioButton(dialog_->radio_width_percent_column,
	                                 InsetGraphicsParams::PERCENT_COLUMN);

	// Add the widgets of the height radio buttons to their group
	heightButtons.reset();
	heightButtons.registerRadioButton(dialog_->radio_height_default,
	                                  InsetGraphicsParams::DEFAULT_SIZE);
	heightButtons.registerRadioButton(dialog_->radio_height_cm,
	                                  InsetGraphicsParams::CM);
	heightButtons.registerRadioButton(dialog_->radio_height_inch,
	                                  InsetGraphicsParams::INCH);
	heightButtons.registerRadioButton(dialog_->radio_height_percent_page,
	                                  InsetGraphicsParams::PERCENT_PAGE);

	// Add the widgets of the display radio buttons to their group
	displayButtons.reset();
	displayButtons.registerRadioButton(dialog_->radio_display_color,
	                                   InsetGraphicsParams::COLOR);
	displayButtons.registerRadioButton(dialog_->radio_display_grayscale,
	                                   InsetGraphicsParams::GRAYSCALE);
	displayButtons.registerRadioButton(dialog_->radio_display_monochrome,
	                                   InsetGraphicsParams::MONOCHROME);
	displayButtons.registerRadioButton(dialog_->radio_no_display,
	                                   InsetGraphicsParams::NONE);

        // Manage the ok, apply, restore and cancel/close buttons
	bc().setOK(dialog_->button_ok);
	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_cancel);
	bc().setUndoAll(dialog_->button_restore);
	bc().refresh();

	bc().addReadOnly(dialog_->input_filename);
	bc().addReadOnly(dialog_->button_browse);
	bc().addReadOnly(dialog_->input_width);
	bc().addReadOnly(dialog_->input_height);
	bc().addReadOnly(dialog_->radio_button_group_width);
	bc().addReadOnly(dialog_->radio_button_group_height);
	bc().addReadOnly(dialog_->radio_button_group_display);
	bc().addReadOnly(dialog_->input_rotate_angle);
	bc().addReadOnly(dialog_->check_inline);
	bc().addReadOnly(dialog_->input_subcaption);
	bc().addReadOnly(dialog_->check_subcaption);
}


void FormGraphics::apply()
{
	// Create the parameters structure and fill the data from the dialog.
	InsetGraphicsParams & igp = controller().params();

	igp.filename = fl_get_input(dialog_->input_filename);

	igp.display = static_cast < InsetGraphicsParams::DisplayType >
	              (displayButtons.getButton());

	igp.widthResize = static_cast < InsetGraphicsParams::Resize >
	                  (widthButtons.getButton());
	igp.widthSize = strToDbl(fl_get_input(dialog_->input_width));

	igp.heightResize = static_cast < InsetGraphicsParams::Resize >
	                   (heightButtons.getButton());
	igp.heightSize = strToDbl(fl_get_input(dialog_->input_height));

	igp.rotateAngle = strToInt(fl_get_input(dialog_->input_rotate_angle));
	if (igp.rotateAngle >= 360)
		igp.rotateAngle = igp.rotateAngle % 360;
	if (igp.rotateAngle <= -360)
		igp.rotateAngle = - (( -igp.rotateAngle) % 360);

	igp.subcaption = fl_get_button(dialog_->check_subcaption);
	igp.subcaptionText = fl_get_input(dialog_->input_subcaption);

	igp.inlineFigure = fl_get_button(dialog_->check_inline);

	igp.testInvariant();
}


void FormGraphics::update()
{
	// Update dialog with details from inset
	InsetGraphicsParams & igp = controller().params();

	// Update the filename input field
	fl_set_input(dialog_->input_filename,
	             igp.filename.c_str());

	// Update the display depth radio buttons
	displayButtons.setButton(igp.display);

	// Update the width radio buttons and input field
	widthButtons.setButton(igp.widthResize);
	fl_set_input(dialog_->input_width,
	             tostr(igp.widthSize).c_str());

	// Update the height radio buttons and input field
	heightButtons.setButton(igp.heightResize);
	fl_set_input(dialog_->input_height,
	             tostr(igp.heightSize).c_str());

	// Update the rotate angle
	fl_set_input(dialog_->input_rotate_angle,
	             tostr(igp.rotateAngle).c_str());

	// Update the subcaption check button and input field
	fl_set_button(dialog_->check_subcaption,
	              igp.subcaption);
	fl_set_input(dialog_->input_subcaption,
	             igp.subcaptionText.c_str());

	// Update the inline figure check button
	fl_set_button(dialog_->check_inline,
	              igp.inlineFigure);

	// Now make sure that the buttons are set correctly.
	input(0, 0);
}


ButtonPolicy::SMInput FormGraphics::input(FL_OBJECT *, long data)
{
	State cb = static_cast<State>( data );

	ButtonPolicy::SMInput activate = ButtonPolicy::SMI_NOOP;

	switch (cb) {
	case CHECKINPUT:
		activate = checkInput();
		break;
	case BROWSE:
	{
		// Get the filename from the dialog
		string const in_name = fl_get_input(dialog_->input_filename);
		string const out_name = controller().Browse(in_name);

		if (out_name != in_name && !out_name.empty()) {
			fl_set_input(dialog_->input_filename, out_name.c_str());
			input(0, 0);
		}
		break;
	}

	case ADVANCEDINPUT:
		lyxerr << "Advanced Options button depressed, "
		       << "show advanced options dialog"
		       << endl;
		break;
	default:
		break;
	}
	
	return activate;
}


ButtonPolicy::SMInput FormGraphics::checkInput()
{
	// Put verifications that the dialog shows some sane values,
	// if not disallow clicking on ok/apply.
	// Possibly use a label in the bottom of the dialog to give the reason.

	ButtonPolicy::SMInput activate = ButtonPolicy::SMI_VALID;

	// Things that we check (meaning they are incorrect states):
	// 1. No filename specified.
	// 2. Width radio button is not Default and width text is not a number.
	// 3. Height radio button is not Default and height text is a not a number

	// Note: radio button default means that the user asks for the image
	// to be included as is with no size change, in this case we don't need
	// any width or height.

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
		activate = ButtonPolicy::SMI_NOOP;

	// Width radio button not default and no number.
	if (!fl_get_button(dialog_->radio_width_default)
	        && strToDbl(fl_get_input(dialog_->input_width)) <= 0.0) {

		activate = ButtonPolicy::SMI_NOOP;
	}

	// Height radio button not default and no number.
	if (!fl_get_button(dialog_->radio_height_default)
	        && strToDbl(fl_get_input(dialog_->input_height)) <= 0.0) {

		activate = ButtonPolicy::SMI_NOOP;
	}

	return activate;
}
