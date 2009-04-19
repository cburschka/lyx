/**
 * \file GuiSpellchecker.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Edwin Leuven
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiSpellchecker.h"

#include "qt_helpers.h"

#include "ui_SpellcheckerUi.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "buffer_funcs.h"
#include "Cursor.h"
#include "CutAndPaste.h"
#include "Language.h"
#include "LyX.h"
#include "LyXRC.h"
#include "Paragraph.h"
#include "WordLangTuple.h"

#include "support/debug.h"
#include "support/docstring.h"
#include "support/docstring_list.h"
#include "support/ExceptionMessage.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/textutils.h"

#include <QListWidgetItem>

#include "SpellChecker.h"

#include "frontends/alert.h"

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {


struct GuiSpellchecker::Private
{
	Private() : progress_(0), count_(0) {}
	Ui::SpellcheckerUi ui;
	/// current word being checked and lang code
	WordLangTuple word_;
	/// values for progress
	int total_;
	int progress_;
	/// word count
	int count_;
};


GuiSpellchecker::GuiSpellchecker(GuiView & lv)
	: DockView(lv, "spellchecker", qt_("Spellchecker"),
	Qt::RightDockWidgetArea), d(new GuiSpellchecker::Private)
{
	d->ui.setupUi(this);

	connect(d->ui.suggestionsLW, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
		this, SLOT(on_replacePB_clicked()));

	d->ui.wordED->setReadOnly(true);
}


GuiSpellchecker::~GuiSpellchecker()
{
	delete d;
}


void GuiSpellchecker::on_closePB_clicked()
{
	close();
}


void GuiSpellchecker::on_suggestionsLW_changed(QListWidgetItem * item)
{
	if (d->ui.replaceCO->count() != 0)
		d->ui.replaceCO->setItemText(0, item->text());
	else
		d->ui.replaceCO->addItem(item->text());

	d->ui.replaceCO->setCurrentIndex(0);
}


void GuiSpellchecker::on_replaceC0_highlighted(const QString & str)
{
	QListWidget * lw = d->ui.suggestionsLW;
	if (lw->currentItem() && lw->currentItem()->text() == str)
		return;

	for (int i = 0; i != lw->count(); ++i) {
		if (lw->item(i)->text() == str) {
			lw->setCurrentRow(i);
			break;
		}
	}
}


void GuiSpellchecker::updateView()
{
	if (hasFocus())
		check();
}


void GuiSpellchecker::on_replaceAllPB_clicked()
{
	/// replace all occurances of word
	theSpellChecker()->accept(d->word_);
	check();
}


void GuiSpellchecker::on_addPB_clicked()
{
	/// insert word in personal dictionary
	theSpellChecker()->insert(d->word_);
	check();
}


void GuiSpellchecker::on_ignorePB_clicked()
{
	check();
}


void GuiSpellchecker::on_replacePB_clicked()
{
	docstring const replacement = qstring_to_ucs4(d->ui.replaceCO->currentText());

	LYXERR(Debug::GUI, "Replace (" << replacement << ")");
	BufferView * bv = const_cast<BufferView *>(bufferview());
	cap::replaceSelectionWithString(bv->cursor(), replacement, true);
	bv->buffer().markDirty();
	// If we used an LFUN, we would not need that
	bv->processUpdateFlags(Update::Force | Update::FitCursor);
	// fix up the count
	--d->count_;
	check();
}


void GuiSpellchecker::updateSuggestions(docstring_list & words)
{
	QString const suggestion = toqstr(d->word_.word());
	d->ui.wordED->setText(suggestion);
	QListWidget * lw = d->ui.suggestionsLW;
	lw->clear();

	if (words.empty()) {
		on_suggestionsLW_changed(new QListWidgetItem(suggestion));
		return;
	}
	for (size_t i = 0; i != words.size(); ++i)
		lw->addItem(toqstr(words[i]));

	on_suggestionsLW_changed(lw->item(0));
	lw->setCurrentRow(0);
}


bool GuiSpellchecker::initialiseParams(string const &)
{
	LYXERR(Debug::GUI, "Spellchecker::initialiseParams");

	if (!theSpellChecker())
		return false;

	DocIterator const begin = doc_iterator_begin(&buffer());
	Cursor const & cur = bufferview()->cursor();
	d->progress_ = countWords(begin, cur);
	d->total_ = d->progress_ + countWords(cur, doc_iterator_end(&buffer()));
	d->count_ = 0;
	return true;
}


void GuiSpellchecker::check()
{
	LYXERR(Debug::GUI, "Check the spelling of a word");

	DocIterator from = bufferview()->cursor();
	DocIterator to;
	WordLangTuple word_lang;
	docstring_list suggestions;

	int progress;
	try {
		progress = buffer().spellCheck(from, to, word_lang, suggestions);
	} catch (ExceptionMessage const & message) {
		if (message.type_ == WarningException) {
			Alert::warning(message.title_, message.details_);
			close();
			return;
		}
		throw message;
	}
	LYXERR(Debug::GUI, "Found word \"" << word_lang.word() << "\"");
	d->count_ += progress;
	d->progress_ += progress;

	// end of document
	if (from == to) {
		showSummary();
		return;
	}
	if (!isVisible())
		show();

	d->word_ = word_lang;

	int const progress_bar = d->total_
		? int(100.0 * float(d->progress_)/d->total_) : 100;
	LYXERR(Debug::GUI, "Updating spell progress.");
	// set progress bar
	d->ui.spellcheckPR->setValue(progress_bar);
	// set suggestions
	updateSuggestions(suggestions);

	// FIXME: if we used a lfun like in find/replace, dispatch would do
	// that for us
	int const size = to.pos() - from.pos();
	BufferView * bv = const_cast<BufferView *>(bufferview());
	bv->putSelectionAt(from, size, false);
}


void GuiSpellchecker::showSummary()
{
	if (d->count_ == 0) {
		close();
		return;
	}

	docstring message;
	if (d->count_ != 1)
		message = bformat(_("%1$d words checked."), d->count_);
	else
		message = _("One word checked.");

	close();
	Alert::information(_("Spelling check completed"), message);
}


Dialog * createGuiSpellchecker(GuiView & lv) { return new GuiSpellchecker(lv); }

} // namespace frontend
} // namespace lyx

#include "moc_GuiSpellchecker.cpp"
