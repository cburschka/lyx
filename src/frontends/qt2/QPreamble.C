/**
 * \file QPreamble.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>
#include <fstream> 

#ifdef __GNUG__
#pragma implementation
#endif

#include <qmultilineedit.h>
#include <qpushbutton.h>
 
#include "QPreambleDialog.h"
#include "QPreamble.h"
#include "Qt2BC.h"
#include "gettext.h"
 
#include "QtLyXView.h"
#include "ControlPreamble.h"

typedef Qt2CB<ControlPreamble, Qt2DB<QPreambleDialog> > base_class;

QPreamble::QPreamble(ControlPreamble & c)
	: base_class(c, _("LaTeX Preamble"))
{
}


void QPreamble::build_dialog()
{
	dialog_.reset(new QPreambleDialog(this));

	bc().setOK(dialog_->okPB);
	bc().setApply(dialog_->applyPB);
	bc().setCancel(dialog_->closePB);
}


void QPreamble::update_contents()
{
	dialog_->preambleLE->setText(controller().params().c_str());
}


void QPreamble::apply()
{
	controller().params() = dialog_->preambleLE->text().latin1();
}
