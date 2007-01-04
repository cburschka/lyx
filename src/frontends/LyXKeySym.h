// -*- C++ -*-
/**
 * \file LyXKeySym.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger and Jürgen
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYXKEYSYM_H
#define LYXKEYSYM_H

#include <string>

#include "key_state.h"

#include "support/docstring.h"

#include <boost/shared_ptr.hpp>


namespace lyx {

/**
 * This is a base class for representing a keypress.
 * Each frontend has to implement this to provide
 * the functionality that LyX needs in regards to
 * key presses.
 */
class LyXKeySym {
public:
	LyXKeySym() {}

	virtual ~LyXKeySym() {}

	/// Initialize with the name of a key. F. ex. "space" or "a"
	virtual void init(std::string const & symbolname) = 0;

	/// Is this a valid key?
	virtual bool isOK() const = 0;

	/// Is this a modifier key only?
	virtual bool isModifier() const = 0;

	/// Is this normal insertable text ? (last ditch attempt only)
	virtual bool isText() const = 0;

	/// What is the symbolic name of this key? F.ex. "Return" or "c"
	virtual std::string getSymbolName() const = 0;

	/**
	 * Return the value of the keysym into the UCS-4 encoding.
	 * This converts the LyXKeySym to a 32-bit encoded character.
	 */
	virtual size_t getUCSEncoded() const = 0;

	/**
	 * Return a string describing the KeySym with modifier mod.
	 * Use the native UI format when \c forgui is true.
	 */
	virtual docstring const print(key_modifier::state mod, bool forgui) const = 0;
};


/**
 * We need to be able to equality compare these for the
 * sake of the keymap business.
 */
bool operator==(LyXKeySym const & k1, LyXKeySym const & k2);

typedef boost::shared_ptr<LyXKeySym> LyXKeySymPtr;


} // namespace lyx

#endif
