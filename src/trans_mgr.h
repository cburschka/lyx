// -*- C++ -*-
#ifndef Trans_Manager_h
#define Trans_Manager_h

#ifdef __GNUG__
#pragma interface
#endif

#include "tex-accent.h"
#include "trans_decl.h"
#include "chset.h"
#include "LString.h"

class LyXText;
class Trans;

/// Translation State
class TransState {
public:
	///
	virtual ~TransState() {}
#if 0
	///
	virtual string const normalkey(char, string const &) = 0;
#else
	///
	virtual string const normalkey(char) = 0;
#endif
	///
	virtual bool backspace() = 0;
	///
	virtual string const deadkey(char, KmodInfo) = 0;
	///
	static char const TOKEN_SEP;
};


/// Translation FSM
class TransFSMData {
protected:
	///
	virtual ~TransFSMData() {}
	///
	char deadkey_;
	///
	KmodInfo deadkey_info_;
	///
	char deadkey2_;
	///
	KmodInfo deadkey2_info_;
#if 0
	///
	KmodException comb_info_;
#else
	///
	Keyexc comb_info_;
#endif
	///
	TransState * init_state_;
	///
	TransState * deadkey_state_;
	///
	TransState * combined_state_;
	///
public:
	///
	TransFSMData();
	///
	TransState * currentState;
};


/// Init State
class TransInitState : 	virtual public TransFSMData, public TransState {
public:
	///
	TransInitState();
#if 0
	///
	virtual string const normalkey(char, string const &);
#else
	///
	virtual string const normalkey(char);
#endif
	///
	virtual bool backspace() { return true; }
	///
	virtual string const deadkey(char, KmodInfo);
};


/// Deadkey State
class TransDeadkeyState : virtual public TransFSMData, public TransState {
public:
	///
	TransDeadkeyState();
#if 0
	///
	virtual string const normalkey(char, string const &);
#else
	///
	virtual string const normalkey(char);
#endif
	///
	virtual bool backspace() {
		currentState = init_state_;
		return false;
	}
	///
	virtual string const deadkey(char, KmodInfo);
};


/// Combined State
class TransCombinedState : virtual public TransFSMData, public TransState {
public:
	///
	TransCombinedState();
#if 0
	///
	virtual string const normalkey(char, string const &);
#else
	///
	virtual string const normalkey(char);
#endif
	///
	virtual bool backspace() {
		// cancel the second deadkey
		deadkey2_ = 0;
		deadkey2_info_.accent = TEX_NOACCENT;
		currentState = deadkey_state_;
		
		return false;
	}
	///
	virtual string const deadkey(char, KmodInfo);
};


///
class TransFSM : virtual public TransFSMData,
		 public TransInitState,
		 public TransDeadkeyState,
		 public TransCombinedState {
public:
	///
	TransFSM();
};


///
class TransManager {
private:
	///
	TransFSM trans_fsm_;
	///
	Trans * active_;
	///
	Trans * t1_;
	///
	Trans * t2_;
	///
	static Trans default_;
	///
	CharacterSet chset_;
	///
	void insert(string const &, LyXText *);
	///
	void insertVerbatim(string const &, LyXText *);
public:
	///
	TransManager();
	///
	virtual ~TransManager();
	///
	int SetPrimary(string const &);
	///
	int SetSecondary(string const &);
	///
	void EnablePrimary();
	///
	void EnableSecondary();
	///
	void DisableKeymap();
	///
	bool setCharset(string const &);
	///
	bool backspace() {
		return trans_fsm_.currentState->backspace();
	}
	///
	void TranslateAndInsert(char, LyXText *);
	///
	string const deadkey(char, KmodInfo);
#if 0
	///
	string const normalkey(char, string const &);
#else
	///
	string const normalkey(char);
#endif
	///
	void deadkey(char, tex_accent, LyXText *);
};


#if 0
inline
string const TransManager::normalkey(char c, string const & t)
{
	return trans_fsm_.currentState->normalkey(c, t);
}
#else
inline
string const TransManager::normalkey(char c)
{
	return trans_fsm_.currentState->normalkey(c);
}
#endif


inline
string const TransManager::deadkey(char c, KmodInfo t)
{
	return trans_fsm_.currentState->deadkey(c, t);
}

#endif
