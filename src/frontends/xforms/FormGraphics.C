/**
 * \file FormGraphics.C
 * Copyright 2000-2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Baruch Even, baruch.even@writeme.com
 * \author Herbert Voss, voss@perce.de
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "xformsBC.h"
#include "ControlGraphics.h"
#include "FormGraphics.h"
#include "form_graphics.h"
#include "Alert.h"

#include "xforms_helpers.h"
#include "input_validators.h"
#include "debug.h" // for lyxerr
#include "support/lstrings.h"  // for strToDbl & tostr
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
string defaultUnit("cm");

/// Given input and choice widgets, create a LyXLength
LyXLength getLyXLengthFromWidgets(FL_OBJECT * input, FL_OBJECT * choice)
{
	return LyXLength(getLengthFromWidgets(input, choice));
}

} // namespace anon


typedef FormCB<ControlGraphics, FormDB<FD_form_graphics> > base_class;

FormGraphics::FormGraphics(ControlGraphics & c)
	: base_class(c, _("Graphics"), false)
{}


void FormGraphics::redraw()
{
	if (form() && form()->visible)
		fl_redraw_form(form());
	else
		return;
	FL_FORM * outer_form = fl_get_active_folder(dialog_->tabfolder);
	if (outer_form && outer_form->visible)
		fl_redraw_form(outer_form);
}


void FormGraphics::build()
{
	dialog_.reset(build_graphics());

	// Allow the base class to control messages
	setMessageWidget(dialog_->text_warning);

	// Manage the ok, apply, restore and cancel/close buttons
	bc().setOK(dialog_->button_ok);
	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_close);
	bc().setRestore(dialog_->button_restore);

	// the file section
	file_.reset(build_file());

	fl_set_input_return (file_->input_filename, FL_RETURN_CHANGED);
	fl_set_input_return (file_->input_subcaption, FL_RETURN_CHANGED);
	fl_set_input_return (file_->input_rotate_angle, FL_RETURN_CHANGED);
	fl_set_input_maxchars(file_->input_filename,  FILENAME_MAXCHARS);
	fl_set_input_maxchars(file_->input_rotate_angle, ROTATE_MAXCHARS);
	fl_set_input_filter(file_->input_rotate_angle, fl_float_filter);

	setPrehandler(file_->input_filename);
	setPrehandler(file_->input_subcaption);
	setPrehandler(file_->input_rotate_angle);

	string const choice_origin =
		"center|"					// c
		"leftTop|leftBottom|leftBaseline|"		// lt lb lB
		"centerTop|centerBottom|centerBaseline|"	// ct cb cB
		"rightTop|rightBottom|rightBaseline";	// rt rb rB
	fl_addto_choice(file_->choice_origin, choice_origin.c_str());

	bc().addReadOnly(file_->button_browse);
	bc().addReadOnly(file_->check_subcaption);
	bc().addReadOnly(file_->check_rotate);
	bc().addReadOnly(file_->check_draft);
	bc().addReadOnly(file_->check_nounzip);

	// the lyxview section
	lyxview_.reset(build_lyxview());

	fl_set_input_return (lyxview_->input_lyxwidth, FL_RETURN_CHANGED);
	fl_set_input_return (lyxview_->input_lyxheight, FL_RETURN_CHANGED);
	fl_set_input_return (lyxview_->input_lyxscale, FL_RETURN_CHANGED);

	setPrehandler(lyxview_->input_lyxwidth);
	setPrehandler(lyxview_->input_lyxheight);
	setPrehandler(lyxview_->input_lyxscale);

	fl_addto_choice(lyxview_->choice_lyxwidth, choice_Length_WithUnit.c_str());
	fl_addto_choice(lyxview_->choice_lyxheight, choice_Length_WithUnit.c_str());

	bc().addReadOnly(lyxview_->radio_pref);
	bc().addReadOnly(lyxview_->radio_mono);
	bc().addReadOnly(lyxview_->radio_gray);
	bc().addReadOnly(lyxview_->radio_color);
	bc().addReadOnly(lyxview_->radio_nodisplay);

	// the size section
	size_.reset(build_size());

	fl_set_input_return (size_->input_scale, FL_RETURN_CHANGED);
	fl_set_input_return (size_->input_width, FL_RETURN_CHANGED);
	fl_set_input_return (size_->input_height, FL_RETURN_CHANGED);

	setPrehandler(size_->input_scale);
	setPrehandler(size_->input_width);
	setPrehandler(size_->input_height);

	fl_set_input_maxchars(size_->input_scale, SCALE_MAXDIGITS);
	fl_set_input_maxchars(size_->input_width, WIDTH_MAXDIGITS);
	fl_set_input_maxchars(size_->input_height, HEIGHT_MAXDIGITS);

	fl_set_input_filter(size_->input_scale,  fl_unsigned_float_filter);

	fl_addto_choice(size_->choice_width, choice_Length_All.c_str());
	fl_addto_choice(size_->choice_height, choice_Length_All.c_str());

	bc().addReadOnly(size_->radio_asis);
	bc().addReadOnly(size_->radio_wh);
	bc().addReadOnly(size_->radio_scale);
	bc().addReadOnly(size_->check_aspectratio);

	// the bounding box selection
	bbox_.reset(build_bbox());
	fl_set_input_return (bbox_->input_bb_x0, FL_RETURN_CHANGED);
	fl_set_input_return (bbox_->input_bb_y0, FL_RETURN_CHANGED);
	fl_set_input_return (bbox_->input_bb_x1, FL_RETURN_CHANGED);
	fl_set_input_return (bbox_->input_bb_y1, FL_RETURN_CHANGED);

	fl_set_input_filter(bbox_->input_bb_x0,  fl_unsigned_float_filter);
	fl_set_input_filter(bbox_->input_bb_y0,  fl_unsigned_float_filter);
	fl_set_input_filter(bbox_->input_bb_x1,  fl_unsigned_float_filter);
	fl_set_input_filter(bbox_->input_bb_y1,  fl_unsigned_float_filter);

	setPrehandler(bbox_->input_bb_x0);
	setPrehandler(bbox_->input_bb_y0);
	setPrehandler(bbox_->input_bb_x1);
	setPrehandler(bbox_->input_bb_y1);

	string const bb_units = "bp|cm|in";
	fl_addto_choice(bbox_->choice_bb_units, bb_units.c_str());
	bc().addReadOnly(bbox_->button_getBB);
	bc().addReadOnly(bbox_->check_clip);

	// the rotate section
	special_.reset(build_special());

	fl_set_input_return (special_->input_special, FL_RETURN_CHANGED);
	setPrehandler(special_->input_special);

	// add the different tabfolders
	fl_addto_tabfolder(dialog_->tabfolder, _("File"), file_->form);
	fl_addto_tabfolder(dialog_->tabfolder, _("LyX View"), lyxview_->form);
	fl_addto_tabfolder(dialog_->tabfolder, _("LaTeX Size"), size_->form);
	fl_addto_tabfolder(dialog_->tabfolder, _("Bounding Box"), bbox_->form);
	fl_addto_tabfolder(dialog_->tabfolder, _("Extras"), special_->form);
	// set the right default unit
	if (lyxrc.default_papersize < 3)
		defaultUnit = "in";
}


void FormGraphics::apply()
{
	// Create the parameters structure and fill the data from the dialog.
	InsetGraphicsParams & igp = controller().params();

	// the file section
	igp.filename = getStringFromInput(file_->input_filename);
	igp.subcaption = fl_get_button(file_->check_subcaption);
	igp.subcaptionText = getStringFromInput(file_->input_subcaption);
	igp.rotate = fl_get_button(file_->check_rotate);
	igp.rotateAngle =
		strToDbl(getStringFromInput(file_->input_rotate_angle));
	while (igp.rotateAngle < 0.0 || igp.rotateAngle > 360.0) {
		if (igp.rotateAngle < 0.0) {
			igp.rotateAngle += 360.0;
		} else if (igp.rotateAngle > 360.0) {
			igp.rotateAngle -= 360.0;
		}
	}
	if (fl_get_choice(file_->choice_origin) > 0)
		igp.rotateOrigin = fl_get_choice_text(file_->choice_origin);
	else
		igp.rotateOrigin = string();
	igp.draft = fl_get_button(file_->check_draft);
	igp.noUnzip = fl_get_button(file_->check_nounzip);

	// the lyxview section
	if (fl_get_button(lyxview_->radio_pref))
		igp.display = InsetGraphicsParams::DEFAULT;
	else if (fl_get_button(lyxview_->radio_mono))
		igp.display = InsetGraphicsParams::MONOCHROME;
	else if (fl_get_button(lyxview_->radio_gray))
		igp.display = InsetGraphicsParams::GRAYSCALE;
	else if (fl_get_button(lyxview_->radio_color))
		igp.display = InsetGraphicsParams::COLOR;
	else if (fl_get_button(lyxview_->radio_nodisplay))
		igp.display = InsetGraphicsParams::NONE;

	if (fl_get_button(lyxview_->radio_lyxasis))
		igp.lyxsize_type = InsetGraphicsParams::DEFAULT_SIZE;
	else if (fl_get_button(lyxview_->radio_lyxwh))
		igp.lyxsize_type = InsetGraphicsParams::WH;
	else
		igp.lyxsize_type = InsetGraphicsParams::SCALE;

	igp.lyxwidth = getLyXLengthFromWidgets(lyxview_->input_lyxwidth,
					       lyxview_->choice_lyxwidth);

	igp.lyxheight = getLyXLengthFromWidgets(lyxview_->input_lyxheight,
						lyxview_->choice_lyxheight);

	igp.lyxscale = strToInt(getStringFromInput(lyxview_->input_lyxscale));

	// the size section
	if (fl_get_button(size_->radio_asis))
		igp.size_type = InsetGraphicsParams::DEFAULT_SIZE;
	else if (fl_get_button(size_->radio_wh))
		igp.size_type = InsetGraphicsParams::WH;
	else
		igp.size_type = InsetGraphicsParams::SCALE;

	igp.width = getLyXLengthFromWidgets(size_->input_width,
					    size_->choice_width);

	igp.height = getLyXLengthFromWidgets(size_->input_height,
				   size_->choice_height);

	igp.scale = strToInt(getStringFromInput(size_->input_scale));
	igp.keepAspectRatio = fl_get_button(size_->check_aspectratio);

	// the bb section
	if (!controller().bbChanged)		// different to the original one?
		igp.bb = string();			// don't write anything
	else {
		string bb;
		if (getStringFromInput(bbox_->input_bb_x0).empty())
			bb = "0 ";
		else
			bb = getLengthFromWidgets(bbox_->input_bb_x0,
						  bbox_->choice_bb_units)+" ";
		if (getStringFromInput(bbox_->input_bb_y0).empty())
			bb += "0 ";
		else
			bb += (getLengthFromWidgets(bbox_->input_bb_y0,
						    bbox_->choice_bb_units)+" ");
		if (getStringFromInput(bbox_->input_bb_x1).empty())
			bb += "0 ";
		else
			bb += (getLengthFromWidgets(bbox_->input_bb_x1,
						    bbox_->choice_bb_units)+" ");
		if (getStringFromInput(bbox_->input_bb_y1).empty())
			bb += "0 ";
		else
			bb += (getLengthFromWidgets(bbox_->input_bb_y1,
						    bbox_->choice_bb_units)+" ");
		igp.bb = bb;
	}
	igp.clip = fl_get_button(bbox_->check_clip);

	// the special section
	igp.special = getStringFromInput(special_->input_special);
}



void FormGraphics::update() {
	// Update dialog with details from inset
	InsetGraphicsParams & igp = controller().params();

	// the file section
	fl_set_input(file_->input_filename, igp.filename.c_str());
	fl_set_button(file_->check_subcaption, igp.subcaption);
	fl_set_input(file_->input_subcaption, igp.subcaptionText.c_str());
	setEnabled(file_->input_subcaption,
		   fl_get_button(file_->check_subcaption));
	fl_set_button(file_->check_rotate, igp.rotate);
	fl_set_input(file_->input_rotate_angle,
		     tostr(igp.rotateAngle).c_str());
	if (igp.rotateOrigin.empty())
		fl_set_choice(file_->choice_origin,1);
	else
		fl_set_choice_text(file_->choice_origin,igp.rotateOrigin.c_str());
	setEnabled(file_->input_rotate_angle,
		   fl_get_button(file_->check_rotate));
	setEnabled(file_->choice_origin,
		   fl_get_button(file_->check_rotate));
	fl_set_button(file_->check_draft, igp.draft);
	fl_set_button(file_->check_nounzip, igp.noUnzip);

	// the lyxview section
	switch (igp.display) {
	case InsetGraphicsParams::DEFAULT:
		fl_set_button(lyxview_->radio_pref, 1);
		break;
	case InsetGraphicsParams::MONOCHROME:
		fl_set_button(lyxview_->radio_mono, 1);
		break;
	case InsetGraphicsParams::GRAYSCALE:
		fl_set_button(lyxview_->radio_gray, 1);
		break;
	case InsetGraphicsParams::COLOR:
		fl_set_button(lyxview_->radio_color, 1);
		break;
	case InsetGraphicsParams::NONE:
		fl_set_button(lyxview_->radio_nodisplay, 1);
		break;
	}
	updateWidgetsFromLength(lyxview_->input_lyxwidth,
				lyxview_->choice_lyxwidth, igp.lyxwidth, defaultUnit);
	updateWidgetsFromLength(lyxview_->input_lyxheight,
				lyxview_->choice_lyxheight, igp.lyxheight, defaultUnit);
	fl_set_input(lyxview_->input_lyxscale, tostr(igp.lyxscale).c_str());
	switch (igp.lyxsize_type) {
	case InsetGraphicsParams::DEFAULT_SIZE: {
		fl_set_button(lyxview_->radio_lyxasis,1);
		setEnabled(lyxview_->input_lyxwidth, 0);
		setEnabled(lyxview_->choice_lyxwidth, 0);
		setEnabled(lyxview_->input_lyxheight, 0);
		setEnabled(lyxview_->choice_lyxheight, 0);
		setEnabled(lyxview_->input_lyxscale, 0);
		break;
	}
	case InsetGraphicsParams::WH: {
		fl_set_button(lyxview_->radio_lyxwh, 1);
		setEnabled(lyxview_->input_lyxwidth, 1);
		setEnabled(lyxview_->choice_lyxwidth, 1);
		setEnabled(lyxview_->input_lyxheight, 1);
		setEnabled(lyxview_->choice_lyxheight, 1);
		setEnabled(lyxview_->input_lyxscale, 0);
		break;
	}
	case InsetGraphicsParams::SCALE: {
		fl_set_button(lyxview_->radio_lyxscale, 1);
		setEnabled(lyxview_->input_lyxwidth, 0);
		setEnabled(lyxview_->choice_lyxwidth, 0);
		setEnabled(lyxview_->input_lyxheight, 0);
		setEnabled(lyxview_->choice_lyxheight, 0);
		setEnabled(lyxview_->input_lyxscale, 1);
		break;
	}
	}

	// the size section
	// Update the draft and clip mode
	updateWidgetsFromLength(size_->input_width,
				size_->choice_width, igp.width, defaultUnit);
	updateWidgetsFromLength(size_->input_height,
				size_->choice_height, igp.height, defaultUnit);
	fl_set_input(size_->input_scale, tostr(igp.scale).c_str());
	switch (igp.size_type) {
	case InsetGraphicsParams::DEFAULT_SIZE: {
		fl_set_button(size_->radio_asis,1);
		setEnabled(size_->input_width, 0);
		setEnabled(size_->choice_width, 0);
		setEnabled(size_->input_height, 0);
		setEnabled(size_->choice_height, 0);
		setEnabled(size_->check_aspectratio, 0);
		setEnabled(size_->input_scale, 0);
		break;
	}
	case InsetGraphicsParams::WH: {
		fl_set_button(size_->radio_wh, 1);
		setEnabled(size_->input_width, 1);
		setEnabled(size_->choice_width, 1);
		setEnabled(size_->input_height, 1);
		setEnabled(size_->choice_height, 1);
		setEnabled(size_->check_aspectratio, 1);
		setEnabled(size_->input_scale, 0);
		break;
	}
	case InsetGraphicsParams::SCALE: {
		fl_set_button(size_->radio_scale, 1);
		setEnabled(size_->input_width, 0);
		setEnabled(size_->choice_width, 0);
		setEnabled(size_->input_height, 0);
		setEnabled(size_->choice_height, 0);
		setEnabled(size_->check_aspectratio, 0);
		setEnabled(size_->input_scale, 1);
		break;
	}
	}
	fl_set_button(size_->check_aspectratio, igp.keepAspectRatio);

	// the bb section
	// set the bounding box values, if exists. First we need the whole
	// path, because the controller knows nothing about the doc-dir
	updateBB(igp.filename, igp.bb);
	fl_set_button(bbox_->check_clip, igp.clip);

	// the special section
	fl_set_input(special_->input_special, igp.special.c_str());
}


void FormGraphics::updateBB(string const & filename, string const & bb_inset)
{
	// Update dialog with details from inset
	// set the bounding box values, if exists. First we need the whole
	// path, because the controller knows nothing about the doc-dir
	controller().bbChanged = false;
	if (bb_inset.empty()) {
		lyxerr[Debug::GRAPHICS] << "update:: no BoundingBox" << endl;
		string const bb = controller().readBB(filename);
		if (!bb.empty()) {
			// get the values from the file
			// in this case we always have the point-unit
			fl_set_input(bbox_->input_bb_x0,
				     token(bb,' ',0).c_str());
			fl_set_input(bbox_->input_bb_y0,
				     token(bb,' ',1).c_str());
			fl_set_input(bbox_->input_bb_x1,
				     token(bb,' ',2).c_str());
			fl_set_input(bbox_->input_bb_y1,
				     token(bb,' ',3).c_str());

		} else {
			// no bb from file
			fl_set_input(bbox_->input_bb_x0, bb.c_str());
			fl_set_input(bbox_->input_bb_y0, bb.c_str());
			fl_set_input(bbox_->input_bb_x1, bb.c_str());
			fl_set_input(bbox_->input_bb_y1, bb.c_str());
		}
		// "bp"
		fl_set_choice(bbox_->choice_bb_units, 1);

	} else {
		// get the values from the inset
		lyxerr[Debug::GRAPHICS] << "update:: igp has BoundingBox"
					<< endl;
		controller().bbChanged = true;
		LyXLength anyLength;
		anyLength = LyXLength(token(bb_inset,' ',0));
		updateWidgetsFromLength(bbox_->input_bb_x0,
					bbox_->choice_bb_units,anyLength,"bp");
		anyLength = LyXLength(token(bb_inset,' ',1));
		updateWidgetsFromLength(bbox_->input_bb_y0,
					bbox_->choice_bb_units,anyLength,"bp");
		anyLength = LyXLength(token(bb_inset,' ',2));
		updateWidgetsFromLength(bbox_->input_bb_x1,
					bbox_->choice_bb_units,anyLength,"bp");
		anyLength = LyXLength(token(bb_inset,' ',3));
		updateWidgetsFromLength(bbox_->input_bb_y1,
					bbox_->choice_bb_units,anyLength,"bp");
	}
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
	// the file section
	if (ob == file_->button_browse) {
		// Get the filename from the dialog
		string const in_name = getStringFromInput(file_->input_filename);
		string const out_name = controller().Browse(in_name);
		lyxerr[Debug::GRAPHICS] << "[FormGraphics]out_name: " << out_name << endl;
		if (out_name != in_name && !out_name.empty()) {
			fl_set_input(file_->input_filename, out_name.c_str());
		}
		if (controller().isFilenameValid(out_name))
  			updateBB(out_name, string());

	} else if (ob == file_->check_subcaption) {
		setEnabled(file_->input_subcaption,
			   fl_get_button(file_->check_subcaption));
	} else if (ob == file_->check_rotate) {
		setEnabled(file_->input_rotate_angle,
			   fl_get_button(file_->check_rotate));
		setEnabled(file_->choice_origin,
			   fl_get_button(file_->check_rotate));

		// the lyxview section
	} else if (ob == lyxview_->radio_lyxasis) {
		setEnabled(lyxview_->input_lyxwidth, 0);
		setEnabled(lyxview_->choice_lyxwidth, 0);
		setEnabled(lyxview_->input_lyxheight, 0);
		setEnabled(lyxview_->choice_lyxheight, 0);
		setEnabled(lyxview_->input_lyxscale, 0);
	} else if (ob == lyxview_->radio_lyxwh) {
		setEnabled(lyxview_->input_lyxwidth, 1);
		setEnabled(lyxview_->choice_lyxwidth, 1);
		setEnabled(lyxview_->input_lyxheight, 1);
		setEnabled(lyxview_->choice_lyxheight, 1);
		setEnabled(lyxview_->input_lyxscale, 0);
	} else if (ob == lyxview_->radio_lyxscale) {
		setEnabled(lyxview_->input_lyxwidth, 0);
		setEnabled(lyxview_->choice_lyxwidth, 0);
		setEnabled(lyxview_->input_lyxheight, 0);
		setEnabled(lyxview_->choice_lyxheight, 0);
		setEnabled(lyxview_->input_lyxscale, 1);
	} else if (ob == lyxview_->button_latex_values) {
		if (contains(fl_get_choice_text(size_->choice_width),'%'))
			Alert::alert(_("Warning!"),
				     _("The units text%, page%, col% and line% are not allowed here."),
				     _("Cannot use the values from LaTeX size!"));
		else {
			LyXLength dummy =
				getLyXLengthFromWidgets(size_->input_width,
							size_->choice_width);
			updateWidgetsFromLength(lyxview_->input_lyxwidth,
						lyxview_->choice_lyxwidth,
						dummy, defaultUnit);

			dummy = getLyXLengthFromWidgets(size_->input_height,
							size_->choice_height);
			updateWidgetsFromLength(lyxview_->input_lyxheight,
						lyxview_->choice_lyxheight,
						dummy, defaultUnit);
			string const scale = getStringFromInput(size_->input_scale);
			fl_set_input(lyxview_->input_lyxscale, scale.c_str());
			if (fl_get_button (size_->radio_asis) == 1) {
				fl_set_button (lyxview_->radio_lyxasis, 1);
				setEnabled(lyxview_->input_lyxwidth, 0);
				setEnabled(lyxview_->choice_lyxwidth, 0);
				setEnabled(lyxview_->input_lyxheight, 0);
				setEnabled(lyxview_->choice_lyxheight, 0);
				setEnabled(lyxview_->input_lyxscale, 0);
			} else if (fl_get_button (size_->radio_wh) == 1) {
				fl_set_button (lyxview_->radio_lyxwh, 1);
				setEnabled(lyxview_->input_lyxwidth, 1);
				setEnabled(lyxview_->choice_lyxwidth, 1);
				setEnabled(lyxview_->input_lyxheight, 1);
				setEnabled(lyxview_->choice_lyxheight, 1);
				setEnabled(lyxview_->input_lyxscale, 0);
			} else if (fl_get_button (size_->radio_scale) ==1) {
				fl_set_button (lyxview_->radio_lyxscale, 1);
				setEnabled(lyxview_->input_lyxwidth, 0);
				setEnabled(lyxview_->choice_lyxwidth, 0);
				setEnabled(lyxview_->input_lyxheight, 0);
				setEnabled(lyxview_->choice_lyxheight, 0);
				setEnabled(lyxview_->input_lyxscale, 1);
			}
		}

		// the bb section
	} else if (!controller().bbChanged &&
		   (ob == bbox_->choice_bb_units ||
		    ob == bbox_->input_bb_x0 || ob == bbox_->input_bb_y0 ||
		    ob == bbox_->input_bb_x1 || ob == bbox_->input_bb_y1)) {
		controller().bbChanged = true;
	} else if (ob == bbox_->button_getBB) {
		string const filename = getStringFromInput(file_->input_filename);
		if (!filename.empty()) {
			string bb = controller().readBB(filename);
			if (!bb.empty()) {
				fl_set_input(bbox_->input_bb_x0, token(bb,' ',0).c_str());
				fl_set_input(bbox_->input_bb_y0, token(bb,' ',1).c_str());
				fl_set_input(bbox_->input_bb_x1, token(bb,' ',2).c_str());
				fl_set_input(bbox_->input_bb_y1, token(bb,' ',3).c_str());
				string const unit("bp");
				fl_set_choice_text(bbox_->choice_bb_units, unit.c_str());
			}
			controller().bbChanged = false;
		} else {
			fl_set_input(bbox_->input_bb_x0, "");
			fl_set_input(bbox_->input_bb_y0, "");
			fl_set_input(bbox_->input_bb_x1, "");
			fl_set_input(bbox_->input_bb_y1, "");
			fl_set_choice_text(bbox_->choice_bb_units, "bp");
		}

		// the size section
	} else if (ob == size_->radio_asis) {
		setEnabled(size_->input_width, 0);
		setEnabled(size_->choice_width, 0);
		setEnabled(size_->input_height, 0);
		setEnabled(size_->choice_height, 0);
		setEnabled(size_->check_aspectratio, 0);
		setEnabled(size_->input_scale, 0);
	} else if (ob == size_->radio_wh) {
		setEnabled(size_->input_width, 1);
		setEnabled(size_->choice_width, 1);
		setEnabled(size_->input_height, 1);
		setEnabled(size_->choice_height, 1);
		setEnabled(size_->check_aspectratio, 1);
		setEnabled(size_->input_scale, 0);
	} else if (ob == size_->radio_scale) {
		setEnabled(size_->input_width, 0);
		setEnabled(size_->choice_width, 0);
		setEnabled(size_->input_height, 0);
		setEnabled(size_->choice_height, 0);
		setEnabled(size_->check_aspectratio, 0);
		setEnabled(size_->input_scale, 1);
	} else if (ob == size_->button_lyx_values) {
		LyXLength dummy = getLyXLengthFromWidgets(lyxview_->input_lyxwidth,
							  lyxview_->choice_lyxwidth);
		updateWidgetsFromLength(size_->input_width,
					size_->choice_width,
					dummy, defaultUnit);
		dummy = getLyXLengthFromWidgets(lyxview_->input_lyxheight,
						lyxview_->choice_lyxheight);
		updateWidgetsFromLength(size_->input_height,
					size_->choice_height,
					dummy, defaultUnit);
		string const scale = getStringFromInput(lyxview_->input_lyxscale);
		fl_set_input(size_->input_scale, scale.c_str());
		if (fl_get_button (lyxview_->radio_lyxasis) == 1) {
			fl_set_button (size_->radio_asis, 1);
			setEnabled(size_->input_width, 0);
			setEnabled(size_->choice_width, 0);
			setEnabled(size_->input_height, 0);
			setEnabled(size_->choice_height, 0);
			setEnabled(size_->check_aspectratio, 0);
			setEnabled(size_->input_scale, 0);
		} else if (fl_get_button (lyxview_->radio_lyxwh) == 1) {
			fl_set_button (size_->radio_wh, 1);
			setEnabled(size_->input_width, 1);
			setEnabled(size_->choice_width, 1);
			setEnabled(size_->input_height, 1);
			setEnabled(size_->choice_height, 1);
			setEnabled(size_->check_aspectratio, 1);
			setEnabled(size_->input_scale, 0);
		} else if (fl_get_button (lyxview_->radio_lyxscale) ==1) {
			fl_set_button (size_->radio_scale, 1);
			setEnabled(size_->input_width, 0);
			setEnabled(size_->choice_width, 0);
			setEnabled(size_->input_height, 0);
			setEnabled(size_->choice_height, 0);
			setEnabled(size_->check_aspectratio, 0);
			setEnabled(size_->input_scale, 1);
		}
	}

	// check if the input is valid
	bool invalid = !isValid(bbox_->input_bb_x0);
	invalid = invalid || !isValid(bbox_->input_bb_x1);
	invalid = invalid || !isValid(bbox_->input_bb_y0);
	invalid = invalid || !isValid(bbox_->input_bb_y1);
	invalid = invalid || !isValid(size_->input_width);
	invalid = invalid || !isValid(size_->input_height);
	invalid = invalid || !isValid(lyxview_->input_lyxwidth);
	invalid = invalid || !isValid(lyxview_->input_lyxheight);

	// deactivate OK/ Apply buttons and
	// spit out warnings if invalid
	if (ob == bbox_->input_bb_x0 || ob == bbox_->input_bb_x1 ||
	    ob == bbox_->input_bb_y0 || ob == bbox_->input_bb_y1 ||
	    ob == size_->input_width || ob == size_->input_height ||
	    ob == lyxview_->input_lyxwidth || ob == lyxview_->input_lyxheight) {
		if (invalid) {
			postWarning(_("Invalid Length!"));
			return ButtonPolicy::SMI_INVALID;
		} else {
			clearMessage();
		}
	}

	return ButtonPolicy::SMI_VALID;
}
