/**
 * \file QThesaurus.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "QThesaurus.h"
#include "Qt2BC.h"
#include "qt_helpers.h"
#include "debug.h"

#include "controllers/ControlThesaurus.h"

#include <QHeaderView>
#include <QLineEdit>
#include <QPushButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>

using std::string;

namespace lyx {
namespace frontend {

/////////////////////////////////////////////////////////////////////
//
// QTheasurusDialog
//
/////////////////////////////////////////////////////////////////////

QThesaurusDialog::QThesaurusDialog(QThesaurus * form)
	: form_(form)
{
	setupUi(this);

	meaningsTV->setColumnCount(1);
	meaningsTV->header()->hide();

	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));
	connect(replaceED, SIGNAL(returnPressed()),
		this, SLOT(replaceClicked()));
	connect(replaceED, SIGNAL(textChanged(const QString &)),
		this, SLOT(change_adaptor() ) );
	connect(entryED, SIGNAL(returnPressed()),
		this, SLOT(entryChanged()));
	connect(replacePB, SIGNAL(clicked()),
		this, SLOT(replaceClicked()));
	connect(meaningsTV, SIGNAL(itemClicked(QTreeWidgetItem *, int)),
		this, SLOT(itemClicked(QTreeWidgetItem *, int)));
	connect(meaningsTV, SIGNAL(itemSelectionChanged()),
		this, SLOT(selectionChanged()));
	connect(meaningsTV, SIGNAL(itemActivated(QTreeWidgetItem *, int)),
		this, SLOT(selectionClicked(QTreeWidgetItem *, int)));
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


void QThesaurusDialog::itemClicked(QTreeWidgetItem * /*item*/, int /*col*/)
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

	Thesaurus::Meanings meanings = form_->controller().getMeanings(qstring_to_ucs4(entryED->text()));

	for (Thesaurus::Meanings::const_iterator cit = meanings.begin();
		cit != meanings.end(); ++cit) {
		QTreeWidgetItem * i = new QTreeWidgetItem(meaningsTV);
		i->setText(0, toqstr(cit->first));
		meaningsTV->expandItem(i);
		for (std::vector<docstring>::const_iterator cit2 = cit->second.begin();
			cit2 != cit->second.end(); ++cit2) {
				QTreeWidgetItem * i2 = new QTreeWidgetItem(i);
				i2->setText(0, toqstr(*cit2));
			}
	}

	meaningsTV->setUpdatesEnabled(true);
	meaningsTV->update();
}


/////////////////////////////////////////////////////////////////////
//
// QThesuarus
//
/////////////////////////////////////////////////////////////////////

typedef QController<ControlThesaurus, QView<QThesaurusDialog> > ThesaurusBase;

QThesaurus::QThesaurus(Dialog & parent)
	: ThesaurusBase(parent, _("Thesaurus"))
{
}


void QThesaurus::build_dialog()
{
	dialog_.reset(new QThesaurusDialog(this));

	bcview().setCancel(dialog_->closePB);
	bcview().setApply(dialog_->replacePB);
	bcview().addReadOnly(dialog_->replaceED);
	bcview().addReadOnly(dialog_->replacePB);
}


void QThesaurus::update_contents()
{
	dialog_->entryED->setText(toqstr(controller().text()));
	dialog_->replaceED->setText("");
	dialog_->updateLists();
}


void QThesaurus::replace()
{
	controller().replace(qstring_to_ucs4(dialog_->replaceED->text()));
}

} // namespace frontend
} // namespace lyx


#include "QThesaurus_moc.cpp"
