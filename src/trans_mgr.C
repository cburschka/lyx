#include <config.h>

#ifdef __GNUG__
#pragma implementation "trans_mgr.h"
#endif

#include "trans_mgr.h"
#include "trans.h"
#include "lyxtext.h"
#include "LString.h"
#include "debug.h"
#include "chset.h"
#include "insets/insetlatexaccent.h"
#include "BufferView.h"
#include "buffer.h"
#include "lyxrc.h"
#include "support/lstrings.h"

using std::endl;
using std::pair;

extern string const DoAccent(string const &, tex_accent);
extern string const DoAccent(char, tex_accent);
extern BufferView * current_view;


// TransFSMData
TransFSMData::TransFSMData()
{
	deadkey_ = deadkey2_ = 0;
	deadkey_info_.accent = deadkey2_info_.accent = TEX_NOACCENT;
	comb_info_ = 0;
}


// TransState
char const TransState::TOKEN_SEP = 4;


// TransInitState
TransInitState::TransInitState()
{
	init_state_ = this;
}


string const TransInitState::normalkey(char c, string const & t)
{
	string res;
	if (!t.empty()) res = t;
	else res = c;
	
	return res;
}


string const TransInitState::deadkey(char c, KmodInfo d)
{
	deadkey_ = c;
	deadkey_info_ = d;
	currentState = deadkey_state_;
	return string();
}


// TransDeadkeyState
TransDeadkeyState::TransDeadkeyState()
{
	deadkey_state_ = this;
}


string const TransDeadkeyState::normalkey(char c, string const & trans)
{
	string res;
	
	// Check if it is an exception
	KmodException l = deadkey_info_.exception_list;
	while(l != 0) {
		if (l->c == c) {
			res = l->data;
			break;
		}
		l = l->next;
	}
	if (l == 0) {
		// Not an exception. Check if it allowed
		if (countChar(deadkey_info_.allowed, c) > 0) {
			res = DoAccent(c, deadkey_info_.accent);
		} else {
			// Not allowed
			if (deadkey_!= 0)
				res = deadkey_;
			res+= TOKEN_SEP;
			res+= trans;
		}
	}
	currentState = init_state_;
	return res;
}


string const TransDeadkeyState::deadkey(char c, KmodInfo d)
{
	string res;
	
	// Check if the same deadkey was typed twice
	if (deadkey_ == c) {
		res = deadkey_;
		deadkey_ = 0;
		deadkey_info_.accent = TEX_NOACCENT;
		currentState = init_state_;
		return res;
	}
	
	// Check if it is a combination or an exception
	KmodException l;
	l = deadkey_info_.exception_list;
	
	while(l) {
		if (l->combined == true && l->accent == d.accent) {
			deadkey2_ = c;
			deadkey2_info_ = d;
			comb_info_ = l;
			currentState = combined_state_;
			return string();
		}
		if (l->c == c) {
			res = l->data;
			deadkey_ = 0;
			deadkey_info_.accent = TEX_NOACCENT;
			currentState = init_state_;
			return res;
		}
		l = l->next;
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


string const TransCombinedState::normalkey(char c, string const & trans)
{
	string res;
	
	// Check if the key is allowed on the combination
	if (countChar(comb_info_->data, c) > 0) {
		string temp;
		temp = DoAccent(c, deadkey2_info_.accent);
		res = DoAccent(temp, deadkey_info_.accent);
		currentState = init_state_;
	} else {
		// Not allowed. Output deadkey1 and check deadkey2 + c
		if (deadkey_ != 0)
			res += deadkey_;
		res += TOKEN_SEP;
		deadkey_ = deadkey2_;
		deadkey_info_ = deadkey2_info_;
		// Call deadkey state and leave it to setup the FSM
		res += deadkey_state_->normalkey(c, trans);
	}
	return res;
}


string const TransCombinedState::deadkey(char c, KmodInfo d)
{
	// Third key in a row. Output the first one and
	// reenter with shifted deadkeys
	string res;
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

TransManager::TransManager()
	: active_(0), t1_(new Trans), t2_(new Trans)
{}


Trans * TransManager::default_ = new Trans;


TransManager::~TransManager() 
{ 
	delete t1_;
	delete t2_;
}


int TransManager::SetPrimary(string const & language)
{
	if (t1_->GetName() == language) 
		return 0;
	
	return t1_->Load(language);
}


int TransManager::SetSecondary(string const & language)
{
	if (t2_->GetName() == language)
		return 0;
	
	return t2_->Load(language);
}


bool TransManager::setCharset(string const & str)
{
	return chset_.loadFile(str);
}


void TransManager::EnablePrimary()
{
	if (t1_->IsDefined())
		active_ = t1_;
	
	lyxerr[Debug::KBMAP] << "Enabling primary keymap" << endl;
}


void TransManager::EnableSecondary()
{
	if (t2_->IsDefined())
		active_ = t2_;
	lyxerr[Debug::KBMAP] << "Enabling secondary keymap" << endl;
}


void TransManager::DisableKeymap()
{
	active_ = default_;
	lyxerr[Debug::KBMAP] << "Disabling keymap" << endl;
}


void  TransManager::TranslateAndInsert(char c, LyXText * text)
{
	string res = active_->process(c, *this);
	
	// Process with tokens
	string temp;
	
	while(res.length() > 0) {
		res = split(res, temp, TransState::TOKEN_SEP);
		insert(temp, text);
	}
}


void TransManager::insertVerbatim(string const & str, LyXText * text)
{	
	int const l = str.length();
	
	for (int i = 0; i < l; ++i){
		if (str[i] == '\"' 
		    && text->GetFont(current_view->buffer(),text->cursor.par(),
				     text->cursor.pos()).latex() == LyXFont::OFF
		    && text->GetFont(current_view->buffer(),text->cursor.par(),
				     text->cursor.pos()).language()->lang() != "hebrew")
			current_view->insertCorrectQuote();
		else
			text->InsertChar(current_view, str[i]);
	}
}


void TransManager::insert(string const & str, LyXText * text)
{
	// Go through the character encoding only if the current 
	// encoding (chset_->name()) matches the current font_norm
	// (lyrxc->font_norm
	
	// Is false to speak about "only if" the current encoding will
	// almost always be equal to font_norm.
	pair<bool, int> enc = chset_.encodeString(str);
	if (chset_.getName() != lyxrc.font_norm || 
	    !enc.first) {
		// Could not find an encoding
		InsetLatexAccent ins(str);
		if (ins.CanDisplay()) {
			text->InsertInset(current_view, new InsetLatexAccent(ins));
		} else {
			insertVerbatim(str, text);
		}
		return;
	}
	string tmp; tmp += static_cast<char>(enc.second);
	insertVerbatim(tmp, text);
}


void TransManager::deadkey(char c, tex_accent accent, LyXText * t)
{
	if (c == 0 && active_ != default_) {
		// A deadkey was pressed that cannot be printed
		// or a accent command was typed in the minibuffer
		KmodInfo i;
		if (active_->isAccentDefined(accent, i) == true) {
			string res = trans_fsm_.currentState->deadkey(c, i);
			insert(res, t);
			return;
		}
	}
	
	if (active_ == default_ || c == 0) {
		KmodInfo i;
		i.accent = accent;
		i.allowed = lyx_accent_table[accent].native;
		i.data.erase();
		i.exception_list = 0;
		
		string res = trans_fsm_.currentState->deadkey(c, i);
		insert(res, t);
	} else {
		// Go through the translation
		TranslateAndInsert(c, t);
	}
}
