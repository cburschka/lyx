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
#include "Qt2BC.h"
#include "qt_helpers.h"

#include "controllers/ButtonController.h"
#include "controllers/ControlRef.h"

#include "insets/InsetRef.h"

#include <QLineEdit>
#include <QCheckBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QToolTip>
#include <QCloseEvent>


using std::vector;
using std::string;


namespace lyx {
namespace frontend {

/////////////////////////////////////////////////////////////////////
//
// QRefDialog
//
/////////////////////////////////////////////////////////////////////

QRefDialog::QRefDialog(QRef * form)
	: form_(form)
{
	setupUi(this);

	connect(okPB, SIGNAL(clicked()), form_, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()), form_, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()), form_, SLOT(slotClose()));
	connect(closePB, SIGNAL(clicked()), this, SLOT(reset_dialog()));

	connect(typeCO, SIGNAL(activated(int)), 
		this, SLOT(changed_adaptor()));
	connect(referenceED, SIGNAL(textChanged(const QString &)), 
		this, SLOT(changed_adaptor()));
	connect(nameED, SIGNAL(textChanged(const QString &)), 
		this, SLOT(changed_adaptor()));
	connect(refsLW, SIGNAL(itemClicked(QListWidgetItem *)), 
		this, SLOT(refHighlighted(QListWidgetItem *)));
	connect(refsLW, SIGNAL(itemSelectionChanged()),
		this, SLOT(selectionChanged()));
	connect(refsLW, SIGNAL(itemActivated(QListWidgetItem *)), 
		this, SLOT(refSelected(QListWidgetItem *)));
	connect(sortCB, SIGNAL(clicked(bool)),
		this, SLOT(sortToggled(bool)));
	connect(gotoPB, SIGNAL(clicked()), 
		this, SLOT(gotoClicked()));
	connect(updatePB, SIGNAL(clicked()), 
		this, SLOT(updateClicked()));
	connect(bufferCO, SIGNAL(activated(int)), 
		this, SLOT(updateClicked()));

	setFocusProxy(refsLW);
}

void QRefDialog::show()
{
	QDialog::show();
}


void QRefDialog::changed_adaptor()
{
	form_->changed();
}


void QRefDialog::gotoClicked()
{
	form_->gotoRef();
}

void QRefDialog::selectionChanged()
{
	if (form_->readOnly())
		return;
	
	QList<QListWidgetItem *> selections = refsLW->selectedItems();
	if (selections.isEmpty())
		return;
	QListWidgetItem * sel = selections.first();
	refHighlighted(sel);
	return;
}


void QRefDialog::refHighlighted(QListWidgetItem * sel)
{
	if (form_->readOnly())
		return;

/*	int const cur_item = refsLW->currentRow();
	bool const cur_item_selected = cur_item >= 0 ?
		refsLB->isSelected(cur_item) : false;*/
	bool const cur_item_selected = refsLW->isItemSelected(sel);

	if (cur_item_selected)
		referenceED->setText(sel->text());

	if (form_->at_ref_)
		form_->gotoRef();
	gotoPB->setEnabled(true);
	if (form_->typeAllowed())
		typeCO->setEnabled(true);
	if (form_->nameAllowed())
		nameED->setEnabled(true);
}


void QRefDialog::refSelected(QListWidgetItem * sel)
{
	if (form_->readOnly())
		return;

/*	int const cur_item = refsLW->currentRow();
	bool const cur_item_selected = cur_item >= 0 ?
		refsLB->isSelected(cur_item) : false;*/
	bool const cur_item_selected = refsLW->isItemSelected(sel);

	if (cur_item_selected)
		referenceED->setText(sel->text());
	// <enter> or double click, inserts ref and closes dialog
	form_->slotOK();
}


void QRefDialog::sortToggled(bool on)
{
	form_->sort_ = on;
	form_->redoRefs();
}


void QRefDialog::updateClicked()
{
	form_->updateRefs();
}


void QRefDialog::reset_dialog() {
	form_->at_ref_ = false;
	form_->setGotoRef();
}


void QRefDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	reset_dialog();
	e->accept();
}


/////////////////////////////////////////////////////////////////////
//
// QRef
//
/////////////////////////////////////////////////////////////////////

// full qualification because qt4 has also a ControlRef type
typedef QController<lyx::frontend::ControlRef, QView<QRefDialog> > RefBase;


QRef::QRef(Dialog & parent)
	: RefBase(parent, _("Cross-reference")),
	sort_(false), at_ref_(false)
{
}


