/**
 * \file FormIndex.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>

#include "QtLyXView.h"
#include "FormIndex.h"
#include "indexdlg.h" 
#include "ControlIndex.h" 
#include "gettext.h"

FormIndex::FormIndex(ControlIndex & c)
	: KFormBase<ControlIndex, IndexDialog>(c) 
{
}


void FormIndex::build()
{
	dialog_.reset(new IndexDialog(this, 0, _("LyX: Index")));

	// FIXME
 
	//bc().setUndoAll(dialog_->buttonRestore);
	bc().setOK(dialog_->button_ok);
	//bc().setApply(dialog_->buttonApply); 
	bc().setCancel(dialog_->button_cancel);
	bc().addReadOnly(dialog_->line_index);
}
 

void FormIndex::update()
{
	dialog_->setIndexText(controller().params().getContents().c_str());
}

 
void FormIndex::apply()
{
	controller().params().setContents(dialog_->getIndexText());
}
