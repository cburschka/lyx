/**
 * \file GGraphics.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Spray
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

// Too hard to make concept checks work with this file
#ifdef _GLIBCPP_CONCEPT_CHECKS
#undef _GLIBCPP_CONCEPT_CHECKS
#endif

#include "GGraphics.h"
#include "ControlGraphics.h"

#include "ghelpers.h"

#include "controllers/helper_funcs.h"

#include "insets/insetgraphicsParams.h"
#include "paper.h"
#include "lyxrc.h" // for lyxrc.default_papersize

#include "support/lyxlib.h"  // for float_equal
#include "support/lstrings.h"
#include "support/convert.h"

#include "debug.h"

using std::vector;
using std::string;

namespace lyx {

using support::float_equal;
using support::strToDbl;
using support::token;

namespace frontend {

namespace {
string defaultUnit("cm");
} // namespace anon

GGraphics::GGraphics(Dialog & parent)
	: GViewCB<ControlGraphics, GViewGladeB>(parent, _("Graphics"), false)
{}


void GGraphics::doBuild()
{
	string const gladeName = findGladeFile("graphics");
	xml_ = Gnome::Glade::Xml::create(gladeName);

	Gtk::Button * button;
	xml_->get_widget("Close", button);
	setCancel(button);
	xml_->get_widget("Ok", button);
	setOK(button);
	xml_->get_widget("Apply", button);
	setApply(button);
	xml_->get_widget("Restore", button);
	setRestore(button);

	xml_->get_widget("Notebook", notebook_);

	// File Page
	xml_->get_widget("File", fileentry_);
	xml_->get_widget("DisplayScale", displayscalespin_);
	xml_->get_widget("OutputScale", outputscalespin_);
	xml_->get_widget("Browse", browsebutton_);
	xml_->get_widget("Edit", editbutton_);
	xml_->get_widget("Display", displaycombo_);
	xml_->get_widget("Width", widthspin_);
	xml_->get_widget("Height", heightspin_);
	xml_->get_widget("MaintainAspectRatio", aspectcheck_);
	xml_->get_widget("WidthUnits", widthunitscombo_);
	xml_->get_widget("HeightUnits", heightunitscombo_);
	xml_->get_widget("SetScaling", setscalingradio_);
	xml_->get_widget("SetSize", setsizeradio_);

	// Bounding Box Page
	xml_->get_widget("ClipToBoundingBox", clipcheck_);
	xml_->get_widget("RightTopX", righttopxspin_);
	xml_->get_widget("RightTopY", righttopyspin_);
	xml_->get_widget("LeftBottomX", leftbottomxspin_);
	xml_->get_widget("LeftBottomY", leftbottomyspin_);
	xml_->get_widget("BoundingUnits", bbunitscombo_);
	xml_->get_widget("GetFromFile", bbfromfilebutton_);

	// Extra Page
	xml_->get_widget("Angle", anglespin_);
	xml_->get_widget("Origin", origincombo_);
	xml_->get_widget("UseSubfigure", subfigcheck_);
	xml_->get_widget("SubfigureCaption", subfigentry_);
	xml_->get_widget("LatexOptions", latexoptsentry_);
	xml_->get_widget("DraftMode", draftcheck_);
	xml_->get_widget("UnzipOnExport", unzipcheck_);

	// Setup the columnrecord we use for combos
	cols_.add(stringcol_);

	// The file page

	// Disable for read-only documents.
	bcview().addReadOnly(browsebutton_);
	bcview().addReadOnly(aspectcheck_);

	fileentry_->signal_changed().connect(
		sigc::mem_fun(*this, &GGraphics::onInput));
	displayscalespin_->signal_changed().connect(
		sigc::mem_fun(*this, &GGraphics::onInput));
	displaycombo_->signal_changed().connect(
		sigc::mem_fun(*this, &GGraphics::onInput));
	outputscalespin_->signal_changed().connect(
		sigc::mem_fun(*this, &GGraphics::onInput));
	heightspin_->signal_changed().connect(
		sigc::mem_fun(*this, &GGraphics::onInput));
	heightunitscombo_->signal_changed().connect(
		sigc::mem_fun(*this, &GGraphics::onInput));
	widthspin_->signal_changed().connect(
		sigc::mem_fun(*this, &GGraphics::onInput));
	widthunitscombo_->signal_changed().connect(
		sigc::mem_fun(*this, &GGraphics::onInput));
	aspectcheck_->signal_toggled().connect(
		sigc::mem_fun(*this, &GGraphics::onInput));

	setscalingradio_->signal_toggled().connect(
		sigc::mem_fun(*this, &GGraphics::onSizingModeChange));
	setsizeradio_->signal_toggled().connect(
		sigc::mem_fun(*this, &GGraphics::onSizingModeChange));

	browsebutton_->signal_clicked().connect(
		sigc::mem_fun(*this, &GGraphics::onBrowseClicked));

	editbutton_->signal_clicked().connect(
		sigc::mem_fun(*this, &GGraphics::onEditClicked));

	vector<string> const unit_list = buildLengthUnitList();
	PopulateComboBox(heightunitscombo_, unit_list);
	PopulateComboBox(widthunitscombo_, unit_list);

	// the bounding box page
	leftbottomxspin_->signal_changed().connect(
		sigc::mem_fun(*this, &GGraphics::onBBChange));
	leftbottomyspin_->signal_changed().connect(
		sigc::mem_fun(*this, &GGraphics::onBBChange));
	righttopxspin_->signal_changed().connect(
		sigc::mem_fun(*this, &GGraphics::onBBChange));
	righttopyspin_->signal_changed().connect(
		sigc::mem_fun(*this, &GGraphics::onBBChange));
	bbunitscombo_->signal_changed().connect(
		sigc::mem_fun(*this, &GGraphics::onBBChange));
	clipcheck_->signal_toggled().connect(
		sigc::mem_fun(*this, &GGraphics::onBBChange));

	bbfromfilebutton_->signal_clicked().connect(
		sigc::mem_fun(*this, &GGraphics::onBBFromFileClicked));

	// disable for read-only documents
	bcview().addReadOnly(bbfromfilebutton_);
	bcview().addReadOnly(clipcheck_);
	PopulateComboBox(bbunitscombo_, getBBUnits());


	// the extra section

	// disable for read-only documents
	bcview().addReadOnly(anglespin_);
	bcview().addReadOnly(origincombo_);
	bcview().addReadOnly(subfigcheck_);
	bcview().addReadOnly(latexoptsentry_);
	bcview().addReadOnly(draftcheck_);
	bcview().addReadOnly(unzipcheck_);

	anglespin_->signal_changed().connect(
		sigc::mem_fun(*this, &GGraphics::onInput));
	origincombo_->signal_changed().connect(
		sigc::mem_fun(*this, &GGraphics::onInput));
	subfigentry_->signal_changed().connect(
		sigc::mem_fun(*this, &GGraphics::onInput));
	latexoptsentry_->signal_changed().connect(
		sigc::mem_fun(*this, &GGraphics::onInput));
	draftcheck_->signal_toggled().connect(
		sigc::mem_fun(*this, &GGraphics::onInput));
	unzipcheck_->signal_toggled().connect(
		sigc::mem_fun(*this, &GGraphics::onInput));

	subfigcheck_->signal_toggled().connect(
		sigc::mem_fun(*this, &GGraphics::onSubFigCheckToggled));

	vector<RotationOriginPair> origindata = getRotationOriginData();

	// Store the identifiers for later
	origins_ = getSecond(origindata);
	PopulateComboBox(origincombo_, getFirst(origindata));
}


void GGraphics::onSizingModeChange()
{
	bool const scalingmode = setscalingradio_->get_active();

	outputscalespin_->set_sensitive(scalingmode);
	widthspin_->set_sensitive(!scalingmode);
	heightspin_->set_sensitive(!scalingmode);
	widthunitscombo_->set_sensitive(!scalingmode);
	heightunitscombo_->set_sensitive(!scalingmode);
	aspectcheck_->set_sensitive(!scalingmode);
	bc().input(ButtonPolicy::SMI_VALID);
}


void GGraphics::PopulateComboBox(Gtk::ComboBox * combo,
				  vector<string> const & strings)
{
	Glib::RefPtr<Gtk::ListStore> model = Gtk::ListStore::create(cols_);
	vector<string>::const_iterator it = strings.begin();
	vector<string>::const_iterator end = strings.end();
	for (int rowindex = 0; it != end; ++it, ++rowindex) {
		Gtk::TreeModel::iterator row = model->append();
		(*row)[stringcol_] = *it;
	}

	combo->set_model(model);
	Gtk::CellRendererText * cell = Gtk::manage(new Gtk::CellRendererText);
	combo->pack_start(*cell, true);
	combo->add_attribute(*cell, "text", 0);
}


void GGraphics::apply()
{
	// Create the parameters structure and fill the data from the dialog.
	InsetGraphicsParams & igp = controller().params();

	// the file section
	igp.filename.set(fileentry_->get_text(),
			 kernel().bufferFilepath());

	igp.lyxscale =
		static_cast<int>(displayscalespin_->get_adjustment()->get_value());
	if (igp.lyxscale == 0) {
		igp.lyxscale = 100;
	}

	switch (displaycombo_->get_active_row_number()) {
	case 4:
		igp.display = graphics::NoDisplay;
		break;
	case 3:
		igp.display = graphics::ColorDisplay;
		break;
	case 2:
		igp.display = graphics::GrayscaleDisplay;
		break;
	case 1:
		igp.display = graphics::MonochromeDisplay;
		break;
	case 0:
		igp.display = graphics::DefaultDisplay;
	}

	if (setscalingradio_->get_active()) {
		float scaleValue = outputscalespin_->get_adjustment()->get_value();
		igp.scale = convert<string>(scaleValue);
		if (float_equal(scaleValue, 0.0, 0.05))
			igp.scale = string();
		igp.width = LyXLength();
	} else {
		igp.scale = string();
		Glib::ustring const widthunit =
			(*widthunitscombo_->get_active())[stringcol_];
		igp.width = LyXLength(widthspin_->get_text() + widthunit);
	}


	Glib::ustring const heightunit =
		(*heightunitscombo_->get_active())[stringcol_];
	igp.height = LyXLength(heightspin_->get_text() + heightunit);

	igp.keepAspectRatio = aspectcheck_->get_active();
	igp.draft = draftcheck_->get_active();
	igp.noUnzip = !unzipcheck_->get_active();

	// the bb section

	if (!controller().bbChanged) {
		// don't write anything
		igp.bb.erase();
	} else {
		Glib::ustring const bbunit = (*bbunitscombo_->get_active())[stringcol_];
		string bb;

		if (leftbottomxspin_->get_text().empty())
			bb = "0";
		else
			bb = leftbottomxspin_->get_text() + bbunit;

		bb += ' ';

		if (leftbottomyspin_->get_text().empty())
			bb += "0";
		else
			bb += leftbottomyspin_->get_text() + bbunit;

		bb += ' ';

		if (righttopxspin_->get_text().empty())
			bb += "0";
		else
			bb += righttopxspin_->get_text() + bbunit;

		bb += ' ';

		if (righttopyspin_->get_text().empty())
			bb += "0";
		else
			bb += righttopyspin_->get_text() + bbunit;

		/// The bounding box with "xLB yLB yRT yRT ", divided by a space!
		igp.bb = bb;
	}
	igp.clip = clipcheck_->get_active();

	// the extra section
	igp.rotateAngle = convert<string>(anglespin_->get_adjustment()->get_value());

	int const origin_pos = origincombo_->get_active_row_number();
	igp.rotateOrigin = origins_[origin_pos];

	igp.subcaption = subfigcheck_->get_active();
	igp.subcaptionText = subfigentry_->get_text();

	igp.special = latexoptsentry_->get_text();
}


void GGraphics::update() {
	// set the right default unit
	defaultUnit = getDefaultUnit();

	// Update dialog with details from inset
	InsetGraphicsParams & igp = controller().params();

	// the file section
	string const name =
		igp.filename.outputFilename(kernel().bufferFilepath());
	fileentry_->set_text(name);
	displayscalespin_->get_adjustment()->set_value(igp.lyxscale);


	switch (igp.display) {
	case graphics::NoDisplay:
		displaycombo_->set_active(4);
		break;
	case graphics::ColorDisplay:
		displaycombo_->set_active(3);
		break;
	case graphics::GrayscaleDisplay:
		displaycombo_->set_active(2);
		break;
	case graphics::MonochromeDisplay:
		displaycombo_->set_active(1);
		break;
	case graphics::DefaultDisplay:
		displaycombo_->set_active(0);
	}

	outputscalespin_->get_adjustment()->set_value(strToDbl(igp.scale));
	widthspin_->get_adjustment()->set_value(igp.width.value());
	unitsComboFromLength(widthunitscombo_, stringcol_,
	                     igp.width, defaultUnit);
	heightspin_->get_adjustment()->set_value(igp.height.value());
	unitsComboFromLength(heightunitscombo_, stringcol_,
	                     igp.height, defaultUnit);

	if (!igp.scale.empty()
		&& !float_equal(strToDbl(igp.scale), 0.0, 0.05)) {
		// scaling sizing mode
		setscalingradio_->set_active(true);
	} else {
		setsizeradio_->set_active(true);
	}
	onSizingModeChange();

	aspectcheck_->set_active(igp.keepAspectRatio);
	draftcheck_->set_active(igp.draft);
	unzipcheck_->set_active(!igp.noUnzip);

	// the bb section
	// set the bounding box values, if exists. First we need the whole
	// path, because the controller knows nothing about the doc-dir
	updateBB(igp.filename.absFilename(), igp.bb);
	clipcheck_->set_active(igp.clip);

	// the extra section
	anglespin_->get_adjustment()->set_value(strToDbl(igp.rotateAngle));

	int origin_pos;
	if (igp.rotateOrigin.empty()) {
		origin_pos = 0;
	} else {
		origin_pos = findPos(origins_, igp.rotateOrigin);
	}
	origincombo_->set_active(origin_pos);


	subfigcheck_->set_active(igp.subcaption);
	subfigentry_->set_text(igp.subcaptionText);
	subfigentry_->set_sensitive(subfigcheck_->get_active());
	latexoptsentry_->set_text(igp.special);

	// open dialog in the file-tab, whenever filename is empty
	if (igp.filename.empty()) {
		notebook_->set_current_page(0);
	}


	bc().input(ButtonPolicy::SMI_INVALID);
}


void GGraphics::updateBB(string const & filename, string const & bb_inset)
{
	// Update dialog with details from inset
	// set the bounding box values, if exists. First we need the whole
	// path, because the controller knows nothing about the doc-dir

	if (bb_inset.empty()) {
		lyxerr[Debug::GRAPHICS]
			<< "GGraphics::updateBB() [no BoundingBox]\n";
		string const bb = controller().readBB(filename);
		if (!bb.empty()) {
			// get the values from the file
			// in this case we always have the point-unit
			leftbottomxspin_->set_text(token(bb, ' ', 0));
			leftbottomyspin_->set_text(token(bb, ' ', 1));
			righttopxspin_->set_text(token(bb, ' ', 2));
			righttopyspin_->set_text(token(bb, ' ', 3));
		} else {
			// no bb from file
			leftbottomxspin_->set_text("");
			leftbottomyspin_->set_text("");
			righttopxspin_->set_text("");
			righttopyspin_->set_text("");
		}
		unitsComboFromLength(bbunitscombo_, stringcol_,
		                     LyXLength("bp"), defaultUnit);
	} else {
		// get the values from the inset
		lyxerr[Debug::GRAPHICS]
			<< "FormGraphics::updateBB(): igp has BoundingBox"
			<< " ["<< bb_inset << "]\n";

		LyXLength anyLength;
		anyLength = LyXLength(token(bb_inset, ' ', 0));

		unitsComboFromLength(bbunitscombo_, stringcol_, anyLength, defaultUnit);

		leftbottomxspin_->get_adjustment()->set_value(anyLength.value());

		anyLength = LyXLength(token(bb_inset, ' ', 1));
		leftbottomyspin_->get_adjustment()->set_value(anyLength.value());

		anyLength = LyXLength(token(bb_inset, ' ', 2));
		righttopxspin_->get_adjustment()->set_value(anyLength.value());

		anyLength = LyXLength(token(bb_inset, ' ', 3));
		righttopyspin_->get_adjustment()->set_value(anyLength.value());
	}
	controller().bbChanged = false;
}


void GGraphics::onBrowseClicked()
{
	// Get the filename from the dialog
	string const in_name = fileentry_->get_text();
	string const out_name = controller().browse(in_name);
	lyxerr[Debug::GRAPHICS]
		<< "[FormGraphics]out_name: " << out_name << "\n";
	if (out_name != in_name && !out_name.empty()) {
		fileentry_->set_text(out_name);
	}
	if (controller().isFilenameValid(out_name) &&
	    !controller().bbChanged) {
		updateBB(out_name, string());
	}

	bc().input(ButtonPolicy::SMI_VALID);
}


void GGraphics::onBBChange()
{
	controller().bbChanged = true;
	bc().input(ButtonPolicy::SMI_VALID);
}


void GGraphics::onBBFromFileClicked()
{
	string const filename = fileentry_->get_text();
	if (!filename.empty()) {
		string bb = controller().readBB(filename);
		if (!bb.empty()) {
			leftbottomxspin_->set_text(token(bb, ' ', 0));
			leftbottomyspin_->set_text(token(bb, ' ', 1));
			righttopxspin_->set_text(token(bb, ' ', 2));
			righttopyspin_->set_text(token(bb, ' ', 3));
			unitsComboFromLength(bbunitscombo_, stringcol_,
			                     LyXLength("bp"), defaultUnit);
		}
		controller().bbChanged = false;
	} else {
		leftbottomxspin_->set_text("");
		leftbottomyspin_->set_text("");
		righttopxspin_->set_text("");
		righttopyspin_->set_text("");
		unitsComboFromLength(bbunitscombo_, stringcol_,
		                     LyXLength("bp"), defaultUnit);
	}
	bc().input(ButtonPolicy::SMI_VALID);
}


void GGraphics::onSubFigCheckToggled()
{
	subfigentry_->set_sensitive(subfigcheck_->get_active());
	bc().input(ButtonPolicy::SMI_VALID);
}


void GGraphics::onEditClicked()
{
	controller().editGraphics();
}


void GGraphics::onInput()
{
	bc().input(ButtonPolicy::SMI_VALID);
}

} // namespace frontend
} // namespace lyx
