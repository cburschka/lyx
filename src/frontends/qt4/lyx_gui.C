/**
 * \file qt4/lyx_gui.C
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

#include "lyx_gui.h"

// FIXME: move this stuff out again
#include "Color.h"
#include "funcrequest.h"
#include "LColor.h"
#include "lyx_main.h"
#include "LyXAction.h"
#include "lyxfunc.h"
#include "lyxrc.h"


#include "support/lstrings.h"

#include "GuiView.h"
#include "QLImage.h"
#include "qt_helpers.h"
#include "socket_callback.h"
#include "GuiApplication.h"

#include <QApplication>
#include <QEventLoop>
#include <QTranslator>
#include <QTextCodec>
#include <QLocale>
#include <QLibraryInfo>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>


using lyx::support::ltrim;

using lyx::frontend::GuiImplementation;
using lyx::frontend::GuiView;
using lyx::frontend::GuiApplication;

using boost::shared_ptr;

using std::map;
using std::vector;
using std::string;

lyx::frontend::GuiApplication * guiApp;

namespace {

map<int, shared_ptr<socket_callback> > socket_callbacks;

} // namespace anon

namespace lyx_gui {

int exec(int & argc, char * argv[])
{
	/*
	FIXME : Abdel 29/05/2006 (younes.a@free.fr)
	reorganize this code. In particular make sure that this
	advice from Qt documentation is respected:

		Since the QApplication object does so much initialization, it
		must be created before any other objects related to the user
		interface are created.

	Right now this is not the case, I suspect that a number of global variables
	contains Qt object that are initialized before the passage through
	parse_init(). This might also explain the message displayed by Qt
	that caused the hanging:

	QObject::killTimer: timers cannot be stopped from another thread

	I hope that the problem will disappear automagically when we get rid of
	lyx_gui entirely, thus using theApp directly throughout the code for LyXFunc,
	Clipboard and Selection access.
	*/

#if defined(Q_WS_WIN) && !defined(Q_CYGWIN_WIN)
	static GuiApplication app(argc, argv);
#else
	GuiApplication app(argc, argv);
#endif

	guiApp = &app;
	theApp = guiApp;

	return LyX::ref().exec2(argc, argv);
}


void sync_events()
{
	// This is the ONLY place where processEvents may be called.
	// During screen update/ redraw, this method is disabled to
	// prevent keyboard events being handed to the LyX core, where
	// they could cause re-entrant calls to screen update.
	guiApp->processEvents(QEventLoop::ExcludeUserInputEvents);
}


bool getRGBColor(LColor_color col, lyx::RGBColor & rgbcol)
{
	QColor const & qcol = guiApp->colorCache().get(col);
	if (!qcol.isValid()) {
		rgbcol.r = 0;
		rgbcol.g = 0;
		rgbcol.b = 0;
		return false;
	}
	rgbcol.r = qcol.red();
	rgbcol.g = qcol.green();
	rgbcol.b = qcol.blue();
	return true;
}


string const hexname(LColor_color col)
{
	return ltrim(fromqstr(guiApp->colorCache().get(col).name()), "#");
}


void update_color(LColor_color)
{
	// FIXME: Bleh, can't we just clear them all at once ?
	guiApp->colorCache().clear();
}


void register_socket_callback(int fd, boost::function<void()> func)
{
	socket_callbacks[fd] = shared_ptr<socket_callback>(new socket_callback(fd, func));
}


void unregister_socket_callback(int fd)
{
	socket_callbacks.erase(fd);
}

}; // namespace lyx_gui
