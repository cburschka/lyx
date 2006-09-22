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

#ifndef GUI_H
#define GUI_H

#include "frontends/Gui.h"

#include <boost/shared_ptr.hpp>

#include <map>

class LyXView;

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
	GuiImplementation();
	virtual ~GuiImplementation() {}

	int newView(unsigned int width, unsigned int height);
	LyXView& view(int id);
	void destroyView(int id);
	int newWorkArea(unsigned int width, unsigned int height, int view_id);
	int newWorkArea(int w, int h);
	WorkArea& workArea(int id);
	void destroyWorkArea(int id);


private:
	///
	std::map<int, boost::shared_ptr<GuiView> > views_;
	///
	std::map<int, boost::shared_ptr<GuiWorkArea> > work_areas_;
	///
	size_t max_view_id_;
	///
	size_t max_wa_id_;
};

} // namespace frontend
} // namespace lyx

#endif // GUI_H
