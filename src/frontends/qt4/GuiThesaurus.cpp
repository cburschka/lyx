/**
 * \file GuiThesaurus.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiThesaurus.h"
#include "GuiApplication.h"

#include "qt_helpers.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "FuncRequest.h"
#include "Language.h"
#include "lyxfind.h"

#include "support/debug.h"
#include "support/gettext.h"
#include "support/lstrings.h"

#include <QAbstractItemModel>
#include <QHeaderView>
#include <QLineEdit>
#include <QPushButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>


using namespace lyx::support;
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
	connect(entryCO, SIGNAL(editTextChanged(const QString &)),
		this, SLOT(entryChanged()));
	connect(entryCO, SIGNAL(activated(int)),
		this, SLOT(entryChanged()));
	connect(lookupPB, SIGNAL(clicked()),
		this, SLOT(entryChanged()));
	connect(replacePB, SIGNAL(clicked()),
		this, SLOT(replaceClicked()));
	connect(languageCO, SIGNAL(activated(int)),
		this, SLOT(entryChanged()));
	connect(meaningsTV, SIGNAL(itemClicked(QTreeWidgetItem *, int)),
		this, SLOT(itemClicked(QTreeWidgetItem *, int)));
	connect(meaningsTV, SIGNAL(itemSelectionChanged()),
		this, SLOT(selectionChanged()));
	connect(meaningsTV, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)),
		this, SLOT(selectionClicked(QTreeWidgetItem *, int)));

	// language
	QAbstractItemModel * language_model = guiApp->languageModel();
	// FIXME: it would be nice if sorting was enabled/disabled via a checkbox.
	language_model->sort(0);
	languageCO->setModel(language_model);

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

	QString item = meaningsTV->currentItem()->text(col);
	// cut out the classification in brackets:
	// "hominid (generic term)" -> "hominid"
	QRegExp re("^([^\\(\\)]+)\\b\\(?.*\\)?.*$");
	// This is for items with classifications at the beginning:
	// "(noun) man" -> "man"; "(noun) male (generic term)" -> "male"
	QRegExp rex("^(\\(.+\\))\\s*([^\\(\\)]+)\\s*\\(?.*\\)?.*$");
	int pos = re.indexIn(item);
	if (pos > -1)
		item = re.cap(1).trimmed();
	pos = rex.indexIn(item);
	if (pos > -1)
		item = rex.cap(2).trimmed();
	replaceED->setText(item);
	replacePB->setEnabled(true);
	changed();
}


void GuiThesaurus::itemClicked(QTreeWidgetItem * /*item*/, int /*col*/)
{
	selectionChanged();
}


void GuiThesaurus::selectionClicked(QTreeWidgetItem * item, int col)
{
	QString str = item->text(col);
	// cut out the classification in brackets:
	// "hominid (generic term)" -> "hominid"
	QRegExp re("^([^\\(\\)]+)\\b\\(?.*\\)?.*$");
	// This is for items with classifications at the beginning:
	// "(noun) man" -> "man"; "(noun) male (generic term)" -> "male"
	QRegExp rex("^(\\(.+\\))\\s*([^\\(\\)]+)\\s*\\(?.*\\)?.*$");
	int pos = re.indexIn(str);
	if (pos > -1)
		str = re.cap(1).trimmed();
	pos = rex.indexIn(str);
	if (pos > -1)
		str = rex.cap(2).trimmed();
	entryCO->insertItem(0, str);
	entryCO->setCurrentIndex(0);

	selectionChanged();
	updateLists();
}


void GuiThesaurus::updateLists()
{
	meaningsTV->clear();
	meaningsTV->setUpdatesEnabled(false);

	QString const lang = languageCO->itemData(
		languageCO->currentIndex()).toString();
	docstring const lang_code =
		from_ascii(lyx::languages.getLanguage(fromqstr(lang))->code());

	Thesaurus::Meanings meanings =
		getMeanings(qstring_to_ucs4(entryCO->currentText()), lang_code);

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
		meaningsTV->setEnabled(true);
		lookupPB->setEnabled(true);
		replaceED->setEnabled(true);
		replacePB->setEnabled(true);
	}

	if (meanings.empty()) {
		if (!thesaurus.thesaurusAvailable(lang_code)) {
			QTreeWidgetItem * i = new QTreeWidgetItem(meaningsTV);
			i->setText(0, qt_("No thesaurus available for this language!"));
			meaningsTV->setEnabled(false);
			lookupPB->setEnabled(false);
			replaceED->setEnabled(false);
			replacePB->setEnabled(false);
		}
	}

	meaningsTV->setUpdatesEnabled(true);
	meaningsTV->update();
}


void GuiThesaurus::updateContents()
{
	entryCO->clear();
	entryCO->addItem(toqstr(text_));
	entryCO->setCurrentIndex(0);
	replaceED->setText("");
	int const pos = languageCO->findData(toqstr(lang_));
	if (pos != -1)
		languageCO->setCurrentIndex(pos);
	updateLists();
}


void GuiThesaurus::replaceClicked()
{
	replace(qstring_to_ucs4(replaceED->text()));
}


bool GuiThesaurus::initialiseParams(string const & data)
{
	string arg;
	string const lang = rsplit(data, arg, ' ');
	if (prefixIs(lang, "lang=")) {
		lang_ = from_utf8(split(lang, '='));
		text_ = from_utf8(arg);
	} else {
		text_ = from_utf8(data);
		if (bufferview())
			lang_ = from_ascii(
				bufferview()->buffer().params().language->lang());
	}
	return true;
}


void GuiThesaurus::clearParams()
{
	text_.erase();
	lang_.erase();
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


Thesaurus::Meanings const & GuiThesaurus::getMeanings(docstring const & str,
	docstring const & lang)
{
	if (str != laststr_)
		meanings_ = thesaurus.lookup(str, lang);
	return meanings_;
}


Dialog * createGuiThesaurus(GuiView & lv) { return new GuiThesaurus(lv); }


} // namespace frontend
} // namespace lyx


#include "moc_GuiThesaurus.cpp"
