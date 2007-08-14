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

#include <boost/shared_ptr.hpp>

#include <map>
#include <vector>

namespace lyx {

class BufferView;

namespace frontend {

class LyXView;
class WorkArea;


/**
 * A Gui class manages the different frontend elements.
 */
class Gui
{
public:
	virtual ~Gui() {}

	///
	virtual LyXView& createRegisteredView() = 0;
	///
	virtual bool unregisterView(int id) = 0;
	///
	virtual bool closeAllViews()= 0;

	///
	virtual LyXView& view(int id) const = 0;
	///
	std::vector<int> const & viewIds()
	{
		return view_ids_;
	}


	virtual int newWorkArea(unsigned int width, unsigned int height, int view_id) = 0;
	///
	virtual WorkArea & workArea(int id) = 0;

protected:

	std::vector<int> view_ids_;
};

} // namespace frontend
} // namespace lyx

#endif // BASE_GUI_H
