/**
 * \file QGraphics.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Edwin Leuven
 * \author Herbert Voﬂ
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlGraphics.h"
#include "controllers/helper_funcs.h"
#include "support/lstrings.h"
#include "support/tostr.h"
#include "support/lyxlib.h"
#include "insets/insetgraphicsParams.h"

#include "lyxrc.h"
#include "lengthcombo.h"
#include "qt_helpers.h"
#include "lengthcommon.h"

#include <qlineedit.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlabel.h>

#include "QGraphicsDialog.h"
#include "QGraphics.h"
#include "Qt2BC.h"

using lyx::support::float_equal;
using lyx::support::strToDbl;
using lyx::support::strToInt;
using lyx::support::token;

using std::vector;


typedef QController<ControlGraphics, QView<QGraphicsDialog> > base_class;

QGraphics::QGraphics(Dialog & parent)
	: base_class(parent, _("LyX: Graphics"))
{
}


void QGraphics::build_dialog()
{
	dialog_.reset(new QGraphicsDialog(this));

	bcview().setOK(dialog_->okPB);
	bcview().setApply(dialog_->applyPB);
	bcview().setRestore(dialog_->restorePB);
	bcview().setCancel(dialog_->closePB);

	bcview().addReadOnly(dialog_->rotateGB);
	bcview().addReadOnly(dialog_->latexoptions);
	bcview().addReadOnly(dialog_->subfigure);
	bcview().addReadOnly(dialog_->subcaption);
	bcview().addReadOnly(dialog_->filenameL);
	bcview().addReadOnly(dialog_->filename);
	bcview().addReadOnly(dialog_->browsePB);
	bcview().addReadOnly(dialog_->unzipCB);
	bcview().addReadOnly(dialog_->filename);
	bcview().addReadOnly(dialog_->lbX);
	bcview().addReadOnly(dialog_->lbY);
	bcview().addReadOnly(dialog_->rtX);
	bcview().addReadOnly(dialog_->rtY);
	bcview().addReadOnly(dialog_->lbXunit);
	bcview().addReadOnly(dialog_->lbYunit);
	bcview().addReadOnly(dialog_->rtXunit);
	bcview().addReadOnly(dialog_->rtYunit);
	bcview().addReadOnly(dialog_->draftCB);
	bcview().addReadOnly(dialog_->clip);
	bcview().addReadOnly(dialog_->unzipCB);
	bcview().addReadOnly(dialog_->subfigure);
	bcview().addReadOnly(dialog_->subcaption);
	bcview().addReadOnly(dialog_->showCB);
	bcview().addReadOnly(dialog_->width);
	bcview().addReadOnly(dialog_->height);
	bcview().addReadOnly(dialog_->displayCB);
	bcview().addReadOnly(dialog_->displayscale);
	bcview().addReadOnly(dialog_->widthUnit);
	bcview().addReadOnly(dialog_->heightUnit);
	bcview().addReadOnly(dialog_->aspectratio);
	bcview().addReadOnly(dialog_->angle);
	bcview().addReadOnly(dialog_->origin);
	bcview().addReadOnly(dialog_->latexoptions);
	bcview().addReadOnly(dialog_->getPB);
}


namespace {

// returns the number of the string s in the vector v
int getItemNo(vector<string> v, string const & s) {
	vector<string>::const_iterator cit =
		    find(v.begin(), v.end(), s);
	return (cit != v.end()) ? int(cit - v.begin()) : 0;
}

// returns the number of the unit in the array unit_name,
// which is defined in lengthcommon.C
int getUnitNo(char const * c[], string const & s) {
	int i = 0;
	while (i < num_units && s != c[i])
		++i;
	return (i < num_units) ? i : 0;
}

}


void QGraphics::update_contents()
{
	// clear and fill in the comboboxes
	vector<string> const bb_units = frnt::getBBUnits();
	dialog_->lbXunit->clear();
	dialog_->lbYunit->clear();
	dialog_->rtXunit->clear();
	dialog_->rtYunit->clear();
	for (vector<string>::const_iterator it = bb_units.begin();
	    it != bb_units.end(); ++it) {
		dialog_->lbXunit->insertItem(toqstr(*it), -1);
		dialog_->lbYunit->insertItem(toqstr(*it), -1);
		dialog_->rtXunit->insertItem(toqstr(*it), -1);
		dialog_->rtYunit->insertItem(toqstr(*it), -1);
	}

	InsetGraphicsParams & igp = controller().params();

	// set the right default unit
	LyXLength::UNIT unitDefault = LyXLength::CM;
	switch (lyxrc.default_papersize) {
		case PAPER_DEFAULT: break;

		case PAPER_USLETTER:
		case PAPER_LEGALPAPER:
		case PAPER_EXECUTIVEPAPER:
			unitDefault = LyXLength::IN;
			break;

		case PAPER_A3PAPER:
		case PAPER_A4PAPER:
		case PAPER_A5PAPER:
		case PAPER_B5PAPER:
			unitDefault = LyXLength::CM;
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
		dialog_->lbXunit->setCurrentItem(0);
		dialog_->lbYunit->setCurrentItem(0);
		dialog_->rtXunit->setCurrentItem(0);
		dialog_->rtYunit->setCurrentItem(0);
		controller().bbChanged = false;
	} else {
		// get the values from the inset
		LyXLength anyLength;
		string const xl(token(igp.bb, ' ', 0));
		string const yl(token(igp.bb, ' ', 1));
		string const xr(token(igp.bb, ' ', 2));
		string const yr(token(igp.bb, ' ', 3));
		if (isValidLength(xl, &anyLength)) {
			dialog_->lbX->setText(toqstr(tostr(anyLength.value())));
			string const unit(unit_name[anyLength.unit()]);
			dialog_->lbXunit->setCurrentItem(getItemNo(bb_units, unit));
		} else {
			dialog_->lbX->setText(toqstr(xl));
		}
		if (isValidLength(yl, &anyLength)) {
			dialog_->lbY->setText(toqstr(tostr(anyLength.value())));
			string const unit(unit_name[anyLength.unit()]);
			dialog_->lbYunit->setCurrentItem(getItemNo(bb_units, unit));
		} else {
			dialog_->lbY->setText(toqstr(xl));
		}
		if (isValidLength(xr, &anyLength)) {
			dialog_->rtX->setText(toqstr(tostr(anyLength.value())));
			string const unit(unit_name[anyLength.unit()]);
			dialog_->rtXunit->setCurrentItem(getItemNo(bb_units, unit));
		} else {
			dialog_->rtX->setText(toqstr(xl));
		}
		if (isValidLength(yr, &anyLength)) {
			dialog_->rtY->setText(toqstr(tostr(anyLength.value())));
			string const unit(unit_name[anyLength.unit()]);
			dialog_->rtYunit->setCurrentItem(getItemNo(bb_units, unit));
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
		case lyx::graphics::DefaultDisplay: item = 0; break;
		case lyx::graphics::MonochromeDisplay: item = 1; break;
		case lyx::graphics::GrayscaleDisplay: item = 2; break;
		case lyx::graphics::ColorDisplay: item = 3; break;
		case lyx::graphics::NoDisplay: item = 0; break;
	}
	dialog_->showCB->setCurrentItem(item);
	dialog_->showCB->setEnabled(igp.display != lyx::graphics::NoDisplay && !readOnly());
	dialog_->displayCB->setChecked(igp.display != lyx::graphics::NoDisplay);
	dialog_->displayscale->setEnabled(igp.display != lyx::graphics::NoDisplay && !readOnly());
	dialog_->displayscale->setText(toqstr(tostr(igp.lyxscale)));

	//// the output section (width/height)
	// set the length combo boxes
	// only the width has the possibility for scale%. The original
	// units are defined in lengthcommon.C
	// 1. the width (a listttype)
	dialog_->widthUnit->clear();
	dialog_->widthUnit->insertItem(qt_("Scale%"));
	for (int i = 0; i < num_units; i++)
		dialog_->widthUnit->insertItem(unit_name_gui[i], -1);

	if (!float_equal(igp.scale, 0.0, 0.05)) {
		// there is a scale value > 0.05
		dialog_->width->setText(toqstr(tostr(igp.scale)));
		dialog_->widthUnit->setCurrentItem(0);
	} else {
		// no scale means default width/height
		dialog_->width->setText(toqstr(tostr(igp.width.value())));
		// the width cannot have a unitDefault, because
		// it is a "Scale%" or another user defined unit!
		// +1 instead of the "Scale%" option
		int unit_ = igp.width.unit();
		dialog_->widthUnit->setCurrentItem(unit_ + 1);
	}
	// 2. the height (a lengthgcombo type)
	dialog_->height->setText(toqstr(tostr(igp.height.value())));
	LyXLength::UNIT unit_ = (igp.height.value() > 0.0) ?
		igp.height.unit() : unitDefault;
	dialog_->heightUnit->setCurrentItem(unit_);

	// enable height input in case of non "Scale%" as width-unit
	bool use_height = (dialog_->widthUnit->currentItem() > 0);
	dialog_->height->setEnabled(use_height);
	dialog_->heightUnit->setEnabled(use_height);

	dialog_->aspectratio->setChecked(igp.keepAspectRatio);

	dialog_->angle->setText(toqstr(tostr(igp.rotateAngle)));

	dialog_->origin->clear();

	using namespace frnt;
	vector<RotationOriginPair> origindata = getRotationOriginData();
	vector<string> const origin_lang = getFirst(origindata);
	QGraphics::origin_ltx = getSecond(origindata);

	for (vector<string>::const_iterator it = origin_lang.begin();
	    it != origin_lang.end(); ++it)
		dialog_->origin->insertItem(toqstr(*it), -1);

	if (!igp.rotateOrigin.empty())
		dialog_->origin->setCurrentItem(
			::getItemNo(origin_ltx, igp.rotateOrigin));
	else
		dialog_->origin->setCurrentItem(0);

	//// latex section
	dialog_->latexoptions->setText(toqstr(igp.special));
}


void QGraphics::apply()
{
	InsetGraphicsParams & igp = controller().params();

	igp.filename.set(fromqstr(dialog_->filename->text()),
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
			strToInt(lbX) + strToInt(lbY) +
			strToInt(rtX) + strToInt(rtX);
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

	switch (dialog_->showCB->currentItem()) {
		case 0: igp.display = lyx::graphics::DefaultDisplay; break;
		case 1: igp.display = lyx::graphics::MonochromeDisplay; break;
		case 2: igp.display = lyx::graphics::GrayscaleDisplay; break;
		case 3: igp.display = lyx::graphics::ColorDisplay; break;
		default:;
	}

	if (!dialog_->displayCB->isChecked())
		igp.display = lyx::graphics::NoDisplay;

	string value(fromqstr(dialog_->width->text()));
	if (dialog_->widthUnit->currentItem() > 0) {
		// width/height combination
		int const unitNo = getUnitNo(unit_name_gui,
			fromqstr(dialog_->widthUnit->currentText()));
		igp.width = LyXLength(value + unit_name_ltx[unitNo]);
		igp.scale = 0.0;
	} else {
		// scaling instead of a width
		igp.scale = strToDbl(value);
		igp.width = LyXLength();
	}
	value = fromqstr(dialog_->height->text());
	int const unitNo = getUnitNo(unit_name_gui,
		fromqstr(dialog_->heightUnit->currentText()));
	igp.height = LyXLength(value + unit_name_ltx[unitNo]);

	igp.keepAspectRatio = dialog_->aspectratio->isChecked();

	igp.noUnzip = dialog_->unzipCB->isChecked();

	igp.lyxscale = strToInt(fromqstr(dialog_->displayscale->text()));

	igp.rotateAngle = strToDbl(fromqstr(dialog_->angle->text()));
	while (igp.rotateAngle < -360.0)
		igp.rotateAngle += 360.0;
	while (igp.rotateAngle >  360.0)
		igp.rotateAngle -= 360.0;

	// save the latex name for the origin. If it is the default
	// then origin_ltx returns ""
	igp.rotateOrigin =
		QGraphics::origin_ltx[dialog_->origin->currentItem()];

	// more latex options
	igp.special = fromqstr(dialog_->latexoptions->text());
}


void QGraphics::getBB()
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
	return !dialog_->filename->text().isEmpty();
}
