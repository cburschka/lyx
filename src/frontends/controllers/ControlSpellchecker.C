/**
 * \file ControlSpellchecker.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlSpellchecker.h"

#include "buffer.h"
#include "bufferparams.h"
#include "BufferView.h"
#include "cursor.h"
#include "CutAndPaste.h"
#include "debug.h"
#include "gettext.h"
#include "language.h"
#include "lyxrc.h"
#include "paragraph.h"

#if defined(USE_ASPELL)
# include "aspell_local.h"
#elif defined(USE_PSPELL)
# include "pspell.h"
#endif

#if defined(USE_ISPELL)
# include "ispell.h"
#else
# include "SpellBase.h"
#endif

#include "support/textutils.h"
#include "support/convert.h"

#include "frontends/Alert.h"

using std::advance;
using std::distance;
using std::endl;
using std::string;

namespace lyx {

using support::bformat;
using support::contains;

namespace frontend {


ControlSpellchecker::ControlSpellchecker(Dialog & parent)
	: Dialog::Controller(parent),
	  oldval_(0), newvalue_(0), count_(0)
{}


ControlSpellchecker::~ControlSpellchecker()
{}


namespace {

SpellBase * getSpeller(BufferParams const & bp)
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
	lang = (lyxrc.isp_use_alt_lang) ?
		lyxrc.isp_alt_lang : bp.language->lang();

	return new ISpell(bp, lang);
#else
	return new SpellBase;
#endif
}

} // namespace anon


bool ControlSpellchecker::initialiseParams(std::string const &)
{
	lyxerr[Debug::GUI] << "Spellchecker::initialiseParams" << endl;

	speller_.reset(getSpeller(kernel().buffer().params()));
	if (!speller_.get())
		return false;

	// reset values to initial
	oldval_ = 0;
	newvalue_ = 0;
	count_ = 0;

	bool const success = speller_->error().empty();

	if (!success) {
		Alert::error(_("The spell-checker could not be started"),
			     speller_->error());
		speller_.reset(0);
	}

	return success;
}


void ControlSpellchecker::clearParams()
{
	lyxerr[Debug::GUI] << "Spellchecker::clearParams" << endl;
	speller_.reset(0);
}


namespace {

bool isLetter(DocIterator const & cur)
{
	return cur.inTexted()
		&& cur.inset().allowSpellCheck()
		&& cur.pos() != cur.lastpos()
		&& (cur.paragraph().isLetter(cur.pos())
		    // We want to pass the ' and escape chars to ispell
		    || contains(lyxrc.isp_esc_chars + '\'',
				cur.paragraph().getChar(cur.pos())))
		&& !isDeletedText(cur.paragraph(), cur.pos());
}


WordLangTuple nextWord(DocIterator & cur, ptrdiff_t & progress,
	BufferParams & bp)
{
	bool inword = false;
	bool ignoreword = false;
	string word, lang_code;

	while (cur.depth()) {
		if (isLetter(cur)) {
			if (!inword) {
				inword = true;
				ignoreword = false;
				word.clear();
				lang_code = cur.paragraph().getFontSettings(bp, cur.pos()).language()->code();
			}
			// Insets like optional hyphens and ligature
			// break are part of a word.
			if (!cur.paragraph().isInset(cur.pos())) {
				Paragraph::value_type const c =
					cur.paragraph().getChar(cur.pos());
				word += c;
				if (IsDigit(c))
					ignoreword = true;
			}
		} else { // !isLetter(cur)
			if (inword)
				if (!ignoreword)
					return WordLangTuple(word, lang_code);
				else
					inword = false;
		}

		cur.forwardPos();
		++progress;
	}

	return WordLangTuple(string(), string());
}

} // namespace anon



void ControlSpellchecker::check()
{
	lyxerr[Debug::GUI] << "spell check a word" << endl;

	SpellBase::Result res = SpellBase::OK;

	DocIterator cur = kernel().bufferview()->cursor();

	ptrdiff_t start = 0, total = 0;
	DocIterator it = DocIterator(kernel().buffer().inset());
	for (start = 0; it != cur; it.forwardPos())
		++start;

	for (total = start; it; it.forwardPos())
		++total;

	for (; cur && isLetter(cur); cur.forwardPos())
		++start;

	BufferParams & bufferparams = kernel().buffer().params();

	while (res == SpellBase::OK || res == SpellBase::IGNORED_WORD) {
		word_ = nextWord(cur, start, bufferparams);

		// end of document
		if (getWord().empty()) {
			showSummary();
			return;
		}

		++count_;

		// Update slider if and only if value has changed
		float progress = total ? float(start)/total : 1;
		newvalue_ = int(100.0 * progress);
		if (newvalue_!= oldval_) {
			lyxerr[Debug::GUI] << "Updating spell progress." << endl;
			oldval_ = newvalue_;
			// set progress bar
			dialog().view().partialUpdate(SPELL_PROGRESSED);
		}

		// speller might be dead ...
		if (!checkAlive())
			return;

		res = speller_->check(word_);

		// ... or it might just be reporting an error
		if (!checkAlive())
			return;
	}

	lyxerr[Debug::GUI] << "Found word \"" << getWord() << "\"" << endl;

	int const size = getWord().size();
	cur.pos() -= size;
	kernel().bufferview()->putSelectionAt(cur, size, false);
	// if we used a lfun like in find/replace, dispatch would do
	// that for us
	kernel().bufferview()->update();

	// set suggestions
	if (res != SpellBase::OK && res != SpellBase::IGNORED_WORD) {
		lyxerr[Debug::GUI] << "Found a word needing checking." << endl;
		dialog().view().partialUpdate(SPELL_FOUND_WORD);
	}
}


bool ControlSpellchecker::checkAlive()
{
	if (speller_->alive() && speller_->error().empty())
		return true;

	string message = speller_->error();
	if (message.empty())
		message = _("The spell-checker has died for some reason.\n"
			 "Maybe it has been killed.");

	dialog().CancelButton();

	Alert::error(_("The spell-checker has failed"), message);
	return false;
}


void ControlSpellchecker::showSummary()
{
	if (!checkAlive() || count_ == 0) {
		dialog().CancelButton();
		return;
	}

	string message;
	if (count_ != 1)
		message = bformat(_("%1$d words checked."), count_);
	else
		message = _("One word checked.");

	dialog().CancelButton();
	Alert::information(_("Spell-checking is complete"), message);
}


void ControlSpellchecker::replace(string const & replacement)
{
	lyxerr << "ControlSpellchecker::replace("
	       << replacement << ")" << std::endl;
	BufferView & bufferview = *kernel().bufferview();
	cap::replaceWord(bufferview.cursor(), replacement);
	kernel().buffer().markDirty();
	bufferview.update();
	// fix up the count
	--count_;
	check();
}


void ControlSpellchecker::replaceAll(string const & replacement)
{
	// TODO: add to list
	replace(replacement);
}


void ControlSpellchecker::insert()
{
	speller_->insert(word_);
	check();
}


string const ControlSpellchecker::getSuggestion() const
{
	return speller_->nextMiss();
}


string const ControlSpellchecker::getWord() const
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
