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
#elif defined(USE_PSPELL)
# include "PSpell.h"
#endif

#if defined(USE_ISPELL)
# include "ISpell.h"
#else
# include "SpellBase.h"
#endif

#include "frontends/alert.h"

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {


GuiSpellchecker::GuiSpellchecker(GuiView & lv)
	: GuiDialog(lv, "spellchecker", qt_("Spellchecker")), exitEarly_(false),
	  oldval_(0), newvalue_(0), count_(0), speller_(0)
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


GuiSpellchecker::~GuiSpellchecker()
{
	delete speller_;
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
	if (suggestionsLW->currentItem()->text() == str)
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
			spellcheckPR->setValue(getProgress());
			break;

		case SPELL_FOUND_WORD: {
			wordED->setText(toqstr(getWord()));
			suggestionsLW->clear();

			docstring w;
			while (!(w = getSuggestion()).empty())
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


static SpellBase * createSpeller(BufferParams const & bp)
{
	string lang = (lyxrc.isp_use_alt_lang)
		      ? lyxrc.isp_alt_lang
		      : bp.language->code();

#if defined(USE_ASPELL)
	if (lyxrc.use_spell_lib)
		return new ASpell(bp, lang);
#elif defined(USE_PSPELL)
	if (lyxrc.use_spell_lib)
		return new PSpell(bp, lang);
#endif

#if defined(USE_ISPELL)
	lang = lyxrc.isp_use_alt_lang ?
		lyxrc.isp_alt_lang : bp.language->lang();

	return new ISpell(bp, lang);
#else
	return new SpellBase;
#endif
}


bool GuiSpellchecker::initialiseParams(string const &)
{
	LYXERR(Debug::GUI, "Spellchecker::initialiseParams");

	speller_ = createSpeller(buffer().params());
	if (!speller_)
		return false;

	// reset values to initial
	oldval_ = 0;
	newvalue_ = 0;
	count_ = 0;

	bool const success = speller_->error().empty();

	if (!success) {
		Alert::error(_("Spellchecker error"),
			     _("The spellchecker could not be started\n")
			     + speller_->error());
		delete speller_;
		speller_ = 0;
	}

	return success;
}


void GuiSpellchecker::clearParams()
{
	LYXERR(Debug::GUI, "Spellchecker::clearParams");
	delete speller_;
	speller_ = 0;
}


static bool isLetter(DocIterator const & dit)
{
	return dit.inTexted()
		&& dit.inset().allowSpellCheck()
		&& dit.pos() != dit.lastpos()
		&& (dit.paragraph().isLetter(dit.pos())
		    // We want to pass the ' and escape chars to ispell
		    || contains(from_utf8(lyxrc.isp_esc_chars + '\''),
				dit.paragraph().getChar(dit.pos())))
		&& !dit.paragraph().isDeleted(dit.pos());
}


static WordLangTuple nextWord(Cursor & cur, ptrdiff_t & progress)
{
	BufferParams const & bp = cur.bv().buffer().params();
	bool inword = false;
	bool ignoreword = false;
	cur.resetAnchor();
	docstring word;
	string lang_code;

	while (cur.depth()) {
		if (isLetter(cur)) {
			if (!inword) {
				inword = true;
				ignoreword = false;
				cur.resetAnchor();
				word.clear();
				lang_code = cur.paragraph().getFontSettings(bp, cur.pos()).language()->code();
			}
			// Insets like optional hyphens and ligature
			// break are part of a word.
			if (!cur.paragraph().isInset(cur.pos())) {
				char_type const c = cur.paragraph().getChar(cur.pos());
				word += c;
				if (isDigit(c))
					ignoreword = true;
			}
		} else { // !isLetter(cur)
			if (inword)
				if (!word.empty() && !ignoreword) {
					cur.setSelection();
					return WordLangTuple(word, lang_code);
				}
				inword = false;
		}

		cur.forwardPos();
		++progress;
	}

	return WordLangTuple(docstring(), string());
}


void GuiSpellchecker::check()
{
	LYXERR(Debug::GUI, "Check the spelling of a word");

	SpellBase::Result res = SpellBase::OK;

	Cursor cur = bufferview()->cursor();
	while (cur && cur.pos() && isLetter(cur))
		cur.backwardPos();

	ptrdiff_t start = 0;
	ptrdiff_t total = 0;
	DocIterator it = doc_iterator_begin(buffer().inset());
	for (start = 1; it != cur; it.forwardPos())
		++start;

	for (total = start; it; it.forwardPos())
		++total;

	exitEarly_ = false;

	while (res == SpellBase::OK || res == SpellBase::IGNORED_WORD) {
		word_ = nextWord(cur, start);

		// end of document
		if (getWord().empty()) {
			showSummary();
			exitEarly_ = true;
			return;
		}

		++count_;

		// Update slider if and only if value has changed
		float progress = total ? float(start)/total : 1;
		newvalue_ = int(100.0 * progress);
		if (newvalue_!= oldval_) {
			LYXERR(Debug::GUI, "Updating spell progress.");
			oldval_ = newvalue_;
			// set progress bar
			partialUpdate(SPELL_PROGRESSED);
		}

		// speller might be dead ...
		if (!checkAlive())
			return;

		res = speller_->check(word_);

		// ... or it might just be reporting an error
		if (!checkAlive())
			return;
	}

	LYXERR(Debug::GUI, "Found word \"" << to_utf8(getWord()) << "\"");

	int const size = cur.selEnd().pos() - cur.selBegin().pos();
	cur.pos() -= size;
	BufferView * bv = const_cast<BufferView *>(bufferview());
	bv->putSelectionAt(cur, size, false);
	// FIXME: if we used a lfun like in find/replace, dispatch would do
	// that for us
	// FIXME: this Controller is very badly designed...
	bv->processUpdateFlags(Update::Force | Update::FitCursor);

	// set suggestions
	if (res != SpellBase::OK && res != SpellBase::IGNORED_WORD) {
		LYXERR(Debug::GUI, "Found a word needing checking.");
		partialUpdate(SPELL_FOUND_WORD);
	}
}


bool GuiSpellchecker::checkAlive()
{
	if (speller_->alive() && speller_->error().empty())
		return true;

	docstring message;
	if (speller_->error().empty())
		message = _("The spellchecker has died for some reason.\n"
					 "Maybe it has been killed.");
	else
		message = _("The spellchecker has failed.\n") + speller_->error();

	slotClose();

	Alert::error(_("The spellchecker has failed"), message);
	return false;
}


void GuiSpellchecker::showSummary()
{
	if (!checkAlive() || count_ == 0) {
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


docstring GuiSpellchecker::getSuggestion() const
{
	return speller_->nextMiss();
}


docstring GuiSpellchecker::getWord() const
{
	return word_.word();
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
