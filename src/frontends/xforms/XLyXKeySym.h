// -*- C++ -*-
/**
 * \file XLyXKeySym.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Asger and Juergen
 */

#ifndef XLYXKEYSYM_H
#define XLYXKEYSYM_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"
#include <X11/Xlib.h>
#include "frontends/LyXKeySym.h"

/**
 * This is the XForms (X11) version of LyXKeySym.
 */
class XLyXKeySym : public LyXKeySym {
public:
	XLyXKeySym();

	/// X11 specific initialization with an X11 KeySym
	void initFromKeySym(KeySym);

	virtual void init(string const & symbolname);

	virtual ~XLyXKeySym() {}

	/// Is this a valid key?
	virtual bool isOK() const;

	/// Is this a modifier key only?
	virtual bool isModifier() const;
	
	virtual string getSymbolName() const;

	/**
	 * Return the value of the keysym into the local ISO encoding.
	 * This converts the LyXKeySym to a 8-bit encoded character.
	 * This relies on user to use the right encoding.
	 */
	virtual char getISOEncoded() const;

	virtual bool operator==(LyXKeySym const & k) const;

private:
	unsigned int keysym;
};

#endif // XLYXKEYSYM_H
