/**
 * \file GuiApplication.cpp
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

#include "GuiApplication.h"

#include "GuiView.h"
#include "qt_helpers.h"
#include "GuiImage.h"

#include "frontends/alert.h"

#include "graphics/LoaderQueue.h"

#include "support/ExceptionMessage.h"
#include "support/FileName.h"
#include "support/lstrings.h"
#include "support/os.h"
#include "support/Package.h"

#include "BufferList.h"
#include "BufferView.h"
#include "debug.h"
#include "FuncRequest.h"
#include "gettext.h"
#include "LyX.h"
#include "LyXFunc.h"
#include "LyXRC.h"

#include <QApplication>
#include <QClipboard>
#include <QEventLoop>
#include <QFileOpenEvent>
#include <QLocale>
#include <QLibraryInfo>
#include <QPixmapCache>
#include <QRegExp>
#include <QSessionManager>
#include <QSocketNotifier>
#include <QTextCodec>
#include <QTimer>
#include <QTranslator>
#include <QWidget>

#ifdef Q_WS_X11
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#undef CursorShape
#undef None
#endif

#include <boost/bind.hpp>

#include <exception>

using std::string;
using std::endl;


namespace lyx {

frontend::Application * createApplication(int & argc, char * argv[])
{
	return new frontend::GuiApplication(argc, argv);
}


namespace frontend {

class SocketNotifier : public QSocketNotifier
{
public:
	/// connect a connection notification from the LyXServerSocket
	SocketNotifier(QObject * parent, int fd, Application::SocketCallback func)
		: QSocketNotifier(fd, QSocketNotifier::Read, parent), func_(func)
	{}

public:
	/// The callback function
	Application::SocketCallback func_;
};


////////////////////////////////////////////////////////////////////////
// Mac specific stuff goes here...

class MenuTranslator : public QTranslator
{
public:
	MenuTranslator(QObject * parent)
		: QTranslator(parent)
	{}

	QString translate(const char * /*context*/, 
	  const char * sourceText, 
	  const char * /*comment*/ = 0) 
	{
		string const s = sourceText;
		if (s == N_("About %1")	|| s == N_("Preferences") 
				|| s == N_("Reconfigure") || s == N_("Quit %1"))
			return qt_(s);
		else 
			return QString();
	}
};


///////////////////////////////////////////////////////////////
// You can find more platform specific stuff
// at the end of this file...
///////////////////////////////////////////////////////////////


using support::FileName;

GuiApplication * guiApp;


