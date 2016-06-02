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
#include "KeySequence.h"

#include "support/strfwd.h"

#include <memory>
#include <vector>


namespace lyx {

namespace support {
	class FileName;	
}

/// Defines key maps and actions for key sequences
class KeyMap {
public:
	enum ItemType {
		System,         //< loaded from a bind file
		UserBind,       //< \bind loaded from user.bind
		UserUnbind,     //< \unbind loaded from user.bind, with corresponding
		                //<    entry in system bind file
		UserExtraUnbind	//< \unbind loaded from user.bind, without
		                //<    corresponding entry in system bind file.
	};
	enum BindReadType {
		MissingOK,      //< It's OK if this file is missing.
		Fallback,       //< If missing, fallback to default "cua". This should only 
		                //< be used when attempting to read the user-secified bind file.
		Default         //< Report error and return.
	};
	/**
	 * Bind/Unbind a key sequence to an action.
	 * @return 0 on success, or position in string seq where error
	 * occurs.
	 * See KeySequence::parse for the syntax of the seq string
	 */
	size_t bind(std::string const & seq, FuncRequest const & func);
	size_t unbind(std::string const & seq, FuncRequest const & func);

	/**
	 * Define/Undefine an action for a key sequence.
	 * @param r internal recursion level
	 */
	void bind(KeySequence * seq, FuncRequest const & func,
		    unsigned int r = 0);
	void unbind(KeySequence * seq, FuncRequest const & func,
		    unsigned int r = 0);


	/// returns the function bound to this key sequence, or 
	/// FuncRequest::unknown if no binding exists for it.
	/// @param r an internal recursion counter
	// FIXME Surely there's a better way to do that?
	FuncRequest getBinding(KeySequence const & seq, unsigned int r = 0);

	/// clear all bindings
	void clear();

	/** Parse a bind file. If a valid unbind_map is given, put \unbind 
	 * bindings to a separate KeyMap. This is used in the Shortcut preference
	 * dialog where main and user bind files are loaded separately so \unbind
	 * in user.bind can not nullify \bind in the master bind file.
	 *
	 * @param bind_file bind file
	 * @param unbind_map pointer to a KeyMap that holds \unbind bindings
	 * @param rt how to respond if the file can't be found
	 */
	bool read(std::string const & bind_file, KeyMap * unbind_map = 0, 
			BindReadType rt = Default);

	/** write to a bind file.
	 * @param append append to the bind_file instead of overwrite it
	 * @param unbind use \unbind instead of \bind, indicating this KeyMap
	 *        actually record unbind maps.
	 */
	void write(std::string const & bind_file, bool append, bool unbind = false) const;

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
	typedef std::vector<KeySequence> Bindings;

	/// Given an action, find all keybindings.
	Bindings findBindings(FuncRequest const & func) const;

	/// Given an action, print the keybindings.
	docstring printBindings(FuncRequest const & func,
				KeySequence::outputFormat format) const;

	struct Binding {
		Binding(FuncRequest const & r, KeySequence const & s, ItemType t)
			: request(r), sequence(s), tag(t) {}
		FuncRequest request;
		KeySequence sequence;
		KeyMap::ItemType tag;
	}; 
	typedef std::vector<Binding> BindingList;
	/**
	 * Return all lfun and their associated bindings.
	 * @param unbound list unbound (func without any keybinding) as well
	 * @param tag an optional tag to indicate the source of the bindinglist
	 */
	BindingList listBindings(bool unbound, ItemType tag = System) const;

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

private:
	///
	typedef std::pair<KeyModifier, KeyModifier> ModifierPair;

	///
	struct Key {
		/// Keysym
		KeySymbol code;
		/// Modifier masks
		ModifierPair mod;
		/// Keymap for prefix keys
		std::shared_ptr<KeyMap> prefixes;
		/// Action for !prefix keys
		FuncRequest func;
	};

	enum ReturnValues {
		ReadOK,
		ReadError,
		FileError,
		FormatMismatch
	};
	///
	bool read(support::FileName const & bind_file, KeyMap * unbind_map = 0);
	///
	ReturnValues readWithoutConv(support::FileName const & bind_file, KeyMap * unbind_map = 0);

	/**
	 * Given an action, find all keybindings
	 * @param func the action
	 * @param prefix a sequence to prepend the results
	 */
	Bindings findBindings(FuncRequest const & func,
	                      KeySequence const & prefix) const;
	
	void listBindings(BindingList & list, KeySequence const & prefix,
	                  ItemType tag) const;

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
