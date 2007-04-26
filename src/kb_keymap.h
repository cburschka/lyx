// -*- C++ -*-
/**
 * \file kb_keymap.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef KB_KEYMAP_H
#define KB_KEYMAP_H

#include "FuncRequest.h"

#include "frontends/key_state.h"
#include "frontends/LyXKeySym.h"

#include "support/docstream.h"

#include <boost/shared_ptr.hpp>

#include <vector>
#include <deque>


namespace lyx {

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
	std::string::size_type bind(std::string const & seq, FuncRequest const & func);

	// Parse a bind file
	bool read(std::string const & bind_file);

	/**
	 * print all available keysyms
	 * @param forgui true if the string should use translations and 
	 *   special characters. 
	 */
	docstring const print(bool forgui) const;

	/**
	 * Look up a key press in the keymap.
	 * @param key the keysym
	 * @param mod the modifiers
	 * @param seq the current key sequence so far
	 * @return the action / LFUN_COMMAND_PREFIX / LFUN_UNKNOWN_ACTION
	 */
	FuncRequest const &
	lookup(LyXKeySymPtr key,
	       key_modifier::state mod, kb_sequence * seq) const;

	///
	typedef std::deque<kb_sequence> Bindings;

	/// Given an action, find all keybindings.
	Bindings findbindings(FuncRequest const & func) const;

	/// Given an action, print the keybindings.
	docstring const printbindings(FuncRequest const & func) const;

	/**
	 *  Given an action, find the first 1-key binding (if it exists).
	 *  The LyXKeySym pointer is 0 is no key is found.
	 *  [only used by the Qt/Mac frontend]
	 */
	std::pair<LyXKeySym const *, key_modifier::state>
	find1keybinding(FuncRequest const & func) const;


	/**
	 * Returns a string of the given keysym, with modifiers.
	 * @param key the key as a keysym
	 * @param mod the modifiers
	 */
	static std::string const printKeySym(LyXKeySym const & key,
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
		FuncRequest func;
	};

	/**
	 * Define an action for a key sequence.
	 * @param r internal recursion level
	 */
	void defkey(kb_sequence * seq, FuncRequest const & func,
		    unsigned int r = 0);

	/**
	 * Given an action, find all keybindings
	 * @param func the action
	 * @param prefix a sequence to prepend the results
	 */
	Bindings findbindings(FuncRequest const & func,
			      kb_sequence const & prefix) const;

	/// is the table empty ?
	bool empty() const {
		return table.empty();
	}
	///
	typedef std::vector<kb_key> Table;
	///
	Table table;
};

/// Implementation is in LyX.cpp
extern kb_keymap & theTopLevelKeymap();


} // namespace lyx

#endif // KB_KEYMAP_H
