// -*- C++ -*-
/**
 * \file QLyXKeySym.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Asger and Juergen
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QLYXKEYSYM_H
#define QLYXKEYSYM_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"
#include "frontends/LyXKeySym.h"

#include <qstring.h> 
 
/**
 * Qt-specific key press.
 */
class QLyXKeySym : public LyXKeySym {
public:
	QLyXKeySym();

	virtual ~QLyXKeySym() {}

	/// delayed constructor
	void set(int key, QString const & text);

	/// set from a LyX symbolic name
	virtual void init(string const & symbolname);

	/// Is this a valid key?
	virtual bool isOK() const;

	/// Is this a modifier key only?
	virtual bool isModifier() const;
	
	/// return the LyX symbolic name
	virtual string getSymbolName() const;

	/**
	 * Return the value of the keysym into the local ISO encoding.
	 * This converts the LyXKeySym to a 8-bit encoded character.
	 * This relies on user to use the right encoding.
	 */
	virtual char getISOEncoded() const;

	virtual bool operator==(LyXKeySym const & k) const;

private:
	/// the Qt sym value
	int key_;
 
	/**
	 * The generated text from the symbol. This will only be
	 * filled when the QLyXKeySym is used to pass a key press
	 * event, not when stored in a kbsequence etc.
	 */
	QString text_;
};

#endif // QLYXKEYSYM_H
