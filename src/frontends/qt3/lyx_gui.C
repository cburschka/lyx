/**
 * \file qt3/lyx_gui.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "lyx_gui.h"

// FIXME: move this stuff out again
#include "BufferView.h"
#include "Color.h"
#include "funcrequest.h"
#include "LColor.h"
#include "lyx_main.h"
#include "LyXAction.h"
#include "lyxfunc.h"
#include "lyxrc.h"
#include "lyxserver.h"
#include "lyxsocket.h"

#include "support/lstrings.h"
#include "support/os.h"
#include "support/package.h"
#include "debug.h"

// Dear Lord, deliver us from Evil, aka the Qt headers
// Qt defines a macro 'signals' that clashes with a boost namespace.
// All is well if the namespace is visible first.
#include <boost/signal.hpp> // FIXME: Is this needed? (Lgb)
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include "frontends/LyXView.h"
#include "frontends/WorkArea.h"

#include "GuiApplication.h"
#include "QtView.h"
#include "lcolorcache.h"
#include "qfont_loader.h"
#include "QLImage.h"
#include "qt_helpers.h"
#include "socket_callback.h"

#ifdef Q_WS_MACX
#include <Carbon/Carbon.h>
#endif

#include <qapplication.h>
#if QT_VERSION >= 0x030100
#include <qeventloop.h>
#endif
#include <qpaintdevicemetrics.h>
#include <qtranslator.h>
#include <qtextcodec.h>

using lyx::support::ltrim;
using lyx::support::package;

using lyx::frontend::GuiApplication;
using lyx::frontend::QtView;

namespace os = lyx::support::os;

using boost::shared_ptr;

using std::map;
using std::vector;
using std::string;


namespace {

map<int, shared_ptr<socket_callback> > socket_callbacks;

} // namespace anon


GuiApplication * guiApp;

namespace lyx_gui {

int exec(int & argc, char * argv[])
{
	GuiApplication app(argc, argv);

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
#if QT_VERSION >= 0x030100
	qApp->eventLoop()->processEvents(QEventLoop::ExcludeUserInput);
#endif
}


bool getRGBColor(LColor_color col, lyx::RGBColor & rgbcol)
{
	QColor const & qcol = lcolorcache.get(col);
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
	return ltrim(fromqstr(lcolorcache.get(col).name()), "#");
}


void update_color(LColor_color)
{
	// FIXME: Bleh, can't we just clear them all at once ?
	lcolorcache.clear();
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
