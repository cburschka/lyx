/**
 * \file GuiGraphics.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Edwin Leuven
 * \author Herbert Voﬂ
 * \author Richard Heck
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiGraphics.h"

#include "CheckedLineEdit.h"
#include "LengthCombo.h"
#include "GuiGraphicsDialog.h"
#include "Qt2BC.h"
#include "qt_helpers.h"

#include "lengthcommon.h"
#include "LyXRC.h"

#include "controllers/ControlGraphics.h"
#include "controllers/frontend_helpers.h"

#include "insets/InsetGraphicsParams.h"

#include "support/convert.h"
#include "support/lstrings.h"
#include "support/lyxlib.h"
#include "support/os.h"

#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>

#include <cmath>

using lyx::support::float_equal;
using lyx::support::token;

using lyx::support::os::internal_path;

#ifndef CXX_GLOBAL_CSTD
using std::floor;
#endif

using std::vector;
using std::string;

namespace lyx {
namespace frontend {

typedef QController<ControlGraphics, GuiView<GuiGraphicsDialog> > graphics_base_class;

GuiGraphics::GuiGraphics(Dialog & parent)
	: graphics_base_class(parent, _("Graphics"))
{
}


void GuiGraphics::build_dialog()
{
	dialog_.reset(new GuiGraphicsDialog(this));

	bcview().setOK(dialog_->okPB);
	bcview().setApply(dialog_->applyPB);
	bcview().setRestore(dialog_->restorePB);
	bcview().setCancel(dialog_->closePB);

	bcview().addReadOnly(dialog_->latexoptions);
	bcview().addReadOnly(dialog_->subfigure);
	bcview().addReadOnly(dialog_->filenameL);
	bcview().addReadOnly(dialog_->filename);
	bcview().addReadOnly(dialog_->browsePB);
	bcview().addReadOnly(dialog_->unzipCB);
	bcview().addReadOnly(dialog_->bbFrame);
	bcview().addReadOnly(dialog_->draftCB);
	bcview().addReadOnly(dialog_->clip);
	bcview().addReadOnly(dialog_->unzipCB);
	bcview().addReadOnly(dialog_->displayGB);
	bcview().addReadOnly(dialog_->sizeGB);
	bcview().addReadOnly(dialog_->rotationGB);
	bcview().addReadOnly(dialog_->latexoptions);
	bcview().addReadOnly(dialog_->getPB);
	bcview().addReadOnly(dialog_->rotateOrderCB);

	// initialize the length validator
	addCheckedLineEdit(bcview(), dialog_->Scale, dialog_->scaleCB);
	addCheckedLineEdit(bcview(), dialog_->Width, dialog_->WidthCB);
	addCheckedLineEdit(bcview(), dialog_->Height, dialog_->HeightCB);
	addCheckedLineEdit(bcview(), dialog_->displayscale, dialog_->scaleLA);
	addCheckedLineEdit(bcview(), dialog_->angle, dialog_->angleL);
	addCheckedLineEdit(bcview(), dialog_->lbX, dialog_->xL);
	addCheckedLineEdit(bcview(), dialog_->lbY, dialog_->yL);
	addCheckedLineEdit(bcview(), dialog_->rtX, dialog_->xL_2);
	addCheckedLineEdit(bcview(), dialog_->rtY, dialog_->yL_2);
	addCheckedLineEdit(bcview(), dialog_->filename, dialog_->filenameL);
}


namespace {

// returns the number of the string s in the vector v
int getItemNo(vector<string> v, string const & s) {
	vector<string>::const_iterator cit =
		    find(v.begin(), v.end(), s);
	return (cit != v.end()) ? int(cit - v.begin()) : 0;
}

}


void GuiGraphics::update_contents()
{
	// clear and fill in the comboboxes
	vector<string> const bb_units = frontend::getBBUnits();
	dialog_->lbXunit->clear();
	dialog_->lbYunit->clear();
	dialog_->rtXunit->clear();
	dialog_->rtYunit->clear();
	for (vector<string>::const_iterator it = bb_units.begin();
	    it != bb_units.end(); ++it) {
		dialog_->lbXunit->addItem(toqstr(*it));
		dialog_->lbYunit->addItem(toqstr(*it));
		dialog_->rtXunit->addItem(toqstr(*it));
		dialog_->rtYunit->addItem(toqstr(*it));
	}

	InsetGraphicsParams & igp = controller().params();

	// set the right default unit
	Length::UNIT unitDefault = Length::CM;
	switch (lyxrc.default_papersize) {
		case PAPER_USLETTER:
		case PAPER_USLEGAL:
		case PAPER_USEXECUTIVE:
			unitDefault = Length::IN;
			break;
		default:
			break;
	}

	string const name =
		igp.filename.outputFilename(kernel().bufferFilepath());
	dialog_->filename->setText(toqstr(name));

	// set the bounding box values
	if (igp.bb.empty()) {
		string const bb = controller().readBB(igp.filename.absFilename());
		// the values from the file always have the bigpoint-unit bp
		dialog_->lbX->setText(toqstr(token(bb, ' ', 0)));
		dialog_->lbY->setText(toqstr(token(bb, ' ', 1)));
		dialog_->rtX->setText(toqstr(token(bb, ' ', 2)));
		dialog_->rtY->setText(toqstr(token(bb, ' ', 3)));
		dialog_->lbXunit->setCurrentIndex(0);
		dialog_->lbYunit->setCurrentIndex(0);
		dialog_->rtXunit->setCurrentIndex(0);
		dialog_->rtYunit->setCurrentIndex(0);
		controller().bbChanged = false;
	} else {
		// get the values from the inset
		Length anyLength;
		string const xl(token(igp.bb, ' ', 0));
		string const yl(token(igp.bb, ' ', 1));
		string const xr(token(igp.bb, ' ', 2));
		string const yr(token(igp.bb, ' ', 3));
		if (isValidLength(xl, &anyLength)) {
			dialog_->lbX->setText(toqstr(convert<string>(anyLength.value())));
			string const unit(unit_name[anyLength.unit()]);
			dialog_->lbXunit->setCurrentIndex(getItemNo(bb_units, unit));
		} else {
			dialog_->lbX->setText(toqstr(xl));
		}
		if (isValidLength(yl, &anyLength)) {
			dialog_->lbY->setText(toqstr(convert<string>(anyLength.value())));
			string const unit(unit_name[anyLength.unit()]);
			dialog_->lbYunit->setCurrentIndex(getItemNo(bb_units, unit));
		} else {
			dialog_->lbY->setText(toqstr(xl));
		}
		if (isValidLength(xr, &anyLength)) {
			dialog_->rtX->setText(toqstr(convert<string>(anyLength.value())));
			string const unit(unit_name[anyLength.unit()]);
			dialog_->rtXunit->setCurrentIndex(getItemNo(bb_units, unit));
		} else {
			dialog_->rtX->setText(toqstr(xl));
		}
		if (isValidLength(yr, &anyLength)) {
			dialog_->rtY->setText(toqstr(convert<string>(anyLength.value())));
			string const unit(unit_name[anyLength.unit()]);
			dialog_->rtYunit->setCurrentIndex(getItemNo(bb_units, unit));
		} else {
			dialog_->rtY->setText(toqstr(xl));
		}
		controller().bbChanged = true;
	}

	// Update the draft and clip mode
	dialog_->draftCB->setChecked(igp.draft);
	dialog_->clip->setChecked(igp.clip);
	dialog_->unzipCB->setChecked(igp.noUnzip);

	// Update the subcaption check button and input field
	dialog_->subfigure->setChecked(igp.subcaption);
	dialog_->subcaption->setText(toqstr(igp.subcaptionText));

	int item = 0;
	switch (igp.display) {
		case graphics::DefaultDisplay: item = 0; break;
		case graphics::MonochromeDisplay: item = 1; break;
		case graphics::GrayscaleDisplay: item = 2; break;
		case graphics::ColorDisplay: item = 3; break;
		case graphics::NoDisplay: item = 0; break;
	}
	dialog_->showCB->setCurrentIndex(item);
	dialog_->displayscale->setText(toqstr(convert<string>(igp.lyxscale)));
	dialog_->displayGB->setChecked(igp.display != graphics::NoDisplay);

	// the output section (width/height)

	dialog_->Scale->setText(toqstr(igp.scale));
	//igp.scale defaults to 100, so we treat it as empty
	bool const scaleChecked = !igp.scale.empty() && igp.scale != "100";
	dialog_->scaleCB->blockSignals(true);
	dialog_->scaleCB->setChecked(scaleChecked);
	dialog_->scaleCB->blockSignals(false);
	dialog_->Scale->setEnabled(scaleChecked);

	lengthAutoToWidgets(dialog_->Width, dialog_->widthUnit, igp.width,
		unitDefault);
	bool const widthChecked = !dialog_->Width->text().isEmpty() &&
		dialog_->Width->text() != "auto";
	dialog_->WidthCB->blockSignals(true);
	dialog_->WidthCB->setChecked(widthChecked);
	dialog_->WidthCB->blockSignals(false);
	dialog_->Width->setEnabled(widthChecked);
	dialog_->widthUnit->setEnabled(widthChecked);

	lengthAutoToWidgets(dialog_->Height, dialog_->heightUnit, igp.height,
		unitDefault);
	bool const heightChecked = !dialog_->Height->text().isEmpty()
		&& dialog_->Height->text() != "auto";
	dialog_->HeightCB->blockSignals(true);
	dialog_->HeightCB->setChecked(heightChecked);
	dialog_->HeightCB->blockSignals(false);
	dialog_->Height->setEnabled(heightChecked);
	dialog_->heightUnit->setEnabled(heightChecked);

	dialog_->scaleCB->setEnabled(!widthChecked && !heightChecked);
	dialog_->WidthCB->setEnabled(!scaleChecked);
	dialog_->HeightCB->setEnabled(!scaleChecked);
	dialog_->aspectratio->setEnabled(widthChecked && heightChecked);

	dialog_->setAutoText();

	dialog_->angle->setText(toqstr(igp.rotateAngle));
	dialog_->rotateOrderCB->setChecked(igp.scaleBeforeRotation);

	dialog_->rotateOrderCB->setEnabled((widthChecked ||
					   heightChecked ||
					   scaleChecked) &&
					   (igp.rotateAngle != "0"));

	dialog_->origin->clear();

	vector<RotationOriginPair> origindata = getRotationOriginData();
	vector<docstring> const origin_lang = getFirst(origindata);
	GuiGraphics::origin_ltx = getSecond(origindata);

	for (vector<docstring>::const_iterator it = origin_lang.begin();
	    it != origin_lang.end(); ++it)
		dialog_->origin->addItem(toqstr(*it));

	if (!igp.rotateOrigin.empty())
		dialog_->origin->setCurrentIndex(
			getItemNo(origin_ltx, igp.rotateOrigin));
	else
		dialog_->origin->setCurrentIndex(0);

	// disable edit button when no filename is present
	dialog_->editPB->setDisabled(dialog_->filename->text().isEmpty());

	//// latex section
	dialog_->latexoptions->setText(toqstr(igp.special));
}


void GuiGraphics::apply()
{
	InsetGraphicsParams & igp = controller().params();

	igp.filename.set(internal_path(fromqstr(dialog_->filename->text())),
			 kernel().bufferFilepath());

	// the bb section
	igp.bb.erase();
	if (controller().bbChanged) {
		string bb;
		string lbX(fromqstr(dialog_->lbX->text()));
		string lbY(fromqstr(dialog_->lbY->text()));
		string rtX(fromqstr(dialog_->rtX->text()));
		string rtY(fromqstr(dialog_->rtY->text()));
		int bb_sum =
			convert<int>(lbX) + convert<int>(lbY) +
			convert<int>(rtX) + convert<int>(rtX);
		if (bb_sum) {
			if (lbX.empty())
				bb = "0 ";
			else
				bb = lbX + fromqstr(dialog_->lbXunit->currentText()) + ' ';
			if (lbY.empty())
				bb += "0 ";
			else
				bb += (lbY + fromqstr(dialog_->lbYunit->currentText()) + ' ');
			if (rtX.empty())
				bb += "0 ";
			else
				bb += (rtX + fromqstr(dialog_->rtXunit->currentText()) + ' ');
			if (rtY.empty())
				bb += '0';
			else
				bb += (rtY + fromqstr(dialog_->rtYunit->currentText()));
			igp.bb = bb;
		}
	}

	igp.draft = dialog_->draftCB->isChecked();
	igp.clip = dialog_->clip->isChecked();
	igp.subcaption = dialog_->subfigure->isChecked();
	igp.subcaptionText = fromqstr(dialog_->subcaption->text());

	switch (dialog_->showCB->currentIndex()) {
		case 0: igp.display = graphics::DefaultDisplay; break;
		case 1: igp.display = graphics::MonochromeDisplay; break;
		case 2: igp.display = graphics::GrayscaleDisplay; break;
		case 3: igp.display = graphics::ColorDisplay; break;
		default:;
	}

	if (!dialog_->displayGB->isChecked())
		igp.display = graphics::NoDisplay;

	//the graphics section
	if (dialog_->scaleCB->isChecked()	&& !dialog_->Scale->text().isEmpty()) {
		igp.scale = fromqstr(dialog_->Scale->text());
		igp.width = Length("0pt");
		igp.height = Length("0pt");
		igp.keepAspectRatio = false;
	} else {
		igp.scale = string();
		igp.width = dialog_->WidthCB->isChecked() ?
			//Note that this works even if dialog_->Width is "auto", since in
			//that case we get "0pt".
			Length(widgetsToLength(dialog_->Width, dialog_->widthUnit)):
			Length("0pt");
		igp.height = dialog_->HeightCB->isChecked() ?
			Length(widgetsToLength(dialog_->Height, dialog_->heightUnit)) :
			Length("0pt");
		igp.keepAspectRatio = dialog_->aspectratio->isEnabled() &&
			dialog_->aspectratio->isChecked() &&
			igp.width.value() > 0 && igp.height.value() > 0;
	}

	igp.noUnzip = dialog_->unzipCB->isChecked();

	igp.lyxscale = convert<int>(fromqstr(dialog_->displayscale->text()));

	igp.rotateAngle = fromqstr(dialog_->angle->text());

	double rotAngle = convert<double>(igp.rotateAngle);
	if (std::abs(rotAngle) > 360.0) {
		rotAngle -= 360.0 * floor(rotAngle / 360.0);
		igp.rotateAngle = convert<string>(rotAngle);
	}

	// save the latex name for the origin. If it is the default
	// then origin_ltx returns ""
	igp.rotateOrigin =
		GuiGraphics::origin_ltx[dialog_->origin->currentIndex()];

	igp.scaleBeforeRotation = dialog_->rotateOrderCB->isChecked();

	// more latex options
	igp.special = fromqstr(dialog_->latexoptions->text());
}


void GuiGraphics::getBB()
{
	string const filename(fromqstr(dialog_->filename->text()));
	if (!filename.empty()) {
		string const bb(controller().readBB(filename));
		if (!bb.empty()) {
			dialog_->lbX->setText(toqstr(token(bb, ' ', 0)));
			dialog_->lbY->setText(toqstr(token(bb, ' ', 1)));
			dialog_->rtX->setText(toqstr(token(bb, ' ', 2)));
			dialog_->rtY->setText(toqstr(token(bb, ' ', 3)));
			// the default units for the bb values when reading
			// it from the file
			dialog_->lbXunit->setCurrentIndex(0);
			dialog_->lbYunit->setCurrentIndex(0);
			dialog_->rtXunit->setCurrentIndex(0);
			dialog_->rtYunit->setCurrentIndex(0);
		}
		controller().bbChanged = false;
	}
}


bool GuiGraphics::isValid()
{
	return !dialog_->filename->text().isEmpty();
}

} // namespace frontend
} // namespace lyx
