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
#include "support/FileInfo.h"
#include "support/filetools.h"  // for MakeAbsPath etc
#include "insets/insetgraphicsParams.h"
#include "lyxrc.h" // for lyxrc.display_graphics

using std::endl;

namespace {

// Bound the number of input characters
int const SCALE_MAXDIGITS = 3;
int const WIDTH_MAXDIGITS = 10;
int const HEIGHT_MAXDIGITS = 10;
int const ROTATE_MAXCHARS = 4;
int const FILENAME_MAXCHARS = 1024;
 
} // namespace anon


typedef FormCB<ControlGraphics, FormDB<FD_form_graphics> > base_class;

FormGraphics::FormGraphics(ControlGraphics & c)
	: base_class(c, _("Graphics"))
{}


void FormGraphics::redraw()
{
	if(form() && form()->visible)
		fl_redraw_form(form());
	else
		return;

	FL_FORM * outer_form = fl_get_active_folder(dialog_->tabFolder);
	if (outer_form && outer_form->visible)
		fl_redraw_form(outer_form);
}


void FormGraphics::build()
{
	dialog_.reset(build_graphics());

        // Manage the ok, apply, restore and cancel/close buttons
	bc().setOK(dialog_->button_ok);
//	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_cancel);
	bc().setRestore(dialog_->button_restore);

	// the file section
	file_.reset(build_file());

	fl_set_input_return (file_->input_filename, FL_RETURN_CHANGED);
	fl_set_input_return (file_->input_subcaption, FL_RETURN_CHANGED);
	fl_set_input_return (file_->input_lyxwidth, FL_RETURN_CHANGED);
	fl_set_input_return (file_->input_lyxheight, FL_RETURN_CHANGED);
	fl_set_input_maxchars(file_->input_filename,  FILENAME_MAXCHARS);

	fl_addto_choice(file_->choice_width_lyxwidth, choice_Length_All.c_str());
	fl_addto_choice(file_->choice_width_lyxheight, choice_Length_All.c_str());
	fl_addto_choice(file_->choice_display,
		_(" Default | Monochrome | Grayscale | Color | Do not display "));
	//FIXME: This does not work (always set to monochrome by default. Why? JSpitzm)
	fl_set_choice(file_->choice_display, 1);

	bc().addReadOnly(file_->button_browse);
	bc().addReadOnly(file_->check_subcaption);
	bc().addReadOnly(file_->choice_display);

	// the size section
	size_.reset(build_size());

	fl_set_input_return (size_->input_scale, FL_RETURN_CHANGED);
	fl_set_input_return (size_->input_width, FL_RETURN_CHANGED);
	fl_set_input_return (size_->input_height, FL_RETURN_CHANGED);

	fl_set_input_maxchars(size_->input_scale, SCALE_MAXDIGITS);
	fl_set_input_maxchars(size_->input_width, WIDTH_MAXDIGITS);
	fl_set_input_maxchars(size_->input_height, HEIGHT_MAXDIGITS);

	fl_set_input_filter(size_->input_scale,  fl_unsigned_float_filter);

	fl_addto_choice(size_->choice_width_units, choice_Length_All.c_str());
	fl_addto_choice(size_->choice_height_units, choice_Length_All.c_str());

	bc().addReadOnly(size_->button_default);
	bc().addReadOnly(size_->button_wh);
	bc().addReadOnly(size_->button_scale);
	bc().addReadOnly(size_->check_aspectratio);

	// the bounding box selection
	bbox_.reset(build_bbox());
	fl_set_input_return (bbox_->input_bb_x0, FL_RETURN_CHANGED);
	fl_set_input_return (bbox_->input_bb_y0, FL_RETURN_CHANGED);
	fl_set_input_return (bbox_->input_bb_x1, FL_RETURN_CHANGED);
	fl_set_input_return (bbox_->input_bb_y1, FL_RETURN_CHANGED);

	bc().addReadOnly(bbox_->button_getBB);
	bc().addReadOnly(bbox_->button_clip);
	bc().addReadOnly(bbox_->button_draft);

	string const bb_units = "pt|cm|in";
	fl_addto_choice(bbox_->choice_bb_x0, bb_units.c_str());
	fl_addto_choice(bbox_->choice_bb_y0, bb_units.c_str());
	fl_addto_choice(bbox_->choice_bb_x1, bb_units.c_str());
	fl_addto_choice(bbox_->choice_bb_y1, bb_units.c_str());


	// the rotate section
	special_.reset(build_special());

	fl_set_input_return (special_->input_rotate_angle, FL_RETURN_CHANGED);

	string const choice_origin = 
	    "default|"					// not important
	    "leftTop|leftBottom|leftBaseline|"		// lt lb lB
	    "center|"					// c
	    "centerTop|centerBottom|centerBaseline|"	// ct cb cB
	    "rightTop|rightBottom|rightBaseline|"	// rt rb rB
	    "referencePoint";				// special
	fl_addto_choice(special_->choice_origin, choice_origin.c_str());

	fl_set_input_return (special_->input_special, FL_RETURN_CHANGED);
	fl_set_input_maxchars(special_->input_rotate_angle, ROTATE_MAXCHARS);
	fl_set_input_filter(special_->input_rotate_angle, fl_float_filter);

	// add the different tabfolders
	fl_addto_tabfolder(dialog_->tabFolder, _("File"), file_->form);
	fl_addto_tabfolder(dialog_->tabFolder, _("Size"), size_->form);
	fl_addto_tabfolder(dialog_->tabFolder, _("Bounding Box"), bbox_->form);
	fl_addto_tabfolder(dialog_->tabFolder, _("Extras"), special_->form);
}


