// -*- C++ -*-
/**
 * \file trans_mgr.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Matthias Ettrich
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef TRANS_MANAGER_H
#define TRANS_MANAGER_H

#include "chset.h"
#include "trans_decl.h"

#include <boost/scoped_ptr.hpp>


namespace lyx {

class LCursor;
class LyXText;
class Trans;

/// Translation state
class TransState {
public:
	///
	virtual ~TransState() {}
	///
	virtual docstring const normalkey(char_type) = 0;
	///
	virtual bool backspace() = 0;
	///
	virtual docstring const deadkey(char_type, KmodInfo) = 0;
	///
	static char_type const TOKEN_SEP;
};


/// Translation FSM
class TransFSMData {
protected:
	///
	virtual ~TransFSMData() {}
	///
	char_type deadkey_;
	///
	KmodInfo deadkey_info_;
	///
	char_type deadkey2_;
	///
	KmodInfo deadkey2_info_;
	///
	Keyexc comb_info_;
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
class TransInitState :virtual public TransFSMData, public TransState {
public:
	///
	TransInitState();
	///
	virtual docstring const normalkey(char_type);
	///
	virtual bool backspace() { return true; }
	///
	virtual docstring const deadkey(char_type, KmodInfo);
};


/// Deadkey State
class TransDeadkeyState : virtual public TransFSMData, public TransState {
public:
	///
	TransDeadkeyState();
	///
	virtual docstring const normalkey(char_type);
	///
	virtual bool backspace() {
		currentState = init_state_;
		return false;
	}
	///
	virtual docstring const deadkey(char_type, KmodInfo);
};


/// Combined State
class TransCombinedState : virtual public TransFSMData, public TransState {
public:
	///
	TransCombinedState();
	///
	virtual docstring const normalkey(char_type);
	///
	virtual bool backspace() {
		// cancel the second deadkey
		deadkey2_ = 0;
		deadkey2_info_.accent = TEX_NOACCENT;
		currentState = deadkey_state_;

		return false;
	}
	///
	virtual docstring const deadkey(char_type, KmodInfo);
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
	boost::scoped_ptr<Trans> t1_;
	///
	boost::scoped_ptr<Trans> t2_;
	///
	static Trans default_;
	///
	CharacterSet chset_;
	///
	void insert(docstring const &, LyXText *, LCursor & cur);
	///
	void insertVerbatim(docstring const &, LyXText *, LCursor & cur);
public:
	///
	TransManager();
	///
	~TransManager();
	///
	int setPrimary(std::string const &);
	///
	int setSecondary(std::string const &);
	///
	void enablePrimary();
	///
	void enableSecondary();
	///
	void disableKeymap();
	///
	bool setCharset(std::string const &);
	///
	bool backspace() {
		return trans_fsm_.currentState->backspace();
	}
	///
	void translateAndInsert(char_type, LyXText *, LCursor &);
	///
	docstring const deadkey(char_type, KmodInfo);
	///
	docstring const normalkey(char_type);
	///
	void deadkey(char_type, tex_accent, LyXText *, LCursor &);
};


inline
docstring const TransManager::normalkey(char_type c)
{
	return trans_fsm_.currentState->normalkey(c);
}


inline
docstring const TransManager::deadkey(char_type c, KmodInfo t)
{
	return trans_fsm_.currentState->deadkey(c, t);
}


} // namespace lyx

#endif // TRANS_MANAGER_H
