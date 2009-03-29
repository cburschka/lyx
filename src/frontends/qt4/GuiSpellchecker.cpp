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
#include "Cursor.h"
#include "CutAndPaste.h"
#include "Language.h"
#include "LyX.h"
#include "LyXRC.h"
#include "Paragraph.h"

#include "support/debug.h"
#include "support/docstring.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/textutils.h"

#include <QListWidgetItem>

#if defined(USE_ASPELL)
# include "ASpell_local.h"
#endif

#include "SpellChecker.h"

#include "frontends/alert.h"

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {


GuiSpellchecker::GuiSpellchecker(GuiView & lv)
	: GuiDialog(lv, "spellchecker", qt_("Spellchecker")), exitEarly_(false),
	  oldprogress_(0), newprogress_(0), count_(0), speller_(0)
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
	// The clauses below are needed because the spellchecker
	// has many flaws (see bugs 1950, 2218).
	// Basically, we have to distinguish the case where a
	// spellcheck has already been performed for the whole
	// document (exitEarly() == true, isVisible() == false) 
	// from the rest (exitEarly() == false, isVisible() == true).
	// FIXME: rewrite the whole beast!
	static bool check_after_early_exit;
	if (exitEarly()) {
		// a spellcheck has already been performed,
		check();
		check_after_early_exit = true;
	}
	else if (isVisible()) {
		// the above check triggers a second update,
		// and isVisible() is true then. Prevent a
		// second check that skips the first word
		if (check_after_early_exit)
			// don't check, but reset the bool.
			// business as usual after this.
			check_after_early_exit = false;
		else
			// perform spellcheck (default case)
			check();
	}
}


void GuiSpellchecker::accept()
{
	ignoreAll();
}


void GuiSpellchecker::add()
{
	insert();
}


void GuiSpellchecker::ignore()
{
	check();
}


void GuiSpellchecker::replace()
{
	replace(qstring_to_ucs4(replaceCO->currentText()));
}


void GuiSpellchecker::partialUpdate(int state)
{
	switch (state) {
		case SPELL_PROGRESSED:
			spellcheckPR->setValue(oldprogress_);
			break;

		case SPELL_FOUND_WORD: {
			wordED->setText(toqstr(word_.word()));
			suggestionsLW->clear();

			docstring w;
			while (!(w = speller_->nextMiss()).empty())
				suggestionsLW->addItem(toqstr(w));

			if (suggestionsLW->count() == 0)
				suggestionChanged(new QListWidgetItem(wordED->text()));
			else
				suggestionChanged(suggestionsLW->item(0));

			suggestionsLW->setCurrentRow(0);
			break;
		}
	}
}


bool GuiSpellchecker::initialiseParams(string const &)
{
	LYXERR(Debug::GUI, "Spellchecker::initialiseParams");

	speller_ = theSpellChecker();
	if (!speller_)
		return false;

	// reset values to initial
	oldprogress_ = 0;
	newprogress_ = 0;
	count_ = 0;

	bool const success = speller_->error().empty();

	if (!success) {
		Alert::error(_("Spellchecker error"),
			     _("The spellchecker could not be started\n")
			     + speller_->error());
		speller_ = 0;
	}

	return success;
}


void GuiSpellchecker::clearParams()
{
	LYXERR(Debug::GUI, "Spellchecker::clearParams");
	speller_ = 0;
}


static WordLangTuple nextWord(Cursor & cur, ptrdiff_t & progress)
{
	Buffer const & buf = cur.bv().buffer();
	cur.resetAnchor();
	docstring word;
	DocIterator from = cur;
	DocIterator to;
	if (!buf.nextWord(from, to, word))
		return WordLangTuple(docstring(), string());

	cur.setCursor(from);
	cur.resetAnchor();
	cur.setCursor(to);
	cur.setSelection();
	string lang_code = lyxrc.spellchecker_use_alt_lang
		      ? lyxrc.spellchecker_alt_lang
		      : from.paragraph().getFontSettings(buf.params(), cur.pos()).language()->code();
	++progress;
	return WordLangTuple(word, lang_code);
}


void GuiSpellchecker::check()
{
	LYXERR(Debug::GUI, "Check the spelling of a word");

	SpellChecker::Result res = SpellChecker::OK;

	Cursor cur = bufferview()->cursor();
	while (cur && cur.pos() && isLetter(cur))
		cur.backwardPos();

	ptrdiff_t start = 0;
	ptrdiff_t total = 0;
	DocIterator it = doc_iterator_begin(&buffer());
	for (start = 1; it != cur; it.forwardPos())
		++start;

	for (total = start; it; it.forwardPos())
		++total;

	exitEarly_ = false;

	while (res == SpellChecker::OK || res == SpellChecker::IGNORED_WORD) {
		word_ = nextWord(cur, start);

		// end of document
		if (word_.word().empty()) {
			showSummary();
			exitEarly_ = true;
			return;
		}

		++count_;

		// Update slider if and only if value has changed
		float progress = total ? float(start)/total : 1;
		newprogress_ = int(100.0 * progress);
		if (newprogress_!= oldprogress_) {
			LYXERR(Debug::GUI, "Updating spell progress.");
			oldprogress_ = newprogress_;
			// set progress bar
			partialUpdate(SPELL_PROGRESSED);
		}

		res = speller_->check(word_);

		// ... just bail out if the spellchecker reports an error.
		if (!speller_->error().empty()) {
			docstring const message =
				_("The spellchecker has failed.\n") + speller_->error();
			slotClose();
			return;
		}	
	}

	LYXERR(Debug::GUI, "Found word \"" << to_utf8(word_.word()) << "\"");

	int const size = cur.selEnd().pos() - cur.selBegin().pos();
	cur.pos() -= size;
	BufferView * bv = const_cast<BufferView *>(bufferview());
	bv->putSelectionAt(cur, size, false);
	// FIXME: if we used a lfun like in find/replace, dispatch would do
	// that for us
	// FIXME: this Controller is very badly designed...
	bv->processUpdateFlags(Update::Force | Update::FitCursor);

	// set suggestions
	if (res != SpellChecker::OK && res != SpellChecker::IGNORED_WORD) {
		LYXERR(Debug::GUI, "Found a word needing checking.");
		partialUpdate(SPELL_FOUND_WORD);
	}
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


void GuiSpellchecker::insert()
{
	speller_->insert(word_);
	check();
}


void GuiSpellchecker::ignoreAll()
{
	speller_->accept(word_);
	check();
}


Dialog * createGuiSpellchecker(GuiView & lv) { return new GuiSpellchecker(lv); }

} // namespace frontend
} // namespace lyx

#include "moc_GuiSpellchecker.cpp"
