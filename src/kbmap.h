// -*- C++ -*-
/* ======================================================================= *\
   File   : kbmap.h, kbmap.h,v 1.3 1996/12/10 04:35:57 larsbj Exp
   Author : chb, 30.Oct.1995
   Docu   : see kbmap.C
   Purpose: class definitions for XKeyEvent keymap handling
   \* ==================================================================== */

#ifndef KBMAP_H
#define KBMAP_H

#ifdef __GNUG__
#pragma interface
#endif

#include <X11/Xlib.h>

#include "LString.h"

#define KB_PREALLOC  16
#define KB_HASHSIZE 128   // yes, yes - I know. 128 is not exactly prime :-)
// ... but we are dealing with ASCII chars mostly.

class kb_keymap;
class kb_sequence;

///
struct kb_key {
	/// Keysym
	unsigned int code;
	
	/// Modifier masks
	unsigned int mod;
	
	/// Keymap for prefix keys
	kb_keymap * table;
	
	/// Action for !prefix keys
	int action;
};


/// Defines key maps and actions for key sequences
class kb_keymap {
public:
	///
	kb_keymap() {
		size = 0; 
		table = 0;
	}
	///
	~kb_keymap();
	
	/// Bind a key-sequence to an action
	/** Returns 0 on success. Otherwise, position in string where
	  error occured. */
	int bind(char const * seq, int action);
	
	///
	int print(char * buf, int maxlen) const;
	
	/// Look up a key in the keymap
	int lookup(KeySym key, unsigned mod, kb_sequence * seq);

	/// Given an action, find all keybindings.
	string findbinding(int action) const;
private:
	/// Define a new key sequence
	int defkey(kb_sequence * seq, int action, int idx = 0);
	
	/// Size of the table (<0: hashtab)
       int size;
	
	/// Holds the defined keys
	/** Both kinds of tables ends with NoSymbol */
	union
	{
		/// Table for linear array
		kb_key * table;
		
		/// Hash table holding key lists
		kb_key ** htable;
	};
};


/// Holds a key sequence and the current and standard keymaps
class kb_sequence {
public:
	///
	kb_sequence() {
		stdmap = curmap = 0;
		sequence = staticseq;
		modifiers = staticmod;
		length = 0; 
		size = KB_PREALLOC;
	}
	
	///
	
	
	///
	~kb_sequence()
	{
		if (sequence != staticseq) {
			delete sequence;
			delete modifiers;
		}
	}
	
	/// Add a key to the key sequence and look it up in the curmap
	/** Add a key to the key sequence and look it up in the curmap
	  if the latter is defined. */
	int addkey(KeySym key, unsigned mod, unsigned nmod = 0);
	
	///
	int print(char * buf, int maxlen, bool when_defined = false) const; //RVDK_PATCH_5
	
        ///
	int printOptions(char * buf, int maxlen) const;
	
	/// Make length negative to mark the sequence as deleted
	void delseq();
	
	///
	char getiso();
	
	///
	KeySym getsym();
	
	///
	void reset();
	
	///
	int parse(char const * s);
	
	/// Keymap to use if a new sequence is starting
	kb_keymap * stdmap;
	
	/// Keymap to use for the next key
	kb_keymap * curmap;
	
	/// Array holding the current key sequence
	/** If sequence[length-1] < 0xff it can be used as ISO8859 char */
	unsigned int * sequence;
	
	///
	unsigned int * modifiers;
	
	/// Current length of key sequence
	int length;
	
private:
	/// Static array preallocated for sequence
	unsigned int staticseq[KB_PREALLOC];
	
	///
	unsigned int staticmod[KB_PREALLOC];
	
	/// Physically allocated storage size
	int size;
};

#endif
