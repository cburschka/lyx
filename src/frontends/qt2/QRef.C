/**
 * \file QRef.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>


#include "ControlRef.h"
#include "QRef.h"
#include "QRefDialog.h"
#include "Qt2BC.h"

#include <qlineedit.h>
#include <qcheckbox.h>
#include <qlistbox.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qtooltip.h>

#include "helper_funcs.h" // getStringFromVector
#include "support/lstrings.h" // frontStrip, strip
#include "qt_helpers.h"
#include "insets/insetref.h"

using std::find;
using std::max;
using std::sort;
using std::vector;
using std::endl;


typedef QController<ControlRef, QView<QRefDialog> > base_class;


QRef::QRef(Dialog & parent)
	: base_class(parent, qt_("LyX: Insert Cross-reference")),
	sort_(false), at_ref_(false)
{
}


void QRef::build_dialog()
{
	dialog_.reset(new QRefDialog(this));

	bcview().setOK(dialog_->okPB);
	bcview().setApply(dialog_->applyPB);
	bcview().setCancel(dialog_->closePB);
	bcview().addReadOnly(dialog_->refsLB);
	bcview().addReadOnly(dialog_->sortCB);
	bcview().addReadOnly(dialog_->nameED);
	bcview().addReadOnly(dialog_->referenceED);
	bcview().addReadOnly(dialog_->typeCO);
	bcview().addReadOnly(dialog_->bufferCO);
}


void QRef::update_contents()
{
	InsetCommandParams const & params = controller().params();

	dialog_->referenceED->setText(toqstr(params.getContents()));

	dialog_->nameED->setText(toqstr(params.getOptions()));
	dialog_->nameED->setReadOnly(!nameAllowed() && !readOnly());

	dialog_->typeCO->setCurrentItem(InsetRef::getType(params.getCmdName()));
	dialog_->typeCO->setEnabled(typeAllowed() && !readOnly());
	if (!typeAllowed())
		dialog_->typeCO->setCurrentItem(0);

	dialog_->sortCB->setChecked(sort_);

	// insert buffer list
	dialog_->bufferCO->clear();
	vector<string> const buffers = controller().getBufferList();
	for (vector<string>::const_iterator it = buffers.begin();
		it != buffers.end(); ++it) {
		dialog_->bufferCO->insertItem(toqstr(*it));
	}
	dialog_->bufferCO->setCurrentItem(controller().getBufferNum());

	updateRefs();
}


void QRef::apply()
{
	InsetCommandParams & params = controller().params();

	params.setCmdName(InsetRef::getName(dialog_->typeCO->currentItem()));
	params.setContents(fromqstr(dialog_->referenceED->text()));
	params.setOptions(fromqstr(dialog_->nameED->text()));
}


bool QRef::nameAllowed()
{
	Kernel::DocTypes doc_type = kernel().docType();
	return doc_type != Kernel::LATEX &&
		doc_type != Kernel::LITERATE;
}


bool QRef::typeAllowed()
{
	Kernel::DocTypes doc_type = kernel().docType();
	return doc_type != Kernel::LINUXDOC &&
		doc_type != Kernel::DOCBOOK;
}


void QRef::setGoBack()
{
	dialog_->gotoPB->setText(qt_("&Go Back"));
	QToolTip::remove(dialog_->gotoPB);
	QToolTip::add(dialog_->gotoPB, qt_("Jump back"));
}


void QRef::setGotoRef()
{
	dialog_->gotoPB->setText(qt_("&Go to Reference"));
	QToolTip::remove(dialog_->gotoPB);
	QToolTip::add(dialog_->gotoPB, qt_("Jump to reference"));
}


void QRef::gotoRef()
{
	string ref(fromqstr(dialog_->referenceED->text()));

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
	QString const tmp(dialog_->referenceED->text());

	for (std::vector<string>::const_iterator iter = refs_.begin();
		iter != refs_.end(); ++iter) {
		if (sort_)
			dialog_->refsLB->inSort(toqstr(*iter));
		else
			dialog_->refsLB->insertItem(toqstr(*iter));
	}

	dialog_->referenceED->setText(tmp);

	for (unsigned int i = 0; i < dialog_->refsLB->count(); ++i) {
		if (tmp == dialog_->refsLB->text(i))
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
	string const name = controller().getBufferName(dialog_->bufferCO->currentItem());
	refs_ = controller().getLabelList(name);
	dialog_->sortCB->setEnabled(!refs_.empty());
	dialog_->refsLB->setEnabled(!refs_.empty());
	redoRefs();
}
