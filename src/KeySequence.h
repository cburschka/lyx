// -*- C++ -*-
/**
 * \file KeySequence.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef KEYSEQUENCE_H
#define KEYSEQUENCE_H

#include "frontends/KeyModifier.h"
#include "frontends/KeySymbol.h"

#include <string>
#include <vector>


namespace lyx {

class KeyMap;
class FuncRequest;

/// Holds a key sequence and the current and standard keymaps
class KeySequence {
public:
	typedef std::vector<KeySymbol> Sequence;

	friend class KeyMap;

	KeySequence() : stdmap(0), curmap(0), deleted_(true) {}
	///
	KeySequence(KeyMap * std, KeyMap * cur)
		: stdmap(std), curmap(cur), deleted_(false) {}

	/**
	 * Add a key to the key sequence and look it up in the curmap
	 * if the latter is defined.
	 * @param keysym the key to add
	 * @param mod modifier mask
	 * @param nmod which modifiers to mask out for equality test
	 * @return the action matching this key sequence or LFUN_UNKNOWN_ACTION
	 */
	FuncRequest const & addkey(KeySymbol const & keysym, KeyModifier mod,
	       KeyModifier nmod = NoModifier);

	/**
	 * Add a sequence of keys from a string to the sequence
	 * @return string::npos if OK, else error position in string
	 *
	 * Keys in the string must be separated with whitespace;
	 * Use the keysym names used by XStringToKeysym, f.ex.
	 * "Space", "a", "Return", ...
	 * Prefixes are S-, C-, M- for shift, control, meta
	 * Prefixes can also be ignored by using the Tilde "~"
	 * f.ex.: "~S-Space".
	 */
	size_t parse(std::string const & s);

	enum outputFormat {
		Portable,	//< use a more portable format
		ForGui,		//< use platform specific translations and special characters
		BindFile	//< the format used in lyx bind files
	};
	
	/**
	 * Return the current sequence as a string.
	 * @param format output format
	 * @see parse()
	 */
	docstring const print(outputFormat format) const;

	/**
	 * Return the current sequence and available options as
	 * a string. No options are added if no curmap kb map exists.
	 * @param forgui true if the string should use translations and
	 *   special characters.
	 */
	docstring const printOptions(bool forgui) const;

	/// Reset sequence to become "deleted"
	void reset();

	/// clear in full
	void clear();

	/// remove last key in sequence
	void removeKey();

	bool deleted() const { return deleted_; }

	/// length of sequence
	size_t length() const { return sequence.size(); }

	/// Keymap to use if a new sequence is starting
	KeyMap * stdmap;

	/// Keymap to use for the next key
	KeyMap * curmap;

private:
	/**
	 * Array holding the current key sequence as KeySyms.
	 * If sequence[length - 1] < 0xff it can be used as ISO8859 char
	 */
	Sequence sequence;

	typedef std::pair<KeyModifier, KeyModifier> ModifierPair;

	/// modifiers for keys in the sequence
	std::vector<ModifierPair> modifiers;

	/// is keysequence deleted ?
	bool deleted_;
};


} // namespace lyx

#endif
