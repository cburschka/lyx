// -*- C++ -*-
#ifndef LYXFUNC_H
#define LYXFUNC_H

#ifdef __GNUG__
#pragma interface
#endif


#include "FuncStatus.h"
#include "kbsequence.h"
#include "commandtags.h"
#include "LString.h"

#include <boost/signals/trackable.hpp>

class LyXView;
class LyXText;
class FuncRequest;
class BufferView;


/** This class encapsulates all the LyX command operations.
    This is the class of the LyX's "high level event handler".
    Every user command is processed here, either invocated from
    keyboard or from the GUI. All GUI objects, including buttons and
    menus should use this class and never call kernel functions directly.
*/
class LyXFunc : public boost::signals::trackable {
public:
	///
	explicit
	LyXFunc(LyXView *);

	/// LyX dispatcher, executes lyx actions.
	void dispatch(FuncRequest const &, bool verbose = false);

	/// Dispatch via a string argument
	void dispatch(string const & s, bool verbose = false);
 
	/// Dispatch via a pseudo action, also displaying shortcut/command name
	void dispatch(int ac, bool verbose = false);

	/// return the status bar state string
	string const view_status_message();

	///
	void processKeySym(LyXKeySymPtr key, key_modifier::state state);

	/// we need one internal which is called from inside LyXAction and
	/// can contain the string argument.
	FuncStatus getStatus(int ac) const;
	///
	FuncStatus getStatus(FuncRequest const & action) const;

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
	/// Handle a accented char key sequence
	void handleKeyFunc(kb_action action);

private:
	///
	BufferView * view() const;

	///
	LyXView * owner;
	///
	static int psd_idx;
	///
	kb_sequence keyseq;
	///
	kb_sequence cancel_meta_seq;
	///
	key_modifier::state meta_fake_bit;
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

	/// send a post-dispatch status message
	void sendDispatchMessage(string const & msg, FuncRequest const & ev, bool verbose);

	// I think the following should be moved to BufferView. (Asger)

	///
	void menuNew(string const & argument, bool fromTemplate);

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
	return (meta_fake_bit != key_modifier::none);
}


#endif
