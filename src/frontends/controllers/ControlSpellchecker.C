/**
 * \file ControlSpellchecker.C
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Edwin Leuven <leuven@fee.uva.nl>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlSpellchecker.h"
#include "buffer.h"
#include "lyxrc.h"
#include "BufferView.h"
#include "lyxtext.h"
#include "gettext.h"
#include "language.h"

#include "ViewBase.h"
#include "Dialogs.h"
#include "Liason.h"

#include "frontends/LyXView.h"
#include "frontends/Alert.h"

#include "support/lstrings.h"

#include "ispell.h"
#ifdef USE_PSPELL
# include "pspell.h"
#endif

ControlSpellchecker::ControlSpellchecker(LyXView & lv, Dialogs & d)
	: ControlDialogBD(lv, d),
	  rtl_(false), newval_(0.0), oldval_(0), newvalue_(0), count_(0),
	  stop_(false), speller_(0)
{}


void ControlSpellchecker::setParams()
{
	if (!speller_) {
		// create spell object
		string tmp;
#ifdef USE_PSPELL
		if (lyxrc.use_pspell) {
			tmp = (lyxrc.isp_use_alt_lang) ?
				lyxrc.isp_alt_lang : lv_.buffer()->params.language->code();

			speller_ = new PSpell(lv_.view()->buffer()->params, tmp);
		} else {
#endif
			tmp = (lyxrc.isp_use_alt_lang) ?
				lyxrc.isp_alt_lang : lv_.buffer()->params.language->lang();

			speller_ = new ISpell(lv_.view()->buffer()->params, tmp);
#ifdef USE_PSPELL
		}
#endif

		if (lyxrc.isp_use_alt_lang) {
			Language const * lang = languages.getLanguage(tmp);
			if (lang)
				rtl_ = lang->RightToLeft();
		} else {
			rtl_ = lv_.buffer()->params.language->RightToLeft();
		}

		if (!speller_->error().empty()) {
			clearParams();
			return;
		}
	}
}


void ControlSpellchecker::check()
{
	SpellBase::Result res = SpellBase::OK;
	stop_ = false;

	// clear any old selection
	LyXText * text = lv_.view()->getLyXText();
	lv_.view()->toggleSelection(true);
	lv_.view()->update(text, BufferView::SELECT);

	while ((res == SpellBase::OK || res == SpellBase::IGNORE) && !stop_) {
		word_ = lv_.view()->nextWord(newval_);

		if (word_.word().empty()) {
			clearParams();
			break;
		}

		++count_;

		// Update slider if and only if value has changed
		newvalue_ = int(100.0 * newval_);
		if (newvalue_!= oldval_) {
			oldval_ = newvalue_;
			// set progress bar
			view().partialUpdate(0);
		}

		if (!speller_->alive()) {
			clearParams();
			stop();
			return;
		}

		res = speller_->check(word_);
	}

	if (!stop_ && !word_.word().empty())
		lv_.view()->selectLastWord();

	// set suggestions
	if (res != SpellBase::OK && res != SpellBase::IGNORE) {
		view().partialUpdate(1);
	}
}


void ControlSpellchecker::replace(string const & replacement)
{
	lv_.view()->replaceWord(replacement);
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


string ControlSpellchecker::getSuggestion()
{
	string miss(speller_->nextMiss());

	if (rtl_)
		std::reverse(miss.begin(), miss.end());

	return miss;
}


string ControlSpellchecker::getWord()
{
	string tmp = word_.word();
	if (rtl_)
		std::reverse(tmp.begin(), tmp.end());
	return tmp;
}


void ControlSpellchecker::ignoreAll()
{
	speller_->accept(word_);
	check();
}


void ControlSpellchecker::stop()
{
	stop_ = true;
	lv_.view()->endOfSpellCheck();
}


void ControlSpellchecker::clearParams()
{
	if (!speller_) return;

	if (speller_->alive()) {
		speller_->close();
		message_ = tostr(count_);
		if (count_ != 1) {
			message_ += _(" words checked.");

		} else {
			message_ += _(" word checked.");
		}
		message_ = "\n" + message_;
		message_ = _("Spellchecking completed! ") + message_;

	} else {
		message_ = speller_->error();
		speller_->cleanUp();
		if (message_.empty())
		    message_ = _("The spell checker has died for some reason.\n"
				 "Maybe it has been killed.");

		// make sure that the dialog is not launched
		emergency_exit_ = true;
		Alert::alert("The spellchecker has failed", message_);
	}

	delete speller_;

	lv_.view()->endOfSpellCheck();

	// show closing message if any words were checked.
	if (count_ > 0)
		view().partialUpdate(2);

	// reset values to initial
	rtl_ = false;
	newval_ = 0.0;
	oldval_ = 0;
	newvalue_ = 0;
	count_ = 0;
	message_.erase();
	stop_ = false;
	speller_ = 0;
}
