/**
 * \file lyx_gui.C
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author unknown
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>

#include "support/lyxlib.h"
#include "support/os.h"
#include "support/filetools.h"
#include "debug.h"
#include "gettext.h"

#include <fcntl.h>
#include <boost/bind.hpp>

#include "lyx_gui.h"
#include "lyx_main.h"
#include "lyxrc.h"
#include "lyxfont.h"

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

#include "QtView.h"
#include "QLImage.h"
#include "qfont_loader.h"
#include "io_callback.h"

#include <qapplication.h>

#ifndef CXX_GLOBAL_CSTD
using std::exit;
#endif

using std::vector;
using std::hex;
using std::endl;

extern BufferList bufferlist;

// FIXME: wrong place !
LyXServer * lyxserver;

void lyx_gui::parse_init(int & argc, char * argv[])
{
	static QApplication a(argc, argv);

	using namespace grfx;

	Image::newImage = boost::bind(&QLImage::newImage);
	Image::loadableFormats = boost::bind(&QLImage::loadableFormats);
}


void lyx_gui::parse_lyxrc()
{
	// FIXME !!!!
	lyxrc.dpi = 95;
}


void lyx_gui::start(string const & batch, vector<string> files)
{
	// initial geometry
	int xpos = -1;
	int ypos = -1;
	unsigned int width = 690;
	unsigned int height = 510;

	QtView view(width, height);
	view.show(xpos, ypos, "LyX");
	view.init();

	Buffer * last = 0;

	// FIXME: some code below needs moving

	lyxserver = new LyXServer(view.getLyXFunc(), lyxrc.lyxpipes);

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
		view.getLyXFunc()->dispatch(batch);
	}

	qApp->exec();

	// FIXME
	delete lyxserver;
}


void lyx_gui::exit()
{
	qApp->exit(0);
}


string const lyx_gui::hexname(LColor::color col)
{
	QColor color(lcolor.getX11Name(col).c_str());
	return color.name().latin1();
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
	// FIXME: "leak"
	new io_callback(fd, comm);
}
