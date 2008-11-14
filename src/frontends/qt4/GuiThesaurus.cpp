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

#include "qt_helpers.h"

#include "FuncRequest.h"
#include "lyxfind.h"

#include "support/debug.h"
#include "support/gettext.h"

#include <QHeaderView>
#include <QLineEdit>
#include <QPushButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>


using namespace std;

namespace lyx {
namespace frontend {

GuiThesaurus::GuiThesaurus(GuiView & lv)
	: GuiDialog(lv, "thesaurus", qt_("Thesaurus"))
{
	setupUi(this);

	meaningsTV->setColumnCount(1);
	meaningsTV->header()->hide();

	connect(closePB, SIGNAL(clicked()),
		this, SLOT(slotClose()));
	connect(replaceED, SIGNAL(returnPressed()),
		this, SLOT(replaceClicked()));
	connect(replaceED, SIGNAL(textChanged(QString)),
		this, SLOT(change_adaptor()));
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


void GuiThesaurus::change_adaptor()
{
	changed();
}


void GuiThesaurus::entryChanged()
{
	updateLists();
}


void GuiThesaurus::selectionChanged()
{
	int const col = meaningsTV->currentColumn();
	if (col < 0 || isBufferReadonly())
		return;

	replaceED->setText(meaningsTV->currentItem()->text(col));
	replacePB->setEnabled(true);
	changed();
}


void GuiThesaurus::itemClicked(QTreeWidgetItem * /*item*/, int /*col*/)
{
	selectionChanged();
}


void GuiThesaurus::selectionClicked(QTreeWidgetItem * item, int col)
{
	entryED->setText(item->text(col));
	selectionChanged();
	updateLists();
}


void GuiThesaurus::updateLists()
{
	meaningsTV->clear();
	meaningsTV->setUpdatesEnabled(false);

	Thesaurus::Meanings meanings = getMeanings(qstring_to_ucs4(entryED->text()));

	for (Thesaurus::Meanings::const_iterator cit = meanings.begin();
		cit != meanings.end(); ++cit) {
		QTreeWidgetItem * i = new QTreeWidgetItem(meaningsTV);
		i->setText(0, toqstr(cit->first));
		meaningsTV->expandItem(i);
		for (vector<docstring>::const_iterator cit2 = cit->second.begin();
			cit2 != cit->second.end(); ++cit2) {
				QTreeWidgetItem * i2 = new QTreeWidgetItem(i);
				i2->setText(0, toqstr(*cit2));
			}
	}

	meaningsTV->setUpdatesEnabled(true);
	meaningsTV->update();
}


void GuiThesaurus::updateContents()
{
	entryED->setText(toqstr(text_));
	replaceED->setText("");
	updateLists();
}


void GuiThesaurus::replaceClicked()
{
	replace(qstring_to_ucs4(replaceED->text()));
}


bool GuiThesaurus::initialiseParams(string const & data)
{
	text_ = from_utf8(data);
	return true;
}


void GuiThesaurus::clearParams()
{
	text_.erase();
}


void GuiThesaurus::replace(docstring const & newstr)
{
	/* FIXME: this is not suitable ! We need to have a "lock"
	 * on a particular charpos in a paragraph that is broken on
	 * deletion/change !
	 */
	docstring const data =
		replace2string(text_, newstr,
				     true,  // case sensitive
				     true,  // match word
				     false, // all words
				     true); // forward
	dispatch(FuncRequest(LFUN_WORD_REPLACE, data));
}


Thesaurus::Meanings const & GuiThesaurus::getMeanings(docstring const & str)
{
	if (str != laststr_)
		meanings_ = thesaurus.lookup(str);
	return meanings_;
}


Dialog * createGuiThesaurus(GuiView & lv) { return new GuiThesaurus(lv); }


} // namespace frontend
} // namespace lyx


#include "moc_GuiThesaurus.cpp"
