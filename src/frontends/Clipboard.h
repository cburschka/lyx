// -*- C++ -*-
/**
 * \file Clipboard.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef BASE_CLIPBOARD_H
#define BASE_CLIPBOARD_H

#include <string>

namespace lyx {
namespace frontend {

/**
 * A Clipboard class manages the clipboard.
 */
class Clipboard
{
public:
	virtual ~Clipboard() {}

	/// a selection exists
	virtual void haveSelection(bool) = 0;
	/// get the X clipboard contents
	virtual std::string const get() const = 0;
	/// fill the clipboard
	virtual void put(std::string const &) = 0;
};

} // namespace frontend
} // namespace lyx

#endif // BASE_CLIPBOARD_H
