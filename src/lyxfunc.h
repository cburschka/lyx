// -*- C++ -*-
#ifndef LYXFUNC_H
#define LYXFUNC_H

#ifdef __GNUG__
#pragma interface
#endif

#include FORMS_H_LOCATION
#include "commandtags.h"
#include "kbsequence.h"
#include "insets/lyxinset.h"
#include "LString.h"

class LyXView;
class auto_mem_buffer;

/** This class encapsulates all the LyX command operations. 
    This is the class of the LyX's "high level event handler".
    Every user command is processed here, either invocated from
    keyboard or from the GUI. All GUI objects, including buttons and
    menus should use this class and never call kernel functions directly.
*/
class LyXFunc {
public:
	/// The status of a function.
	enum func_status {
		/// No problem
		OK = 0,
		///
		Unknown = 1,
		/// Command cannot be executed
		Disabled = 2,
		///
		ToggleOn = 4,
		///
		ToggleOff = 8
	};
	///
	explicit
	LyXFunc(LyXView *);
    
	/// LyX dispatcher, executes lyx actions.
	string const Dispatch(int action, string const & arg = string());
			 
	/// The same but uses the name of a lyx command.
	string const Dispatch(string const & cmd);

	/// Same again but for xtl buffers.  Still looking for better idea.
	bool Dispatch(int action, auto_mem_buffer &);

#if 0
	/// A keyboard event is processed to execute a lyx action. 
 	int processKeyEvent(XEvent * ev);
#else
	///
	int processKeySym(KeySym k, unsigned int state);
#endif
	///
	func_status getStatus(int ac) const;
	
	/// The last key was meta
	bool wasMetaKey() const;

	// These can't be global because are part of the
	// internal state (ale970227)
	/// Get the current keyseq string
	string const keyseqStr() const;

	/// Is the key sequence uncomplete?
	bool keyseqUncomplete() const;

	/// get options for the current keyseq
	string const keyseqOptions() const;

        /// True if lyxfunc reports an error
        bool errorStat() const { return errorstat; }
        /// Buffer to store result messages
        void setMessage(string const & m);
        /// Buffer to store result messages
        void setErrorMessage(string const &) const; 
        /// Buffer to store result messages
        string const getMessage() const { return dispatch_buffer; }
        /// Get next inset of this class from current cursor position  
        Inset * getInsetByCode(Inset::Code);
	
	/// Should a hint message be displayed?
	void setHintMessage(bool);
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
	void moveCursorUpdate(bool selecting = false);
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
	/// Command name and shortcut information
	string commandshortcut;

	// I think the following should be moved to BufferView. (Asger)

	///
	void MenuNew(bool fromTemplate);

	///
	void MenuOpen();

	///
	void doImport(string const &);
	///
	void doImportHelper(string const &, string const &, string const &,
		bool func(BufferView *, string const &) );

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
bool LyXFunc::wasMetaKey() const 
{ 
	return (meta_fake_bit != 0);
}
     

inline
string const LyXFunc::keyseqStr() const
{
	// Why not just remove this function
	string text;
	keyseq.print(text, true);
	return text;
} 


inline
string const LyXFunc::keyseqOptions() const
{
	// Why not just remove this function
	string text;
	keyseq.printOptions(text);
	return text;
} 


inline
bool LyXFunc::keyseqUncomplete() const
{ 
	return (keyseq.length > 0);
}


inline
void LyXFunc::setHintMessage(bool hm) 
{ 
	show_sc = hm;
}

///
inline
void operator|=(LyXFunc::func_status & fs, LyXFunc::func_status f)
{
	fs = static_cast<LyXFunc::func_status>(fs | f);
}

#endif
