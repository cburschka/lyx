// -*- C++ -*-
/**
 * \file Menubar.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MENUBAR_H
#define MENUBAR_H

#include "support/docstring.h"


namespace lyx {
namespace frontend {

/**
 * The LyX GUI independent menubar class
 * The GUI interface is implemented in the frontends
 */
class Menubar {
public:
	///
	virtual ~Menubar() {}
	/// Opens a top-level submenu given its name
	virtual void openByName(docstring const &) = 0;
	/// update the state of the menuitems
	virtual void update() = 0;
};

} // namespace frontend
} // namespace lyx

#endif // MENUBAR_H
