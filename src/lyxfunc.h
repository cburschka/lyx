// -*- C++ -*-
#ifndef LYXFUNC_H
#define LYXFUNC_H

#ifdef __GNUG__
#pragma interface
#endif

#include "commandtags.h"
#include "kbmap.h"
#include "tex-accent.h"
#include "LyXAction.h"
#include "insets/lyxinset.h"
#include "LString.h"
class LyXView;

/** This class encapsulates all the LyX command operations. 
    This is the class of the LyX's "high level event handler".
    Every user command is processed here, either invocated from
    keyboard or from the GUI. All GUI objects, including buttons and
    menus should use this class and never call kernel functions directly.
*/
class LyXFunc {
public:
	///
	LyXFunc(LyXView*);
	///
	~LyXFunc();
    
	/// LyX distpatcher, executes lyx actions.
	string Dispatch(int action, char const* arg = 0);
			 
	/// The same but uses the name of a lyx command.
	string Dispatch(string const &cmd);

	/// A keyboard event is processed to execute a lyx action. 
	int  processKeyEvent(XEvent *ev);
     
	/// The last key was meta
	bool wasMetaKey();

	// These can't be global because are part of the internat state (ale970227)
	/// Get the current keyseq string
	string keyseqStr(int l = 190);

	/// Is the key sequence uncomplete?
	bool keyseqUncomplete();

	/// get options for the current keyseq
	string keyseqOptions(int l = 190);

	/// Returns the name of a funcion given a keyseq
	char const* getFunc(char*) 
	{ /* unimplemented */ return 0; }

	/// Return a string with the keybind to an action, if any
	char const* getKeybind(int) 
	{ /* unimplemented */ return 0; }

        /// True if lyxfunc reports an error
        bool errorStat() const { return errorstat; }
        /// Buffer to store result messages
        void setMessage(string const &m);
        /// Buffer to store result messages
        void setErrorMessage(string const &); 
        /// Buffer to store result messages
        string getMessage() const { return dispatch_buffer; }
        /// Get next inset of this class from current cursor position  
        Inset* getInsetByCode(Inset::Code);
	
	/// Should a hint message be displayed?
	void setHintMessage(bool);

private:
	///
	LyXView *owner;
	///
	static kb_func_table* lyx_func_table;
	///
	static kb_func_table *lyx_func_args;
	///
	static int psd_idx;
 	///
 	kb_sequence keyseq;
 	///
 	kb_sequence cancel_meta_seq;
	///
	unsigned meta_fake_bit;
	///
	void moveCursorUpdate(bool selecting = false);
	///
	void setupLocalKeymap();
        ///
        kb_action lyx_dead_action;
        ///
        kb_action lyx_calling_dead_action;
        /// Error status, only Dispatch can change this flag
        bool errorstat;

        /** Buffer to store messages and result data. Is there a
	  good reason to have this one as static in Dispatch? (Ale)
         */
        string dispatch_buffer;
	/// Command name and shortcut information
	string commandshortcut;

	// I think the following should be moved to BufferView. (Asger)

	///
	void MenuNew(bool fromTemplate);

	///
	void MenuOpen();

	///
	void doImportLaTeX(bool);

	///
	void doImportASCII(bool);

	///
	void MenuInsertLyXFile(string const &);

	///
	void CloseBuffer();
	///
	void reloadBuffer();
	/// This is the same for all lyxfunc objects
	static bool show_sc;
};
     
     
/*--------------------  inlines  --------------------------*/

inline
bool LyXFunc::wasMetaKey() 
{ 
	return (meta_fake_bit != 0);
}
     

inline
string LyXFunc::keyseqStr(int l)
{
	char text[200];
	keyseq.print(text, l, true);
	string tmp(text);
	return tmp;
} 


inline
string LyXFunc::keyseqOptions(int l)
{
	char text[200];
	keyseq.printOptions(text, l);
	string tmp(text);
	return tmp;
} 


inline
bool LyXFunc::keyseqUncomplete() 
{ 
	return (keyseq.length > 0);
}

inline
void LyXFunc::setHintMessage(bool hm) 
{ 
	show_sc = hm;
}

#endif
