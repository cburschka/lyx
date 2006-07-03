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

#include "WorkArea.h"

#include "font_metrics.h"
#include "lyx_gui.h"
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

WorkArea::WorkArea(BufferView * buffer_view)
	:  buffer_view_(buffer_view), greyed_out_(true)
{
	// Start loading the pixmap as soon as possible
	if (lyxrc.show_banner) {
		SplashScreen const & splash = SplashScreen::get();
		splash.connect(boost::bind(&WorkArea::checkAndGreyOut, this));
		splash.startLoading();
	}
}


void WorkArea::setBufferView(BufferView * buffer_view)
{
	buffer_view_ = buffer_view;
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


void WorkArea::redraw(BufferView & bv, ViewMetricsInfo const & vi)
{
	greyed_out_ = false;
	getPainter().start();
	paintText(*buffer_view_, vi);
	lyxerr[Debug::DEBUG] << "Redraw screen" << endl;
	int const ymin = std::max(vi.y1, 0);
	int const ymax =
		( vi.p2 < vi.size - 1 ?  vi.y2 : height() );
	expose(0, ymin, width(), ymax - ymin);
	getPainter().end();
	theCoords.doneUpdating();
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

		getPainter().text(x, y, splash.text(), splash.font());
	}
	expose(0, 0, width(), height());
	getPainter().end();
}

} // namespace frontend
} // namespace lyx
