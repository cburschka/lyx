/**
 * \file qt4/GuiApplication.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

// Too hard to make concept checks work with this file
#ifdef _GLIBCXX_CONCEPT_CHECKS
#undef _GLIBCXX_CONCEPT_CHECKS
#endif
#ifdef _GLIBCPP_CONCEPT_CHECKS
#undef _GLIBCPP_CONCEPT_CHECKS
#endif

#include "GuiApplication.h"
#include "io_callback.h"

#include "GtkmmX.h"

#include "BufferView.h"
#include "Color.h"

#include "graphics/LoaderQueue.h"

#include "support/lstrings.h"
#include "support/os.h"
#include "support/package.h"

#include "lyx_main.h"
#include "lyxrc.h"
#include "debug.h"

#include <gtkmm.h>

#include "LyXGdkImage.h"

#include <iomanip>


using lyx::support::subst;

using std::string;
using std::endl;


namespace {

std::map<int, boost::shared_ptr<io_callback> > callbacks;

/// estimate DPI from X server
int getDPI()
{
	//TODO use GDK instead
	Screen * scr = ScreenOfDisplay(getDisplay(), getScreen());
	return int(((HeightOfScreen(scr) * 25.4 / HeightMMOfScreen(scr)) +
		(WidthOfScreen(scr) * 25.4 / WidthMMOfScreen(scr))) / 2);
}

} // namespace anon


namespace lyx {

frontend::Application * createApplication(int & argc, char * argv[])
{
	return new frontend::GuiApplication(argc, argv);
}

namespace frontend {

GuiApplication::GuiApplication(int & argc, char ** argv)
	: Gtk::Main(argc, argv), Application(argc, argv)
{
	using namespace lyx::graphics;
	Image::newImage = boost::bind(&LyXGdkImage::newImage);
	Image::loadableFormats = boost::bind(&LyXGdkImage::loadableFormats);

	// needs to be done before reading lyxrc
	lyxrc.dpi = getDPI();

	LoaderQueue::setPriority(10,100);
}


Clipboard& GuiApplication::clipboard()
{
	return clipboard_;
}


Selection& GuiApplication::selection()
{
	return selection_;
}


int const GuiApplication::exec()
{
	run();
	return EXIT_SUCCESS;
}


void GuiApplication::exit(int /*status*/)
{
	// FIXME: Don't ignore status
	guiApp->quit();
}


string const GuiApplication::romanFontName()
{
	return "times";
}


string const GuiApplication::sansFontName()
{
	return "helvetica";
}


string const GuiApplication::typewriterFontName()
{
	return "courier";
}


void GuiApplication::syncEvents()
{
	// FIXME
}


bool GuiApplication::getRgbColor(LColor_color col,
	lyx::RGBColor & rgbcol)
{
	Gdk::Color gdkColor;
	Gdk::Color * gclr = colorCache.getColor(col);
	if (!gclr) {
		gclr = &gdkColor;
		if(!gclr->parse(lcolor.getX11Name(col))) {
			rgbcol.r = 0;
			rgbcol.g = 0;
			rgbcol.b = 0;
			return false;
		}
	}

	// Note that X stores the RGB values in the range 0 - 65535
	// whilst we require them in the range 0 - 255.
	rgbcol.r = gclr->get_red() / 256;
	rgbcol.g = gclr->get_green() / 256;
	rgbcol.b = gclr->get_blue() / 256;
	return true;
}


string const GuiApplication::hexName(LColor_color col)
{
	lyx::RGBColor rgbcol;
	if (!getRgbColor(col, rgbcol)) {
		lyxerr << "X can't find color for \"" << lcolor.getLyXName(col)
		       << '"' << std::endl;
		return string();
	}

	std::ostringstream os;

	os << std::setbase(16) << std::setfill('0')
	   << std::setw(2) << rgbcol.r
	   << std::setw(2) << rgbcol.g
	   << std::setw(2) << rgbcol.b;

	return os.str();
}


void GuiApplication::updateColor(LColor_color)
{
	colorCache.clear();
}


void GuiApplication::registerSocketCallback(int fd, boost::function<void()> func)
{
	callbacks[fd] = boost::shared_ptr<io_callback>(new io_callback(fd, func));
}


void GuiApplication::unregisterSocketCallback(int fd)
{
	callbacks.erase(fd);
}

} // namespace frontend
} // namespace lyx
