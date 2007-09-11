/**
 * \file ControlSpellchecker.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlSpellchecker.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Cursor.h"
#include "CutAndPaste.h"
#include "debug.h"
#include "gettext.h"
#include "Language.h"
#include "LyXRC.h"
#include "Paragraph.h"

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

#include "support/textutils.h"
#include "support/convert.h"
#include "support/docstring.h"

#include "frontends/alert.h"
// FIXME: those two headers are needed because of the
// WorkArea::redraw() call below.
#include "frontends/LyXView.h"
#include "frontends/WorkArea.h"

using std::advance;
using std::distance;
using std::endl;
using std::string;

namespace lyx {

using support::bformat;
using support::contains;

namespace frontend {


ControlSpellchecker::ControlSpellchecker(Dialog & parent)
	: Controller(parent), exitEarly_(false),
	  oldval_(0), newvalue_(0), count_(0), speller_(0)
{
}


ControlSpellchecker::~ControlSpellchecker()
{
	delete speller_;
}


static SpellBase * getSpeller(BufferParams const & bp)
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


bool ControlSpellchecker::initialiseParams(std::string const &)
{
	LYXERR(Debug::GUI) << "Spellchecker::initialiseParams" << endl;

	speller_ = getSpeller(buffer().params());
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


void ControlSpellchecker::clearParams()
{
	LYXERR(Debug::GUI) << "Spellchecker::clearParams" << endl;
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
				Paragraph::value_type const c =
					cur.paragraph().getChar(cur.pos());
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


void ControlSpellchecker::check()
{
	LYXERR(Debug::GUI) << "Check the spelling of a word" << endl;

	SpellBase::Result res = SpellBase::OK;

	Cursor cur = bufferview()->cursor();
	while (cur && cur.pos() && isLetter(cur))
		cur.backwardPos();

	ptrdiff_t start = 0;
	ptrdiff_t total = 0;
	DocIterator it = DocIterator(buffer().inset());
	for (start = 0; it != cur; it.forwardPos())
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
			LYXERR(Debug::GUI) << "Updating spell progress." << endl;
			oldval_ = newvalue_;
			// set progress bar
			dialog().partialUpdateView(SPELL_PROGRESSED);
		}

		// speller might be dead ...
		if (!checkAlive())
			return;

		res = speller_->check(word_);

		// ... or it might just be reporting an error
		if (!checkAlive())
			return;
	}

	LYXERR(Debug::GUI) << "Found word \"" << to_utf8(getWord()) << "\"" << endl;

	int const size = cur.selEnd().pos() - cur.selBegin().pos();
	cur.pos() -= size;
	bufferview()->putSelectionAt(cur, size, false);
	// FIXME: if we used a lfun like in find/replace, dispatch would do
	// that for us
	bufferview()->update();
	// FIXME: this Controller is very badly designed...
	lyxview().currentWorkArea()->redraw();

	// set suggestions
	if (res != SpellBase::OK && res != SpellBase::IGNORED_WORD) {
		LYXERR(Debug::GUI) << "Found a word needing checking." << endl;
		dialog().partialUpdateView(SPELL_FOUND_WORD);
	}
}


bool ControlSpellchecker::checkAlive()
{
	if (speller_->alive() && speller_->error().empty())
		return true;

	docstring message;
	if (speller_->error().empty())
		message = _("The spellchecker has died for some reason.\n"
					 "Maybe it has been killed.");
	else
		message = _("The spellchecker has failed.\n") + speller_->error();

	dialog().slotClose();

	Alert::error(_("The spellchecker has failed"), message);
	return false;
}


void ControlSpellchecker::showSummary()
{
	if (!checkAlive() || count_ == 0) {
		dialog().slotClose();
		return;
	}

	docstring message;
	if (count_ != 1)
		message = bformat(_("%1$d words checked."), count_);
	else
		message = _("One word checked.");

	dialog().slotClose();
	Alert::information(_("Spelling check completed"), message);
}


void ControlSpellchecker::replace(docstring const & replacement)
{
	LYXERR(Debug::GUI) << "ControlSpellchecker::replace("
			   << to_utf8(replacement) << ")" << std::endl;
	cap::replaceSelectionWithString(bufferview()->cursor(), replacement, true);
	buffer().markDirty();
	// If we used an LFUN, we would not need that
	bufferview()->update();
	// fix up the count
	--count_;
	check();
}


void ControlSpellchecker::replaceAll(docstring const & replacement)
{
	// TODO: add to list
	replace(replacement);
}


void ControlSpellchecker::insert()
{
	speller_->insert(word_);
	check();
}


docstring const ControlSpellchecker::getSuggestion() const
{
	return speller_->nextMiss();
}


docstring const ControlSpellchecker::getWord() const
{
	return word_.word();
}


void ControlSpellchecker::ignoreAll()
{
	speller_->accept(word_);
	check();
}

} // namespace frontend
} // namespace lyx