void FormGraphics::apply()
{
	// Create the parameters structure and fill the data from the dialog.
	InsetGraphicsParams & igp = controller().params();
	igp.filename = getStringFromInput(file_->input_filename);
	if (!controller().bbChanged)		// different to the original one?
	    igp.bb = string();			// don't write anything	    
	else {
	    string bb;
	    if (getStringFromInput(bbox_->input_bb_x0).empty())
		bb = "0 ";
	    else
		bb = getLengthFromWidgets(bbox_->input_bb_x0,
			bbox_->choice_bb_x0)+" ";
	    if (getStringFromInput(bbox_->input_bb_y0).empty())
		bb += "0 ";
	    else
		bb += (getLengthFromWidgets(bbox_->input_bb_y0,
			bbox_->choice_bb_y0)+" ");
	    if (getStringFromInput(bbox_->input_bb_x1).empty())
		bb += "0 ";
	    else
		bb += (getLengthFromWidgets(bbox_->input_bb_x1,
			bbox_->choice_bb_x1)+" ");
	    if (getStringFromInput(bbox_->input_bb_y1).empty())
		bb += "0 ";
	    else
		bb += (getLengthFromWidgets(bbox_->input_bb_y1,
			bbox_->choice_bb_y1)+" ");
	    igp.bb = bb;
	}
	igp.draft = fl_get_button(bbox_->button_draft);
	igp.clip = fl_get_button(bbox_->button_clip);
	igp.subcaption = fl_get_button(file_->check_subcaption);
	igp.subcaptionText = getStringFromInput(file_->input_subcaption);
	// use preferences settings if choice is set to default
	if (fl_get_choice(file_->choice_display) == 1) {
		if (lyxrc.display_graphics == "mono")
			igp.display = InsetGraphicsParams::MONOCHROME;
		else if (lyxrc.display_graphics == "gray")
			igp.display = InsetGraphicsParams::GRAYSCALE;
		else if (lyxrc.display_graphics == "color")
			igp.display = InsetGraphicsParams::COLOR;
		else if (lyxrc.display_graphics == "no")
			igp.display = InsetGraphicsParams::NONE;
	} else if (fl_get_choice(file_->choice_display) == 2) {
		igp.display = InsetGraphicsParams::MONOCHROME;
	} else if (fl_get_choice(file_->choice_display) == 3) {
		igp.display = InsetGraphicsParams::GRAYSCALE;
	} else if (fl_get_choice(file_->choice_display) == 4) {
		igp.display = InsetGraphicsParams::COLOR;
	} else if (fl_get_choice(file_->choice_display) == 5) {
		igp.display = InsetGraphicsParams::NONE;
	}
	if (fl_get_button(size_->button_default))
	    igp.size_type = InsetGraphicsParams::DEFAULT_SIZE;
	else if (fl_get_button(size_->button_wh))
	    igp.size_type = InsetGraphicsParams::WH;
	else
	    igp.size_type = InsetGraphicsParams::SCALE;
	igp.width = LyXLength(getLengthFromWidgets(size_->input_width,
		size_->choice_width_units));
	igp.height = LyXLength(getLengthFromWidgets(size_->input_height,
		size_->choice_height_units));
	igp.scale = strToInt(getStringFromInput(size_->input_scale));
	igp.keepAspectRatio = fl_get_button(size_->check_aspectratio);
	igp.lyxwidth = LyXLength(getLengthFromWidgets(file_->input_lyxwidth,
		file_->choice_width_lyxwidth));
	igp.lyxheight = LyXLength(getLengthFromWidgets(file_->input_lyxheight,
		file_->choice_width_lyxheight));

	igp.rotateAngle =
		strToDbl(getStringFromInput(special_->input_rotate_angle));
	while (igp.rotateAngle < 0.0 || igp.rotateAngle > 360.0) {
		if (igp.rotateAngle < 0.0) {
			igp.rotateAngle += 360.0;
		} else if (igp.rotateAngle > 360.0) {
			igp.rotateAngle -= 360.0;
		}
	}
	if (fl_get_choice(special_->choice_origin) > 0)
	    igp.rotateOrigin = fl_get_choice_text(special_->choice_origin);
	else
	    igp.rotateOrigin = string();
	igp.special = getStringFromInput(special_->input_special);
	igp.testInvariant();
}


