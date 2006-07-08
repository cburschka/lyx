// -*- C++ -*-
/**
 * \file qt4/GuiClipboard.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CLIPBOARD_H
#define CLIPBOARD_H

#include "frontends/Clipboard.h"

namespace lyx {
namespace frontend {

/**
 * The Qt4 version of the Clipboard.
 */
class GuiClipboard: public Clipboard
{
public:
	GuiClipboard() {}

	virtual ~GuiClipboard() {}

	/** Clipboard overloaded methods
	 */
	//@{
	void haveSelection(bool own);
	std::string const get() const;
	void put(std::string const & str);
	//@}
};

} // namespace frontend
} // namespace lyx

#endif // CLIPBOARD_H
