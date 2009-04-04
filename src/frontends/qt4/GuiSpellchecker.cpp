/**
 * \file GuiSpellchecker.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiSpellchecker.h"

#include "qt_helpers.h"

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


GuiSpellchecker::GuiSpellchecker(GuiView & lv)
	: GuiDialog(lv, "spellchecker", qt_("Spellchecker")),
	  progress_(0), count_(0)
{
	setupUi(this);

	connect(closePB, SIGNAL(clicked()), this, SLOT(slotClose()));
	connect(replacePB, SIGNAL(clicked()), this, SLOT(replace()));
	connect(ignorePB, SIGNAL(clicked()), this, SLOT(ignore()));
	connect(replacePB_3, SIGNAL(clicked()), this, SLOT(accept()));
	connect(addPB, SIGNAL(clicked()), this, SLOT(add()));

	connect(replaceCO, SIGNAL(highlighted(QString)),
		this, SLOT(replaceChanged(QString)));
	connect(suggestionsLW, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
		this, SLOT(replace()));
	connect(suggestionsLW, SIGNAL(itemClicked(QListWidgetItem*)),
		this, SLOT(suggestionChanged(QListWidgetItem*)));

	wordED->setReadOnly(true);

	bc().setPolicy(ButtonPolicy::NoRepeatedApplyReadOnlyPolicy);
	bc().setCancel(closePB);
}


void GuiSpellchecker::suggestionChanged(QListWidgetItem * item)
{
	if (replaceCO->count() != 0)
		replaceCO->setItemText(0, item->text());
	else
		replaceCO->addItem(item->text());

	replaceCO->setCurrentIndex(0);
}


void GuiSpellchecker::replaceChanged(const QString & str)
{
	if (suggestionsLW->currentItem()
	    && suggestionsLW->currentItem()->text() == str)
		return;

	for (int i = 0; i != suggestionsLW->count(); ++i) {
		if (suggestionsLW->item(i)->text() == str) {
			suggestionsLW->setCurrentRow(i);
			break;
		}
	}
}


void GuiSpellchecker::reject()
{
	slotClose();
	QDialog::reject();
}


void GuiSpellchecker::updateContents()
{
	if (hasFocus())
		check();
}


void GuiSpellchecker::accept()
{
	theSpellChecker()->accept(word_);
	check();
}


void GuiSpellchecker::add()
{
	theSpellChecker()->insert(word_);
	check();
}


void GuiSpellchecker::ignore()
{
	check();
}


void GuiSpellchecker::replace()
{
	replace(qstring_to_ucs4(replaceCO->currentText()));
}


void GuiSpellchecker::updateSuggestions(docstring_list & words)
{
	wordED->setText(toqstr(word_.word()));
	suggestionsLW->clear();

	if (words.empty()) {
		suggestionChanged(new QListWidgetItem(wordED->text()));
		return;
	}
	for (size_t i = 0; i != words.size(); ++i)
		suggestionsLW->addItem(toqstr(words[i]));

	suggestionChanged(suggestionsLW->item(0));
	suggestionsLW->setCurrentRow(0);
}


bool GuiSpellchecker::initialiseParams(string const &)
{
	LYXERR(Debug::GUI, "Spellchecker::initialiseParams");

	if (!theSpellChecker())
		return false;

	DocIterator const begin = doc_iterator_begin(&buffer());
	Cursor const & cur = bufferview()->cursor();
	progress_ = countWords(begin, cur);
	total_ = progress_ + countWords(cur, doc_iterator_end(&buffer()));
	count_ = 0;
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
			slotClose();
			return;
		}
		throw message;
	}
	LYXERR(Debug::GUI, "Found word \"" << word_lang.word() << "\"");
	count_ += progress;
	progress_ += progress;

	// end of document
	if (from == to) {
		showSummary();
		return;
	}
	if (!isVisible())
		show();

	word_ = word_lang;

	int const progress_bar = total_
		? int(100.0 * float(progress_)/total_) : 100;
	LYXERR(Debug::GUI, "Updating spell progress.");
	// set progress bar
	spellcheckPR->setValue(progress_bar);
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
	if (count_ == 0) {
		slotClose();
		return;
	}

	docstring message;
	if (count_ != 1)
		message = bformat(_("%1$d words checked."), count_);
	else
		message = _("One word checked.");

	slotClose();
	Alert::information(_("Spelling check completed"), message);
}


void GuiSpellchecker::replace(docstring const & replacement)
{
	LYXERR(Debug::GUI, "GuiSpellchecker::replace("
			   << to_utf8(replacement) << ")");
	BufferView * bv = const_cast<BufferView *>(bufferview());
	cap::replaceSelectionWithString(bv->cursor(), replacement, true);
	bv->buffer().markDirty();
	// If we used an LFUN, we would not need that
	bv->processUpdateFlags(Update::Force | Update::FitCursor);
	// fix up the count
	--count_;
	check();
}


void GuiSpellchecker::replaceAll(docstring const & replacement)
{
	// TODO: add to list
	replace(replacement);
}


Dialog * createGuiSpellchecker(GuiView & lv) { return new GuiSpellchecker(lv); }

} // namespace frontend
} // namespace lyx

#include "moc_GuiSpellchecker.cpp"
