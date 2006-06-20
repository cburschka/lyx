// -*- C++ -*-
/**
 * \file Gui.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef BASE_GUI_H
#define BASE_GUI_H

#include "frontends/GuiCursor.h"

namespace lyx {
namespace frontend {

class Clipboard;
class WorkArea;


/**
 * A Gui class manages the different frontend elements.
 */
class Gui
{
public:
	virtual ~Gui() {}

	///
	virtual Clipboard& clipboard() = 0;
	///
	virtual int newWorkArea(int w, int h) = 0;
	///
	virtual WorkArea& workArea(int id) = 0;
	///
	virtual void destroyWorkArea(int id) = 0;

	///
	GuiCursor & guiCursor() {return cursor_;}

private:
	GuiCursor cursor_;
};

} // namespace frontend
} // namespace lyx

#endif // BASE_GUI_H
