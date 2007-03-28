/**
 * \file QRefDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Kalle Dalheimer
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QRefDialog.h"
#include "QRef.h"

#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QCloseEvent>

namespace lyx {
namespace frontend {

QRefDialog::QRefDialog(QRef * form)
	: form_(form)
{
	setupUi(this);

	connect(okPB, SIGNAL(clicked()),
		form_, SLOT(slotOK()));
	connect(applyPB, SIGNAL(clicked()),
		form_, SLOT(slotApply()));
	connect(closePB, SIGNAL(clicked()),
		form_, SLOT(slotClose()));

	connect( typeCO, SIGNAL( activated(int) ), 
		this, SLOT( changed_adaptor() ) );
	connect( referenceED, SIGNAL( textChanged(const QString&) ), 
		this, SLOT( changed_adaptor() ) );
	connect( nameED, SIGNAL( textChanged(const QString&) ), 
		this, SLOT( changed_adaptor() ) );
	connect( refsLW, SIGNAL(  itemClicked(QListWidgetItem *) ), 
		this, SLOT( refHighlighted(QListWidgetItem *) ) );
	connect( refsLW, SIGNAL(  itemSelectionChanged() ),
		this, SLOT( selectionChanged() ) );
	connect( refsLW, SIGNAL(  itemActivated(QListWidgetItem *) ), 
		this, SLOT( refSelected(QListWidgetItem *) ) );
	connect( sortCB, SIGNAL( clicked(bool) ),
		this, SLOT( sortToggled(bool) ) );
	connect( gotoPB, SIGNAL( clicked() ), 
		this, SLOT( gotoClicked() ) );
	connect( updatePB, SIGNAL( clicked() ), 
		this, SLOT( updateClicked() ) );
	connect( bufferCO, SIGNAL( activated(int) ), 
		this, SLOT( updateClicked() ) );

}

void QRefDialog::show()
{
	QDialog::show();
	refsLW->setFocus();
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


void QRefDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}

} // namespace frontend
} // namespace lyx

#include "QRefDialog_moc.cpp"
