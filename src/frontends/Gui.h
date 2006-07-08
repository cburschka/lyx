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

class LyXView;
class BufferView;

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
	virtual Clipboard & clipboard() = 0;

	///
	virtual int newView(unsigned int width, unsigned int height) = 0;
	///
	virtual LyXView & view(int id) = 0;
	///
	virtual void destroyView(int id) = 0;

	///
	virtual int newWorkArea(unsigned int width, unsigned int height, int view_id) = 0;
	///
	virtual WorkArea & workArea(int id) = 0;
	///
	virtual void destroyWorkArea(int id) = 0;

protected:
	/// view of a buffer. Eventually there will be several.
	std::map<int, boost::shared_ptr<BufferView> > buffer_views_;
};

} // namespace frontend
} // namespace lyx

#endif // BASE_GUI_H
