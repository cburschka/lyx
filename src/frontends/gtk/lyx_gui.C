/**
 * \file gtk/lyx_gui.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjnes
 * \author John Levon
 * \author Huang Ying
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

#include "lyx_gui.h"

#include "debug.h"
#include "funcrequest.h"
#include "gettext.h"

#include "Color.h"
#include "LColor.h"
#include "LyXAction.h"
#include "lyx_main.h"
#include "lyxrc.h"
#include "lyxfont.h"
#include "graphics/LoaderQueue.h"

#include "io_callback.h"

// FIXME: move this stuff out again
#include "lyxfunc.h"
#include "lyxserver.h"
#include "lyxsocket.h"
#include "BufferView.h"

#include "GuiApplication.h"
#include "GuiImplementation.h"
#include "GView.h"
#include "GtkmmX.h"

#include "GWorkArea.h"

#include "support/lyxlib.h"
#include "support/os.h"
#include "support/filetools.h"
#include "support/package.h"

#include <gtkmm.h>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include <fcntl.h>

#include <sstream>
#include <iomanip>

namespace os = lyx::support::os;

using std::ostringstream;
using std::string;

using lyx::support::package;

using lyx::frontend::colorCache;
using lyx::frontend::Gui;
using lyx::frontend::GuiApplication;
using lyx::frontend::GuiImplementation;
using lyx::frontend::GView;


namespace {

/// estimate DPI from X server
int getDPI()
{
	//TODO use GDK instead
	Screen * scr = ScreenOfDisplay(getDisplay(), getScreen());
	return int(((HeightOfScreen(scr) * 25.4 / HeightMMOfScreen(scr)) +
		(WidthOfScreen(scr) * 25.4 / WidthMMOfScreen(scr))) / 2);
}

} // namespace anon

GuiApplication * guiApp;

int lyx_gui::exec(int & argc, char * argv[])
{
	guiApp = new GuiApplication(argc, argv);
	theApp = guiApp;

	return LyX::ref().exec2(argc, argv);
}


void lyx_gui::exit(int status)
{
	guiApp->exit(status);
}


bool lyx_gui::getRGBColor(LColor_color col, lyx::RGBColor & rgbcol)
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


string const lyx_gui::hexname(LColor_color col)
{
	lyx::RGBColor rgbcol;
	if (!getRGBColor(col, rgbcol)) {
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


void lyx_gui::update_color(LColor_color /*col*/)
{
	colorCache.clear();
}


void lyx_gui::update_fonts()
{
	fontLoader.update();
}


bool lyx_gui::font_available(LyXFont const & font)
{
	return fontLoader.available(font);
}


namespace {

std::map<int, boost::shared_ptr<io_callback> > callbacks;

} // NS anon


void lyx_gui::register_socket_callback(int fd,
				       boost::function<void()> func)
{
	callbacks[fd] = boost::shared_ptr<io_callback>(new io_callback(fd, func));
}


void lyx_gui::unregister_socket_callback(int fd)
{
	callbacks.erase(fd);
}


void lyx_gui::sync_events()
{
	// FIXME
}
