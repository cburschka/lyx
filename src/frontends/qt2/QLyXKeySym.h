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

	void set(int key, QString const & text);

	virtual void init(string const & symbolname);

	virtual ~QLyXKeySym() {}

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
	int key_;
	QString text_;
};

#endif // QLYXKEYSYM_H
