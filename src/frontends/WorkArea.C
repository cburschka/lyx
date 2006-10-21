/**
 * \file WorkArea.C
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

#include "funcrequest.h"
#include "lyxfunc.h"
#include "Painter.h"

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
#include "LyXView.h"
#include "metricsinfo.h"
#include "paragraph.h"
#include "rowpainter.h"
#include "version.h"

#include "graphics/GraphicsImage.h"
#include "graphics/GraphicsLoader.h"

#include "gettext.h"
#include "support/filetools.h" // LibFileSearch
#include "support/forkedcontr.h"

#include <boost/utility.hpp>
#include <boost/bind.hpp>
#include <boost/current_function.hpp>

using lyx::support::libFileSearch;
using lyx::support::ForkedcallsController;

using std::endl;
using std::min;
using std::max;
using std::string;


namespace lyx {
namespace frontend {

// FIXME: The SplashScreen should be transfered to the
// LyXView and create a WorkArea only when a new buffer exists. This
// will allow to call WorkArea::redraw() in the constructor.
class SplashScreen : boost::noncopyable, boost::signals::trackable {
public:
	/// This is a singleton class. Get the instance.
	static SplashScreen const & get();
	///
	lyx::graphics::Image const * image() const { return loader_.image(); }
	///
	string const & text() const { return text_; }
	///
	LyXFont const & font() const { return font_; }
	///
	void connect(lyx::graphics::Loader::slot_type const & slot) const {
		loader_.connect(slot);
	}
	///
	void startLoading() const {
		if (loader_.status() == lyx::graphics::WaitingToLoad)
			loader_.startLoading();
	}

private:
	/** Make the c-tor private so we can control how many objects
	 *  are instantiated.
	 */
	SplashScreen();

	///
	lyx::graphics::Loader loader_;
	/// The text to be written on top of the pixmap
	string const text_;
	/// in this font...
	LyXFont font_;
};


SplashScreen const & SplashScreen::get()
{
	static SplashScreen singleton;
	return singleton;
}


SplashScreen::SplashScreen()
	: text_(lyx_version ? lyx_version : "unknown")
{
	if (!lyxrc.show_banner)
		return;

	string const file = libFileSearch("images", "banner", "ppm");
	if (file.empty())
		return;

	// The font used to display the version info
	font_.setFamily(LyXFont::SANS_FAMILY);
	font_.setSeries(LyXFont::BOLD_SERIES);
	font_.setSize(LyXFont::SIZE_NORMAL);
	font_.setColor(LColor::yellow);

	// Load up the graphics file
	loader_.reset(file);
}

namespace {

// All the below connection objects are needed because of a bug in some
// versions of GCC (<=2.96 are on the suspects list.) By having and assigning
// to these connections we avoid a segfault upon startup, and also at exit.
// (Lgb)

boost::signals::connection timecon;

} // anon namespace

