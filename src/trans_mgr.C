
#ifdef __GNUG__
#pragma implementation "trans_mgr.h"
#endif

#include <assert.h>
#include "trans_mgr.h"
#include "trans.h"
#include "lyxtext.h"
#include "LString.h"
#include "error.h"
#include "chset.h"
#include "insets/insetlatexaccent.h"
#include "BufferView.h"
#include "buffer.h"
#include "lyxrc.h"

extern LyXRC* lyxrc;
extern LString DoAccent(const LString&,tex_accent);
extern void InsertCorrectQuote();
extern LString DoAccent(char,tex_accent);
extern BufferView* current_view;


// TransFSMData
TransFSMData::TransFSMData()
{
    deadkey_=deadkey2_=0;
    deadkey_info_.accent=deadkey2_info_.accent=TEX_NOACCENT;
    comb_info_=NULL;
}


// TransState
const char TransState::TOKEN_SEP=4;


TransState::~TransState()
{}


// TransInitState
TransInitState::TransInitState()
{
    init_state_=this;
}


LString TransInitState::normalkey(char c,char *t)
{
    LString res;
    if (t!=NULL)
	res=t;
    else
	res=c;
	
    return res;
}


//bool TransInitState::backspace()
//{
//    return true;
//}


LString TransInitState::deadkey(char c,KmodInfo d)
{
    deadkey_=c;
    deadkey_info_=d;
    currentState=deadkey_state_;
    return LString();
}


// TransDeadkeyState
TransDeadkeyState::TransDeadkeyState()
{
    deadkey_state_=this;
}


LString TransDeadkeyState::normalkey(char c,char *trans)
{
    LString res;
    
    // Check if it is an exception
    KmodException l=deadkey_info_.exception_list;
    while(l!=NULL) {
	if (l->c==c) {
	    res=l->data;
	    break;
	}
	l=l->next;
    }
    if (l==NULL) {
	// Not an exception. Check if it allowed
	if (current_view->currentBuffer()->params.allowAccents==true ||
	    deadkey_info_.allowed.countChar(c)>0) {
	    res=DoAccent(c,deadkey_info_.accent);
	} else {
	    // Not allowed
	    if (deadkey_!=0)
		res=deadkey_;
	    res+=TOKEN_SEP;
	    res+=trans;
	}
    }
    currentState=init_state_;
    return res;
}


//bool TransDeadkeyState::backspace()
//{
//    currentState=init_state_;
//    return false;
//}


LString TransDeadkeyState::deadkey(char c,KmodInfo d)
{
    LString res;
    
    // Check if the same deadkey was typed twice
    if (deadkey_==c) {
	res=deadkey_;
	deadkey_=0;
	deadkey_info_.accent=TEX_NOACCENT;
	currentState=init_state_;
	return res;
    }
    
    // Check if it is a combination or an exception
    KmodException l;
    l=deadkey_info_.exception_list;
    
    while(l) {
	if (l->combined==true && l->accent==d.accent) {
	    deadkey2_=c;
	    deadkey2_info_=d;
	    comb_info_=l;
	    currentState=combined_state_;
	    return LString();
	}
	if (l->c==c) {
	    res=l->data;
	    deadkey_=0;
	    deadkey_info_.accent=TEX_NOACCENT;
	    currentState=init_state_;
	    return res;
	}
	
	l=l->next;
    }
    
    // Not a combination or an exception. 
    // Output deadkey1 and keep deadkey2
    
    if (deadkey_!=0)
	res=deadkey_;
    deadkey_=c;
    deadkey_info_=d;
    currentState=deadkey_state_;
    return res;
}


TransCombinedState::TransCombinedState()
{
    combined_state_=this;
}


LString TransCombinedState::normalkey(char c,char *trans)
{
    LString res;
    
    // Check if the key is allowed on the combination
    if (comb_info_->data.countChar(c)>0) {
	LString temp;
	temp=DoAccent(c,deadkey2_info_.accent);
	res=DoAccent(temp,deadkey_info_.accent);
	currentState=init_state_;
    } else {
	// Not allowed. Output deadkey1 and check deadkey2 + c
	if (deadkey_!=0)
	    res+=deadkey_;
	res+=TOKEN_SEP;
	deadkey_=deadkey2_;
	deadkey_info_=deadkey2_info_;
	// Call deadkey state and leave it to setup the FSM
	res+=deadkey_state_->normalkey(c,trans);
    }
    return res;
}


