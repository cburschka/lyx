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

#include <list>
#include <boost/smart_ptr.hpp>

#include "LString.h"

class kb_sequence;

/// Defines key maps and actions for key sequences
class kb_keymap {
public:
	/** Bind a key-sequence to an action.
	    Returns 0 on success. Otherwise, position in string where
	    error occured. */
	int bind(string const & seq, int action);

	///
	void print(string & buf) const;
	
	/// Look up a key in the keymap
	int lookup(unsigned int key,
		   unsigned int mod, kb_sequence * seq) const;

	/// Given an action, find all keybindings.
	string const findbinding(int action,
				 string const & prefix = string()) const;
private:
	///
	struct kb_key {
		/// Keysym
		unsigned int code;
		
		/// Modifier masks
		unsigned int mod;
		
		/// Keymap for prefix keys
		boost::shared_ptr<kb_keymap> table;
		
		/// Action for !prefix keys
		int action;
	};


	/// Define a new key sequence
	int defkey(kb_sequence * seq, int action, int idx = 0);
	///
	static string const keyname(kb_key const & k);
	
	///
	static
	void printKey(kb_key const & key, string & buf);
	///
	bool empty() const {
		return table.empty();
	}
	///
	typedef std::list<kb_key> Table;
	///
	Table table;
};

#endif
