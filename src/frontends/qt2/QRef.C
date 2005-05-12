/**
 * \file QRef.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QRef.h"
#include "QRefDialog.h"
#include "Qt2BC.h"
#include "qt_helpers.h"

#include "controllers/ControlRef.h"

#include "insets/insetref.h"

#include <qlineedit.h>
#include <qcheckbox.h>
#include <qlistbox.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qtooltip.h>


using std::vector;
using std::string;

namespace lyx {
namespace frontend {

typedef QController<ControlRef, QView<QRefDialog> > base_class;


QRef::QRef(Dialog & parent)
	: base_class(parent, _("LyX: Cross-reference")),
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

	restored_buffer_ = -1;
}


void QRef::update_contents()
{
	InsetCommandParams const & params = controller().params();

	int orig_type = dialog_->typeCO->currentItem();

	dialog_->referenceED->setText(toqstr(params.getContents()));

	dialog_->nameED->setText(toqstr(params.getOptions()));
	dialog_->nameED->setReadOnly(!nameAllowed() && !readOnly());

	// restore type settings for new insets
	if (params.getContents().empty())
		dialog_->typeCO->setCurrentItem(orig_type);
	else
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
	// restore the buffer combo setting for new insets
	if (params.getContents().empty() && restored_buffer_ != -1
	&& restored_buffer_ < dialog_->bufferCO->count())
		dialog_->bufferCO->setCurrentItem(restored_buffer_);
	else
		dialog_->bufferCO->setCurrentItem(controller().getBufferNum());

	updateRefs();
}


void QRef::apply()
{
	InsetCommandParams & params = controller().params();

	params.setCmdName(InsetRef::getName(dialog_->typeCO->currentItem()));
	params.setContents(fromqstr(dialog_->referenceED->text()));
	params.setOptions(fromqstr(dialog_->nameED->text()));

	restored_buffer_ = dialog_->bufferCO->currentItem();
}


bool QRef::nameAllowed()
{
	Kernel::DocType const doc_type = kernel().docType();
	return doc_type != Kernel::LATEX &&
		doc_type != Kernel::LITERATE;
}


bool QRef::typeAllowed()
{
	Kernel::DocType const doc_type = kernel().docType();
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
	// Prevent these widgets from emitting any signals whilst
	// we modify their state.
	dialog_->refsLB->blockSignals(true);
	dialog_->referenceED->blockSignals(true);

	int lastref = dialog_->refsLB->currentItem();

	dialog_->refsLB->setAutoUpdate(false);
	dialog_->refsLB->clear();

	// need this because Qt will send a highlight() here for
	// the first item inserted
	QString const tmp(dialog_->referenceED->text());

	for (std::vector<string>::const_iterator iter = refs_.begin();
		iter != refs_.end(); ++iter) {
		dialog_->refsLB->insertItem(toqstr(*iter));
	}

	if (sort_)
		dialog_->refsLB->sort();

	dialog_->referenceED->setText(tmp);

	// restore the last selection for new insets
	if (tmp.isEmpty() && lastref != -1
	    && lastref < int(dialog_->refsLB->count()))
		dialog_->refsLB->setCurrentItem(lastref);
	else
		for (unsigned int i = 0; i < dialog_->refsLB->count(); ++i) {
			if (tmp == dialog_->refsLB->text(i))
				dialog_->refsLB->setCurrentItem(i);
		}

	dialog_->refsLB->setAutoUpdate(true);
	dialog_->refsLB->update();

	// Re-activate the emission of signals by these widgets.
	dialog_->refsLB->blockSignals(false);
	dialog_->referenceED->blockSignals(false);
}


void QRef::updateRefs()
{
	refs_.clear();
	if (at_ref_)
		gotoRef();
	string const name = controller().getBufferName(dialog_->bufferCO->currentItem());
	refs_ = controller().getLabelList(name);
	dialog_->sortCB->setEnabled(!refs_.empty());
	dialog_->refsLB->setEnabled(!refs_.empty());
	dialog_->gotoPB->setEnabled(!refs_.empty());
	redoRefs();
}

} // namespace frontend
} // namespace lyx
