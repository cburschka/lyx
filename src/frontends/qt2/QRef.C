/**
 * \file QRef.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 */

#include <config.h>

#include "support/lstrings.h"
 
#include "QRefDialog.h"
#include "ControlRef.h"
#include "QRef.h"
#include "Qt2BC.h" 
#include "gettext.h"
#include "insets/insetref.h"

#include <qlineedit.h>
#include <qcheckbox.h>
#include <qlistbox.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qtooltip.h>

using std::endl;

typedef Qt2CB<ControlRef, Qt2DB<QRefDialog> > base_class;
 
QRef::QRef(ControlRef & c)
	: base_class(c, _("Cross Reference")),
	sort_(false), at_ref_(false)
{
}


void QRef::build_dialog()
{
	dialog_.reset(new QRefDialog(this));

	bc().setOK(dialog_->okPB); 
	bc().setCancel(dialog_->closePB); 
	bc().addReadOnly(dialog_->refsLB);
	bc().addReadOnly(dialog_->sortCB);
	bc().addReadOnly(dialog_->nameED);
	bc().addReadOnly(dialog_->referenceED);
	bc().addReadOnly(dialog_->typeCO);
}

 
void QRef::update_contents()
{
	dialog_->referenceED->setText(controller().params().getContents().c_str());
	dialog_->nameED->setText(controller().params().getOptions().c_str());

	dialog_->typeCO->setCurrentItem(InsetRef::getType(controller().params().getCmdName()));

	dialog_->nameED->setReadOnly(!nameAllowed() && !readOnly());

	dialog_->typeCO->setEnabled(!typeAllowed() && !readOnly());
	if (!typeAllowed())
		dialog_->typeCO->setCurrentItem(0);

	dialog_->sortCB->setChecked(sort_);

	updateRefs();
}


void QRef::apply()
{
	controller().params().setCmdName(InsetRef::getName(dialog_->typeCO->currentItem()));
	controller().params().setContents(dialog_->referenceED->text().latin1());
	controller().params().setOptions(dialog_->nameED->text().latin1());
}


bool QRef::nameAllowed()
{
	return controller().docType() != ControlRef::LATEX &&
		controller().docType() != ControlRef::LITERATE;
}

 
bool QRef::typeAllowed()
{
	return controller().docType() == ControlRef::LINUXDOC ||
	    controller().docType() == ControlRef::DOCBOOK;
}

 
void QRef::setGoBack()
{
	dialog_->gotoPB->setText(_("&Go back"));
	QToolTip::remove(dialog_->gotoPB);
	QToolTip::add(dialog_->gotoPB, _("Go back"));
}

 
void QRef::setGotoRef()
{
	dialog_->gotoPB->setText(_("&Goto"));
	QToolTip::remove(dialog_->gotoPB);
	QToolTip::add(dialog_->gotoPB, _("Go to reference"));
}

 
void QRef::gotoRef()
{
	string ref(dialog_->referenceED->text());

	if (at_ref_) {
		// go back
		setGotoRef();
		controller().gotoBookmark();
	} else {
		// go to the ref
		setGoBack();
		controller().gotoRef(ref);
	}
	at_ref_ = !at_ref_;
}

 
void QRef::redoRefs()
{
	dialog_->refsLB->setAutoUpdate(false);

	// need this because Qt will send a highlight() here for
	// the first item inserted
	string tmp(dialog_->referenceED->text());

	for (std::vector<string>::const_iterator iter = refs_.begin();
		iter != refs_.end(); ++iter) {
		if (sort_)
			dialog_->refsLB->inSort(iter->c_str());
		else
			dialog_->refsLB->insertItem(iter->c_str());
	}

	dialog_->referenceED->setText(tmp.c_str());

	for (unsigned int i = 0; i < dialog_->refsLB->count(); ++i) {
		if (!compare(tmp.c_str(), dialog_->refsLB->text(i).latin1()))
			dialog_->refsLB->setCurrentItem(i);
	}

	dialog_->refsLB->setAutoUpdate(true);
	dialog_->refsLB->update();
}


void QRef::updateRefs()
{
	refs_.clear();
	if (at_ref_)
		gotoRef();
	dialog_->refsLB->clear();
	refs_ = controller().getLabelList(string());
	dialog_->sortCB->setEnabled(!refs_.empty());
	dialog_->refsLB->setEnabled(!refs_.empty()); 
	redoRefs();
}
