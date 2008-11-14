// -*- C++ -*-
/**
 * \file KeySymbol.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger and Jürgen
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef KEYSYMBOL_H
#define KEYSYMBOL_H

#include "KeyModifier.h"

#include "support/docstring.h"


namespace lyx {

/**
 * This is a class representing a keypress.
 */
class KeySymbol
{
public:
	KeySymbol() : key_(0) {}

	///
	bool operator==(KeySymbol const & ks) const;

	/// Initialize with the name of a key. F. ex. "space" or "a"
	void init(std::string const & symbolname);

	/// Is this a valid key?
	bool isOK() const;

	/// Is this a modifier key only?
	bool isModifier() const;

	/// Is this normal insertable text ? (last ditch attempt only)
	bool isText() const;

	/// What is the symbolic name of this key? F.ex. "Return" or "c"
	std::string getSymbolName() const;

	/**
	 * Return the value of the keysym into the UCS-4 encoding.
	 * This converts the KeySymbol to a 32-bit encoded character.
	 */
	char_type getUCSEncoded() const;

	/**
	 * Return a string describing the KeySym with modifier mod.
	 * Use the native UI format when \c forgui is true.
	 * i.e. (translated and with special characters for Mac OS X)
	 */
	docstring const print(KeyModifier mod, bool forgui) const;

	///
	int key() const { return key_; }
	///
	void setKey(int key) { key_ = key; }
	///
	docstring text() const { return text_; }
	///
	void setText(docstring const & text) { text_ = text; }
private:
	/// some platform specific sym value
	int key_;
	/// the event string value
	docstring text_;
};


} // namespace lyx

#endif
