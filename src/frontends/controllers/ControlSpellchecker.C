/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file ControlSpellchecker.C
 * \author Edwin Leuven <leuven@fee.uva.nl>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include FORMS_H_LOCATION

#ifdef HAVE_SYS_SELECT_H
# ifdef HAVE_STRINGS_H
   // <strings.h> is needed at least on AIX because FD_ZERO uses bzero().
   // BUT we cannot include both string.h and strings.h on Irix 6.5 :(
#  ifdef _AIX
#   include <strings.h>
#  endif
# endif
#include <sys/select.h>
#endif

#include "buffer.h"
#include "lyxrc.h"
#include "BufferView.h"
#include "LyXView.h" 
#include "gettext.h"
#include "support/lstrings.h"
#include "language.h"

#include "ViewBase.h"
#include "ButtonControllerBase.h"
#include "ControlSpellchecker.h"
#include "Dialogs.h"
#include "Liason.h"

# include "sp_ispell.h"
#ifdef USE_PSPELL
# include "sp_pspell.h"
#endif

#include "debug.h"

using SigC::slot;

ControlSpellchecker::ControlSpellchecker(LyXView & lv, Dialogs & d)
	: ControlDialog<ControlConnectBD>(lv, d)
{
	d_.showSpellchecker.connect(SigC::slot(this, &ControlSpellchecker::show));

	rtl_ = false;
	word_ = "";
	newval_ = 0.0;
	oldval_ = 0;
	newvalue_ = 0;
	count_ = 0;
	message_ = "";
	stop_ = false;
	result_ = SpellBase::ISP_UNKNOWN;
	speller_ = 0;
	
}


ControlSpellchecker::~ControlSpellchecker()
{
}


void ControlSpellchecker::show()
{
	if (isBufferDependent() && !lv_.view()->available())
		return;

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
		
		if (speller_->error() != 0) {
			message_ = speller_->error();
			view().partialUpdate(2);
			hide();
			return;
		}
	}

	bc().readOnly(isReadonly());
	view().show();
}


void ControlSpellchecker::hide()
{
	delete speller_;
	speller_ = 0;

	disconnect();
	view().hide();
}


void ControlSpellchecker::check()
{
	result_ = SpellBase::ISP_UNKNOWN;
	stop_ = false;
	
	while (result_!=SpellBase::ISP_MISSED && !stop_) {
		word_ = lv_.view()->nextWord(newval_);
		if (word_.empty()) {
			quit();
			break;
		}
		
		++count_;
		
		// Update slider if and only if value has changed
		newvalue_ = int(100.0*newval_);
		if (newvalue_!= oldval_) {
			oldval_ = newvalue_;
			// set progress bar
			view().partialUpdate(0);
		}
		
		if (!speller_->alive()) quit();
		
		result_ = speller_->check(word_);
	}
	
	if (!stop_ && !word_.empty())
		lv_.view()->selectLastWord();

	// set suggestions
	if (result_==SpellBase::ISP_MISSED) {
		view().partialUpdate(1);
	}
}


void ControlSpellchecker::replace(string const & replacement)
{
	lv_.view()->replaceWord(replacement);
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
}


string ControlSpellchecker::getSuggestion()
{
	// this is needed because string tmp = nextmiss()
	// segfaults when nextMiss is 0
	string tmp;
	char const * w = speller_->nextMiss();
	
	if (w!=0) {
		tmp = w;
		if (rtl_) std::reverse(tmp.begin(), tmp.end());
	}
	
	return tmp;
}


string ControlSpellchecker::getWord()
{
	string tmp = word_;
	if (rtl_) std::reverse(tmp.begin(), tmp.end());
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
}


void ControlSpellchecker::quit()
{
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
		speller_->cleanUp();
		message_ = _("The spell checker has died for some reason.\n"
				  "Maybe it has been killed.");
	}

	lv_.view()->endOfSpellCheck();

	// hide dialog, disconnect and delete speller
	hide();
	
	// show closing message
	view().partialUpdate(2);

	// reset values to initial
	rtl_ = false;
	word_ = "";
	newval_ = 0.0;
	oldval_ = 0;
	newvalue_ = 0;
	count_ = 0;
	message_ = "";
	stop_ = false;
	result_ = SpellBase::ISP_UNKNOWN;
}


void ControlSpellchecker::options()
{
	lv_.getDialogs()->showSpellcheckerPreferences();
}


