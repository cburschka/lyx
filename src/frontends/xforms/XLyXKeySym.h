// -*- C++ -*-
/**
 * \file XLyXKeySym.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger and Jürgen
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef XLYXKEYSYM_H
#define XLYXKEYSYM_H

#include <X11/Xlib.h>
#include "frontends/LyXKeySym.h"

namespace lyx {
namespace frontend {

/**
 * This is the XForms (X11) version of LyXKeySym.
 */
class XLyXKeySym : public LyXKeySym {
public:
	XLyXKeySym();

	/// X11 specific initialization with an X11 KeySym
	void initFromKeySym(KeySym);

	virtual void init(std::string const & symbolname);

	virtual ~XLyXKeySym() {}

	/// Is this a valid key?
	virtual bool isOK() const;

	/// Is this a modifier key only?
	virtual bool isModifier() const;

	virtual std::string getSymbolName() const;

	/**
	 * Return the value of the keysym into the local ISO encoding.
	 * This converts the LyXKeySym to a 8-bit encoded character.
	 * This relies on user to use the right encoding.
	 */
	virtual char getISOEncoded(std::string const & encoding) const;

	///
	virtual std::string const print(key_modifier::state mod) const;

	///
	unsigned int keysym() const {
		return keysym_;
	}
private:
	unsigned int keysym_;
};

} // namespace frontend
} // namespace lyx

#endif // XLYXKEYSYM_H
