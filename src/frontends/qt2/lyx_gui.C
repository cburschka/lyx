/**
 * \file qt2/lyx_gui.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>


#include "support/lyxlib.h"
#include "support/os.h"
#include "support/filetools.h"
#include "support/lstrings.h"
#include "debug.h"
#include "qt_helpers.h"

#include "lyx_gui.h"
#include "lyx_main.h"
#include "lyxrc.h"
#include "lyxfont.h"
#include "funcrequest.h"
#include "graphics/LoaderQueue.h"

// FIXME: move this stuff out again
#include "bufferlist.h"
#include "lyxfunc.h"
#include "lyxserver.h"
#include "BufferView.h"

// Dear Lord, deliver us from Evil,
// aka the Qt headers
#include <boost/shared_ptr.hpp>
#include <boost/function/function0.hpp>
#include <boost/signals/signal1.hpp>
#include <boost/bind.hpp>

#include "QtView.h"
#include "QLImage.h"
#include "qfont_loader.h"
#include "io_callback.h"

#include <qapplication.h>
#include <qwidget.h>
#include <qpaintdevicemetrics.h>

#include <fcntl.h>
#include <cstdlib>

#ifndef CXX_GLOBAL_CSTD
using std::exit;
#endif

using std::vector;
using std::map;
using std::endl;

extern BufferList bufferlist;

namespace {

float getDPI()
{
	QWidget w;
	QPaintDeviceMetrics pdm(&w);
	return 0.5 * (pdm.logicalDpiX() + pdm.logicalDpiY());
}

map<int, io_callback *> io_callbacks;

} // namespace anon


// FIXME: wrong place !
LyXServer * lyxserver;

// in QLyXKeySym.C
extern void initEncodings();

#ifdef Q_WS_X11
extern bool lyxX11EventFilter(XEvent * xev);
#endif


class LQApplication : public QApplication
{
public:
	LQApplication(int & argc, char ** argv);
	~LQApplication();
#ifdef Q_WS_X11
	bool x11EventFilter (XEvent * ev) { return lyxX11EventFilter(ev); }
#endif
};


LQApplication::LQApplication(int & argc, char ** argv)
	: QApplication(argc, argv)
{}


LQApplication::~LQApplication()
{}


void lyx_gui::parse_init(int & argc, char * argv[])
{
	static LQApplication a(argc, argv);

	using namespace grfx;

	Image::newImage = boost::bind(&QLImage::newImage);
	Image::loadableFormats = boost::bind(&QLImage::loadableFormats);

	// needs to be done before reading lyxrc
	lyxrc.dpi = getDPI();

	initEncodings();

	LoaderQueue::setPriority(10,100);
}


void lyx_gui::parse_lyxrc()
{
}


void lyx_gui::start(string const & batch, vector<string> const & files)
{
	// initial geometry
	int xpos = -1;
	int ypos = -1;
	unsigned int width = 690;
	unsigned int height = 510;

	QtView view(width, height);
	view.show();
	view.init();

	Buffer * last = 0;

	// FIXME: some code below needs moving

	lyxserver = new LyXServer(&view.getLyXFunc(), lyxrc.lyxpipes);

	vector<string>::const_iterator cit = files.begin();
	vector<string>::const_iterator end = files.end();
	for (; cit != end; ++cit) {
		Buffer * b = bufferlist.loadLyXFile(*cit);
		if (b) {
			last = b;
		}
	}

	// switch to the last buffer successfully loaded
	if (last) {
		view.view()->buffer(last);
	}

	// handle the batch commands the user asked for
	if (!batch.empty()) {
		view.getLyXFunc().dispatch(batch);
	}

	qApp->exec();

	// FIXME
	delete lyxserver;
	lyxserver = 0;
}


void lyx_gui::exit()
{
	delete lyxserver;
	lyxserver = 0;

	// we cannot call qApp->exit(0) - that could return us
	// into a static dialog return in the lyx code (for example,
	// load autosave file QMessageBox. We have to just get the hell
	// out.

	::exit(0);
}


FuncStatus lyx_gui::getStatus(FuncRequest const & ev)
{
	FuncStatus flag;
	switch (ev.action) {
	case LFUN_LAYOUT_PREAMBLE:
	case LFUN_TOOLTIPS_TOGGLE:
	case LFUN_FORKS_SHOW:
		flag.unknown(true);
		break;
	default:
		break;
	}
	return flag;
}


string const lyx_gui::hexname(LColor::color col)
{
	QColor color(toqstr(lcolor.getX11Name(col)));
	return ltrim(fromqstr(color.name()), "#");
}


void lyx_gui::update_color(LColor::color)
{
	// no need
}


void lyx_gui::update_fonts()
{
	fontloader.update();
}


bool lyx_gui::font_available(LyXFont const & font)
{
	return fontloader.available(font);
}


void lyx_gui::set_read_callback(int fd, LyXComm * comm)
{
	io_callbacks[fd] = new io_callback(fd, comm);
}


void lyx_gui::remove_read_callback(int fd)
{
	map<int, io_callback *>::iterator it = io_callbacks.find(fd);
	if (it != io_callbacks.end()) {
		delete it->second;
		io_callbacks.erase(it);
	}
}
