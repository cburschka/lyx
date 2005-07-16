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
#include "support/package.h"
#include "debug.h"

// Dear Lord, deliver us from Evil, aka the Qt headers
// Qt defines a macro 'signals' that clashes with a boost namespace.
// All is well if the namespace is visible first.
#include <boost/signal.hpp> // FIXME: Is this needed? (Lgb)
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

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

using lyx::frontend::QtView;

namespace os = lyx::support::os;

using boost::shared_ptr;

#ifndef CXX_GLOBAL_CSTD
using std::exit;
#endif

using std::map;
using std::vector;
using std::string;


extern BufferList bufferlist;

namespace {

int getDPI()
{
	QWidget w;
	QPaintDeviceMetrics pdm(&w);
	return int(0.5 * (pdm.logicalDpiX() + pdm.logicalDpiY()));
}

map<int, shared_ptr<socket_callback> > socket_callbacks;

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
{
#ifdef QT_THREAD_SUPPORT
	if (locked())
		unlock();
#endif
}


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
	// Force adding of font path _before_ QApplication is initialized
	FontLoader::initFontPath();

	static LQApplication app(argc, argv);

#if QT_VERSION >= 0x030200
	// install translation file for Qt built-in dialogs
	// These are only installed since Qt 3.2.x
	static QTranslator qt_trans(0);
	if (qt_trans.load(QString("qt_") + QTextCodec::locale(),
			  qInstallPathTranslations())) {
		app.installTranslator(&qt_trans);
		// even if the language calls for RtL, don't do that
		app.setReverseLayout(false);
		lyxerr[Debug::GUI]
			<< "Successfully installed Qt translations for locale "
			<< QTextCodec::locale() << std::endl;
	} else
		lyxerr[Debug::GUI]
			<< "Could not find  Qt translations for locale "
			<< QTextCodec::locale() << std::endl;
#endif

#ifdef Q_WS_MACX
	// These translations are meant to break Qt/Mac menu merging
	// algorithm on some entries. It lists the menu names that
	// should not be moved to the LyX menu
	static QTranslator aqua_trans(0);
	aqua_trans.insert(QTranslatorMessage("QMenuBar", "Setting", 0,
					     "do_not_merge_me"));
	aqua_trans.insert(QTranslatorMessage("QMenuBar", "Config", 0,
					     "do_not_merge_me"));
	aqua_trans.insert(QTranslatorMessage("QMenuBar", "Options", 0,
					     "do_not_merge_me"));
	aqua_trans.insert(QTranslatorMessage("QMenuBar", "Setup", 0,
					     "do_not_merge_me"));

	app.installTranslator(&aqua_trans);
#endif

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
			  os::internal_path(package().temp_dir() + "/lyxsocket"));

	for_each(files.begin(), files.end(),
		 bind(&BufferView::loadLyXFile, view.view(), _1, true));

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
	// This is the ONLY place where processEvents may be called.
	// During screen update/ redraw, this method is disabled to
	// prevent keyboard events being handed to the LyX core, where
	// they could cause re-entrant calls to screen update.
#if QT_VERSION >= 0x030100
	qApp->eventLoop()->processEvents(QEventLoop::ExcludeUserInput);
#endif
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


void register_socket_callback(int fd, boost::function<void()> func)
{
	socket_callbacks[fd] = shared_ptr<socket_callback>(new socket_callback(fd, func));
}


void unregister_socket_callback(int fd)
{
	socket_callbacks.erase(fd);
}


string const roman_font_name()
{
	if (!use_gui)
		return "serif";

	QFont font;
	font.setStyleHint(QFont::Serif);
	font.setFamily("serif");

	return fromqstr(QFontInfo(font).family());
}


string const sans_font_name()
{
	if (!use_gui)
		return "sans";

	QFont font;
	font.setStyleHint(QFont::SansSerif);
	font.setFamily("sans");

	return fromqstr(QFontInfo(font).family());
}


string const typewriter_font_name()
{
	if (!use_gui)
		return "monospace";

	QFont font;
	font.setStyleHint(QFont::TypeWriter);
	font.setFamily("monospace");

	return fromqstr(QFontInfo(font).family());
}

}; // namespace lyx_gui
