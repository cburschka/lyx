// -*- C++ -*-
#ifndef LYXFUNC_H
#define LYXFUNC_H

#ifdef __GNUG__
#pragma interface
#endif

#include <X11/Xlib.h>
#include <sigc++/signal_system.h>

#include "commandtags.h" // for kb_action enum
#include "FuncStatus.h"
#include "kbsequence.h"
#include "LString.h"

class LyXView;
class LyXText;


/** This class encapsulates all the LyX command operations. 
    This is the class of the LyX's "high level event handler".
    Every user command is processed here, either invocated from
    keyboard or from the GUI. All GUI objects, including buttons and
    menus should use this class and never call kernel functions directly.
*/
class LyXFunc : public SigC::Object {
public:
	///
	explicit
	LyXFunc(LyXView *);
    
	/// LyX dispatcher, executes lyx actions.
	string const dispatch(kb_action ac, string argument = string());

	/// The same as dispatch, but also shows shortcuts and command
	/// name in minibuffer if show_sc is true (more to come?) 
	void verboseDispatch(kb_action action,
			     string const & argument,
			     bool show_sc);
	
	/// Same as above, using a pseudoaction as argument
	void verboseDispatch(int ac, bool show_sc);

	/// Same as above, when the command is provided as a string
	void verboseDispatch(string const & s, bool show_sc);

	/// 
	void miniDispatch(string const & s);

	///
	void initMiniBuffer();
		
	///
	void processKeySym(KeySym k, unsigned int state);

	/// we need one internal which is called from inside LyXAction and
	/// can contain the string argument.
	FuncStatus getStatus(int ac) const;
	///
	FuncStatus getStatus(kb_action action, 
			     string const & argument = string()) const;
	
	/// The last key was meta
	bool wasMetaKey() const;

        /// True if lyxfunc reports an error
        bool errorStat() const { return errorstat; }
        /// Buffer to store result messages
        void setMessage(string const & m) const;
        /// Buffer to store result messages
        void setErrorMessage(string const &) const; 
        /// Buffer to store result messages from getStatus
        void setStatusMessage(string const &) const; 
        /// Buffer to store result messages
        string const getMessage() const { return dispatch_buffer; }
        /// Buffer to store result messages
        string const getStatusMessage() const { return status_buffer; }
	/// Handle a accented char keysequenze
	void handleKeyFunc(kb_action action);

private:
	///
	LyXView * owner;
	///
	static int psd_idx;
 	///
 	kb_sequence keyseq;
 	///
 	kb_sequence cancel_meta_seq;
	///
	unsigned meta_fake_bit;
	///
	void moveCursorUpdate(bool flag = true, bool selecting = false);
	///
	void setupLocalKeymap();
        ///
        kb_action lyx_dead_action;
        ///
        kb_action lyx_calling_dead_action;
        /// Error status, only Dispatch can change this flag
        mutable bool errorstat;

        /** Buffer to store messages and result data. Is there a
	    good reason to have this one as static in Dispatch? (Ale)
	*/
        mutable string dispatch_buffer;
        /** Buffer to store messages and result data from getStatus
	*/
        mutable string status_buffer;
	/// Command name and shortcut information
	string commandshortcut;

	// I think the following should be moved to BufferView. (Asger)

	///
	void menuNew(bool fromTemplate);

	///
	void open(string const &);

	///
	void doImport(string const &);

	///
	void closeBuffer();
	///
	void reloadBuffer();
	///
	//  This return or directly text (default) of getLyXText()
	///
	LyXText * TEXT(bool) const;
	///
};
     
     
/*--------------------  inlines  --------------------------*/

inline
bool LyXFunc::wasMetaKey() const 
{ 
	return (meta_fake_bit != 0);
}
     

#endif
