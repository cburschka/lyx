/**
 * \file QIndex.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>

#include "QIndexDialog.h"
#include "QtLyXView.h" 
#include "BufferView.h"

#include "Dialogs.h"
#include "qt2BC.h"
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


QIndex::~QIndex()
{
}


void QIndex::build()
{
	dialog_.reset(new QIndexDialog(this));

	bc().setOK(dialog_->okPB);
	bc().setCancel(dialog_->cancelPB);
	bc().addReadOnly(dialog_->keywordED);
}

 
void QIndex::update()
{
	dialog_->keywordED->setText(controller().params().getContents().c_str());

	if (readonly) {
		dialog_->keywordED->setFocusPolicy(QWidget::NoFocus);
		dialog_->okPB->setEnabled(false);
		dialog_->cancelPB->setText(_("Close"));
	} else {
		dialog_->keywordED->setFocusPolicy(QWidget::StrongFocus);
		dialog_->keywordED->setFocus();
		dialog_->okPB->setEnabled(true);
		dialog_->cancelPB->setText(_("Cancel"));
	}
}

 
void QIndex::apply()
{
	if (readonly)
		return;

	controller().params().setContents(dialog_->keywordED->text().latin1());
}
