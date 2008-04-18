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
#include "frontends/FontMetrics.h"

#include "FuncRequest.h"
#include "LyXFunc.h"

#include "BufferView.h"
#include "Buffer.h"
#include "BufferParams.h"
#include "CoordCache.h"
#include "Cursor.h"
#include "debug.h"
#include "Language.h"
#include "Color.h"
#include "Font.h"
#include "LyXRC.h"
#include "Text.h"
#include "LyXView.h"
#include "MetricsInfo.h"

#include "gettext.h"
#include "support/ForkedcallsController.h"

#include <boost/utility.hpp>
#include <boost/bind.hpp>
#include <boost/current_function.hpp>

using lyx::support::ForkedcallsController;

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
namespace frontend {

WorkArea::WorkArea(int id, LyXView & lyx_view)
	: buffer_view_(0), lyx_view_(lyx_view), greyed_out_(true),
	  id_(id), cursor_visible_(false), cursor_timeout_(400)
{
	// Start loading the pixmap as soon as possible
	//if (lyxrc.show_banner) {
	//	showBanner();
	//}

	// Setup the signals
	timecon = cursor_timeout_.timeout
		.connect(boost::bind(&WorkArea::toggleCursor, this));

	cursor_timeout_.start();
}


void WorkArea::setBufferView(BufferView * buffer_view)
{
	if (buffer_view_) {
		message_connection_.disconnect();
		lyx_view_.disconnectBufferView();
	}

	hideCursor();
	buffer_view_ = buffer_view;
	toggleCursor();

	message_connection_ = buffer_view_->message.connect(
			boost::bind(&WorkArea::displayMessage, this, _1));

	lyx_view_.connectBufferView(*buffer_view);
}


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
	if (!buffer_view_ || !buffer_view_->buffer()) {
		greyed_out_ = true;
		// The argument here are meaningless.
		expose(1,1,1,1);
		return;
	}

	// No need to do anything if this is the current view. The BufferView
	// metrics are already up to date.
	if (&lyx_view_ != theApp()->currentView()) {
		// FIXME: it would be nice to optimize for the off-screen case.
		buffer_view_->updateMetrics(false);
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
	greyed_out_ = false;

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


void WorkArea::processKeySym(KeySymbolPtr key, key_modifier::state state)
{
	// In order to avoid bad surprise in the middle of an operation, we better stop
	// the blinking cursor.
	stopBlinkingCursor();

	theLyXFunc().setLyXView(&lyx_view_);
	theLyXFunc().processKeySym(key, state);

	/* When we move around, or type, it's nice to be able to see
	 * the cursor immediately after the keypress.
	 */
	startBlinkingCursor();
}


void WorkArea::dispatch(FuncRequest const & cmd0, key_modifier::state k)
{
	// Handle drag&drop
	if (cmd0.action == LFUN_FILE_OPEN) {
		lyx_view_.dispatch(cmd0);
		return;
	}

	theLyXFunc().setLyXView(&lyx_view_);

	FuncRequest cmd;

	if (cmd0.action == LFUN_MOUSE_PRESS) {
		if (k == key_modifier::shift)
			cmd = FuncRequest(cmd0, "region-select");
		else if (k == key_modifier::ctrl)
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

	bool const needRedraw = buffer_view_->workAreaDispatch(cmd);

	if (needRedraw)
		buffer_view_->buffer()->changed();

	// Skip these when selecting
	if (cmd.action != LFUN_MOUSE_MOTION) {
		lyx_view_.updateLayoutChoice();
		lyx_view_.updateMenubar();
		lyx_view_.updateToolbars();
	}

	// GUI tweaks except with mouse motion with no button pressed.
	if (!(cmd.action == LFUN_MOUSE_MOTION
		&& cmd.button() == mouse_button::none)) {
		// Slight hack: this is only called currently when we
		// clicked somewhere, so we force through the display
		// of the new status here.
		lyx_view_.clearMessage();

		// Show the cursor immediately after any operation.
		startBlinkingCursor();
	}
}


void WorkArea::resizeBufferView()
{
	buffer_view_->workAreaResize(width(), height());
}


void WorkArea::updateScrollbar()
{
	buffer_view_->updateScrollbar();
	ScrollbarParameters const & scroll_ = buffer_view_->scrollbarParameters();
	setScrollbarParams(scroll_.height, scroll_.position,
		scroll_.lineScrollHeight);
}


void WorkArea::scrollBufferView(int position)
{
	stopBlinkingCursor();
	buffer_view_->scrollDocView(position);
	redraw();
	if (lyxrc.cursor_follows_scrollbar) {
		buffer_view_->setCursorFromScrollbar();
		lyx_view_.updateLayoutChoice();
	}
	// Show the cursor immediately after any operation.
	startBlinkingCursor();
}


void WorkArea::showCursor()
{
	if (cursor_visible_)
		return;

	if (!buffer_view_->buffer())
		return;

	CursorShape shape = BAR_SHAPE;

	Text const & text = *buffer_view_->cursor().innerText();
	Font const & realfont = text.real_current_font;
	BufferParams const & bp = buffer_view_->buffer()->params();
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
	if (buffer_view_->buffer()) {

		if (cursor_visible_)
			hideCursor();
		else
			showCursor();

		// Use this opportunity to deal with any child processes that
		// have finished but are waiting to communicate this fact
		// to the rest of LyX.
		ForkedcallsController & fcc = ForkedcallsController::get();
		fcc.handleCompletedProcesses();
	}

	cursor_timeout_.restart();
}


void WorkArea::displayMessage(lyx::docstring const & message)
{
	lyx_view_.message(message);
}

} // namespace frontend
} // namespace lyx
