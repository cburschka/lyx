/**
 * \file WorkArea.cpp
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

#include "frontends/WorkArea.h"

#include "frontends/Application.h"
#include "frontends/Dialogs.h"
#include "frontends/FontMetrics.h"
#include "frontends/LyXView.h"
#include "frontends/WorkAreaManager.h"

#include "BufferView.h"
#include "Buffer.h"
#include "BufferParams.h"
#include "CoordCache.h"
#include "Cursor.h"
#include "debug.h"
#include "Font.h"
#include "FuncRequest.h"
#include "KeySymbol.h"
#include "Language.h"
#include "LyXFunc.h"
#include "LyXRC.h"
#include "MetricsInfo.h"

#include "gettext.h"
#include "support/FileName.h"
#include "support/filetools.h"
#include "support/ForkedcallsController.h"

#include <boost/noncopyable.hpp>
#include <boost/bind.hpp>
#include <boost/current_function.hpp>

using std::endl;
using std::min;
using std::max;
using std::string;


namespace {

// All the below connection objects are needed because of a bug in some
// versions of GCC (<=2.96 are on the suspects list.) By having and assigning
// to these connections we avoid a segfault upon startup, and also at exit.
// (Lgb)

boost::signals::connection timecon;

} // anon namespace

namespace lyx {

using support::ForkedcallsController;
using support::makeDisplayPath;
using support::onlyFilename;

namespace frontend {

WorkArea::WorkArea(Buffer & buffer, LyXView & lv)
	: buffer_view_(new BufferView(buffer)), lyx_view_(&lv),
	cursor_visible_(false), cursor_timeout_(400)
{
	buffer.workAreaManager().add(this);
	// Setup the signals
	timecon = cursor_timeout_.timeout
		.connect(boost::bind(&WorkArea::toggleCursor, this));

	cursor_timeout_.start();
}


WorkArea::~WorkArea()
{
	buffer_view_->buffer().workAreaManager().remove(this);
	delete buffer_view_;
}


void WorkArea::close()
{
	lyx_view_->removeWorkArea(this);
}

//void WorkArea::setLyXView(LyXView * lyx_view)
//{
//	lyx_view_ = lyx_view;
//}


BufferView & WorkArea::bufferView()
{
	return *buffer_view_;
}


BufferView const & WorkArea::bufferView() const
{
	return *buffer_view_;
}


void WorkArea::stopBlinkingCursor()
{
	cursor_timeout_.stop();
	hideCursor();
}


void WorkArea::startBlinkingCursor()
{
	showCursor();
	cursor_timeout_.restart();
}


void WorkArea::redraw()
{
	if (!isVisible())
		// No need to redraw in this case.
		return;

	// No need to do anything if this is the current view. The BufferView
	// metrics are already up to date.
	if (lyx_view_ != theApp()->currentView()) {
		// FIXME: it would be nice to optimize for the off-screen case.
		buffer_view_->updateMetrics();
		buffer_view_->cursor().fixIfBroken();
	}

	updateScrollbar();

	// update cursor position, because otherwise it has to wait until
	// the blinking interval is over
	if (cursor_visible_) {
		hideCursor();
		showCursor();
	}
	
	ViewMetricsInfo const & vi = buffer_view_->viewMetricsInfo();

	LYXERR(Debug::WORKAREA) << "WorkArea::redraw screen" << endl;

	int const ymin = std::max(vi.y1, 0);
	int const ymax = vi.p2 < vi.size - 1 ? vi.y2 : height();

	expose(0, ymin, width(), ymax - ymin);

	//LYXERR(Debug::WORKAREA)
	//<< "  ymin = " << ymin << "  width() = " << width()
//		<< "  ymax-ymin = " << ymax-ymin << std::endl;

	if (lyxerr.debugging(Debug::WORKAREA))
		buffer_view_->coordCache().dump();
}


void WorkArea::processKeySym(KeySymbol const & key, KeyModifier mod)
{
	// In order to avoid bad surprise in the middle of an operation, we better stop
	// the blinking cursor.
	stopBlinkingCursor();

	theLyXFunc().setLyXView(lyx_view_);
	theLyXFunc().processKeySym(key, mod);
}


void WorkArea::dispatch(FuncRequest const & cmd0, KeyModifier mod)
{
	// Handle drag&drop
	if (cmd0.action == LFUN_FILE_OPEN) {
		lyx_view_->dispatch(cmd0);
		return;
	}

	theLyXFunc().setLyXView(lyx_view_);

	FuncRequest cmd;

	if (cmd0.action == LFUN_MOUSE_PRESS) {
		if (mod == ShiftModifier)
			cmd = FuncRequest(cmd0, "region-select");
		else if (mod == ControlModifier)
			cmd = FuncRequest(cmd0, "paragraph-select");
		else
			cmd = cmd0;
	}
	else
		cmd = cmd0;

	// In order to avoid bad surprise in the middle of an operation, we better stop
	// the blinking cursor.
	if (!(cmd.action == LFUN_MOUSE_MOTION
		&& cmd.button() == mouse_button::none))
		stopBlinkingCursor();

	buffer_view_->mouseEventDispatch(cmd);

	// Skip these when selecting
	if (cmd.action != LFUN_MOUSE_MOTION) {
		lyx_view_->updateLayoutChoice(false);
		lyx_view_->updateToolbars();
	}

	// GUI tweaks except with mouse motion with no button pressed.
	if (!(cmd.action == LFUN_MOUSE_MOTION
		&& cmd.button() == mouse_button::none)) {
		// Slight hack: this is only called currently when we
		// clicked somewhere, so we force through the display
		// of the new status here.
		lyx_view_->clearMessage();

		// Show the cursor immediately after any operation.
		startBlinkingCursor();
	}
}


void WorkArea::resizeBufferView()
{
	// WARNING: Please don't put any code that will trigger a repaint here!
	// We are already inside a paint event.
	lyx_view_->setBusy(true);
	buffer_view_->resize(width(), height());
	lyx_view_->updateLayoutChoice(false);
	lyx_view_->setBusy(false);
}


void WorkArea::updateScrollbar()
{
	buffer_view_->updateScrollbar();
	ScrollbarParameters const & scroll_ = buffer_view_->scrollbarParameters();
	setScrollbarParams(scroll_.height, scroll_.position,
		scroll_.lineScrollHeight);
}


void WorkArea::showCursor()
{
	if (cursor_visible_)
		return;

	CursorShape shape = BAR_SHAPE;

	Font const & realfont = buffer_view_->cursor().real_current_font;
	BufferParams const & bp = buffer_view_->buffer().params();
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

	Font const font = buffer_view_->cursor().getFont();
	FontMetrics const & fm = theFontMetrics(font);
	int const asc = fm.maxAscent();
	int const des = fm.maxDescent();
	int h = asc + des;
	int x = 0;
	int y = 0;
	buffer_view_->cursor().getPos(x, y);
	y -= asc;

	// if it doesn't touch the screen, don't try to show it
	if (y + h < 0 || y >= height())
		return;

	cursor_visible_ = true;
	showCursor(x, y, h, shape);
}


void WorkArea::hideCursor()
{
	if (!cursor_visible_)
		return;

	cursor_visible_ = false;
	removeCursor();
}


void WorkArea::toggleCursor()
{
	if (cursor_visible_)
		hideCursor();
	else
		showCursor();

	// Use this opportunity to deal with any child processes that
	// have finished but are waiting to communicate this fact
	// to the rest of LyX.
	ForkedcallsController & fcc = ForkedcallsController::get();
	fcc.handleCompletedProcesses();

	cursor_timeout_.restart();
}

void WorkArea::updateWindowTitle()
{
	docstring maximize_title;
	docstring minimize_title;

	Buffer & buf = buffer_view_->buffer();
	string const cur_title = buf.absFileName();
	if (!cur_title.empty()) {
		maximize_title = makeDisplayPath(cur_title, 30);
		minimize_title = from_utf8(onlyFilename(cur_title));
		if (!buf.isClean()) {
			maximize_title += _(" (changed)");
			minimize_title += char_type('*');
		}
		if (buf.isReadonly())
			maximize_title += _(" (read only)");
	}

	setWindowTitle(maximize_title, minimize_title);
}


void WorkArea::setReadOnly(bool)
{
	updateWindowTitle();
	if (this == lyx_view_->currentWorkArea())
		lyx_view_->getDialogs().updateBufferDependent(false);
}

} // namespace frontend
} // namespace lyx
