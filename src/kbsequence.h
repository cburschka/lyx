// -*- C++ -*-
/**
 * \file kbsequence.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 */

#ifndef KBSEQUENCE_H
#define KBSEQUENCE_H

#ifdef __GNUG__
#pragma interface
#endif

#include <config.h>

#include "frontends/key_state.h"
#include "frontends/LyXKeySym.h"
#include "LString.h"
#include <vector>

class kb_keymap;

/// Holds a key sequence and the current and standard keymaps
class kb_sequence {
public:
	typedef std::vector<LyXKeySymPtr> KeySequence;

	friend class kb_keymap;

	///
	kb_sequence(kb_keymap * std, kb_keymap * cur)
		: stdmap(std), curmap(cur), deleted_(false) {}

	/**
	 * Add a key to the key sequence and look it up in the curmap
	 * if the latter is defined.
	 * @param keysym the key to add
	 * @param mod modifier mask
	 * @param nmod which modifiers to mask out for equality test
	 * @return the action matching this key sequence or LFUN_UNKNOWN_ACTION
	 */
	int addkey(LyXKeySymPtr keysym, key_modifier::state mod,
		   key_modifier::state nmod = key_modifier::none);

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
	string::size_type parse(string const & s);

	/**
	 * Return the current sequence as a string.
	 * @see parse()
	 */
	string const print() const;

	/**
	 * Return the current sequence and available options as
	 * a string. No options are added if no curmap kb map exists.
	 */
	string const printOptions() const;

	/// Mark the sequence as deleted.
	void mark_deleted();

	/**
	 * Return the value of the last keysym in the sequence
	 * in the local ISO encoding. If it does not encode
	 * in this encoding, return 0.
	 */
	char getLastKeyEncoded() const;

	/// Reset sequence to become "deleted"
	void reset();

	/// clear in full
	void clear();

	bool deleted() const {
		return deleted_;
	}

	/// length of sequence
	KeySequence::size_type length() const {
		return sequence.size();
	}

	/// Keymap to use if a new sequence is starting
	kb_keymap * stdmap;

	/// Keymap to use for the next key
	kb_keymap * curmap;

private:
	/// get the keysym of last in sequence
	LyXKeySymPtr getsym() const;

	/**
	 * Array holding the current key sequence as KeySyms.
	 * If sequence[length - 1] < 0xff it can be used as ISO8859 char
	 */
	KeySequence sequence;

	typedef std::pair<key_modifier::state, key_modifier::state>
		modifier_pair;

	/// modifiers for keys in the sequence
	std::vector<modifier_pair> modifiers;

	/// is keysequence deleted ?
	bool deleted_;
};

#endif
