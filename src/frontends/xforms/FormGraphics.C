/* FormGraphics.C
 * FormGraphics Interface Class Implementation
 */

/* TODO:
 *      * Handle the case when the buffer is read-only.
 *          Initial work is done, if we are read-only the ok/cancel are 
 *          disabled. Probably we need to find a better way to deal with it.
 *      
 */

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>

#include "lyx_gui_misc.h"
#include "gettext.h"
#include FORMS_H_LOCATION

#include "xform_macros.h"
#include "input_validators.h"
#include "FormGraphics.h"
#include "form_graphics.h"
#include "Dialogs.h"
#include "LyXView.h"
#include "BufferView.h"

#include "debug.h" // for lyxerr

#include "support/lstrings.h" // for strToDbl & tostr
#include "support/FileInfo.h" // for FileInfo
#include "filedlg.h" // for LyXFileDlg
#include "support/filetools.h" // for AddName
#include "insets/insetgraphics.h"
#include "insets/insetgraphicsParams.h"

#include "RadioButtonGroup.h"

#ifdef ENABLE_ASSERTIONS
#include "support/LAssert.h"
#endif

using std::endl;

C_RETURNCB(FormGraphics, WMHideCB)
C_GENERICCB(FormGraphics, OKCB)
C_GENERICCB(FormGraphics, ApplyCB)
C_GENERICCB(FormGraphics, CancelCB)
C_GENERICCB(FormGraphics, BrowseCB)    
C_GENERICCB(FormGraphics, AdvancedOptionsCB)
C_GENERICCB(FormGraphics, InputCB)

    
FormGraphics::FormGraphics(LyXView * lv, Dialogs * d)
	: dialog_(0), lv_(lv), d_(d), inset_(0), 
    // The buttons c-tor values are the number of buttons we use
    // This is only to reduce memory waste.
      widthButtons(5), heightButtons(4), displayButtons(4),
      ih_(0), h_(0), u_(0), 
      last_image_path(".")
{
	// let the dialog be shown
	// This is a permanent connection so we won't bother
	// storing a copy because we won't be disconnecting.
	d->showGraphics.connect(slot(this, &FormGraphics::showDialog));
}


FormGraphics::~FormGraphics()
{
	free();
}


