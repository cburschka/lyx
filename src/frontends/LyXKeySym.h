// -*- C++ -*-
/**
 * \file LyXKeySym.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Asger and Juergen
 */

#ifndef LYXKEYSYM_H
#define LYXKEYSYM_H

#include "LString.h"
#include <boost/shared_ptr.hpp>

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
	virtual void init(string const & symbolname) = 0;

	/// Is this a valid key?
	virtual bool isOK() const = 0;

	/// Is this a modifier key only?
	virtual bool isModifier() const = 0;

	/// What is the symbolic name of this key? F.ex. "Return" or "c"
	virtual string getSymbolName() const = 0;

	/**
	 * Return the value of the keysym into the local ISO encoding.
	 * This converts the LyXKeySym to a 8-bit encoded character.
	 * This relies on user to use the right encoding.
	 */
	virtual char getISOEncoded() const = 0;

	/**
	 * We need to be able to equality compare these for the
	 * sake of the keymap business.
	 */
	virtual bool operator==(LyXKeySym const & k) const = 0;
};

typedef boost::shared_ptr<LyXKeySym> LyXKeySymPtr;

#endif
