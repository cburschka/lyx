// -*- C++ -*-
/**
 * \file gtk/WorkArea.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef WORKAREA_H
#define WORKAREA_H

#include "frontends/WorkArea.h"

#include "GScreen.h"
#include "GWorkArea.h"

namespace lyx {
namespace frontend {

typedef GScreen FScreen;
typedef GWorkArea FWorkArea;

/**
 * Temporary wrapper around GWorkArea and GScreen.
 * Please refer to the Qt4 implementation for a proper cleanup of the API.
 */
class GuiWorkArea: public lyx::frontend::WorkArea {
public:
	GuiWorkArea(LyXView & owner, int w, int h,
		FScreen * screen, FWorkArea * work_area)
		: lyx::frontend::WorkArea(owner, w, h),
		old_screen_(screen), old_work_area_(work_area)
	{
	}

	~GuiWorkArea() {}

	/// return the painter object for this work area
	virtual lyx::frontend::Painter & getPainter()
	{
		return old_work_area_->getPainter();
	}

	/// return the width of the work area in pixels
	virtual int width() const
	{
		return old_work_area_->workWidth();
	}

	/// return the height of the work area in pixels
	virtual int height() const
	{
		return old_work_area_->workHeight();
	}

	/**
	 * Update the scrollbar.
	 * @param height the total document height in pixels
	 * @param pos the current position in the document, in pixels
	 * @param line_height the line-scroll amount, in pixels
	 */
	virtual void setScrollbarParams(int height, int pos, int line_height)
	{
		old_work_area_->setScrollbarParams(height, pos, line_height);
	}


	/// paint the cursor and store the background
	virtual void showCursor(int x, int y, int h, Cursor_Shape shape)
	{
		old_screen_->showCursor(x, y, h, shape);
	}

	/// hide the cursor
	virtual void removeCursor()
	{
		old_screen_->removeCursor();
	}

protected:
	/// cause the display of the given area of the work area
	virtual void expose(int x, int y, int w, int h)
	{
		old_screen_->expose(x, y, w, h);
	}

private:
	FScreen * old_screen_;
	FWorkArea * old_work_area_;
};

} // namespace frontend
} // namespace lyx

#endif // WORKAREA_H
