/**
 * \file QGraphics.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Edwin Leuven
 * \author Herbert Voss
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "QtLyXView.h"
#include "ControlGraphics.h"

#include "support/lstrings.h"
#include "support/FileInfo.h"
#include "support/filetools.h"
#include "insets/insetgraphicsParams.h"
#include "lyxrc.h"
#include "lengthcombo.h"
#include "gettext.h"
#include "lengthcommon.h"
#include "lyxlength.h"
#include "debug.h"

#include <qlineedit.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qbuttongroup.h>
#include <qlabel.h>

#include "QGraphicsDialog.h"
#include "QGraphics.h"
#include "Qt2BC.h"

using std::vector;

typedef Qt2CB<ControlGraphics, Qt2DB<QGraphicsDialog> > base_class;


QGraphics::QGraphics()
	: base_class(_("Graphics"))
{
}


void QGraphics::build_dialog()
{
	dialog_.reset(new QGraphicsDialog(this));

	bc().setOK(dialog_->okPB);
	bc().setApply(dialog_->applyPB);
	bc().setRestore(dialog_->restorePB);
	bc().setCancel(dialog_->closePB);

	bc().addReadOnly(dialog_->rotateGB);
	bc().addReadOnly(dialog_->latexoptions);
	bc().addReadOnly(dialog_->subfigure);
	bc().addReadOnly(dialog_->subcaption);
	bc().addReadOnly(dialog_->filenameL);
	bc().addReadOnly(dialog_->filename);
	bc().addReadOnly(dialog_->browsePB);
	bc().addReadOnly(dialog_->unzipCB);
	bc().addReadOnly(dialog_->filename);
	bc().addReadOnly(dialog_->lbX);
	bc().addReadOnly(dialog_->lbY);
	bc().addReadOnly(dialog_->rtX);
	bc().addReadOnly(dialog_->rtY);
	bc().addReadOnly(dialog_->lbXunit);
	bc().addReadOnly(dialog_->lbYunit);
	bc().addReadOnly(dialog_->rtXunit);
	bc().addReadOnly(dialog_->rtYunit);
	bc().addReadOnly(dialog_->draftCB);
	bc().addReadOnly(dialog_->clip);
	bc().addReadOnly(dialog_->unzipCB);
	bc().addReadOnly(dialog_->subfigure);
	bc().addReadOnly(dialog_->subcaption);
	bc().addReadOnly(dialog_->showCB);
	bc().addReadOnly(dialog_->width);
	bc().addReadOnly(dialog_->height);
	bc().addReadOnly(dialog_->displayCB);
	bc().addReadOnly(dialog_->displayscale);
	bc().addReadOnly(dialog_->widthUnit);
	bc().addReadOnly(dialog_->heightUnit);
	bc().addReadOnly(dialog_->aspectratio);
	bc().addReadOnly(dialog_->angle);
	bc().addReadOnly(dialog_->origin);
	bc().addReadOnly(dialog_->latexoptions);
	bc().addReadOnly(dialog_->getPB);
}


namespace {
 
int getItemNo(vector<string> v, string const & s) {
	vector<string>::const_iterator cit =
		    find(v.begin(), v.end(), s);
	return (cit != v.end()) ? int(cit - v.begin()) : 0;
}
 
}


void QGraphics::update_contents()
{
	// clear and fill in the comboboxes
	vector<string> const bb_units = controller().getUnits();
	dialog_->lbXunit->clear();
	dialog_->lbYunit->clear();
	dialog_->rtXunit->clear();
	dialog_->rtYunit->clear();
	for (vector<string>::const_iterator it = bb_units.begin();
	    it != bb_units.end(); ++it) {
		dialog_->lbXunit->insertItem((*it).c_str(), -1);
		dialog_->lbYunit->insertItem((*it).c_str(), -1);
		dialog_->rtXunit->insertItem((*it).c_str(), -1);
		dialog_->rtYunit->insertItem((*it).c_str(), -1);
	}
	
	InsetGraphicsParams & igp = controller().params();

	// set the right default unit
	string unit = "cm";
	switch (lyxrc.default_papersize) {
		case BufferParams::PAPER_DEFAULT: break;

		case BufferParams::PAPER_USLETTER:
		case BufferParams::PAPER_LEGALPAPER:
		case BufferParams::PAPER_EXECUTIVEPAPER:
			unit = "in";
			break;

		case BufferParams::PAPER_A3PAPER:
		case BufferParams::PAPER_A4PAPER:
		case BufferParams::PAPER_A5PAPER:
		case BufferParams::PAPER_B5PAPER:
			unit = "cm";
			break;
	}

	dialog_->filename->setText(igp.filename.c_str());

	// set the bounding box values
	if (igp.bb.empty()) {
		controller().bbChanged = false;
		string const bb = controller().readBB(igp.filename);
		// the values from the file always have the point-unit
		dialog_->lbX->setText(token(bb, ' ', 0).c_str());
		dialog_->lbY->setText(token(bb, ' ', 1).c_str());
		dialog_->rtX->setText(token(bb, ' ', 2).c_str());
		dialog_->rtY->setText(token(bb, ' ', 3).c_str());
	} else {
		// get the values from the inset
		controller().bbChanged = true;
		LyXLength anyLength;
		string const xl(token(igp.bb,' ',0));
		string const yl(token(igp.bb,' ',1));
		string const xr(token(igp.bb,' ',2));
		string const yr(token(igp.bb,' ',3));
		if (isValidLength(xl, &anyLength)) {
			dialog_->lbX->setText(tostr(anyLength.value()).c_str());
			string const unit(unit_name[anyLength.unit()]);
			dialog_->lbXunit->setCurrentItem(getItemNo(bb_units, unit));
		} else {
			dialog_->lbX->setText(xl.c_str());
		}
		if (isValidLength(yl, &anyLength)) {
			dialog_->lbY->setText(tostr(anyLength.value()).c_str());
			string const unit(unit_name[anyLength.unit()]);
			dialog_->lbYunit->setCurrentItem(getItemNo(bb_units, unit));
		} else {
			dialog_->lbY->setText(xl.c_str());
		}
		if (isValidLength(xr, &anyLength)) {
			dialog_->rtX->setText(tostr(anyLength.value()).c_str());
			string const unit(unit_name[anyLength.unit()]);
			dialog_->rtXunit->setCurrentItem(getItemNo(bb_units, unit));
		} else {
			dialog_->rtX->setText(xl.c_str());
		}
		if (isValidLength(yr, &anyLength)) {
			dialog_->rtY->setText(tostr(anyLength.value()).c_str());
			string const unit(unit_name[anyLength.unit()]);
			dialog_->rtYunit->setCurrentItem(getItemNo(bb_units, unit));
		} else {
			dialog_->rtY->setText(xl.c_str());
		}
	}

	// Update the draft and clip mode
	dialog_->draftCB->setChecked(igp.draft);
	dialog_->clip->setChecked(igp.clip);
	dialog_->unzipCB->setChecked(igp.noUnzip);

	// Update the subcaption check button and input field
	dialog_->subfigure->setChecked(igp.subcaption);
	dialog_->subcaption->setText(igp.subcaptionText.c_str());

	int item = 0;
	switch (igp.display) {
		case grfx::DefaultDisplay: item = 0; break;
		case grfx::MonochromeDisplay: item = 1; break;
		case grfx::GrayscaleDisplay: item = 2; break;
		case grfx::ColorDisplay: item = 3; break;
		case grfx::NoDisplay: item = 0; break;
	}
	dialog_->showCB->setCurrentItem(item);
	dialog_->showCB->setEnabled(igp.display != grfx::NoDisplay && !readOnly());
	dialog_->displayCB->setChecked(igp.display != grfx::NoDisplay);
	dialog_->displayscale->setEnabled(igp.display != grfx::NoDisplay && !readOnly());
	dialog_->displayscale->setText(tostr(igp.lyxscale).c_str());

	dialog_->widthUnit->setCurrentItem(igp.width.unit());
	dialog_->heightUnit->setCurrentItem(igp.height.unit());
	dialog_->aspectratio->setChecked(igp.keepAspectRatio);

	// Update the rotate angle
	dialog_->angle->setText(tostr(igp.rotateAngle).c_str());

	if (igp.rotateOrigin.empty()) {
		dialog_->origin->setCurrentItem(0);
	} else {
		// FIXME fl_set_choice_text(special_->choice_origin,igp.rotateOrigin.c_str());
	}

	// latex options
	dialog_->latexoptions->setText(igp.special.c_str());
}


void QGraphics::apply()
{
	InsetGraphicsParams & igp = controller().params();

	igp.filename = dialog_->filename->text();

	// the bb section
	if (!controller().bbChanged) {
		// don't write anything		
		igp.bb.erase();
	} else {
		string bb;
		string lbX(dialog_->lbX->text());
		string lbY(dialog_->lbY->text());
		string rtX(dialog_->rtX->text());
		string rtY(dialog_->rtY->text());

		if (lbX.empty())
			bb = "0 ";
		else
			bb = lbX + dialog_->lbXunit->currentText().latin1() + ' ';
		if (lbY.empty())
			bb += "0 ";
		else 
			bb += (lbY + dialog_->lbYunit->currentText().latin1() + ' ');
		if (rtX.empty())
			bb += "0 ";
		else 
			bb += (rtX + dialog_->rtXunit->currentText().latin1() + ' ');
		if (rtY.empty())
			bb += "0";
		else 
			bb += (rtY + dialog_->rtYunit->currentText().latin1());
		igp.bb = bb;
	}

	igp.draft = dialog_->draftCB->isChecked();
	igp.clip = dialog_->clip->isChecked();
	igp.subcaption = dialog_->subfigure->isChecked();
	igp.subcaptionText = dialog_->subcaption->text();

	switch (dialog_->showCB->currentItem()) {
		case 0: igp.display = grfx::DefaultDisplay; break;
		case 1: igp.display = grfx::MonochromeDisplay; break;
		case 2: igp.display = grfx::GrayscaleDisplay; break;
		case 3: igp.display = grfx::ColorDisplay; break;
		default:;
	}

	if (!dialog_->displayCB->isChecked())
		igp.display = grfx::NoDisplay;

	string value(dialog_->width->text());
	igp.width = LyXLength(strToDbl(value), dialog_->widthUnit->currentLengthItem());
	value = string(dialog_->height->text());
	igp.height = LyXLength(strToDbl(value), dialog_->heightUnit->currentLengthItem());

	igp.keepAspectRatio = dialog_->aspectratio->isChecked();

	igp.noUnzip = dialog_->unzipCB->isChecked();

	igp.lyxscale = strToInt(string(dialog_->displayscale->text()));

	igp.rotateAngle = strToDbl(string(dialog_->angle->text()));
	while (igp.rotateAngle < -360.0)
		igp.rotateAngle += 360.0;
	while (igp.rotateAngle >  360.0)
		igp.rotateAngle -= 360.0;

	if ((dialog_->origin->currentItem()) > 0)
		igp.rotateOrigin = dialog_->origin->currentText();
	else
	    igp.rotateOrigin.erase();

	igp.special = dialog_->latexoptions->text();
}


void QGraphics::browse()
{
	string const & name = controller().Browse(dialog_->filename->text().latin1());
	if (!name.empty())
		dialog_->filename->setText(name.c_str());
}


void QGraphics::get()
{
	string const filename(dialog_->filename->text());
	if (!filename.empty()) {
		string const bb(controller().readBB(filename));
		if (!bb.empty()) {
			dialog_->lbX->setText(token(bb, ' ', 0).c_str());
			dialog_->lbY->setText(token(bb, ' ', 1).c_str());
			dialog_->rtX->setText(token(bb, ' ', 2).c_str());
			dialog_->rtY->setText(token(bb, ' ', 3).c_str());
			// the default units for the bb values when reading
			// it from the file
			dialog_->lbXunit->setCurrentItem(0);
			dialog_->lbYunit->setCurrentItem(0);
			dialog_->rtXunit->setCurrentItem(0);
			dialog_->rtYunit->setCurrentItem(0);
		}
		controller().bbChanged = false;
	}
}


bool QGraphics::isValid()
{
	// FIXME: we need more here.
	// why?? LaTeX needs a filename, the rest is user-specific  (Herbert)
	return !string(dialog_->filename->text().latin1()).empty();
}
