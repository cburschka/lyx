// -*- C++ -*-
/**
 * \file QLyXKeySym.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger and Juergen
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QLYXKEYSYM_H
#define QLYXKEYSYM_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"
#include "frontends/LyXKeySym.h"

#include <qstring.h>

class QKeyEvent;

/**
 * Qt-specific key press.
 *
 * This is some really sick stuff.
 */
class QLyXKeySym : public LyXKeySym {
public:
	QLyXKeySym();

	virtual ~QLyXKeySym() {}

	/// delayed constructor
	void set(QKeyEvent * ev);

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
	///
	int key() const {
		return key_;
	}
private:
	/// the Qt sym value
	int key_;
	/// the event string value
	QString text_;
};

#endif // QLYXKEYSYM_H
