/**
 * \file QGraphics.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 * \author Edwin Leuven <leuven@fee.uva.nl>
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

	// FIXME: and the rest ?
	bc().addReadOnly(dialog_->rotateGB);
	bc().addReadOnly(dialog_->latexoptionsGB);
	bc().addReadOnly(dialog_->bbGB);
	bc().addReadOnly(dialog_->subfigure);
	bc().addReadOnly(dialog_->subcaption);
	bc().addReadOnly(dialog_->filenameL);
	bc().addReadOnly(dialog_->filename);
	bc().addReadOnly(dialog_->browsePB);
}

void QGraphics::update_contents()
{
	InsetGraphicsParams & igp = controller().params();

	// set the right default unit
	string unit = "cm";
	switch (lyxrc.default_papersize) {
		case BufferParams::PAPER_DEFAULT: break;
	
		case BufferParams::PAPER_USLETTER:
		case BufferParams::PAPER_LEGALPAPER:
		case BufferParams::PAPER_EXECUTIVEPAPER: unit = "in"; break;
	
		case BufferParams::PAPER_A3PAPER:
		case BufferParams::PAPER_A4PAPER:
		case BufferParams::PAPER_A5PAPER:
		case BufferParams::PAPER_B5PAPER: unit = "cm"; break;
	}
	// ?? defaultUnit is not used !!
	string const defaultUnit = string(unit);

	// Update dialog with details from inset
	dialog_->filename->setText(igp.filename.c_str());

	// set the bounding box values, if exists. First we need the whole
	// path, because the controller knows nothing about the doc-dir
	controller().bbChanged = false;
	if (igp.bb.empty()) {
		string const fileWithAbsPath(MakeAbsPath(igp.filename, OnlyPath(igp.filename)));
		string const bb(controller().readBB(fileWithAbsPath));
		if (!bb.empty()) {
			// get the values from the file
			// in this case we always have the point-unit
			dialog_->lbX->setText(token(bb, ' ', 0).c_str());
			dialog_->lbY->setText(token(bb, ' ', 1).c_str());
			dialog_->rtX->setText(token(bb, ' ', 2).c_str());
			dialog_->rtY->setText(token(bb, ' ', 3).c_str());
		}
	} else {
		// get the values from the inset
		controller().bbChanged = true;
		dialog_->lbX->setText(token(igp.bb, ' ', 0).c_str());
		dialog_->lbY->setText(token(igp.bb, ' ', 1).c_str());
		dialog_->rtX->setText(token(igp.bb, ' ', 2).c_str());
		dialog_->rtY->setText(token(igp.bb, ' ', 3).c_str());
	}

	// Update the draft and clip mode
	dialog_->draft->setChecked(igp.draft);
	dialog_->clip->setChecked(igp.clip);

	// Update the subcaption check button and input field
	dialog_->subfigure->setChecked(igp.subcaption);
	dialog_->subcaption->setText(igp.subcaptionText.c_str());

	int item;
	switch (igp.display) {
		case grfx::DefaultDisplay: item = 0; break;
		case grfx::MonochromeDisplay: item = 1; break;
		case grfx::GrayscaleDisplay: item = 2; break;
		case grfx::ColorDisplay: item = 3; break;
		case grfx::NoDisplay: item = 4; break;
	}
	dialog_->show->setCurrentItem(item);

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

	if (!controller().bbChanged) {
		igp.bb = string();
	} else {
		string bb;
		string lbX(dialog_->lbX->text());
		string lbY(dialog_->lbY->text());
		string rtX(dialog_->rtX->text());
		string rtY(dialog_->rtY->text());

		if (lbX.empty())
			bb = "0 ";
		else
			bb = lbX + " ";
		if (lbY.empty())
			bb += "0 ";
		else
			bb += (lbY + " ");
		if (rtX.empty())
			bb += "0 ";
		else
			bb += (rtX + " ");
		if (rtY.empty())
			bb += "0 ";
		else
			bb += (rtY + " ");

		igp.bb = bb;
	}

	igp.draft = dialog_->draft->isChecked();
	igp.clip = dialog_->clip->isChecked();
	igp.subcaption = dialog_->subfigure->isChecked();
	igp.subcaptionText = dialog_->subcaption->text();

	switch (dialog_->show->currentItem()) {
		case 0: igp.display = grfx::DefaultDisplay; break;
		case 1: igp.display = grfx::MonochromeDisplay; break;
		case 2: igp.display = grfx::GrayscaleDisplay; break;
		case 3: igp.display = grfx::ColorDisplay; break;
		case 4: igp.display = grfx::NoDisplay; break;
		default:;
	}

	string value(dialog_->width->text());
	igp.width = LyXLength(strToDbl(value), dialog_->widthUnit->currentLengthItem());
	value = string(dialog_->height->text());
	igp.height = LyXLength(strToDbl(value), dialog_->heightUnit->currentLengthItem());

	igp.keepAspectRatio = dialog_->aspectratio->isChecked();
 
	igp.lyxscale = strToInt(string(dialog_->displayscale->text()));

	igp.rotateAngle = strToDbl(string(dialog_->angle->text()));
	while (igp.rotateAngle < -360.0) igp.rotateAngle += 360.0;
	while (igp.rotateAngle >  360.0) igp.rotateAngle -= 360.0;

	if ((dialog_->origin->currentItem()) > 0)
		igp.rotateOrigin = dialog_->origin->currentText();
	else
	    igp.rotateOrigin = string();

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
		string const fileWithAbsPath(MakeAbsPath(filename, OnlyPath(filename)));
		string const bb(controller().readBB(fileWithAbsPath));
		if (!bb.empty()) {
			dialog_->lbX->setText(token(bb, ' ', 0).c_str());
			dialog_->lbY->setText(token(bb, ' ', 1).c_str());
			dialog_->rtX->setText(token(bb, ' ', 2).c_str());
			dialog_->rtY->setText(token(bb, ' ', 3).c_str());
		}
		controller().bbChanged = false;
	}
}


bool QGraphics::isValid()
{
	// FIXME: we need more here. 
	return !string(dialog_->filename->text().latin1()).empty();
}
