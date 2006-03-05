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
//Added by qt3to4:
#include <QCloseEvent>

#include "controllers/ControlThesaurus.h"

#include <qpushbutton.h>
#include <q3listview.h>
#include <qlineedit.h>

using std::string;

namespace lyx {
namespace frontend {


QThesaurusDialog::QThesaurusDialog(QThesaurus * form)
	: form_(form)
{
	setupUi(this);

	// hide the pointless QHeader
	QWidget * w = static_cast<QWidget*>(meaningsLV->child("list view header"));
	if (w)
		w->hide();

	connect(closePB, SIGNAL(clicked()),
		form, SLOT(slotClose()));

    connect( replaceED, SIGNAL( returnPressed() ), this, SLOT( replaceClicked() ) );
    connect( replaceED, SIGNAL( textChanged(const QString&) ), this, SLOT( change_adaptor() ) );
    connect( entryED, SIGNAL( returnPressed() ), this, SLOT( entryChanged() ) );
    connect( replacePB, SIGNAL( clicked() ), this, SLOT( replaceClicked() ) );
    connect( meaningsLV, SIGNAL( currentChanged(QListViewItem*) ), this, SLOT( selectionChanged(QListViewItem *) ) );
    connect( meaningsLV, SIGNAL( doubleClicked(QListViewItem*) ), this, SLOT( selectionClicked(QListViewItem *) ) );
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


void QThesaurusDialog::selectionChanged(Q3ListViewItem * item)
{
	if (form_->readOnly())
		return;

	string const entry(fromqstr(item->text(0)));
	replaceED->setText(toqstr(entry));
	replacePB->setEnabled(true);
	form_->changed();
}


void QThesaurusDialog::selectionClicked(Q3ListViewItem * item)
{
	entryED->setText(item->text(0));
	selectionChanged(item);
	updateLists();
}


void QThesaurusDialog::updateLists()
{
	meaningsLV->clear();

	std::vector<string> matches;

	meaningsLV->setUpdatesEnabled(false);

	Thesaurus::Meanings meanings = form_->controller().getMeanings(fromqstr(entryED->text()));

	for (Thesaurus::Meanings::const_iterator cit = meanings.begin();
		cit != meanings.end(); ++cit) {
		Q3ListViewItem * i = new Q3ListViewItem(meaningsLV);
		i->setText(0, toqstr(cit->first));
		i->setOpen(true);
		for (std::vector<string>::const_iterator cit2 = cit->second.begin();
			cit2 != cit->second.end(); ++cit2) {
				Q3ListViewItem * i2 = new Q3ListViewItem(i);
				i2->setText(0, toqstr(*cit2));
				i2->setOpen(true);
			}
	}

	meaningsLV->setUpdatesEnabled(true);
	meaningsLV->update();
}

} // namespace frontend
} // namespace lyx