GuiApplication::GuiApplication(int & argc, char ** argv)
	: QApplication(argc, argv), Application()
{
	QCoreApplication::setOrganizationName("The LyX Community");
	QCoreApplication::setOrganizationDomain("lyx.org");
	QCoreApplication::setApplicationName("LyX");

	// Qt bug? setQuitOnLastWindowClosed(true); does not work
	setQuitOnLastWindowClosed(false);

#ifdef Q_WS_X11
	// doubleClickInterval() is 400 ms on X11 which is just too long.
	// On Windows and Mac OS X, the operating system's value is used.
	// On Microsoft Windows, calling this function sets the double
	// click interval for all applications. So we don't!
	QApplication::setDoubleClickInterval(300);
#endif

	// install translation file for Qt built-in dialogs
	QString language_name = QString("qt_") + QLocale::system().name();
	
	// language_name can be short (e.g. qt_zh) or long (e.g. qt_zh_CN). 
	// Short-named translator can be loaded from a long name, but not the
	// opposite. Therefore, long name should be used without truncation.
	// c.f. http://doc.trolltech.com/4.1/qtranslator.html#load
	if (qt_trans_.load(language_name,
		QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
	{
		installTranslator(&qt_trans_);
		// even if the language calls for RtL, don't do that
		setLayoutDirection(Qt::LeftToRight);
		LYXERR(Debug::GUI, "Successfully installed Qt translations for locale "
			<< fromqstr(language_name));
	} else
		LYXERR(Debug::GUI, "Could not find  Qt translations for locale "
			<< fromqstr(language_name));

#ifdef Q_WS_MACX
	// This allows to translate the strings that appear in the LyX menu.
	addMenuTranslator();
#endif

	using namespace lyx::graphics;

	Image::newImage = boost::bind(&GuiImage::newImage);
	Image::loadableFormats = boost::bind(&GuiImage::loadableFormats);

	// needs to be done before reading lyxrc
	QWidget w;
	lyxrc.dpi = (w.logicalDpiX() + w.logicalDpiY()) / 2;

	LoaderQueue::setPriority(10,100);

	guiApp = this;

	// Set the cache to 5120 kilobytes which corresponds to screen size of
	// 1280 by 1024 pixels with a color depth of 32 bits.
	QPixmapCache::setCacheLimit(5120);
}


GuiApplication::~GuiApplication()
{
	socket_notifiers_.clear();
}


LyXView & GuiApplication::createView(string const & geometry_arg)
{
	int const id = gui_.createRegisteredView();
	GuiView & view = static_cast<GuiView &>(gui_.view(id));
	theLyXFunc().setLyXView(&view);

	view.init();
	view.show();
	if (!geometry_arg.empty()) {
#ifdef Q_WS_WIN
		int x, y;
		int w, h;
		QRegExp re( "[=]*(?:([0-9]+)[xX]([0-9]+)){0,1}[ ]*(?:([+-][0-9]*)([+-][0-9]*)){0,1}" );
		re.indexIn(toqstr(geometry_arg.c_str()));
		w = re.cap(1).toInt();
		h = re.cap(2).toInt();
		x = re.cap(3).toInt();
		y = re.cap(4).toInt();
		view.setGeometry(x, y, w, h);
#endif
	}
	view.setFocus();

	setCurrentView(view);

	return view;
}




Clipboard & GuiApplication::clipboard()
{
	return clipboard_;
}


Selection & GuiApplication::selection()
{
	return selection_;
}


int GuiApplication::exec()
{
	QTimer::singleShot(1, this, SLOT(execBatchCommands()));
	return QApplication::exec();
}


void GuiApplication::exit(int status)
{
	QApplication::exit(status);
}


void GuiApplication::execBatchCommands()
{
	LyX::ref().execBatchCommands();
}


string const GuiApplication::romanFontName()
{
	QFont font;
	font.setKerning(false);
	font.setStyleHint(QFont::Serif);
	font.setFamily("serif");

	return fromqstr(QFontInfo(font).family());
}


string const GuiApplication::sansFontName()
{
	QFont font;
	font.setKerning(false);
	font.setStyleHint(QFont::SansSerif);
	font.setFamily("sans");

	return fromqstr(QFontInfo(font).family());
}


string const GuiApplication::typewriterFontName()
{
	QFont font;
	font.setKerning(false);
	font.setStyleHint(QFont::TypeWriter);
	font.setFamily("monospace");

	return fromqstr(QFontInfo(font).family());
}


bool GuiApplication::event(QEvent * e)
{
	switch(e->type()) {
	case QEvent::FileOpen: {
		// Open a file; this happens only on Mac OS X for now
		QFileOpenEvent * foe = static_cast<QFileOpenEvent *>(e);

		if (!currentView() || !currentView()->view())
			// The application is not properly initialized yet.
			// So we acknowledge the event and delay the file opening
			// until LyX is ready.
			// FIXME UNICODE: FileName accept an utf8 encoded string.
			LyX::ref().addFileToLoad(FileName(fromqstr(foe->file())));
		else
			lyx::dispatch(FuncRequest(LFUN_FILE_OPEN,
				qstring_to_ucs4(foe->file())));

		e->accept();
		return true;
	}
	default:
		return QApplication::event(e);
	}
}


bool GuiApplication::notify(QObject * receiver, QEvent * event)
{
	try {
		return QApplication::notify(receiver, event);
	}
	catch (support::ExceptionMessage const & e) {
		if (e.type_ == support::ErrorException) {
			Alert::error(e.title_, e.details_);
			LyX::cref().emergencyCleanup();
			QApplication::exit(1);
		} else if (e.type_ == support::WarningException) {
			Alert::warning(e.title_, e.details_);
			return false;
		}
	}
	catch (std::exception const & e) {
		docstring s = _("LyX has caught an exception, it will now "
			"attemp to save all unsaved documents and exit."
			"\n\nException: ");
		s += from_ascii(e.what());
		Alert::error(_("Software exception Detected"), s);
		LyX::cref().emergencyCleanup();
		QApplication::exit(1);
	}
	catch (...) {
		docstring s = _("LyX has caught some really weird exception, it will "
			"now attemp to save all unsaved documents and exit.");
		Alert::error(_("Software exception Detected"), s);
		LyX::cref().emergencyCleanup();
		QApplication::exit(1);
	}

	return false;
}


void GuiApplication::syncEvents()
{
	// This is the ONLY place where processEvents may be called.
	// During screen update/ redraw, this method is disabled to
	// prevent keyboard events being handed to the LyX core, where
	// they could cause re-entrant calls to screen update.
	processEvents(QEventLoop::ExcludeUserInputEvents);
}


bool GuiApplication::getRgbColor(ColorCode col, RGBColor & rgbcol)
{
	QColor const & qcol = color_cache_.get(col);
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


string const GuiApplication::hexName(ColorCode col)
{
	return support::ltrim(fromqstr(color_cache_.get(col).name()), "#");
}


void GuiApplication::updateColor(ColorCode)
{
	// FIXME: Bleh, can't we just clear them all at once ?
	color_cache_.clear();
}


void GuiApplication::registerSocketCallback(int fd, SocketCallback func)
{
	SocketNotifier * sn = new SocketNotifier(this, fd, func);
	socket_notifiers_[fd] = sn;
	connect(sn, SIGNAL(activated(int)), this, SLOT(socketDataReceived(int)));
}


void GuiApplication::socketDataReceived(int fd)
{
	socket_notifiers_[fd]->func_();
}


void GuiApplication::unregisterSocketCallback(int fd)
{
	socket_notifiers_.erase(fd);
}


void GuiApplication::commitData(QSessionManager & sm)
{
	/// The implementation is required to avoid an application exit
	/// when session state save is triggered by session manager.
	/// The default implementation sends a close event to all
	/// visible top level widgets when session managment allows
	/// interaction.
	/// We are changing that to write all unsaved buffers...
	if (sm.allowsInteraction() && !theBufferList().quitWriteAll())
 		sm.cancel();
}


////////////////////////////////////////////////////////////////////////
// X11 specific stuff goes here...
#ifdef Q_WS_X11
bool GuiApplication::x11EventFilter(XEvent * xev)
{
	if (!currentView())
		return false;

	switch (xev->type) {
	case SelectionRequest: {
		if (xev->xselectionrequest.selection != XA_PRIMARY)
			break;
		LYXERR(Debug::GUI, "X requested selection.");
		BufferView * bv = currentView()->view();
		if (bv) {
			docstring const sel = bv->requestSelection();
			if (!sel.empty())
				selection_.put(sel);
		}
		break;
	}
	case SelectionClear: {
		if (xev->xselectionclear.selection != XA_PRIMARY)
			break;
		LYXERR(Debug::GUI, "Lost selection.");
		BufferView * bv = currentView()->view();
		if (bv)
			bv->clearSelection();
		break;
	}
	}
	return false;
}
#endif

void GuiApplication::addMenuTranslator()
{
	installTranslator(new MenuTranslator(this));
}


} // namespace frontend
} // namespace lyx

#include "GuiApplication_moc.cpp"
