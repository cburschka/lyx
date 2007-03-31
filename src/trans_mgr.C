/**
 * \file trans_mgr.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Matthias Ettrich
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "trans_mgr.h"

#include "buffer.h"
#include "BufferView.h"
#include "CutAndPaste.h"
#include "cursor.h"
#include "debug.h"
#include "lyxrc.h"
#include "lyxtext.h"
#include "trans.h"

#include "support/lstrings.h"


namespace lyx {

using support::split;

using std::endl;
using std::string;
using std::pair;


// TransFSMData
TransFSMData::TransFSMData()
{
	deadkey_ = deadkey2_ = 0;
	deadkey_info_.accent = deadkey2_info_.accent = TEX_NOACCENT;
}


// TransState
char_type const TransState::TOKEN_SEP = 4;


// TransInitState
TransInitState::TransInitState()
{
	init_state_ = this;
}


docstring const TransInitState::normalkey(char_type c)
{
	docstring res;
	res = c;
	return res;
}


docstring const TransInitState::deadkey(char_type c, KmodInfo d)
{
	deadkey_ = c;
	deadkey_info_ = d;
	currentState = deadkey_state_;
	return docstring();
}


// TransDeadkeyState
TransDeadkeyState::TransDeadkeyState()
{
	deadkey_state_ = this;
}


docstring const TransDeadkeyState::normalkey(char_type c)
{
	docstring res;

	KmodException::iterator it = deadkey_info_.exception_list.begin();
	KmodException::iterator end = deadkey_info_.exception_list.end();

	for (; it != end; ++it) {
		if (it->c == c) {
			res = it->data;
			break;
		}
	}
	if (it == end) {
		res = DoAccent(c, deadkey_info_.accent);
	}
	currentState = init_state_;
	return res;
}


docstring const TransDeadkeyState::deadkey(char_type c, KmodInfo d)
{
	docstring res;

	// Check if the same deadkey was typed twice
	if (deadkey_ == c) {
		res = deadkey_;
		deadkey_ = 0;
		deadkey_info_.accent = TEX_NOACCENT;
		currentState = init_state_;
		return res;
	}

	// Check if it is a combination or an exception
	KmodException::const_iterator cit = deadkey_info_.exception_list.begin();
	KmodException::const_iterator end = deadkey_info_.exception_list.end();
	for (; cit != end; ++cit) {
		if (cit->combined == true && cit->accent == d.accent) {
			deadkey2_ = c;
			deadkey2_info_ = d;
			comb_info_ = (*cit);
			currentState = combined_state_;
			return docstring();
		}
		if (cit->c == c) {
			res = cit->data;
			deadkey_ = 0;
			deadkey_info_.accent = TEX_NOACCENT;
			currentState = init_state_;
			return res;
		}
	}

	// Not a combination or an exception.
	// Output deadkey1 and keep deadkey2

	if (deadkey_!= 0)
		res = deadkey_;
	deadkey_ = c;
	deadkey_info_ = d;
	currentState = deadkey_state_;
	return res;
}


TransCombinedState::TransCombinedState()
{
	combined_state_ = this;
}


docstring const TransCombinedState::normalkey(char_type c)
{
	docstring const temp = DoAccent(c, deadkey2_info_.accent);
	docstring const res = DoAccent(temp, deadkey_info_.accent);
	currentState = init_state_;
	return res;
}


docstring const TransCombinedState::deadkey(char_type c, KmodInfo d)
{
	// Third key in a row. Output the first one and
	// reenter with shifted deadkeys
	docstring res;
	if (deadkey_ != 0)
		res = deadkey_;
	res += TOKEN_SEP;
	deadkey_ = deadkey2_;
	deadkey_info_ = deadkey2_info_;
	res += deadkey_state_->deadkey(c, d);
	return res;
}


// TransFSM
TransFSM::TransFSM():
	TransFSMData(),
	TransInitState(),
	TransDeadkeyState(),
	TransCombinedState()
{
	currentState = init_state_;
}


// TransManager

// Initialize static member.
Trans TransManager::default_;


TransManager::TransManager()
	: active_(0), t1_(new Trans), t2_(new Trans)
{}


// For the sake of boost::scoped_ptr.
TransManager::~TransManager()
{}


int TransManager::setPrimary(string const & language)
{
	if (t1_->getName() == language)
		return 0;

	return t1_->load(language);
}


int TransManager::setSecondary(string const & language)
{
	if (t2_->getName() == language)
		return 0;

	return t2_->load(language);
}


bool TransManager::setCharset(string const & str)
{
	return chset_.loadFile(str);
}


void TransManager::enablePrimary()
{
	if (t1_->isDefined())
		active_ = t1_.get();

	lyxerr[Debug::KBMAP] << "Enabling primary keymap" << endl;
}


void TransManager::enableSecondary()
{
	if (t2_->isDefined())
		active_ = t2_.get();
	lyxerr[Debug::KBMAP] << "Enabling secondary keymap" << endl;
}


void TransManager::disableKeymap()
{
	active_ = &default_;
	lyxerr[Debug::KBMAP] << "Disabling keymap" << endl;
}


void  TransManager::translateAndInsert(char_type c, LyXText * text, LCursor & cur)
{
	docstring res = active_->process(c, *this);

	// Process with tokens
	docstring temp;

	while (res.length() > 0) {
		res = split(res, temp, TransState::TOKEN_SEP);
		insert(temp, text, cur);
	}
}


void TransManager::insertVerbatim(docstring const & str, LyXText * text, LCursor & cur)
{
	for (string::size_type i = 0, n = str.size(); i < n; ++i)
		text->insertChar(cur, str[i]);
}


void TransManager::insert(docstring const & str, LyXText * text, LCursor & cur)
{
	// Go through the character encoding only if the current
	// encoding (chset_->name()) matches the current font_norm
	// (lyrxc->font_norm)

	// Is false to speak about "only if" the current encoding will
	// almost always be equal to font_norm.
	pair<bool, int> enc = chset_.encodeString(to_utf8(str));
	if (chset_.getName() != lyxrc.font_norm ||
	    !enc.first) {
		// Could not find an encoding
		insertVerbatim(str, text, cur);
		return;
	}
	text->insertChar(cur, enc.second);
}


void TransManager::deadkey(char_type c, tex_accent accent, LyXText * t, LCursor & cur)
{
	if (c == 0 && active_ != &default_) {
		// A deadkey was pressed that cannot be printed
		// or a accent command was typed in the minibuffer
		KmodInfo i;
		if (active_->isAccentDefined(accent, i) == true) {
			docstring const res = trans_fsm_
				.currentState->deadkey(c, i);
			insert(res, t, cur);
			return;
		}
	}

	if (active_ == &default_ || c == 0) {
		KmodInfo i;
		i.accent = accent;
		i.data.erase();
		docstring res = trans_fsm_.currentState->deadkey(c, i);
		insert(res, t, cur);
	} else {
		// Go through the translation
		translateAndInsert(c, t, cur);
	}
}


} // namespace lyx