void QRef::build_dialog()
{
	dialog_.reset(new QRefDialog(this));

	bcview().setOK(dialog_->okPB);
	bcview().setApply(dialog_->applyPB);
	bcview().setCancel(dialog_->closePB);
	bcview().addReadOnly(dialog_->refsLW);
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

	int orig_type = dialog_->typeCO->currentIndex();

	dialog_->referenceED->setText(toqstr(params["reference"]));

	dialog_->nameED->setText(toqstr(params["name"]));
	dialog_->nameED->setReadOnly(!nameAllowed() && !readOnly());

	// restore type settings for new insets
	if (params["reference"].empty())
		dialog_->typeCO->setCurrentIndex(orig_type);
	else
		dialog_->typeCO->setCurrentIndex(InsetRef::getType(params.getCmdName()));
	dialog_->typeCO->setEnabled(typeAllowed() && !readOnly());
	if (!typeAllowed())
		dialog_->typeCO->setCurrentIndex(0);

	dialog_->sortCB->setChecked(sort_);

	// insert buffer list
	dialog_->bufferCO->clear();
	vector<string> const buffers = controller().getBufferList();
	for (vector<string>::const_iterator it = buffers.begin();
		it != buffers.end(); ++it) {
		dialog_->bufferCO->addItem(toqstr(*it));
	}
	// restore the buffer combo setting for new insets
	if (params["reference"].empty() && restored_buffer_ != -1
	&& restored_buffer_ < dialog_->bufferCO->count())
		dialog_->bufferCO->setCurrentIndex(restored_buffer_);
	else
		dialog_->bufferCO->setCurrentIndex(controller().getBufferNum());

	updateRefs();
	bc().valid(false);
}


void QRef::apply()
{
	InsetCommandParams & params = controller().params();
	
	last_reference_ = dialog_->referenceED->text();

	params.setCmdName(InsetRef::getName(dialog_->typeCO->currentIndex()));
	params["reference"] = qstring_to_ucs4(last_reference_);
	params["name"] = qstring_to_ucs4(dialog_->nameED->text());

	restored_buffer_ = dialog_->bufferCO->currentIndex();
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
	return doc_type != Kernel::DOCBOOK;
}


void QRef::setGoBack()
{
	dialog_->gotoPB->setText(qt_("&Go Back"));
	dialog_->gotoPB->setToolTip("");
	dialog_->gotoPB->setToolTip(qt_("Jump back"));
}


void QRef::setGotoRef()
{
	dialog_->gotoPB->setText(qt_("&Go to Label"));
	dialog_->gotoPB->setToolTip("");
	dialog_->gotoPB->setToolTip(qt_("Jump to label"));
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
	dialog_->refsLW->blockSignals(true);
	dialog_->referenceED->blockSignals(true);
	dialog_->refsLW->setUpdatesEnabled(false);
	
	dialog_->refsLW->clear();

	// need this because Qt will send a highlight() here for
	// the first item inserted
	QString const oldSelection(dialog_->referenceED->text());

	for (std::vector<docstring>::const_iterator iter = refs_.begin();
		iter != refs_.end(); ++iter) {
		dialog_->refsLW->addItem(toqstr(*iter));
	}

	if (sort_)
		dialog_->refsLW->sortItems();

	dialog_->referenceED->setText(oldSelection);

	// restore the last selection or, for new insets, highlight
	// the previous selection
	if (!oldSelection.isEmpty() || !last_reference_.isEmpty()) {
		bool const newInset = oldSelection.isEmpty();
		QString textToFind = newInset ? last_reference_ : oldSelection;
		bool foundItem = false;
		for (int i = 0; !foundItem && i < dialog_->refsLW->count(); ++i) {
			QListWidgetItem * item = dialog_->refsLW->item(i);
			if (textToFind == item->text()) {
				dialog_->refsLW->setCurrentItem(item);
				dialog_->refsLW->setItemSelected(item, !newInset);
				//Make sure selected item is visible
				dialog_->refsLW->scrollToItem(item);
				foundItem = true;
			}
		}
		if (foundItem)
			last_reference_ = textToFind;
		else last_reference_ = "";
	}
	dialog_->refsLW->setUpdatesEnabled(true);
	dialog_->refsLW->update();

	// Re-activate the emission of signals by these widgets.
	dialog_->refsLW->blockSignals(false);
	dialog_->referenceED->blockSignals(false);
}


void QRef::updateRefs()
{
	refs_.clear();
	string const name = controller().getBufferName(dialog_->bufferCO->currentIndex());
	refs_ = controller().getLabelList(name);
	dialog_->sortCB->setEnabled(!refs_.empty());
	dialog_->refsLW->setEnabled(!refs_.empty());
	dialog_->gotoPB->setEnabled(!refs_.empty());
	redoRefs();
}

bool QRef::isValid()
{
	return !dialog_->referenceED->text().isEmpty();
}

} // namespace frontend
} // namespace lyx

#include "QRef_moc.cpp"