WorkArea::WorkArea(LyXView & lyx_view)
	:  buffer_view_(0), lyx_view_(lyx_view), greyed_out_(true),
	cursor_visible_(false), cursor_timeout_(400)
{
	// Start loading the pixmap as soon as possible
	if (lyxrc.show_banner) {
		SplashScreen const & splash = SplashScreen::get();
		splash.connect(boost::bind(&WorkArea::checkAndGreyOut, this));
		splash.startLoading();
	}

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

	theApp->setBufferView(buffer_view);

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


void WorkArea::checkAndGreyOut()
{
	if (greyed_out_)
		greyOut();
}


void WorkArea::redraw()
{
	if (!buffer_view_)
		return;

	if (!buffer_view_->buffer()) {
		greyOut();
		updateScrollbar();
		return;
	}

	buffer_view_->updateMetrics(false);

	updateScrollbar();

	ViewMetricsInfo const & vi = buffer_view_->viewMetricsInfo();
	greyed_out_ = false;
	getPainter().start();
	paintText(*buffer_view_, vi, getPainter());
	lyxerr[Debug::WORKAREA] << "WorkArea::redraw screen" << endl;
	int const ymin = std::max(vi.y1, 0);
	int const ymax =
		( vi.p2 < vi.size - 1 ?  vi.y2 : height() );
	expose(0, ymin, width(), ymax - ymin);
	getPainter().end();

	lyxerr[Debug::WORKAREA]
	<< "  ymin = " << ymin << "  width() = " << width()
		<< "  ymax-ymin = " << ymax-ymin << std::endl;

	if (lyxerr.debugging(Debug::WORKAREA)) {
		buffer_view_->coordCache().dump();
	}
}


void WorkArea::processKeySym(LyXKeySymPtr key,
							 key_modifier::state state)
{
	hideCursor();

	theLyXFunc().setLyXView(&lyx_view_);
	theLyXFunc().processKeySym(key, state);

	/* This is perhaps a bit of a hack. When we move
	 * around, or type, it's nice to be able to see
	 * the cursor immediately after the keypress. So
	 * we reset the toggle timeout and force the visibility
	 * of the cursor. Note we cannot do this inside
	 * dispatch() itself, because that's called recursively.
	 */
//	if (buffer_view_->buffer())
	toggleCursor();
	
	// uneeded "redraw()" call commented out for now.
	// When/if the call to LyXView::redrawWorkArea() in "lyxfunc.C:1610"
	// is not needed anymore, this line should be uncommented out
	//redraw();
}


void WorkArea::dispatch(FuncRequest const & cmd0)
{
	// Handle drag&drop
	if (cmd0.action == LFUN_FILE_OPEN) {
		lyx_view_.dispatch(cmd0);
		return;
	}

	theLyXFunc().setLyXView(&lyx_view_);

	buffer_view_->workAreaDispatch(cmd0);

	// Skip these when selecting
	if (cmd0.action != LFUN_MOUSE_MOTION) {
		lyx_view_.updateLayoutChoice();
		lyx_view_.updateMenubar();
		lyx_view_.updateToolbars();
	}

	// Slight hack: this is only called currently when we
	// clicked somewhere, so we force through the display
	// of the new status here.
	lyx_view_.clearMessage();

	redraw();
}


void WorkArea::resizeBufferView()
{
	lyx_view_.busy(true);
	lyx_view_.message(_("Formatting document..."));
	buffer_view_->workAreaResize(width(), height());
	lyx_view_.updateLayoutChoice();
	redraw();
	lyx_view_.busy(false);
	lyx_view_.clearMessage();
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
	buffer_view_->scrollDocView(position);
	redraw();
	hideCursor();
	if (lyxrc.cursor_follows_scrollbar) {
		buffer_view_->setCursorFromScrollbar();
		lyx_view_.updateLayoutChoice();
	}
	toggleCursor();
}


void WorkArea::greyOut()
{
	greyed_out_ = true;
	getPainter().start();

	getPainter().fillRectangle(0, 0,
		width(),
		height(),
		LColor::bottomarea);

	// Add a splash screen to the centre of the work area
	SplashScreen const & splash = SplashScreen::get();
	lyx::graphics::Image const * const splash_image = splash.image();
	if (splash_image) {
		int const w = splash_image->getWidth();
		int const h = splash_image->getHeight();

		int x = (width() - w) / 2;
		int y = (height() - h) / 2;

		getPainter().image(x, y, w, h, *splash_image);

		x += 260;
		y += 265;

		string stext = splash.text();
		docstring dstext(stext.begin(), stext.end());
		getPainter().text(x, y, dstext, splash.font());
	}
	expose(0, 0, width(), height());
	getPainter().end();
}


void WorkArea::showCursor()
{
	if (cursor_visible_)
		return;

	if (!buffer_view_->buffer())
		return;

	CursorShape shape = BAR_SHAPE;

	LyXText const & text = *buffer_view_->getLyXText();
	LyXFont const & realfont = text.real_current_font;
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

	LyXFont const font = buffer_view_->cursor().getFont();
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