void FormGraphics::update()
{	string unit = "cm";
	if (lyxrc.default_papersize < 3)
	    unit = "in";
	string const defaultUnit = string(unit); 
	// Update dialog with details from inset
	InsetGraphicsParams & igp = controller().params();
	fl_set_input(file_->input_filename, igp.filename.c_str());
	// set the bounding box values, if exists. First we need the whole
	// path, because the controller knows nothing about the doc-dir
	lyxerr << "GraphicsUpdate::BoundingBox = " << igp.bb << "\n";
	controller().bbChanged = false;
	if (igp.bb.empty()) {
	    string const fileWithAbsPath = MakeAbsPath(igp.filename, OnlyPath(igp.filename)); 	
	    string bb = controller().readBB(fileWithAbsPath);
	    lyxerr << "file::BoundingBox = " << bb << "\n";
	    if (!bb.empty()) {		
		// get the values from the file
		// in this case we always have the point-unit
		fl_set_input(bbox_->input_bb_x0, token(bb,' ',0).c_str());
		fl_set_input(bbox_->input_bb_y0, token(bb,' ',1).c_str());
		fl_set_input(bbox_->input_bb_x1, token(bb,' ',2).c_str());
		fl_set_input(bbox_->input_bb_y1, token(bb,' ',3).c_str());
	    }
	} else { 				// get the values from the inset
	    controller().bbChanged = true;
	    LyXLength anyLength;
	    anyLength = LyXLength(token(igp.bb,' ',0));
	    updateWidgetsFromLength(bbox_->input_bb_x0,
	    		bbox_->choice_bb_x0,anyLength,"pt");
	    anyLength = LyXLength(token(igp.bb,' ',1));
	    updateWidgetsFromLength(bbox_->input_bb_y0,
	    		bbox_->choice_bb_y0,anyLength,"pt");
	    anyLength = LyXLength(token(igp.bb,' ',2));
	    updateWidgetsFromLength(bbox_->input_bb_x1,
	    		bbox_->choice_bb_x1,anyLength,"pt");
	    anyLength = LyXLength(token(igp.bb,' ',3));
	    updateWidgetsFromLength(bbox_->input_bb_y1,
	    		bbox_->choice_bb_y1,anyLength,"pt");
	}
	// Update the draft and clip mode
	fl_set_button(bbox_->button_draft, igp.draft);
	fl_set_button(bbox_->button_clip, igp.clip);
	// Update the subcaption check button and input field
	fl_set_button(file_->check_subcaption, igp.subcaption);
	fl_set_input(file_->input_subcaption, igp.subcaptionText.c_str());
	setEnabled(file_->input_subcaption,
		   fl_get_button(file_->check_subcaption));

	switch (igp.display) {
	    case InsetGraphicsParams::NONE: {	// dont't display
		fl_set_choice(file_->choice_display, 5);
		break;
	    }
	    case InsetGraphicsParams::MONOCHROME: {
		fl_set_choice(file_->choice_display, 2);
		break;
	    }
	    case InsetGraphicsParams::GRAYSCALE: {
		fl_set_choice(file_->choice_display, 3);
		break;
	    }
	    case InsetGraphicsParams::COLOR: {
		fl_set_choice(file_->choice_display, 4);
		break;
	    }
	}
	updateWidgetsFromLength(size_->input_width,
		size_->choice_width_units,igp.width,defaultUnit);
	updateWidgetsFromLength(size_->input_height,
		size_->choice_height_units,igp.height,defaultUnit);
	fl_set_input(size_->input_scale, tostr(igp.scale).c_str());
	switch (igp.size_type) {
	    case InsetGraphicsParams::DEFAULT_SIZE: {
		fl_set_button(size_->button_default,1);
		setEnabled(size_->input_width, 0);
		setEnabled(size_->choice_width_units, 0);
		setEnabled(size_->input_height, 0);
		setEnabled(size_->choice_height_units, 0);
		setEnabled(size_->input_scale, 0);
		break;
	    }
	    case InsetGraphicsParams::WH: {
		fl_set_button(size_->button_wh, 1);
		setEnabled(size_->input_width, 1);
		setEnabled(size_->choice_width_units, 1);
		setEnabled(size_->input_height, 1);
		setEnabled(size_->choice_height_units, 1);
		setEnabled(size_->input_scale, 0);
		break;
	    }
	    case InsetGraphicsParams::SCALE: {
		fl_set_button(size_->button_scale, 1);
		setEnabled(size_->input_width, 0);
		setEnabled(size_->choice_width_units, 0);
		setEnabled(size_->input_height, 0);
		setEnabled(size_->choice_height_units, 0);
		setEnabled(size_->input_scale, 1);
		break;
	    }
	}
	fl_set_button(size_->check_aspectratio, igp.keepAspectRatio);
	// now the lyx-internally viewsize
	updateWidgetsFromLength(file_->input_lyxwidth,
		file_->choice_width_lyxwidth, igp.lyxwidth,defaultUnit);
	updateWidgetsFromLength(file_->input_lyxheight,
		file_->choice_width_lyxheight, igp.lyxheight,defaultUnit);
	// Update the rotate angle and special commands
	fl_set_input(special_->input_rotate_angle,
	             tostr(igp.rotateAngle).c_str());
	if (igp.rotateOrigin.empty())
	    fl_set_choice(special_->choice_origin,0);
	else
	    fl_set_choice_text(special_->choice_origin,igp.rotateOrigin.c_str());
	fl_set_input(special_->input_special, igp.special.c_str());
}


