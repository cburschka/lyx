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

	/**
	 * Get the window system clipboard contents.
	 * This should be called when the user requests to paste from the
	 * clipboard.
	 */
	virtual std::string const get() const = 0;
	/**
	 * Fill the window system clipboard.
	 * This should be called when the user requests to cut or copy to
	 * the clipboard.
	 */
	virtual void put(std::string const &) = 0;
};

} // namespace frontend
} // namespace lyx

#endif // BASE_CLIPBOARD_H
