/**
 * \file QErrorList.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alfredo Braunstein
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "LyXView.h"
#include "qt_helpers.h"
#include "support/lstrings.h"
#include "debug.h"

#include "ControlErrorList.h"
#include "QErrorList.h"
#include "QErrorListDialog.h"
#include "Qt2BC.h"

#include <qlistbox.h>
#include <qtextbrowser.h>
#include <qpushbutton.h>

typedef QController<ControlErrorList, QView<QErrorListDialog> > base_class;

QErrorList::QErrorList(Dialog & parent)
	: base_class(parent, qt_("LyX: LaTeX error list"))
{}


void QErrorList::build_dialog()
{
	dialog_.reset(new QErrorListDialog(this));
	bcview().setCancel(dialog_->closePB);
}


void QErrorList::select(int item)
{
	controller().goTo(item);
	dialog_->descriptionTB->setText(toqstr(controller().ErrorList()[item].description));
}


void QErrorList::update_contents()
{
	string const caption = string(_("LyX: LaTex error List")) + '(' +
		controller().docName() + ')';

	dialog_->setCaption(qt_(caption));
	dialog_->errorsLB->clear();
	dialog_->descriptionTB->clear();

	std::vector<ControlErrorList::ErrorItem>::const_iterator 
		it = controller().ErrorList().begin();
	std::vector<ControlErrorList::ErrorItem>::const_iterator 
		end = controller().ErrorList().end();
	for(; it != end; ++it) {
		QListBoxItem * error = new QListBoxText(dialog_->errorsLB, 
							toqstr(it->error));
	}

	dialog_->errorsLB->setSelected(controller().currentItem(), true);
}