// bool TransCombinedState::backspace()
// {
//     // cancel the second deadkey
//     deadkey2_=0;
//     deadkey2_info_.accent=TEX_NOACCENT;
//     currentState=deadkey_state_;
    
//     return false;
// }


LString TransCombinedState::deadkey(char c,KmodInfo d)
{
    // Third key in a row. Output the first one and
    // reenter with shifted deadkeys
    LString res;
    if (deadkey_!=0)
	res=deadkey_;
    res+=TOKEN_SEP;
    deadkey_=deadkey2_;
    deadkey_info_=deadkey2_info_;
    res+=deadkey_state_->deadkey(c,d);
    return res;
}


// TransFSM
TransFSM::TransFSM():
    TransFSMData(),
    TransInitState(),
    TransDeadkeyState(),
    TransCombinedState()
{
    currentState=init_state_;
}


// TransManager
    
TransManager::TransManager()
    : active_(NULL),t1_(new Trans),t2_(new Trans),chset_(new CharacterSet)
{}


Trans* TransManager::default_=new Trans;


TransManager::~TransManager() 
{ 
    delete t1_;
    delete t2_;
    delete chset_;
}


int TransManager::SetPrimary(LString const & language)
{
    if (t1_->GetName()==language) 
	return 0;
	
    return t1_->Load(language);
}


int TransManager::SetSecondary(LString const & language)
{
    if (t2_->GetName()==language)
	return 0;
	
    return t2_->Load(language);
}


bool TransManager::setCharset(const char *set)
{
    return chset_->loadFile(set);
}


void TransManager::EnablePrimary()
{
    if (t1_->IsDefined())
	active_=t1_;

    lyxerr.debug("Enabling primary keymap",Error::KBMAP);
}


void TransManager::EnableSecondary()
{
    if (t2_->IsDefined( ))
	active_=t2_;
    lyxerr.debug("Enabling secondary keymap",Error::KBMAP);
}


void TransManager::DisableKeymap()
{
    active_=default_;
    lyxerr.debug("Disabling keymap",Error::KBMAP);
}


void  TransManager::TranslateAndInsert(char c,LyXText *text)
{
    LString res;
	
    res=active_->process(c,*this);
    
    // Process with tokens
    LString temp;
	
    while(res.length()>0) {
	res.split(temp,TransState::TOKEN_SEP);
	insert(temp,text);
    }
}


void TransManager::insertVerbatim(const LString& str,LyXText *text)
{	
    int l=str.length();
	
    for (int i=0;i<l;i++){
	if (str[i]=='\"' 
	    && text->GetFont(text->cursor.par,
			     text->cursor.pos).latex() == LyXFont::OFF)
	    InsertCorrectQuote();
	else
	    text->InsertChar(str[i]);
    }
}


void TransManager::insert(LString str,LyXText *text)
{
    LString t=str;
    
    // Go through the character encoding only if the current 
    // encoding (chset_->name()) matches the current font_norm
    // (lyrxc->font_norm

    if (chset_->getName()!=lyxrc->font_norm || 
	chset_->encodeString(str)==false) {
	// Could not find an encoding
	InsetLatexAccent ins(str);
	if (ins.CanDisplay()) {
	    text->InsertInset(new InsetLatexAccent(ins));
	    return;
	}
    }
    
    insertVerbatim(str,text);
}


//bool TransManager::backspace()
//{
//    return trans_fsm_.currentState->backspace();
//}


void TransManager::deadkey(char c,tex_accent accent,LyXText *t)
{
    KmodInfo i;
    LString res;
    
    if (c==0 && active_!=default_) {
	// A deadkey was pressed that cannot be printed
	// or a accent command was typed in the minibuffer
	
	if (active_->isAccentDefined(accent,i)==true) {
	    res=trans_fsm_.currentState->deadkey(c,i);
	    insert(res,t);
	    return;
	}
    }// else {
	// Fall through to the next case
    //}
    
    if (active_==default_ || c==0) {
	i.accent=accent;
	i.allowed=lyx_accent_table[accent].native;
	i.data.clean();
	i.exception_list=NULL;
	
	LString res=trans_fsm_.currentState->deadkey(c,i);
	insert(res,t);
    } else {
	// Go through the translation
	TranslateAndInsert(c,t);
    }
}
