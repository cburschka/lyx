/**
 * \file ControlSpellchecker.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "ControlSpellchecker.h"
#include "ViewBase.h"
#include "buffer.h"
#include "BufferView.h"
#include "gettext.h"
#include "language.h"
#include "lyxrc.h"
#include "lyxtext.h"
#include "debug.h"

#include "ispell.h"
#ifdef USE_PSPELL
# include "pspell.h"
#else
#ifdef USE_ASPELL
# include "aspell_local.h"
#endif
#endif

#include "support/tostr.h"

#include "frontends/Alert.h"

using namespace lyx::support;

using std::endl;


ControlSpellchecker::ControlSpellchecker(LyXView & lv, Dialogs & d)
	: ControlDialogBD(lv, d),
	  newval_(0.0), oldval_(0), newvalue_(0), count_(0)
{}


ControlSpellchecker::~ControlSpellchecker()
{}


void ControlSpellchecker::setParams()
{
	lyxerr[Debug::GUI] << "spell setParams" << endl;
	startSession();
}


void ControlSpellchecker::clearParams()
{
	lyxerr[Debug::GUI] << "spell clearParams" << endl;
	endSession();
}


namespace {

SpellBase * getSpeller(BufferParams const & bp)
{
	string lang = (lyxrc.isp_use_alt_lang)
	              ? lyxrc.isp_alt_lang
		      : bp.language->code();

#ifdef USE_ASPELL
	if (lyxrc.use_spell_lib)
		return new ASpell(bp, lang);
#endif
#ifdef USE_PSPELL
	if (lyxrc.use_spell_lib)
		return new PSpell(bp, lang);
#endif

	lang = (lyxrc.isp_use_alt_lang) ?
		lyxrc.isp_alt_lang : bp.language->lang();

	return new ISpell(bp, lang);
}

}


void ControlSpellchecker::startSession()
{
	lyxerr[Debug::GUI] << "spell startSession" << endl;

	if (speller_.get()) {
		lyxerr[Debug::GUI] << "startSession: speller exists" << endl;
		speller_.reset(0);
		return;
	}

	speller_.reset(getSpeller(buffer()->params));

	// reset values to initial
	newval_ = 0.0;
	oldval_ = 0;
	newvalue_ = 0;
	count_ = 0;
	emergency_exit_ = false;

	// start off the check
	if (speller_->error().empty()) {
		check();
		return;
	}

	emergency_exit_ = true;
	string message = speller_->error();
	if (message.empty())
		message = _("The spell-checker could not be started.\n"
			 "Maybe it is mis-configured.");

	Alert::error(_("The spell-checker has failed"), message);
	speller_.reset(0);
}


void ControlSpellchecker::endSession()
{
	lyxerr[Debug::GUI] << "spell endSession" << endl;

	bufferview()->endOfSpellCheck();

	emergency_exit_ = true;

	if (!speller_.get()) {
		lyxerr[Debug::GUI] << "endSession with no speller" << endl;
		return;
	}

	speller_.reset(0);
}


void ControlSpellchecker::check()
{
	lyxerr[Debug::GUI] << "spell check a word" << endl;

	SpellBase::Result res = SpellBase::OK;

	// clear any old selection
	LyXText * text = bufferview()->getLyXText();
	bufferview()->toggleSelection(true);
	bufferview()->update();

	while (res == SpellBase::OK || res == SpellBase::IGNORE) {
		word_ = bufferview()->nextWord(newval_);

		// end of document
		if (word_.word().empty())
			break;

		++count_;

		// Update slider if and only if value has changed
		newvalue_ = int(100.0 * newval_);
		if (newvalue_!= oldval_) {
			lyxerr[Debug::GUI] << "Updating spell progress." << endl;
			oldval_ = newvalue_;
			// set progress bar
			view().partialUpdate(SPELL_PROGRESSED);
		}

		// speller might be dead ...
		if (!checkAlive())
			return;

		res = speller_->check(word_);

		// ... or it might just be reporting an error
		if (!checkAlive())
			return;
	}

	lyxerr[Debug::GUI] << "Found word \"" << word_.word() << "\"" << endl;

	if (!word_.word().empty()) {
		bufferview()->selectLastWord();
	} else {
		showSummary();
		endSession();
		return;
	}

	// set suggestions
	if (res != SpellBase::OK && res != SpellBase::IGNORE) {
		lyxerr[Debug::GUI] << "Found a word needing checking." << endl;
		view().partialUpdate(SPELL_FOUND_WORD);
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

	view().hide();
	speller_.reset(0);

	Alert::error(_("The spell-checker has failed"), message);
	return false;
}


void ControlSpellchecker::showSummary()
{
	if (!checkAlive() || count_ == 0) {
		view().hide();
		return;
	}

	string message;
	if (count_ != 1)
		message = bformat(_("%1$s words checked."), tostr(count_));
	else
		message = _("One word checked.");

	view().hide();
	Alert::information(_("Spell-checking is complete"), message);
}


void ControlSpellchecker::replace(string const & replacement)
{
	bufferview()->replaceWord(replacement);
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
