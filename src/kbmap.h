// -*- C++ -*-
/**
 * \file kbmap.h
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 *
 * \author Lars Gullik Bjønnes <larsbj@lyx.org>
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

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
	/**
	 * Bind a key sequence to an action.
	 * @return 0 on success, or position in string seq where error
	 * occurs.
	 */
	string::size_type bind(string const & seq, int action);

	/// print all available keysyms
	string const print() const;
	
	/**
	 * Look up a key press in the keymap.
	 * @param key the keysym
	 * @param mod the modifiers
	 * @param seq the current key sequence so far
	 * @return the action / LFUN_PREFIX / LFUN_UNKNOWN_ACTION
	 */
	int lookup(unsigned int key,
		   unsigned int mod, kb_sequence * seq) const;

	/// Given an action, find all keybindings.
	string const findbinding(int action,
				 string const & prefix = string()) const;

	/**
	 * Returns a string of the given keysym, with modifiers.
	 * @param key the key
	 * @param mod the modifiers
	 */
	static string const printKeysym(unsigned int key, unsigned int mod);

	/// return the ISO value of a keysym
	static char getiso(unsigned int i);

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


	/**
	 * Define an action for a key sequence.
	 * @param r internal recursion level
	 */
	void defkey(kb_sequence * seq, int action, unsigned int r = 0);
	
	///  Returns a string of the given key
	string const printKey(kb_key const & key) const;

	/// is the table empty ?
	bool empty() const {
		return table.empty();
	}
	///
	typedef std::list<kb_key> Table;
	///
	Table table;
};

#endif // KBMAP_H
