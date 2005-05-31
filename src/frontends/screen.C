/**
 * \file screen.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 *
 * Splash screen code added by Angus Leeming
 */

#include <config.h>

#include "screen.h"
#include "font_metrics.h"
#include "lyx_gui.h"
#include "Painter.h"
#include "WorkArea.h"

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

#include "insets/updatableinset.h"

#include "graphics/GraphicsImage.h"
#include "graphics/GraphicsLoader.h"

#include "support/filetools.h" // LibFileSearch

#include <boost/utility.hpp>
#include <boost/bind.hpp>
#include <boost/signals/trackable.hpp>

using lyx::support::LibFileSearch;

using std::endl;
using std::min;
using std::max;
using std::string;


namespace {

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

	string const file = LibFileSearch("images", "banner", "ppm");
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

} // namespace anon


LyXScreen::LyXScreen()
	: greyed_out_(true), cursor_visible_(false)
{
	// Start loading the pixmap as soon as possible
	if (lyxrc.show_banner) {
		SplashScreen const & splash = SplashScreen::get();
		splash.connect(boost::bind(&LyXScreen::checkAndGreyOut, this));
		splash.startLoading();
	}
}


LyXScreen::~LyXScreen()
{
}


void LyXScreen::checkAndGreyOut()
{
	if (greyed_out_)
		greyOut();
}


void LyXScreen::showCursor(BufferView & bv)
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

	if (!samelang || isrtl != bp.language->RightToLeft()) {
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
	//lyxerr << "LyXScreen::showCursor x: " << x << " y: " << y << endl;

	// if it doesn't touch the screen, don't try to show it
	if (y + h < 0 || y >= workarea().workHeight())
		return;

	cursor_visible_ = true;
	showCursor(x, y, h, shape);
}


void LyXScreen::hideCursor()
{
	if (!cursor_visible_)
		return;

	cursor_visible_ = false;
	removeCursor();
}


void LyXScreen::toggleCursor(BufferView & bv)
{
	if (cursor_visible_)
		hideCursor();
	else
		showCursor(bv);
}


void LyXScreen::prepareCursor()
{
	cursor_visible_ = false;
}


void LyXScreen::redraw(BufferView & bv, ViewMetricsInfo const & vi)
{
	greyed_out_ = false;
	workarea().getPainter().start();
	paintText(bv, vi);
	lyxerr[Debug::DEBUG] << "Redraw screen" << endl;
	expose(0, 0, workarea().workWidth(), workarea().workHeight());
	workarea().getPainter().end();
	theCoords.doneUpdating();
}


void LyXScreen::greyOut()
{
	greyed_out_ = true;
	workarea().getPainter().start();

	workarea().getPainter().fillRectangle(0, 0,
		workarea().workWidth(),
		workarea().workHeight(),
		LColor::bottomarea);

	// Add a splash screen to the centre of the work area
	SplashScreen const & splash = SplashScreen::get();
	lyx::graphics::Image const * const splash_image = splash.image();
	if (splash_image) {
		int const w = splash_image->getWidth();
		int const h = splash_image->getHeight();

		int x = (workarea().workWidth() - w) / 2;
		int y = (workarea().workHeight() - h) / 2;

		workarea().getPainter().image(x, y, w, h, *splash_image);

		x += 260;
		y += 265;

		workarea().getPainter().text(x, y, splash.text(), splash.font());
	}
	expose(0, 0, workarea().workWidth(), workarea().workHeight());
	workarea().getPainter().end();
}
