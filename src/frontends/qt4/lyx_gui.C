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

#undef QT3_SUPPORT

#include <config.h>

#include "lyx_gui.h"

// FIXME: move this stuff out again
#include "bufferlist.h"
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


#include "graphics/LoaderQueue.h"

#include "support/lstrings.h"
#include "support/os.h"
#include "support/package.h"
#include "debug.h"


#include "GuiView.h"
#include "ColorCache.h"
#include "FontLoader.h"
#include "QLImage.h"
#include "qt_helpers.h"
#include "socket_callback.h"
#include "Application.h"

#include <QApplication>
#include <QEventLoop>
#include <QTranslator>
#include <QTextCodec>
#include <QLocale>
#include <QLibraryInfo>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>


using lyx::support::ltrim;
using lyx::support::package;

using lyx::frontend::GuiImplementation;
using lyx::frontend::GuiView;
using lyx::frontend::Application;

namespace os = lyx::support::os;

using boost::shared_ptr;

#ifndef CXX_GLOBAL_CSTD
using std::exit;
#endif

using std::map;
using std::vector;
using std::string;

// FIXME: wrong place !
LyXServer * lyxserver;
LyXServerSocket * lyxsocket;

lyx::frontend::Application * theApp;

namespace {

int getDPI()
{
	QWidget w;
	return int(0.5 * (w.logicalDpiX() + w.logicalDpiY()));
}

map<int, shared_ptr<socket_callback> > socket_callbacks;

void cleanup()
{
	delete lyxsocket;
	lyxsocket = 0;
	delete lyxserver;
	lyxserver = 0;
}

} // namespace anon

// in QLyXKeySym.C
extern void initEncodings();

namespace lyx_gui {

bool use_gui = true;

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
	*/

	// Force adding of font path _before_ QApplication is initialized
	FontLoader::initFontPath();

#ifdef Q_WS_WIN
	static Application app(argc, argv);
#else
	Application app(argc, argv);
#endif

	theApp = &app;


	// install translation file for Qt built-in dialogs
	// These are only installed since Qt 3.2.x
	QTranslator qt_trans;
	QString language_name = QString("qt_") + QLocale::system().name();
	language_name.truncate(5);
	if (qt_trans.load(language_name,
		QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
	{
		qApp->installTranslator(&qt_trans);
		// even if the language calls for RtL, don't do that
		qApp->setLayoutDirection(Qt::LeftToRight);
		lyxerr[Debug::GUI]
			<< "Successfully installed Qt translations for locale "
			<< fromqstr(language_name) << std::endl;
	} else
		lyxerr[Debug::GUI]
			<< "Could not find  Qt translations for locale "
			<< fromqstr(language_name) << std::endl;

/*#ifdef Q_WS_MACX
	// These translations are meant to break Qt/Mac menu merging
	// algorithm on some entries. It lists the menu names that
	// should not be moved to the LyX menu
	QTranslator aqua_trans(0);
	aqua_trans.insert(QTranslatorMessage("QMenuBar", "Setting", 0,
					     "do_not_merge_me"));
	aqua_trans.insert(QTranslatorMessage("QMenuBar", "Config", 0,
					     "do_not_merge_me"));
	aqua_trans.insert(QTranslatorMessage("QMenuBar", "Options", 0,
					     "do_not_merge_me"));
	aqua_trans.insert(QTranslatorMessage("QMenuBar", "Setup", 0,
					     "do_not_merge_me"));

	qApp->installTranslator(&aqua_trans);
#endif
*/
	using namespace lyx::graphics;

	Image::newImage = boost::bind(&QLImage::newImage);
	Image::loadableFormats = boost::bind(&QLImage::loadableFormats);

	// needs to be done before reading lyxrc
	lyxrc.dpi = getDPI();

	LoaderQueue::setPriority(10,100);

	return LyX::ref().exec2(argc, argv);
}


void parse_lyxrc()
{}


LyXView * create_view(unsigned int width, unsigned int height, int posx, int posy,
          bool maximize)
{
	// this can't be done before because it needs the Languages object
	initEncodings();

	int view_id = theApp->gui().newView(width, height);
	GuiView & view = static_cast<GuiView &> (theApp->gui().view(view_id));

	// FIXME: for now we assume that there is only one LyXView with id = 0.
	/*int workArea_id_ =*/ theApp->gui().newWorkArea(width, height, 0);
	//WorkArea * workArea_ = & theApp->gui().workArea(workArea_id_);

	LyX::ref().addLyXView(&view);

	view.init();

	// only true when the -geometry option was NOT used
	if (width != 0 && height != 0) {
		if (posx != -1 && posy != -1) {
#ifdef Q_OS_WIN32
			// FIXME: use only setGeoemtry when Trolltech has
			// fixed the qt4/X11 bug
			view.setGeometry(posx, posy,width, height);
#else
			view.resize(width, height);
			view.move(posx, posy);
#endif
		} else {
			view.resize(width, height);
		}

		if (maximize)
			view.setWindowState(Qt::WindowMaximized);
	}

	view.show();

	return &view;
}


int start(LyXView * view, string const & batch)
{
	// FIXME: some code below needs moving

	lyxserver = new LyXServer(&view->getLyXFunc(), lyxrc.lyxpipes);
	lyxsocket = new LyXServerSocket(&view->getLyXFunc(),
			  os::internal_path(package().temp_dir() + "/lyxsocket"));

	// handle the batch commands the user asked for
	if (!batch.empty()) {
		view->getLyXFunc().dispatch(lyxaction.lookupFunc(batch));
	}

	int const status = qApp->exec();

	// FIXME
	cleanup();
	return status;
}


void sync_events()
{
	// This is the ONLY place where processEvents may be called.
	// During screen update/ redraw, this method is disabled to
	// prevent keyboard events being handed to the LyX core, where
	// they could cause re-entrant calls to screen update.
	qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
}


void exit(int status)
{
	cleanup();
	QApplication::exit(status);
}


FuncStatus getStatus(FuncRequest const & ev)
{
	FuncStatus flag;
	switch (ev.action) {
	case LFUN_TOOLTIPS_TOGGLE:
		flag.unknown(true);
		break;
	default:
		break;
	}

	return flag;
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


void update_fonts()
{
	theApp->fontLoader().update();
}


bool font_available(LyXFont const & font)
{
	return theApp->fontLoader().available(font);
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
