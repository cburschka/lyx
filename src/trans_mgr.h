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
	///
	virtual string const normalkey(char, string const &) = 0;
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
	///
	KmodException comb_info_;
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
	///
	virtual string const normalkey(char, string const &);
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
	///
	virtual string const normalkey(char, string const &);
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
	///
	virtual string const normalkey(char, string const &);
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
	static Trans * default_;
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
	inline string const deadkey(char, KmodInfo);
	///
	inline string const normalkey(char, string const &);
	///
	void deadkey(char, tex_accent, LyXText *);
};


string const TransManager::normalkey(char c, string const & t)
{
	return trans_fsm_.currentState->normalkey(c, t);
}


string const TransManager::deadkey(char c, KmodInfo t)
{
	return trans_fsm_.currentState->deadkey(c, t);
}

#endif
