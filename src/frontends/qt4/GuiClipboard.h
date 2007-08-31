// -*- C++ -*-
/**
 * \file GuiClipboard.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUICLIPBOARD_H
#define GUICLIPBOARD_H

#include "frontends/Clipboard.h"

namespace lyx {
namespace frontend {

/**
 * The Qt4 version of the Clipboard.
 */
class GuiClipboard : public Clipboard
{
public:
	virtual ~GuiClipboard() {}

	/** Clipboard overloaded methods
	 */
	//@{
	std::string const getAsLyX() const;
	docstring const getAsText() const;
	void put(std::string const & lyx, docstring const & text);
	bool hasLyXContents() const;
	bool isInternal() const;
	bool empty() const;
	//@}
};

} // namespace frontend
} // namespace lyx

#endif // GUICLIPBOARD_H
