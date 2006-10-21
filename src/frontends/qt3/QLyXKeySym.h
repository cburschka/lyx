// -*- C++ -*-
/**
 * \file QLyXKeySym.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger and Jürgen
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QLYXKEYSYM_H
#define QLYXKEYSYM_H

#include "frontends/LyXKeySym.h"

#include <qstring.h>


namespace lyx {

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
	virtual void init(std::string const & symbolname);

	/// Is this a valid key?
	virtual bool isOK() const;

	/// Is this a modifier key only?
	virtual bool isModifier() const;

	/// return the LyX symbolic name
	virtual std::string getSymbolName() const;

	/// Is this normal insertable text ? (last ditch attempt only)
	virtual bool isText() const;

	/**
	 * Return the value of the keysym into the UCS-4 encoding.
	 * This converts the LyXKeySym to a 32-bit encoded character.
	 */
	virtual size_t getUCSEncoded() const;

	/// Return a human-readable version of a key+modifier pair.
	virtual std::string const print(key_modifier::state mod) const;

	///
	QString const qprint(key_modifier::state mod) const;

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


} // namespace lyx

#endif // QLYXKEYSYM_H