namespace {

bool isValid(FL_OBJECT * ob)
{
	string const input = getStringFromInput(ob);
	return input.empty() || isValidLength(input) || isStrDbl(input);
}

} // namespace anon

 
	
ButtonPolicy::SMInput FormGraphics::input(FL_OBJECT * ob, long)
{
	if (ob == file_->button_browse) {
		// Get the filename from the dialog
		string const in_name = getStringFromInput(file_->input_filename);
		string const out_name = controller().Browse(in_name);
		if (out_name != in_name && !out_name.empty()) {
			fl_set_input(file_->input_filename, out_name.c_str());
		}
	} else if (!controller().bbChanged && 
		    ((ob == bbox_->input_bb_x0) || (ob == bbox_->input_bb_y0) ||
		    (ob == bbox_->input_bb_x1) || (ob == bbox_->input_bb_y1) ||
		    (ob == bbox_->choice_bb_x0) || (ob == bbox_->choice_bb_y0) ||
		    (ob == bbox_->choice_bb_x1) || (ob == bbox_->choice_bb_y1))) {
	    controller().bbChanged = true; 
	} else if (ob == size_->button_default) {
	    	setEnabled(size_->input_width, 0);
	    	setEnabled(size_->choice_width_units, 0);
	    	setEnabled(size_->input_height, 0);
	    	setEnabled(size_->choice_height_units, 0);
	    	setEnabled(size_->input_scale, 0);
	} else if (ob == size_->button_wh) {
	    	setEnabled(size_->input_width, 1);
	    	setEnabled(size_->choice_width_units, 1);
	    	setEnabled(size_->input_height, 1);
	    	setEnabled(size_->choice_height_units, 1);
	    	setEnabled(size_->input_scale, 0);
	} else if (ob == size_->button_scale) {
	    	setEnabled(size_->input_width, 0);
	    	setEnabled(size_->choice_width_units, 0);
	    	setEnabled(size_->input_height, 0);
	    	setEnabled(size_->choice_height_units, 0);
	    	setEnabled(size_->input_scale, 1);
	} else if (ob == file_->check_subcaption) {
	    	setEnabled(file_->input_subcaption,
			   fl_get_button(file_->check_subcaption));
	} else if (ob == bbox_->button_getBB) {
	    string const filename = getStringFromInput(file_->input_filename);
	    if (!filename.empty()) {
		string const fileWithAbsPath = MakeAbsPath(filename, OnlyPath(filename)); 	
		string bb = controller().readBB(fileWithAbsPath);
		lyxerr << "getBB::BoundingBox = " << bb << "\n";
		if (!bb.empty()) {		
		    fl_set_input(bbox_->input_bb_x0, token(bb,' ',0).c_str());
		    fl_set_input(bbox_->input_bb_y0, token(bb,' ',1).c_str());
		    fl_set_input(bbox_->input_bb_x1, token(bb,' ',2).c_str());
		    fl_set_input(bbox_->input_bb_y1, token(bb,' ',3).c_str());
		}
		controller().bbChanged = false;
	    }
	} else if (ob == dialog_->button_help) {
    	    controller().help();
	}

	// check if the input is valid
	bool invalid = !isValid(bbox_->input_bb_x0);
	invalid = invalid || !isValid(bbox_->input_bb_x1);
	invalid = invalid || !isValid(bbox_->input_bb_y0);
	invalid = invalid || !isValid(bbox_->input_bb_y1);
	invalid = invalid || !isValid(size_->input_width);
	invalid = invalid || !isValid(size_->input_height);
	invalid = invalid || !isValid(file_->input_lyxwidth);
	invalid = invalid || !isValid(file_->input_lyxheight);

	// deactivate OK/ Apply buttons and
	// spit out warnings if invalid
	if (ob == bbox_->input_bb_x0
			|| ob == bbox_->input_bb_x1
			|| ob == bbox_->input_bb_y0
			|| ob == bbox_->input_bb_y1
			|| ob == size_->input_width
			|| ob == size_->input_height
			|| ob == file_->input_lyxwidth
			|| ob == file_->input_lyxheight) {
		if (invalid) {
			fl_set_object_label(dialog_->text_warning,
				_("Warning: Invalid Length!"));
			fl_show_object(dialog_->text_warning);
			return ButtonPolicy::SMI_INVALID;
		} else {
			fl_hide_object(dialog_->text_warning);
		}
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
	string filename = getStringFromInput(file_->input_filename);
	FileInfo file(filename);
	if (filename.empty() || !file.isOK() || !file.exist()
	    		    || !file.isRegular() || !file.readable()
	   )
		activate = ButtonPolicy::SMI_INVALID;

	return activate;
}

