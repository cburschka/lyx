// -*- C++ -*-
/* ======================================================================= *\
   File   : kbmap.h, kbmap.h,v 1.3 1996/12/10 04:35:57 larsbj Exp
   Author : chb, 30.Oct.1995
   Docu   : see kbmap.C
   Purpose: class definitions for XKeyEvent keymap handling
   \* ==================================================================== */

#ifndef KBSEQUENCE_H
#define KBSEQUENCE_H

#ifdef __GNUG__
#pragma interface
#endif

#include <vector>
#include "LString.h"

class kb_keymap;

/// Holds a key sequence and the current and standard keymaps
class kb_sequence {
public:
	///
	kb_sequence() {
		stdmap = curmap = 0;
		length = 0;
	}

	/** Add a key to the key sequence and look it up in the curmap
	    if the latter is defined. */
	int addkey(unsigned int key, unsigned int mod, unsigned int nmod = 0);

	///
	int print(string & buf, bool when_defined = false) const;
	
        ///
	int printOptions(string & buf) const;
	
	/// Make length negative to mark the sequence as deleted
	void delseq();

	///
	char getiso() const;
	
	///
	unsigned int getsym() const;
	
	///
	void reset();
	
	///
	string::size_type parse(string const & s);
	
	/// Keymap to use if a new sequence is starting
	kb_keymap * stdmap;
	
	/// Keymap to use for the next key
	kb_keymap * curmap;
	
	/** Array holding the current key sequence.
	    If sequence[length-1] < 0xff it can be used as ISO8859 char */
	std::vector<unsigned int> sequence;
	
	///
	std::vector<unsigned int> modifiers;
	
	/// Current length of key sequence
	int length;
};

#endif
