/**
 * \file QIndex.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>

#include "QIndexDialog.h"
#include "ControlIndex.h"
#include "QtLyXView.h" 
#include "BufferView.h"

#include "Dialogs.h"
#include "Qt2BC.h"
#include "QIndex.h"
#include "gettext.h"
#include "buffer.h"
#include "lyxfunc.h" 

#include <qlineedit.h>
#include <qpushbutton.h>

typedef Qt2CB<ControlIndex, Qt2DB<QIndexDialog> > base_class;
 
QIndex::QIndex(ControlIndex & c)
	: base_class(c, _("Index"))
{
}


void QIndex::build()
{
	dialog_.reset(new QIndexDialog(this));

	bc().setOK(dialog_->okPB);
	bc().setCancel(dialog_->closePB);
	bc().addReadOnly(dialog_->keywordED);
}

 
void QIndex::update()
{
	dialog_->keywordED->setText(controller().params().getContents().c_str());
	reset();
}

 
void QIndex::apply()
{
	controller().params().setContents(dialog_->keywordED->text().latin1());
}
