/**
 * \file qt2/lyx_gui.C
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
#include "bufferlist.h"
#include "BufferView.h"
#include "funcrequest.h"
#include "LColor.h"
#include "lyx_main.h"
#include "LyXAction.h"
#include "lyxfunc.h"
#include "lyxrc.h"
#include "lyxserver.h"
#include "lyxsocket.h"

#include "graphics/LoaderQueue.h"

#include "support/lstrings.h"
#include "support/os.h"

// Dear Lord, deliver us from Evil, aka the Qt headers
// Qt defines a macro 'signals' that clashes with a boost namespace.
// All is well if the namespace is visible first.
#include <boost/signals/signal1.hpp>
#include <boost/bind.hpp>

#include "QtView.h"
#include "io_callback.h"
#include "lcolorcache.h"
#include "qfont_loader.h"
#include "QLImage.h"
#include "qt_helpers.h"
#include "socket_callback.h"

#ifdef Q_WS_MACX
#include <Carbon/Carbon.h>
#endif

#include <qapplication.h>
#include <qpaintdevicemetrics.h>

using lyx::support::ltrim;

using lyx::frontend::QtView;

namespace os = lyx::support::os;

#ifndef CXX_GLOBAL_CSTD
using std::exit;
#endif

using std::map;
using std::vector;
using std::string;


extern BufferList bufferlist;

namespace {

float getDPI()
{
	QWidget w;
	QPaintDeviceMetrics pdm(&w);
	return 0.5 * (pdm.logicalDpiX() + pdm.logicalDpiY());
}

map<int, io_callback *> io_callbacks;
map<int, socket_callback *> socket_callbacks;

} // namespace anon

// FIXME: wrong place !
LyXServer * lyxserver;
LyXServerSocket * lyxsocket;

// in QLyXKeySym.C
extern void initEncodings();

#ifdef Q_WS_X11
extern bool lyxX11EventFilter(XEvent * xev);
#endif

#ifdef Q_WS_MACX
extern bool macEventFilter(EventRef event);
extern pascal OSErr
handleOpenDocuments(const AppleEvent* inEvent, AppleEvent* /*reply*/,
		    long /*refCon*/);
#endif

class LQApplication : public QApplication
{
public:
	LQApplication(int & argc, char ** argv);
	~LQApplication();
#ifdef Q_WS_X11
	bool x11EventFilter (XEvent * ev) { return lyxX11EventFilter(ev); }
#endif
#ifdef Q_WS_MACX
	bool macEventFilter(EventRef event);
#endif
};


LQApplication::LQApplication(int & argc, char ** argv)
	: QApplication(argc, argv)
{
#ifdef Q_WS_MACX
	AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments,
			      NewAEEventHandlerUPP(handleOpenDocuments),
			      0, false);
#endif
}


LQApplication::~LQApplication()
{}


#ifdef Q_WS_MACX
bool LQApplication::macEventFilter(EventRef event)
{
	if (GetEventClass(event) == kEventClassAppleEvent) {
		EventRecord eventrec;
		ConvertEventRefToEventRecord(event, &eventrec);
		AEProcessAppleEvent(&eventrec);

		return false;
	}
	return false;
}
#endif


namespace lyx_gui {

bool use_gui = true;


void parse_init(int & argc, char * argv[])
{
	static LQApplication a(argc, argv);

	using namespace lyx::graphics;

	Image::newImage = boost::bind(&QLImage::newImage);
	Image::loadableFormats = boost::bind(&QLImage::loadableFormats);

	// needs to be done before reading lyxrc
	lyxrc.dpi = getDPI();

	initEncodings();

	LoaderQueue::setPriority(10,100);
}


void parse_lyxrc()
{}


void start(string const & batch, vector<string> const & files)
{
	// initial geometry
	unsigned int width = 690;
	unsigned int height = 510;

	boost::shared_ptr<QtView> view_ptr(new QtView(width, height));
	LyX::ref().addLyXView(view_ptr);

	QtView & view = *view_ptr.get();
	view.show();
	view.init();

	// FIXME: some code below needs moving

	lyxserver = new LyXServer(&view.getLyXFunc(), lyxrc.lyxpipes);
	lyxsocket = new LyXServerSocket(&view.getLyXFunc(),
			  os::slashify_path(os::getTmpDir() + "/lyxsocket"));

	vector<string>::const_iterator cit = files.begin();
	vector<string>::const_iterator end = files.end();
	for (; cit != end; ++cit)
		view.view()->loadLyXFile(*cit, true);

	// handle the batch commands the user asked for
	if (!batch.empty()) {
		view.getLyXFunc().dispatch(lyxaction.lookupFunc(batch));
	}

	qApp->exec();

	// FIXME
	delete lyxsocket;
	delete lyxserver;
	lyxserver = 0;
}


void sync_events()
{
	qApp->processEvents();
}


void exit()
{
	delete lyxsocket;
	delete lyxserver;
	lyxserver = 0;

	// we cannot call qApp->exit(0) - that could return us
	// into a static dialog return in the lyx code (for example,
	// load autosave file QMessageBox. We have to just get the hell
	// out.

	::exit(0);
}


FuncStatus getStatus(FuncRequest const & ev)
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

#ifdef Q_WS_MACX
	// In LyX/Mac, when a dialog is open, the menus of the
	// application can still be accessed without giving focus to
	// the main window. In this case, we want to disable the menu
	// entries that are buffer-related.
	if (use_gui 
	    && qApp->activeWindow() != qApp->mainWidget()
	    && !lyxaction.funcHasFlag(ev.action, LyXAction::NoBuffer)) 
		flag.enabled(false);
#endif

	return flag;
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


void update_fonts()
{
	fontloader.update();
}


bool font_available(LyXFont const & font)
{
	return fontloader.available(font);
}


void set_read_callback(int fd, LyXComm * comm)
{
	io_callbacks[fd] = new io_callback(fd, comm);
}


void remove_read_callback(int fd)
{
	map<int, io_callback *>::iterator it = io_callbacks.find(fd);
	if (it != io_callbacks.end()) {
		delete it->second;
		io_callbacks.erase(it);
	}
}


void set_datasocket_callback(LyXDataSocket * p)
{
	socket_callbacks[p->fd()] = new socket_callback(p);
}

void set_serversocket_callback(LyXServerSocket * p)
{
	socket_callbacks[p->fd()] = new socket_callback(p);
}

void remove_socket_callback(int fd)
{
	map<int, socket_callback *>::iterator it = socket_callbacks.find(fd);
	if (it != socket_callbacks.end()) {
		delete it->second;
		socket_callbacks.erase(it);
	}
}

void remove_datasocket_callback(LyXDataSocket * p)
{
	remove_socket_callback(p->fd());
}

void remove_serversocket_callback(LyXServerSocket * p)
{
	remove_socket_callback(p->fd());
}

string const roman_font_name()
{
	if (!use_gui)
		return "serif";

	QFont font;
	font.setStyleHint(QFont::Serif);
	font.setFamily("serif");

	QString name = QFontInfo(font).family().latin1();
	return name;
}


string const sans_font_name()
{
	if (!use_gui)
		return "sans";

	QFont font;
	font.setStyleHint(QFont::SansSerif);
	font.setFamily("sans");

	QString name = QFontInfo(font).family().latin1();
	return name;
}


string const typewriter_font_name()
{
	if (!use_gui)
		return "monospace";

	QFont font;
	font.setStyleHint(QFont::TypeWriter);
	font.setFamily("monospace");

	QString name = QFontInfo(font).family().latin1();
	return name;
}

}; // namespace lyx_gui
