/**
 * \file FormGraphics.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Baruch Even
 * \author Herbert Voss
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "xformsBC.h"
#include "ControlGraphics.h"
#include "FormGraphics.h"
#include "forms/form_graphics.h"
#include "Alert.h"
#include "Tooltips.h"

#include "xforms_helpers.h"
#include "helper_funcs.h"
#include "input_validators.h"
#include "debug.h" // for lyxerr
#include "support/lstrings.h"  // for strToDbl & tostr
#include "support/filetools.h"  // for MakeAbsPath etc
#include "insets/insetgraphicsParams.h"
#include "lyxrc.h" // for lyxrc.display_graphics
#include FORMS_H_LOCATION

using std::endl;
using std::vector;

namespace {

// Bound the number of input characters
int const SIZE_MAXDIGITS = 10;
int const FILENAME_MAXCHARS = 1024;

string defaultUnit("cm");

/// Given input and choice widgets, create a LyXLength
LyXLength getLyXLengthFromWidgets(FL_OBJECT * input, FL_OBJECT * choice)
{
	return LyXLength(getLengthFromWidgets(input, choice));
}

} // namespace anon


typedef FormCB<ControlGraphics, FormDB<FD_graphics> > base_class;

FormGraphics::FormGraphics()
	: base_class(_("Graphics"), false)
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
	dialog_.reset(build_graphics(this));

	// Allow the base class to control messages
	setMessageWidget(dialog_->text_warning);

	// Manage the ok, apply, restore and cancel/close buttons
	bc().setOK(dialog_->button_ok);
	bc().setApply(dialog_->button_apply);
	bc().setCancel(dialog_->button_close);
	bc().setRestore(dialog_->button_restore);

	// the file section
	file_.reset(build_graphics_file(this));

	fl_set_input_return (file_->input_filename, FL_RETURN_CHANGED);
	fl_set_input_return (file_->input_lyxscale, FL_RETURN_CHANGED);
	fl_set_input_return (file_->input_width, FL_RETURN_CHANGED);
	fl_set_input_return (file_->input_height, FL_RETURN_CHANGED);

	setPrehandler(file_->input_filename);
	setPrehandler(file_->input_lyxscale);
	setPrehandler(file_->input_width);
	setPrehandler(file_->input_height);

	fl_set_input_maxchars(file_->input_filename, FILENAME_MAXCHARS);
	fl_set_input_filter(file_->input_lyxscale, fl_unsigned_int_filter);

	 // width default is scaling, thus unsigned integer input
	fl_set_input_filter(file_->input_width, fl_unsigned_int_filter);
	fl_set_input_maxchars(file_->input_height, SIZE_MAXDIGITS);


	string const display_List = _("Default|Monochrome|Grayscale|Color|Do not display");
	fl_addto_choice(file_->choice_display, display_List.c_str());
	
	string const width_list = _("Scale%%|") + choice_Length_All;
	fl_addto_choice(file_->choice_width, width_list.c_str());

	fl_addto_choice(file_->choice_height, choice_Length_All.c_str());

	bc().addReadOnly(file_->button_browse);   
	bc().addReadOnly(file_->check_aspectratio);
	bc().addReadOnly(file_->check_draft);
	bc().addReadOnly(file_->check_nounzip);

	// set up the tooltips for the filesection
	string str = _("The file you want to insert.");
	tooltips().init(file_->input_filename, str);
	str = _("Browse the directories.");
	tooltips().init(file_->button_browse, str);

	str = _("Scale the image to inserted percentage value");
	tooltips().init(file_->input_lyxscale, str);
	str = _("Select display mode for this image.");
	tooltips().init(file_->choice_display, str);

	str = _("Set the image width to the inserted value.");
	tooltips().init(file_->input_width, str);
	str = _("Select unit for width; Scale% for scaling whole image");
	tooltips().init(file_->choice_width, str);
	str = _("Set the image height to the inserted value.");
	tooltips().init(file_->input_height, str);
	str = _("Select unit for height");
	tooltips().init(file_->choice_height, str);
	str = _("Do not distort the image. " 
                "Keep image within \"width\" by \"height\" and obey aspect ratio.");
	tooltips().init(file_->check_aspectratio, str);

	str = _("Pass a filename like \"file.eps.gz\" to the LaTeX output. "
	    "This is useful when LaTeX should unzip the file. Needs an additional file "
	    "like \"file.eps.bb\" which holds the values for the bounding box.");
	tooltips().init(file_->check_nounzip, str);

	str = _("Show image only as a rectangle of the original size.");
	tooltips().init(file_->check_draft, str);

	// the bounding box selection
	bbox_.reset(build_graphics_bbox(this));
	fl_set_input_return (bbox_->input_bb_x0, FL_RETURN_CHANGED);
	fl_set_input_return (bbox_->input_bb_y0, FL_RETURN_CHANGED);
	fl_set_input_return (bbox_->input_bb_x1, FL_RETURN_CHANGED);
	fl_set_input_return (bbox_->input_bb_y1, FL_RETURN_CHANGED);

	fl_set_input_filter(bbox_->input_bb_x0, fl_unsigned_float_filter);
	fl_set_input_filter(bbox_->input_bb_y0, fl_unsigned_float_filter);
	fl_set_input_filter(bbox_->input_bb_x1, fl_unsigned_float_filter);
	fl_set_input_filter(bbox_->input_bb_y1, fl_unsigned_float_filter);

	setPrehandler(bbox_->input_bb_x0);
	setPrehandler(bbox_->input_bb_y0);
	setPrehandler(bbox_->input_bb_x1);
	setPrehandler(bbox_->input_bb_y1);

	string const bb_units = "bp|cm|mm|in";
	fl_addto_choice(bbox_->choice_bb_units, bb_units.c_str());
	bc().addReadOnly(bbox_->button_getBB);
	bc().addReadOnly(bbox_->check_clip);

	// set up the tooltips for the bounding-box-section
	str = _("The lower left x-value of the bounding box");
	tooltips().init(bbox_->input_bb_x0, str);
	str = _("The lower left y-value of the bounding box");
	tooltips().init(bbox_->input_bb_y0, str);
	str = _("The upper right x-value of the bounding box");
	tooltips().init(bbox_->input_bb_x1, str);
	str = _("The upper right y-value of the bounding box");
	tooltips().init(bbox_->input_bb_y1, str);
	str = _("Select unit for the bounding box values");
	tooltips().init(bbox_->choice_bb_units, str);

	str = _("Read the image coordinates new from file. If it's an (e)ps-file "
		"then the bounding box is read otherwise the imagesize in pixels. "
		"The default unit is \"bp\", the PostScript's b(ig) p(oint).");
	tooltips().init(bbox_->button_getBB, str);

	str = _("Enable this checkbox when the image should be clipped to the "
		"bounding box values.");
	tooltips().init(bbox_->check_clip, str);

	// the extra section
	extra_.reset(build_graphics_extra(this));

	fl_set_input_return (extra_->input_rotate_angle, FL_RETURN_CHANGED);
	fl_set_input_return (extra_->input_subcaption, FL_RETURN_CHANGED);
	fl_set_input_return (extra_->input_special, FL_RETURN_CHANGED);

	fl_set_input_filter(extra_->input_rotate_angle, fl_float_filter);

	setPrehandler(extra_->input_rotate_angle);
	setPrehandler(extra_->input_subcaption);
	setPrehandler(extra_->input_special);

	bc().addReadOnly(extra_->check_subcaption);

	using namespace frnt;
	vector<RotationOriginPair> origindata = getRotationOriginData();

	// Store the identifiers for later
	origins_ = getSecond(origindata);

	string const choice = "Default|" + getStringFromVector(getFirst(origindata), "|");
	fl_addto_choice(extra_->choice_origin, choice.c_str());

	// set up the tooltips for the extra section
	str = _("Insert the rotation angle in degrees. "
	        "Positive value rotates anti-clockwise, negative value clockwise");
	tooltips().init(extra_->input_rotate_angle, str);
	str = _("Insert the point of origin for rotation ");
	tooltips().init(extra_->choice_origin, str);
	str = _("Enables use of subfigure with its own caption.");
	tooltips().init(extra_->check_subcaption, str);
	str = _("Insert the optional subfigure caption");
	tooltips().init(extra_->input_subcaption, str);
	str = _("Add any additional latex option, which is defined in the "
                "graphicx-package and not mentioned in the gui's tabfolders.");
	tooltips().init(extra_->input_special, str);

	// add the different tabfolders
	fl_addto_tabfolder(dialog_->tabfolder, _("File"), file_->form);
	fl_addto_tabfolder(dialog_->tabfolder, _("Bounding Box"), bbox_->form);
	fl_addto_tabfolder(dialog_->tabfolder, _("Extra"), extra_->form);

	// set the right default unit
	switch (lyxrc.default_papersize) {
	case BufferParams::PAPER_DEFAULT: break;
	case BufferParams::PAPER_USLETTER:
	case BufferParams::PAPER_LEGALPAPER:
	case BufferParams::PAPER_EXECUTIVEPAPER: defaultUnit = "in"; break;
	case BufferParams::PAPER_A3PAPER:
	case BufferParams::PAPER_A4PAPER:
	case BufferParams::PAPER_A5PAPER:
	case BufferParams::PAPER_B5PAPER: defaultUnit = "cm"; break;
	}
}


void FormGraphics::apply()
{
	// Create the parameters structure and fill the data from the dialog.
	InsetGraphicsParams & igp = controller().params();

	// the file section
	igp.filename = getString(file_->input_filename);

	igp.lyxscale = strToInt(getString(file_->input_lyxscale));
	if (igp.lyxscale == 0) {
		igp.lyxscale = 100;
	}
	
	switch (fl_get_choice(file_->choice_display)) {
		case 5: igp.display = grfx::NoDisplay; break;
		case 4: igp.display = grfx::ColorDisplay; break;
		case 3: igp.display = grfx::GrayscaleDisplay; break;
		case 2: igp.display = grfx::MonochromeDisplay; break;
		case 1:
		default: igp.display = grfx::DefaultDisplay;
	}

	// first item in choice_width means scaling
	if (fl_get_choice(file_->choice_width) == 1) {
		igp.scale = strToInt(getString(file_->input_width));
		if (igp.scale == 0) {
			igp.scale = 100;
		}
		igp.width = LyXLength();
	} else {
		igp.scale = 0;
		igp.width = getLyXLengthFromWidgets(file_->input_width,
						    file_->choice_width);
	}
	igp.height = getLyXLengthFromWidgets(file_->input_height,
					     file_->choice_height);
	igp.keepAspectRatio = fl_get_button(file_->check_aspectratio);

	igp.draft = fl_get_button(file_->check_draft);
	igp.noUnzip = fl_get_button(file_->check_nounzip);

	// the bb section
	if (!controller().bbChanged) { // different to the original one?
		igp.bb = string();     // don't write anything
	} else {
		string bb;
		if (getString(bbox_->input_bb_x0).empty())
			bb = "0 ";
		else
			bb = getLengthFromWidgets(bbox_->input_bb_x0,
						  bbox_->choice_bb_units)+" ";
		if (getString(bbox_->input_bb_y0).empty())
			bb += "0 ";
		else
			bb += (getLengthFromWidgets(bbox_->input_bb_y0,
						    bbox_->choice_bb_units)+" ");
		if (getString(bbox_->input_bb_x1).empty())
			bb += "0 ";
		else
			bb += (getLengthFromWidgets(bbox_->input_bb_x1,
						    bbox_->choice_bb_units)+" ");
		if (getString(bbox_->input_bb_y1).empty())
			bb += "0 ";
		else
			bb += (getLengthFromWidgets(bbox_->input_bb_y1,
						    bbox_->choice_bb_units)+" ");
		igp.bb = bb;
	}
	igp.clip = fl_get_button(bbox_->check_clip);

	// the extra section
	igp.rotateAngle = strToDbl(getString(extra_->input_rotate_angle));
	
	// map angle into -360 (clock-wise) to +360 (counter clock-wise)
	while (igp.rotateAngle <= -360.0) {
		igp.rotateAngle += 360.0;
	}
	while (igp.rotateAngle >=  360.0) {
		igp.rotateAngle -= 360.0;
	}
	fl_set_input(extra_->input_rotate_angle, tostr(igp.rotateAngle).c_str());

	int const origin_pos = fl_get_choice(extra_->choice_origin);
	if (origin_pos == 1) {
		igp.rotateOrigin.erase();
	} else {
		igp.rotateOrigin = origins_[origin_pos - 2];
	}

	igp.subcaption = fl_get_button(extra_->check_subcaption);
	igp.subcaptionText = getString(extra_->input_subcaption);

	igp.special = getString(extra_->input_special);
}


void FormGraphics::update() {
	// Update dialog with details from inset
	InsetGraphicsParams & igp = controller().params();

	// the file section
	fl_set_input(file_->input_filename, igp.filename.c_str());
	fl_set_input(file_->input_lyxscale, tostr(igp.lyxscale).c_str());

	switch (igp.display) {
		case grfx::NoDisplay:		fl_set_choice(file_->choice_display, 5); break;
		case grfx::ColorDisplay:	fl_set_choice(file_->choice_display, 4); break;
		case grfx::GrayscaleDisplay:	fl_set_choice(file_->choice_display, 3); break;
		case grfx::MonochromeDisplay:	fl_set_choice(file_->choice_display, 2); break;
		case grfx::DefaultDisplay:
		default:			fl_set_choice(file_->choice_display, 1);
	}

	// disable height input in case of scaling
	setEnabled(file_->input_height, !igp.scale);
	setEnabled(file_->choice_height, !igp.scale);

	// set width input fields according to scaling or width/height input
	if (igp.scale) {
		fl_set_input_filter(file_->input_width, fl_unsigned_int_filter);
		fl_set_input_maxchars(file_->input_width, 0);
		fl_set_input(file_->input_width, tostr(igp.scale).c_str());
		fl_set_choice(file_->choice_width, 1);
	} else {
		fl_set_input_filter(file_->input_width, NULL);
		fl_set_input_maxchars(file_->input_width, SIZE_MAXDIGITS);
		updateWidgetsFromLength(file_->input_width,
					file_->choice_width, igp.width, defaultUnit);
	}

	updateWidgetsFromLength(file_->input_height,
				file_->choice_height, igp.height, defaultUnit);
	
	fl_set_button(file_->check_aspectratio, igp.keepAspectRatio);
	fl_set_button(file_->check_draft, igp.draft);
	fl_set_button(file_->check_nounzip, igp.noUnzip);

	// disable aspectratio button in case of scaling or one of width/height is empty
	bool const disable_aspectRatio = igp.scale ||
				getString(file_->input_width).empty() ||
				getString(file_->input_height).empty();
	setEnabled(file_->check_aspectratio, !disable_aspectRatio);

	// the bb section
	// set the bounding box values, if exists. First we need the whole
	// path, because the controller knows nothing about the doc-dir
	updateBB(igp.filename, igp.bb);
	fl_set_button(bbox_->check_clip, igp.clip);


	// the extra section
	fl_set_input(extra_->input_rotate_angle,
		     tostr(igp.rotateAngle).c_str());

	int origin_pos;
	if (igp.rotateOrigin.empty()) {
		origin_pos = 1;
	} else {
		origin_pos = 2 + findPos(origins_, igp.rotateOrigin);
	}
	fl_set_choice(extra_->choice_origin, origin_pos);

	fl_set_button(extra_->check_subcaption, igp.subcaption);
	fl_set_input(extra_->input_subcaption, igp.subcaptionText.c_str());
	setEnabled(extra_->input_subcaption,
		   fl_get_button(extra_->check_subcaption));
	fl_set_input(extra_->input_special, igp.special.c_str());

	// open dialog in the file-tab, whenever filename is empty
	if (igp.filename.empty()) {
		fl_set_folder(dialog_->tabfolder, file_->form);
	}
}


void FormGraphics::updateBB(string const & filename, string const & bb_inset)
{
	// Update dialog with details from inset
	// set the bounding box values, if exists. First we need the whole
	// path, because the controller knows nothing about the doc-dir
	controller().bbChanged = false;
	if (bb_inset.empty()) {
		lyxerr[Debug::GRAPHICS] << "FormGraphics::updateBB() [no BoundingBox]" << endl;
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
		lyxerr[Debug::GRAPHICS] << "FormGraphics::updateBB(): igp has BoundingBox"
					<< " ["<< bb_inset << "]"
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
	string const input = getString(ob);
	return input.empty() || isValidLength(input) || isStrDbl(input);
}

} // namespace anon



ButtonPolicy::SMInput FormGraphics::input(FL_OBJECT * ob, long)
{
	// the file section
	if (ob == file_->button_browse) {
		// Get the filename from the dialog
		string const in_name = getString(file_->input_filename);
		string const out_name = controller().Browse(in_name);
		lyxerr[Debug::GRAPHICS] << "[FormGraphics]out_name: " << out_name << endl;
		if (out_name != in_name && !out_name.empty()) {
			fl_set_input(file_->input_filename, out_name.c_str());
		}
		if (controller().isFilenameValid(out_name) &&
		    !controller().bbChanged) {
			updateBB(out_name, string());
		}
	} else if (ob == file_->input_width || ob == file_->input_height) {
		// disable aspectratio button in case of scaling or one of width/height is empty
		bool const disable = fl_get_choice(file_->choice_width) == 1 ||
				    getString(file_->input_width).empty() ||
				    getString(file_->input_height).empty();
		setEnabled(file_->check_aspectratio, !disable);
	} else if (ob == file_->choice_width) {
		// disable height input in case of scaling
		bool const scaling = fl_get_choice(file_->choice_width) == 1;
		setEnabled(file_->input_height, !scaling);
		setEnabled(file_->choice_height, !scaling);
		
		// allow only integer intput for scaling; float otherwise
		if (scaling) {
			fl_set_input_filter(file_->input_width, fl_unsigned_int_filter);
			fl_set_input_maxchars(file_->input_width, 0);
		} else {
			fl_set_input_filter(file_->input_width, NULL);
			fl_set_input_maxchars(file_->input_width, SIZE_MAXDIGITS);
		}

		// disable aspectratio button in case of scaling or height input is empty
		bool const disable_aspectratio = scaling || getString(file_->input_height).empty();
		setEnabled(file_->check_aspectratio, !disable_aspectratio);
	// the bb section
	} else if (!controller().bbChanged &&
		   (ob == bbox_->check_clip  || ob == bbox_->choice_bb_units ||
		    ob == bbox_->input_bb_x0 || ob == bbox_->input_bb_y0 ||
		    ob == bbox_->input_bb_x1 || ob == bbox_->input_bb_y1)) {
		controller().bbChanged = true;
	} else if (ob == bbox_->button_getBB) {
		string const filename = getString(file_->input_filename);
		if (!filename.empty()) {
			string bb = controller().readBB(filename);
			if (!bb.empty()) {
				fl_set_input(bbox_->input_bb_x0, token(bb,' ',0).c_str());
				fl_set_input(bbox_->input_bb_y0, token(bb,' ',1).c_str());
				fl_set_input(bbox_->input_bb_x1, token(bb,' ',2).c_str());
				fl_set_input(bbox_->input_bb_y1, token(bb,' ',3).c_str());
				fl_set_choice_text(bbox_->choice_bb_units, "bp");
			}
			controller().bbChanged = false;
		} else {
			fl_set_input(bbox_->input_bb_x0, "");
			fl_set_input(bbox_->input_bb_y0, "");
			fl_set_input(bbox_->input_bb_x1, "");
			fl_set_input(bbox_->input_bb_y1, "");
			fl_set_choice_text(bbox_->choice_bb_units, "bp");
		}
	// the extra section
	} else if (ob == extra_->check_subcaption) {
		setEnabled(extra_->input_subcaption,
			   fl_get_button(extra_->check_subcaption));

	}

	// check if the input is valid
	bool const invalid = !isValid(file_->input_width) || !isValid(file_->input_height);

	// deactivate OK / Apply buttons and spit out warnings if invalid
	if (invalid) {
		postWarning(_("Invalid Length in Output size!"));
		return ButtonPolicy::SMI_INVALID;
	} else {
		clearMessage();
		return ButtonPolicy::SMI_VALID;
	}
}
