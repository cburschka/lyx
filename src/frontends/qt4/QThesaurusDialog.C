/**
 * \file QThesaurusDialog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QThesaurusDialog.h"
#include "QThesaurus.h"
#include "qt_helpers.h"
#include "debug.h"

#include "controllers/ControlThesaurus.h"

#include <QCloseEvent>
#include <QPushButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QLineEdit>
#include <QHeaderView>

using std::string;

namespace lyx {
namespace frontend {


QThesaurusDialog::QThesaurusDialog(QThesaurus * form)
	: form_(form)
{
	setupUi(this);

	meaningsTV->setColumnCount(1);
	meaningsTV->header()->hide();

	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
	connect( replaceED, SIGNAL( returnPressed() ), 
		this, SLOT( replaceClicked() ) );
	connect( replaceED, SIGNAL( textChanged(const QString&) ), 
		this, SLOT( change_adaptor() ) );
	connect( entryED, SIGNAL( returnPressed() ), 
		this, SLOT( entryChanged() ) );
	connect( replacePB, SIGNAL( clicked() ), 
		this, SLOT( replaceClicked() ) );
	connect( meaningsTV, SIGNAL( itemClicked(QTreeWidgetItem * , int) ), 
		this, SLOT( itemClicked(QTreeWidgetItem * , int) ) );
	connect( meaningsTV, SIGNAL( itemSelectionChanged() ), 
		this, SLOT( selectionChanged() ) );
	connect( meaningsTV, SIGNAL( itemActivated(QTreeWidgetItem * , int) ), 
		this, SLOT( selectionClicked(QTreeWidgetItem *, int) ) );
}


void QThesaurusDialog::change_adaptor()
{
	form_->changed();
}


void QThesaurusDialog::closeEvent(QCloseEvent * e)
{
	form_->slotWMHide();
	e->accept();
}


void QThesaurusDialog::entryChanged()
{
	updateLists();
}


void QThesaurusDialog::replaceClicked()
{
	form_->replace();
}


void QThesaurusDialog::selectionChanged()
{
	int const col = meaningsTV->currentColumn();
	if (col<0 || form_->readOnly())
		return;

	replaceED->setText(meaningsTV->currentItem()->text(col));
	replacePB->setEnabled(true);
	form_->changed();
}


void QThesaurusDialog::itemClicked(QTreeWidgetItem * item, int col)
{
	selectionChanged();
}


void QThesaurusDialog::selectionClicked(QTreeWidgetItem * item, int col)
{
	entryED->setText(item->text(col));
	selectionChanged();
	updateLists();
}


void QThesaurusDialog::updateLists()
{
	meaningsTV->clear();
	meaningsTV->setUpdatesEnabled(false);

	Thesaurus::Meanings meanings = form_->controller().getMeanings(fromqstr(entryED->text()));

	for (Thesaurus::Meanings::const_iterator cit = meanings.begin();
		cit != meanings.end(); ++cit) {
		QTreeWidgetItem * i = new QTreeWidgetItem(meaningsTV);
		i->setText(0, toqstr(cit->first));
		meaningsTV->expandItem(i);
		for (std::vector<string>::const_iterator cit2 = cit->second.begin();
			cit2 != cit->second.end(); ++cit2) {
				QTreeWidgetItem * i2 = new QTreeWidgetItem(i);
				i2->setText(0, toqstr(*cit2));
			}
	}

	meaningsTV->setUpdatesEnabled(true);
	meaningsTV->update();
}

} // namespace frontend
} // namespace lyx
