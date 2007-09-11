/**
 * \file GuiThesaurus.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiThesaurus.h"

#include "ControlThesaurus.h"
#include "qt_helpers.h"
#include "debug.h"

#include <QCloseEvent>
#include <QHeaderView>
#include <QLineEdit>
#include <QPushButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>

using std::string;


namespace lyx {
namespace frontend {

GuiThesaurusDialog::GuiThesaurusDialog(LyXView & lv)
	: GuiDialog(lv, "thesaurus")
{
	setupUi(this);
	setViewTitle(_("Thesaurus"));
	setController(new ControlThesaurus(*this));

	meaningsTV->setColumnCount(1);
	meaningsTV->header()->hide();

	connect(closePB, SIGNAL(clicked()),
		this, SLOT(slotClose()));
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

	bc().setCancel(closePB);
	bc().setApply(replacePB);
	bc().addReadOnly(replaceED);
	bc().addReadOnly(replacePB);
	bc().setPolicy(ButtonPolicy::OkApplyCancelReadOnlyPolicy);
}


ControlThesaurus & GuiThesaurusDialog::controller()
{
	return static_cast<ControlThesaurus &>(GuiDialog::controller());
}

void GuiThesaurusDialog::change_adaptor()
{
	changed();
}


void GuiThesaurusDialog::closeEvent(QCloseEvent * e)
{
	slotClose();
	e->accept();
}


void GuiThesaurusDialog::entryChanged()
{
	updateLists();
}


void GuiThesaurusDialog::selectionChanged()
{
	int const col = meaningsTV->currentColumn();
	if (col < 0 || controller().isBufferReadonly())
		return;

	replaceED->setText(meaningsTV->currentItem()->text(col));
	replacePB->setEnabled(true);
	changed();
}


void GuiThesaurusDialog::itemClicked(QTreeWidgetItem * /*item*/, int /*col*/)
{
	selectionChanged();
}


void GuiThesaurusDialog::selectionClicked(QTreeWidgetItem * item, int col)
{
	entryED->setText(item->text(col));
	selectionChanged();
	updateLists();
}


void GuiThesaurusDialog::updateLists()
{
	meaningsTV->clear();
	meaningsTV->setUpdatesEnabled(false);

	Thesaurus::Meanings meanings = controller().getMeanings(qstring_to_ucs4(entryED->text()));

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


void GuiThesaurusDialog::updateContents()
{
	entryED->setText(toqstr(controller().text()));
	replaceED->setText("");
	updateLists();
}


void GuiThesaurusDialog::replaceClicked()
{
	controller().replace(qstring_to_ucs4(replaceED->text()));
}

} // namespace frontend
} // namespace lyx


#include "GuiThesaurus_moc.cpp"
