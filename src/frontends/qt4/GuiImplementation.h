// -*- C++ -*-
/**
 * \file GuiImplementation.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUI_IMPLEMENTATION_H
#define GUI_IMPLEMENTATION_H

#include "frontends/Gui.h"
#include "GuiClipboard.h"

#include <boost/shared_ptr.hpp>

#include <map>

namespace lyx {
namespace frontend {

class GuiWorkArea;
class GuiView;

/**
 * The GuiImplementation class is the interface to all Qt4 components.
 */
class GuiImplementation: public Gui
{
public:
	GuiImplementation(GuiView & owner);
	virtual ~GuiImplementation() {}

	Clipboard& clipboard();

	int newWorkArea(int w, int h);
	WorkArea& workArea(int id);
	void destroyWorkArea(int id);

private:
	///
	GuiClipboard clipboard_;
	///
	std::map<int, boost::shared_ptr<GuiWorkArea> > work_areas_;
	///
	GuiView & owner_;
	///
	size_t max_id_;
};

} // namespace frontend
} // namespace lyx

#endif // GUI_IMPLEMENTATION_H
