// -*- C++ -*-
#ifndef _Trans_Manager_h_
#define _Trans_Manager_h_

#ifdef __GNUG__
#pragma interface
#endif

#include "tex-accent.h"
#include "trans_decl.h"

class LyXText;
class LString;
class CharacterSet;
class Trans;

/// Translation State
class TransState {
public:
	///
	virtual ~TransState();
	///
	virtual LString normalkey(char,char*)=0;
	///
	virtual bool backspace()=0;
	///
	virtual LString deadkey(char,KmodInfo)=0;
	///
	static const char TOKEN_SEP;
};


/// Translation FSM
class TransFSMData {
protected:
	///
	virtual ~TransFSMData()
	{}
	///
	char deadkey_;
	///
	KmodInfo deadkey_info_;
	///
	char deadkey2_;
	///
	KmodInfo deadkey2_info_;
	///
	KmodException comb_info_;
	///
	TransState* init_state_;
	///
	TransState* deadkey_state_;
	///
	TransState* combined_state_;
	///
public:
	///
	TransFSMData();
	///
	TransState* currentState;
};


/// Init State
class TransInitState: 
	virtual public TransFSMData,
	public TransState {
public:
		///
		TransInitState();
		///
		virtual LString normalkey(char,char*);
		///
		virtual bool backspace() { return true; }
		///
		virtual LString deadkey(char,KmodInfo);
};


/// Deadkey State
class TransDeadkeyState: 
	virtual public TransFSMData,
	public TransState {
public:
		///
		TransDeadkeyState();
		///
		virtual LString normalkey(char,char*);
		///
		virtual bool backspace()
	{
		currentState=init_state_;
		return false;
	}
		///
		virtual LString deadkey(char,KmodInfo);
};


/// Combined State
class TransCombinedState: 
	virtual public TransFSMData,
	public TransState {
public:
		///
		TransCombinedState();
		///
		virtual LString normalkey(char,char*);
		///
		virtual bool backspace()
	{
		// cancel the second deadkey
		deadkey2_=0;
		deadkey2_info_.accent=TEX_NOACCENT;
		currentState=deadkey_state_;
		
		return false;
	}
		///
		virtual LString deadkey(char,KmodInfo);
};


///
class TransFSM:	
	virtual public TransFSMData,
	public TransInitState,
	public TransDeadkeyState,
	public TransCombinedState
{
public:
	///
	TransFSM();
};

///
class TransManager
{
private:
	///
	TransFSM trans_fsm_;
	///
	Trans* active_;
	///
	Trans* t1_;
	///
	Trans* t2_;
	///
	static Trans* default_;
	///
	CharacterSet* chset_;
	///
	void insert(LString,LyXText*);
	///
	void insertVerbatim(const LString&,LyXText*);
public:
	///
	TransManager();
	///
	virtual ~TransManager();
	///
	int SetPrimary(LString const &);
	///
	int SetSecondary(LString const &);
	///
	void EnablePrimary();
	///
	void EnableSecondary();
	///
	void DisableKeymap();
	///
	bool setCharset(const char*);
	///
	bool backspace()
	{
		return trans_fsm_.currentState->backspace();
	}
	///
	void TranslateAndInsert(char,LyXText*);
	///
	inline LString deadkey(char,KmodInfo);
	///
	inline LString normalkey(char,char*);
	///
	void deadkey(char,tex_accent,LyXText*);
    
};


LString TransManager::normalkey(char c,char *t)
{
	return trans_fsm_.currentState->normalkey(c,t);
}


LString TransManager::deadkey(char c,KmodInfo t)
{
	return trans_fsm_.currentState->deadkey(c,t);
}

#endif
