// -*- C++ -*-
/**
 * \file KeyMap.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef KEYMAP_H
#define KEYMAP_H

#include "FuncRequest.h"

#include "frontends/KeyModifier.h"
#include "frontends/KeySymbol.h"

#include "support/docstream.h"

#include <boost/shared_ptr.hpp>

#include <vector>
#include <deque>


namespace lyx {

class KeySequence;

/// Defines key maps and actions for key sequences
class KeyMap {
public:
	/**
	 * Bind a key sequence to an action.
	 * @return 0 on success, or position in string seq where error
	 * occurs.
	 * See KeySequence::parse for the syntax of the seq string
	 */
	size_t bind(std::string const & seq, FuncRequest const & func);

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
	lookup(KeySymbol const & key, KeyModifier mod, KeySequence * seq) const;

	///
	typedef std::deque<KeySequence> Bindings;

	/// Given an action, find all keybindings.
	Bindings findbindings(FuncRequest const & func) const;

	/// Given an action, print the keybindings.
	docstring const printbindings(FuncRequest const & func) const;

	typedef std::pair<FuncRequest, KeySequence> Binding; 
	typedef std::vector<Binding> BindingList;
	/**
	 * Return all lfun and their associated bindings.
	 * @param unbound list unbound (func without any keybinding) as well
	 */
	BindingList const listBindings(bool unbound) const;

	/**
	 *  Given an action, find the first 1-key binding (if it exists).
	 *  The KeySymbol pointer is 0 is no key is found.
	 *  [only used by the Qt/Mac frontend]
	 */
	std::pair<KeySymbol, KeyModifier>
	find1keybinding(FuncRequest const & func) const;


	/**
	 * Returns a string of the given keysym, with modifiers.
	 * @param key the key as a keysym
	 * @param mod the modifiers
	 */
	static std::string const printKeySym(KeySymbol const & key,
					     KeyModifier mod);

	typedef std::pair<KeyModifier, KeyModifier> ModifierPair;


private:
	///
	struct Key {
		/// Keysym
		KeySymbol code;

		/// Modifier masks
		ModifierPair mod;

		/// Keymap for prefix keys
		boost::shared_ptr<KeyMap> table;

		/// Action for !prefix keys
		FuncRequest func;
	};

	/**
	 * Define an action for a key sequence.
	 * @param r internal recursion level
	 */
	void defkey(KeySequence * seq, FuncRequest const & func,
		    unsigned int r = 0);

	/**
	 * Given an action, find all keybindings
	 * @param func the action
	 * @param prefix a sequence to prepend the results
	 */
	Bindings findbindings(FuncRequest const & func,
			      KeySequence const & prefix) const;
	
	void listBindings(BindingList & list,
				  KeySequence const & prefix) const;

	/// is the table empty ?
	bool empty() const { return table.empty(); }
	///
	typedef std::vector<Key> Table;
	///
	Table table;
};

/// Implementation is in LyX.cpp
extern KeyMap & theTopLevelKeymap();


} // namespace lyx

#endif // KEYMAP_H
