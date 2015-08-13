// -*- C++ -*-
/**
 * \file Trans.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Matthias Ettrich
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef TRANS_H
#define TRANS_H

#include "FuncCode.h"

#include "support/docstring.h"

#include <list>
#include <map>


namespace lyx {

class Cursor;
class Text;
class Lexer;
class TransManager;

///
enum tex_accent {
	///
	TEX_NOACCENT = 0,
	///
	TEX_ACUTE,
	///
	TEX_GRAVE,
	///
	TEX_MACRON,
	///
	TEX_TILDE,
	///
	TEX_PERISPOMENI,
	///
	TEX_UNDERBAR,
	///
	TEX_CEDILLA,
	///
	TEX_UNDERDOT,
	///
	TEX_CIRCUMFLEX,
	///
	TEX_CIRCLE,
	///
	TEX_TIE,
	///
	TEX_BREVE,
	///
	TEX_CARON,
//  TEX_SPECIAL_CARON,
	///
	TEX_HUNGUML,
	///
	TEX_UMLAUT,
	///
	TEX_DOT,
	///
	TEX_OGONEK,
	///
	TEX_MAX_ACCENT = TEX_OGONEK
};


struct TeXAccent {
	///
	tex_accent accent;
	/// UCS4 code point of this accent
	char_type ucs4;
	///
	char const * name;
	///
	FuncCode action;
};

///
extern TeXAccent get_accent(FuncCode action);


///
struct Keyexc {
	/// character to make exception
	char_type c;
	/// exception data
	docstring data;
	/// Combination with another deadkey
	bool combined;
	/// The accent comined with
	tex_accent accent;
};

///
typedef std::list<Keyexc> KmodException;

///
class KmodInfo {
public:
	///
	docstring data;
	///
	tex_accent accent;
	///
	KmodException exception_list;
};


/////////////////////////////////////////////////////////////////////
//
// Trans: holds a .kmap file
//
/////////////////////////////////////////////////////////////////////

class Trans {
public:
	///
	Trans() {}
	///
	~Trans() { freeKeymap(); }

	///
	int load(std::string const & language);
	///
	bool isDefined() const;
	///
	std::string const & getName() const { return name_; }
	///
	docstring const process(char_type, TransManager &);
	///
	bool isAccentDefined(tex_accent, KmodInfo &) const;

private:
	///
	void addDeadkey(tex_accent, docstring const &);
	///
	void freeKeymap();
	///
	int load(Lexer &);
	///
	docstring const & match(char_type c);
	///
	void insertException(KmodException & exclist, char_type c,
			     docstring const & data, bool = false,
			     tex_accent = TEX_NOACCENT);
	///
	void freeException(KmodException & exclist);

	///
	std::string name_;
	///
	std::map<char_type, docstring> keymap_;
	///
	std::map<tex_accent, KmodInfo> kmod_list_;
};


///
inline docstring const & Trans::match(char_type c)
{
	std::map<char_type, docstring>::iterator it = keymap_.find(c);
	if (it != keymap_.end()) {
		return it->second;
	}
	static docstring dummy;
	return dummy;
}


/////////////////////////////////////////////////////////////////////
//
// TransState
//
/////////////////////////////////////////////////////////////////////

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
class TransInitState : virtual public TransFSMData, public TransState {
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



/////////////////////////////////////////////////////////////////////
//
// TransManager
//
/////////////////////////////////////////////////////////////////////

class TransManager {
private:
	///
	TransFSM trans_fsm_;
	///
	Trans * active_;
	///
	Trans t1_;
	///
	Trans t2_;
	///
	static Trans default_;
	///
	void insert(docstring const &, Text *, Cursor & cur);
public:
	///
	TransManager();
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
	bool backspace() { return trans_fsm_.currentState->backspace(); }
	///
	void translateAndInsert(char_type, Text *, Cursor &);
	///
	docstring const deadkey(char_type c, KmodInfo t)
		{ return trans_fsm_.currentState->deadkey(c, t); }
	///
	docstring const normalkey(char_type c)
		{ return trans_fsm_.currentState->normalkey(c); }
	///
	void deadkey(char_type, tex_accent, Text *, Cursor &);
};

} // namespace lyx

#endif // TRANS_H