void FormGraphics::build()
{
    dialog_ = build_graphics();
#ifdef ENABLE_ASSERTIONS
    Assert(dialog_ != 0);
#endif    
    if (!dialog_) 
        return;

    // This is the place to add settings of the dialog that did not go
    // to the .fd file.

    // Set the input widgets to issue a callback to input() whenever
    // they change, so we can verify their content.
    fl_set_input_return(dialog_->input_width,
            FL_RETURN_CHANGED);
    fl_set_input_return(dialog_->input_height,
            FL_RETURN_CHANGED);
    fl_set_input_return(dialog_->input_filename,
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
    
    // Connect a signal to hide the window when the window manager orders it.
    fl_set_form_atclose(dialog_->form,
			    C_FormGraphicsWMHideCB, 0);
}

void FormGraphics::show()
{
    // If the dialog doesn't exist yet, build it.
	if (!dialog_) {
		build();
	}

    // Update the form with the data from the inset.
    update();
    
    // If the form is visible
	if (dialog_->form->visible) {
        // Raise it.
		fl_raise_form(dialog_->form);
	} else {
        // Otherwise (invisible), show it.
		fl_show_form(dialog_->form,
			     FL_PLACE_MOUSE,
			     FL_FULLBORDER,
			     _("Graphics"));

        // And connect the signals 'updateBufferDependent',
        // 'hideBufferDependent' and 'hideGraphics'.
		u_ = d_->updateBufferDependent.connect(slot(this,
							    &FormGraphics::update));
		h_ = d_->hideBufferDependent.connect(slot(this,
							  &FormGraphics::hide));
	}
}

void FormGraphics::showDialog(InsetGraphics* inset)
{
#ifdef ENABLE_ASSERTIONS
    Assert(inset != 0);
#endif 

    // If we are connected to another inset, disconnect.
    if (inset_)
        ih_.disconnect();
    
    inset_ = inset;
    
    if (inset_) {
        ih_ = inset_->hide.connect(slot(this, &FormGraphics::hide));
        show();
    }
}


void FormGraphics::hide()
{
    // If the dialog exists, and the form is allocated and visible.
	if (dialog_
	        && dialog_->form
	        && dialog_->form->visible) {
        // Hide the form
		fl_hide_form(dialog_->form);

        // And disconnect the signals.
        u_.disconnect();
		h_.disconnect();
        ih_.disconnect();

        // Forget the inset.
        inset_ = 0;
	}
    // Most of the time, the dialog is not needed anymore, we'll free it
    // now to save memory.
    free();
}


void FormGraphics::free()
{
	// Remove all associations for the radio buttons
	widthButtons.reset();
	heightButtons.reset();
	displayButtons.reset();
            
	// Free the form.
	delete dialog_;
	dialog_ = 0;
}

void FormGraphics::apply()
{
#ifdef ENABLE_ASSERTIONS
    Assert(inset_ != 0);
#endif    
    if (! inset_)
        return;

    // Take all dialog details and insert them to the inset.

    // Create the parameters structure and fill the data from the dialog.
    InsetGraphicsParams igp;

    igp.filename = fl_get_input(dialog_->input_filename);

    igp.display = static_cast<InsetGraphicsParams::DisplayType>
	    (displayButtons.getButton());
    
    igp.widthResize = static_cast<InsetGraphicsParams::Resize>
	    (widthButtons.getButton());
    igp.widthSize = strToDbl(fl_get_input(dialog_->input_width));

    igp.heightResize = static_cast<InsetGraphicsParams::Resize>
	    (heightButtons.getButton());
    igp.heightSize = strToDbl(fl_get_input(dialog_->input_height));

    igp.rotateAngle = strToInt(fl_get_input(dialog_->input_rotate_angle));
    if (igp.rotateAngle >= 360)
        igp.rotateAngle = igp.rotateAngle % 360;
    if (igp.rotateAngle <= -360)
        igp.rotateAngle = - ((-igp.rotateAngle) % 360);
   
    igp.subcaption = fl_get_button(dialog_->check_subcaption);
    igp.subcaptionText = fl_get_input(dialog_->input_subcaption);

    igp.inlineFigure = fl_get_button(dialog_->check_inline);
    
#ifdef ENABLE_ASSERTIONS    
    igp.testInvariant();
#endif

    // Set the parameters in the inset, it also returns true if the new
    // parameters are different from what was in the inset already.
    bool changed = inset_->setParams(igp);

    // Tell LyX we've got a change, and mark the document dirty, if it changed.
    lv_->view()->updateInset(inset_, changed);
}

void FormGraphics::update()
{
#ifdef ENABLE_ASSERTIONS
    Assert(inset_ != 0);
#endif    
    if (! inset_)
        return;

    // Update dialog with details from inset
    InsetGraphicsParams igp = inset_->getParams();

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
    input();
}

void FormGraphics::input() 
{
    // Put verifications that the dialog shows some sane values,
    // if not disallow clicking on ok/apply.
    // Possibly use a label in the bottom of the dialog to give the reason.

    // Is all input boxes convey a valid meaning?
    bool inputOK = true;

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
        inputOK = false;

    // Width radio button not default and no number.
    if  (!fl_get_button(dialog_->radio_width_default)
           && strToDbl(fl_get_input(dialog_->input_width)) <= 0.0) {
            
        inputOK = false;
    }

    // Height radio button not default and no number.
    if (!fl_get_button(dialog_->radio_height_default) 
            && strToDbl(fl_get_input(dialog_->input_height)) <= 0.0) {
        
        inputOK = false;
    }


    // Now set the buttons to the correct state.
    if (inputOK && ! lv_->buffer()->isReadonly()) {
        fl_activate_object(dialog_->button_ok);
        fl_activate_object(dialog_->button_apply);
        fl_set_object_lcol(dialog_->button_ok, FL_BLACK);
        fl_set_object_lcol(dialog_->button_apply, FL_BLACK);
    } else {
        fl_deactivate_object(dialog_->button_ok);
        fl_deactivate_object(dialog_->button_apply);
        fl_set_object_lcol(dialog_->button_ok, FL_INACTIVE);
        fl_set_object_lcol(dialog_->button_apply, FL_INACTIVE);
    }
}


// We need these in the file browser.
extern string system_lyxdir;
extern string user_lyxdir;
//extern string system_tempdir;

// Need to move this to the form_graphics
string FormGraphics::browseFile(string const & filename)
{
    if (! filename.empty() )
        last_image_path = OnlyPath(filename);

    // Does user clipart directory exist?
    string bufclip = AddName (user_lyxdir, "clipart");	
    FileInfo fileInfo(bufclip);
    if (!(fileInfo.isOK() && fileInfo.isDir()))
        // No - bail out to system clipart directory
        bufclip = AddName (system_lyxdir, "clipart");	

    LyXFileDlg fileDlg;
    fileDlg.SetButton(0, _("Clipart"), bufclip); 

    bool error = false;
    string buf;
    do {
        string p = fileDlg.Select(_("Graphics"),
                last_image_path,
                "*(ps|png)", filename);

        if (p.empty()) return p;

        last_image_path = OnlyPath(p);

        if (p.find_first_of("#~$% ") != string::npos) {
			WriteAlert(_("Filename can't contain any "
				     "of these characters:"),
				   // xgettext:no-c-format
				   _("space, '#', '~', '$' or '%'.")); 
			error = true;
		} else {
            error = false;
            buf = p;
        }
	} while (error);

	return buf;
}

void FormGraphics::browse()
{
    // Get the filename from the dialog
    string filename = fl_get_input(dialog_->input_filename);

    // Show the file browser dialog
    string new_filename = browseFile(filename);

    // Save the filename to the dialog
    if (new_filename != filename && ! new_filename.empty()) {
        fl_set_input(dialog_->input_filename,
                     new_filename.c_str());
        // The above set input doesn't cause an input event so we do
        // it manually. Otherwise the user needs to cause an input event
        // to get the ok/apply buttons to be activated.
        input();
    }
    
}

int FormGraphics::WMHideCB(FL_FORM * form, void *)
{
	// Ensure that the signal h is disconnected even if the
	// window manager is used to close the dialog.
	FormGraphics * pre = static_cast<FormGraphics*>(form->u_vdata);
	pre->hide();
	return FL_CANCEL;
}


void FormGraphics::OKCB(FL_OBJECT * ob, long)
{
	FormGraphics * pre = static_cast<FormGraphics*>(ob->form->u_vdata);
    pre->apply();
	pre->hide();
}

void FormGraphics::ApplyCB(FL_OBJECT * ob, long)
{
	FormGraphics * pre = static_cast<FormGraphics*>(ob->form->u_vdata);
	pre->apply();
}

void FormGraphics::CancelCB(FL_OBJECT * ob, long)
{
	FormGraphics * pre = static_cast<FormGraphics*>(ob->form->u_vdata);
	pre->hide();
}

void FormGraphics::BrowseCB(FL_OBJECT * ob, long)
{
	FormGraphics * pre = static_cast<FormGraphics*>(ob->form->u_vdata);
    pre->browse();
}

void FormGraphics::AdvancedOptionsCB(FL_OBJECT * /* ob */, long)
{
//  FormGraphics * pre = static_cast<FormGraphics*>(ob->form->u_vdata);
//	pre->showAdvancedOptions();
    lyxerr << "Advanced Options button depressed, "
              "show advanced options dialog"
           << endl;
}

void FormGraphics::InputCB(FL_OBJECT * ob, long)
{
    FormGraphics * pre = static_cast<FormGraphics*>(ob->form->u_vdata);
	pre->input();
}

