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

class LyXView;
class BufferView;

namespace frontend {

class WorkArea;


/**
 * A Gui class manages the different frontend elements.
 */
class Gui
{
public:
	virtual ~Gui() {}

	///
	virtual int newView() = 0;
	///
	virtual LyXView & view(int id) = 0;

	///
	virtual int newWorkArea(unsigned int width, unsigned int height, int view_id) = 0;
	///
	virtual WorkArea & workArea(int id) = 0;
	///
	virtual bool closeAll() = 0;

	///
	std::vector<int> const & viewIds() { return view_ids_; };

protected:
	/// view of a buffer. Eventually there will be several.
	std::map<int, boost::shared_ptr<BufferView> > buffer_views_;

	std::vector<int> view_ids_;
};

} // namespace frontend
} // namespace lyx

#endif // BASE_GUI_H
