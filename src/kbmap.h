// -*- C++ -*-
/**
 * \file kbmap.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef KBMAP_H
#define KBMAP_H

#include "support/std_string.h"
#include "frontends/key_state.h"
#include "frontends/LyXKeySym.h"

#include <vector>

class kb_sequence;

/// Defines key maps and actions for key sequences
class kb_keymap {
public:
	/**
	 * Bind a key sequence to an action.
	 * @return 0 on success, or position in string seq where error
	 * occurs.
	 * See kb_sequence::parse for the syntax of the seq string
	 */
	string::size_type bind(string const & seq, int action);

	// Parse a bind file
	bool kb_keymap::read(string const & bind_file);

	/// print all available keysyms
	string const print() const;

	/**
	 * Look up a key press in the keymap.
	 * @param key the keysym
	 * @param mod the modifiers
	 * @param seq the current key sequence so far
	 * @return the action / LFUN_PREFIX / LFUN_UNKNOWN_ACTION
	 */
	int lookup(LyXKeySymPtr key,
		   key_modifier::state mod, kb_sequence * seq) const;

	/// Given an action, find all keybindings.
	string const findbinding(int action,
				 string const & prefix = string()) const;

	/**
	 * Returns a string of the given keysym, with modifiers.
	 * @param key the key as a keysym
	 * @param mod the modifiers
	 */
	static string const printKeysym(LyXKeySymPtr key,
					key_modifier::state mod);

	typedef std::pair<key_modifier::state, key_modifier::state> modifier_pair;

private:
	///
	struct kb_key {
		/// Keysym
		LyXKeySymPtr code;

		/// Modifier masks
		modifier_pair mod;

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
	typedef std::vector<kb_key> Table;
	///
	Table table;
};

#endif // KBMAP_H
