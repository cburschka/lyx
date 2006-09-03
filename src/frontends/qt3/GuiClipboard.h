// -*- C++ -*-
/**
 * \file qt3/GuiClipboard.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
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
 * The Qt3 version of the Clipboard.
 */
class GuiClipboard: public lyx::frontend::Clipboard
{
public:
	virtual ~GuiClipboard() {}

	/** Clipboard overloaded methods
	 */
	//@{

	docstring const get() const;

	void put(docstring const & str);
	//@}
};

} // namespace frontend
} // namespace lyx

#endif // CLIPBOARD_H
