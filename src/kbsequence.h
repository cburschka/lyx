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

#include <vector>
#include "LString.h"

class kb_keymap;

/// Holds a key sequence and the current and standard keymaps
class kb_sequence {
public:
	friend class kb_keymap;

	///
	kb_sequence(kb_keymap * std, kb_keymap * cur)
		: stdmap(std), curmap(cur), length_(0), deleted_(false) {}



	/**
	 * Add a key to the key sequence and look it up in the curmap
	 * if the latter is defined.
	 * @param mod modifier mask
	 * @param nmod which modifiers to mask out for equality test
	 * @return the action matching this key sequence or LFUN_UNKNOWN_ACTION
	 */
	int addkey(unsigned int key, unsigned int mod, unsigned int nmod = 0);

	/**
	 * Add a sequence of keys from a string to the sequence
	 * @return string::npos if OK, else error position in string
	 *
	 * Keys in the string must be separated with whitespace;
	 * Use the keysym names used by XStringToKeysym
	 * Prefixes are S-, C-, M- for shift, control, meta
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

	/// Return the ISO value of the last keysym in sequence, or 0
	char getiso() const;

	/// Reset sequence to become "deleted"
	void reset();

	/// clear in full
	void clear();

	bool deleted() const {
		return deleted_;
	}

	/// length of sequence
	std::vector<unsigned int>::size_type length() const {
		return length_;
	}

	/// Keymap to use if a new sequence is starting
	kb_keymap * stdmap;

	/// Keymap to use for the next key
	kb_keymap * curmap;

private:
	/// get the keysym of last in sequence
	unsigned int getsym() const;

	/**
	 * Array holding the current key sequence.
	 * If sequence[length-1] < 0xff it can be used as ISO8859 char
	 */
	std::vector<unsigned int> sequence;

	/// modifiers for keys in the sequence
	std::vector<unsigned int> modifiers;

	/// Current length of key sequence
	std::vector<unsigned int>::size_type length_;

	/// is keysequence deleted ?
	bool deleted_;
};

#endif
