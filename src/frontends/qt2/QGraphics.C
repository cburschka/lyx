/**
 * \file QGraphics.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "QGraphicsDialog.h"
#include "ControlGraphics.h"
#include "QGraphics.h"
#include "Qt2BC.h"
#include "gettext.h"
#include "debug.h" 

#include "support/lstrings.h"
 
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qcombobox.h>

typedef Qt2CB<ControlGraphics, Qt2DB<QGraphicsDialog> > base_class;

QGraphics::QGraphics(ControlGraphics & c)
	: base_class(c, _("Graphics"))
{
}


void QGraphics::build_dialog()
{
	dialog_.reset(new QGraphicsDialog(this));

	bc().setOK(dialog_->okPB);
	bc().setApply(dialog_->applyPB);
	bc().setCancel(dialog_->closePB);
	bc().addReadOnly(dialog_->filenameED);
	bc().addReadOnly(dialog_->browsePB);
	bc().addReadOnly(dialog_->widthED);
	bc().addReadOnly(dialog_->widthCO);
	bc().addReadOnly(dialog_->heightED);
	bc().addReadOnly(dialog_->heightCO);
	bc().addReadOnly(dialog_->scaleCB);
	bc().addReadOnly(dialog_->rotateED);
	bc().addReadOnly(dialog_->monochromeRB);
	bc().addReadOnly(dialog_->grayscaleRB);
	bc().addReadOnly(dialog_->colorRB);
	bc().addReadOnly(dialog_->dontRB);
	bc().addReadOnly(dialog_->subcaptionED);
}


namespace {
	string const numtostr(double val) {
		string a(tostr(val));
		if (a == "0")
			a = "";
		return a;
	}
} // namespace anon
 
void QGraphics::update_contents()
{
	InsetGraphicsParams & igp = controller().params();

	dialog_->filenameED->setText(igp.filename.c_str());
 
	QRadioButton * button;
 
	switch (igp.display) {
		case InsetGraphicsParams::COLOR: button = dialog_->colorRB; break;
		case InsetGraphicsParams::GRAYSCALE: button = dialog_->grayscaleRB; break;
		case InsetGraphicsParams::MONOCHROME: button = dialog_->monochromeRB; break;
		case InsetGraphicsParams::NONE: button = dialog_->dontRB; break;
	}
	button->setChecked(true);

	int item = 0;
	switch (igp.widthResize) {
	    	case InsetGraphicsParams::INCH: item = 1; break;
	    	case InsetGraphicsParams::PERCENT_PAGE: item = 2; break;
	    	case InsetGraphicsParams::PERCENT_COLUMN: item = 3; break;
		default: break;
	}
 
	dialog_->widthCO->setCurrentItem(item);
 
	item = 0;
	switch (igp.heightResize) {
	    	case InsetGraphicsParams::INCH: item = 1; break;
	    	case InsetGraphicsParams::PERCENT_PAGE: item = 2; break;
		default: break;
	}
 
	dialog_->heightCO->setCurrentItem(item);
 
	// FIXME: scale ???
 
	dialog_->widthED->setText(numtostr(igp.widthSize).c_str());
	dialog_->heightED->setText(numtostr(igp.heightSize).c_str());
	dialog_->rotateED->setText(numtostr(igp.rotateAngle).c_str());
 
	dialog_->subcaptionED->setText(igp.subcaptionText.c_str());
}


void QGraphics::apply()
{
	InsetGraphicsParams & igp = controller().params();
 
	if (dialog_->colorRB->isChecked())
		igp.display = InsetGraphicsParams::COLOR;
	else if (dialog_->grayscaleRB->isChecked())
		igp.display = InsetGraphicsParams::GRAYSCALE;
	else if (dialog_->monochromeRB->isChecked())
		igp.display = InsetGraphicsParams::MONOCHROME;
	else
		igp.display = InsetGraphicsParams::NONE;
	 
	igp.subcaptionText = dialog_->subcaptionED->text().latin1();
	igp.subcaption = !igp.subcaptionText.empty();

	switch (dialog_->widthCO->currentItem()) {
		case 0: igp.widthResize = InsetGraphicsParams::CM; break;
		case 1: igp.widthResize = InsetGraphicsParams::INCH; break;
		case 2: igp.widthResize = InsetGraphicsParams::PERCENT_PAGE; break;
		case 3: igp.widthResize = InsetGraphicsParams::PERCENT_COLUMN; break;
		default:;
	}
	if (string(dialog_->widthED->text().latin1()).empty()) {
		igp.widthResize = InsetGraphicsParams::DEFAULT_SIZE;
		igp.widthSize = 0.0; 
	} else {
		igp.widthSize = strToDbl(dialog_->widthED->text().latin1());
	}
 
	switch (dialog_->heightCO->currentItem()) {
		case 0: igp.heightResize = InsetGraphicsParams::CM; break;
		case 1: igp.heightResize = InsetGraphicsParams::INCH; break;
		case 2: igp.heightResize = InsetGraphicsParams::PERCENT_PAGE; break;
		default:;
	}
	if (string(dialog_->heightED->text().latin1()).empty()) {
		igp.heightResize = InsetGraphicsParams::DEFAULT_SIZE;
		igp.heightSize = 0.0; 
	} else {
		igp.heightSize = strToDbl(dialog_->heightED->text().latin1());
	} 

	// FIXME: scale ??? 

	igp.rotateAngle = strToDbl(dialog_->rotateED->text().latin1());

	igp.filename = dialog_->filenameED->text().latin1();
}


void QGraphics::browse()
{
	string const & name = controller().Browse(dialog_->filenameED->text().latin1());
	if (!name.empty())
		dialog_->filenameED->setText(name.c_str()); 
}


bool QGraphics::isValid()
{
	return !string(dialog_->filenameED->text().latin1()).empty();
}
