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

#include "LColor.h"
#include "LyXAction.h"
#include "lyx_main.h"
#include "lyxrc.h"
#include "lyxfont.h"
#include "graphics/LoaderQueue.h"

#include "io_callback.h"

// FIXME: move this stuff out again
#include "bufferlist.h"
#include "lyxfunc.h"
#include "lyxserver.h"
#include "lyxsocket.h"
#include "BufferView.h"

#include "GView.h"
#include "GtkmmX.h"

#include "xftFontLoader.h"
#include "GWorkArea.h"

#include "support/lyxlib.h"
#include "support/os.h"
#include "support/filetools.h"
#include "support/package.h"

#include <gtkmm.h>

#include "LyXGdkImage.h"

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
using lyx::frontend::GView;


extern BufferList bufferlist;

// FIXME: wrong place !
LyXServer * lyxserver;
LyXServerSocket * lyxsocket;

bool lyx_gui::use_gui = true;

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


void lyx_gui::exec(int & argc, char * argv[])
{
	new Gtk::Main(argc, argv);

	using namespace lyx::graphics;
	Image::newImage = boost::bind(&LyXGdkImage::newImage);
	Image::loadableFormats = boost::bind(&LyXGdkImage::loadableFormats);

	locale_init();

	// must do this /before/ lyxrc gets read
	lyxrc.dpi = getDPI();

	LyX::ref().exec2(argc, argv);
}


void lyx_gui::parse_lyxrc()
{
}


void lyx_gui::start(string const & batch, std::vector<string> const & files)
{
	boost::shared_ptr<GView> view_ptr(new GView);
	LyX::ref().addLyXView(view_ptr);

	GView & view = *view_ptr.get();
	view.show();
	view.init();

	// FIXME: server code below needs moving

	lyxserver = new LyXServer(&view.getLyXFunc(), lyxrc.lyxpipes);
	lyxsocket = new LyXServerSocket(&view.getLyXFunc(),
			  os::internal_path(package().temp_dir() + "/lyxsocket"));

	for_each(files.begin(), files.end(),
		 bind(&BufferView::loadLyXFile, view.view(), _1, true));

	// handle the batch commands the user asked for
	if (!batch.empty()) {
		view.getLyXFunc().dispatch(lyxaction.lookupFunc(batch));
	}

	Gtk::Main::run();

	// FIXME: breaks emergencyCleanup
	delete lyxsocket;
	delete lyxserver;
}


void lyx_gui::exit(int /*status*/)
{
	// FIXME: Don't ignore status
	Gtk::Main::quit();
}


FuncStatus lyx_gui::getStatus(FuncRequest const & ev)
{
	FuncStatus flag;
	switch (ev.action) {
	case LFUN_DIALOG_SHOW:
		if (ev.argument == "preamble")
			flag.unknown(true);
		break;
	case LFUN_TOOLTIPS_TOGGLE:
		flag.unknown(true);
		break;
	default:
		break;
	}

	return flag;
}


string const lyx_gui::hexname(LColor_color col)
{
	Gdk::Color gdkColor;
	Gdk::Color * gclr = colorCache.getColor(col);
	if (!gclr) {
		gclr = &gdkColor;
		gclr->parse(lcolor.getX11Name(col));
	}

	std::ostringstream os;

	// Note that X stores the RGB values in the range 0 - 65535
	// whilst we require them in the range 0 - 255.
	os << std::setbase(16) << std::setfill('0')
	   << std::setw(2) << (gclr->get_red() / 256)
	   << std::setw(2) << (gclr->get_green() / 256)
	   << std::setw(2) << (gclr->get_blue() / 256);

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


string const lyx_gui::roman_font_name()
{
	return "times";
}


string const lyx_gui::sans_font_name()
{
	return "helvetica";
}


string const lyx_gui::typewriter_font_name()
{
	return "courier";
}


void lyx_gui::sync_events()
{
	// FIXME
}
