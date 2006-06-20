/**
 * \file GuiCursor.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 *
 * Splash screen code added by Angus Leeming
 */

#include <config.h>

#include "frontends/GuiCursor.h"

#include "font_metrics.h"
#include "lyx_gui.h"
#include "frontends/Painter.h"
#include "frontends/WorkArea.h"

#include "BufferView.h"
#include "buffer.h"
#include "bufferparams.h"
#include "coordcache.h"
#include "cursor.h"
#include "debug.h"
#include "language.h"
#include "LColor.h"
#include "lyxfont.h"
#include "lyxrc.h"
#include "lyxrow.h"
#include "lyxtext.h"
#include "metricsinfo.h"
#include "paragraph.h"
#include "rowpainter.h"
#include "version.h"

#include "graphics/GraphicsImage.h"
#include "graphics/GraphicsLoader.h"

#include "support/filetools.h" // LibFileSearch

#include <boost/utility.hpp>
#include <boost/bind.hpp>
#include <boost/signals/trackable.hpp>

using lyx::support::libFileSearch;

using std::endl;
using std::min;
using std::max;
using std::string;

namespace lyx {
namespace frontend {

GuiCursor::GuiCursor()
	: cursor_visible_(false), work_area_(NULL)
{
}


GuiCursor::~GuiCursor()
{
}

void GuiCursor::connect(WorkArea * work_area)
{
	work_area_ = work_area;
}


void GuiCursor::show(BufferView & bv)
{
	if (cursor_visible_)
		return;

	if (!bv.available())
		return;

	Cursor_Shape shape = BAR_SHAPE;

	LyXText const & text = *bv.getLyXText();
	LyXFont const & realfont = text.real_current_font;
	BufferParams const & bp = bv.buffer()->params();
	bool const samelang = realfont.language() == bp.language;
	bool const isrtl = realfont.isVisibleRightToLeft();

	if (!samelang || isrtl != bp.language->rightToLeft()) {
		shape = L_SHAPE;
		if (isrtl)
			shape = REVERSED_L_SHAPE;
	}

	// The ERT language hack needs fixing up
	if (realfont.language() == latex_language)
		shape = BAR_SHAPE;

	LyXFont const font = bv.cursor().getFont();
	int const asc = font_metrics::maxAscent(font);
	int const des = font_metrics::maxDescent(font);
	int h = asc + des;
	int x = 0;
	int y = 0;
	bv.cursor().getPos(x, y);
	y -= asc;
	//lyxerr << "Cursor::show x: " << x << " y: " << y << endl;

	BOOST_ASSERT(work_area_);

	// if it doesn't touch the screen, don't try to show it
	if (y + h < 0 || y >= work_area_->height())
		return;

	cursor_visible_ = true;
	work_area_->showCursor(x, y, h, shape);
}


void GuiCursor::hide()
{
	if (!cursor_visible_)
		return;

	cursor_visible_ = false;
	BOOST_ASSERT(work_area_);
	work_area_->removeCursor();
}


void GuiCursor::toggle(BufferView & bv)
{
	if (cursor_visible_)
		hide();
	else
		show(bv);
}


void GuiCursor::prepare()
{
	cursor_visible_ = false;
}

} // namespace frontend
} // namespace lyx

