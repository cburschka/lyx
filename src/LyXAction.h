// -*- C++ -*-
#ifndef _LYXACTION_H_
#define _LYXACTION_H_

#ifdef __GNUG__
#pragma interface
#endif

#include "commandtags.h"

class LString;

/// Command name - action   
struct kb_func_table {
	///
	char const *name;
	///
	kb_action action;
};


/** This class encapsulates LyX action and user command operations.
 */
class LyXAction {
public:
	///
	LyXAction();
	///
	~LyXAction();
    
	/** Returns an action tag from a string. Returns kb_action.
	  Include arguments in func_name ONLY if you
	  want to create new pseudo actions. */
	int LookupFunc(char const *func_name); 

        /** Returns an action tag which name is the most similar to a string.
	    Don't include arguments, they would be ignored. */
        int getApproxFunc(char const *func);

        /** Returns an action name the most similar to a string.
	    Don't include arguments, they would be ignored. */
        const char *getApproxFuncName(char const *func);

	/// Returns a pseudo-action given an action and its argument.
	int getPseudoAction(kb_action action, char const *arg);

	/// Retrieves the real action and its argument.
	int retrieveActionArg(int i, char const **arg);
    
	/// Search for an existent pseudoaction, return -1 if it doesn't exist.
	int searchActionArg(kb_action action, char const *arg);

	/// Check if a value is a pseudo-action. 
	bool isPseudoAction(int);
    
	/// Not sure if this function should be here 
	int bindKey(char const* seq, int action);
	
	/// Return the name associated with command
	char const *getActionName(int action) const;

	/// Return one line help text associated with command
	char const *helpText(kb_action action) const;

	 /// True if the command is Read Only (not allowed for RO buffers)
	bool isFuncRO(kb_action action) const;

private:
     
        ///  Standard commands
        static kb_func_table const * lyx_func_table;
	/// Number of commands
	int funcCount;
	///  Pseudoactions
	static kb_func_table *lyx_func_args;
	///  Pseudoaction index
	static int psd_idx; 
	/// Last action index found
	int last_action_idx;
};
     

/* --------------------   Inlines  ------------------ */
    
     
inline
bool LyXAction::isPseudoAction(int a) 
{ 
	return (a >= (int)LFUN_LASTACTION); 
}
     

#endif
