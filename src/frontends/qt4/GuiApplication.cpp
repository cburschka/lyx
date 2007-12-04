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

#include "qt_helpers.h"
#include "GuiImage.h"
#include "GuiView.h"

#include "frontends/alert.h"
#include "frontends/Application.h"
#include "frontends/NoGuiFontLoader.h"
#include "frontends/NoGuiFontMetrics.h"
#include "frontends/FontLoader.h"
#include "frontends/FontMetrics.h"

#include "support/ExceptionMessage.h"
#include "support/FileName.h"
#include "support/lstrings.h"
#include "support/os.h"
#include "support/Package.h"

#include "Buffer.h"
#include "BufferList.h"
#include "BufferView.h"
#include "support/debug.h"
#include "Font.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "support/gettext.h"
#include "LyX.h"
#include "LyXFunc.h"
#include "LyXRC.h"
#include "Session.h"
#include "version.h"

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

using std::endl;
using std::map;
using std::string;
using std::vector;


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
	: QApplication(argc, argv), Application(), current_view_(0)
{
	QString app_name = "LyX";
	QCoreApplication::setOrganizationName(app_name);
	QCoreApplication::setOrganizationDomain("lyx.org");
	QCoreApplication::setApplicationName(app_name + "-" + lyx_version);

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

	guiApp = this;

	// Set the cache to 5120 kilobytes which corresponds to screen size of
	// 1280 by 1024 pixels with a color depth of 32 bits.
	QPixmapCache::setCacheLimit(5120);

	// Initialize RC Fonts
	if (lyxrc.roman_font_name.empty())
		lyxrc.roman_font_name = fromqstr(romanFontName());

	if (lyxrc.sans_font_name.empty())
		lyxrc.sans_font_name = fromqstr(sansFontName());

	if (lyxrc.typewriter_font_name.empty())
		lyxrc.typewriter_font_name = fromqstr(typewriterFontName());
}


GuiApplication::~GuiApplication()
{
	socket_notifiers_.clear();
}


FuncStatus GuiApplication::getStatus(FuncRequest const & cmd)
{
	FuncStatus flag;
	bool enable = true;

	switch(cmd.action) {

	case LFUN_WINDOW_CLOSE:
		enable = view_ids_.size() > 0;
		break;

	default:
		if (!current_view_) {
			enable = false;
			break;
		}
	}

	if (!enable)
		flag.enabled(false);

	return flag;
}

	
bool GuiApplication::dispatch(FuncRequest const & cmd)
{
	switch(cmd.action) {

	case LFUN_WINDOW_NEW:
		createView(toqstr(cmd.argument()));
		break;

	case LFUN_WINDOW_CLOSE:
		// update bookmark pit of the current buffer before window close
		for (size_t i = 0; i < LyX::ref().session().bookmarks().size(); ++i)
			theLyXFunc().gotoBookmark(i+1, false, false);
		// ask the user for saving changes or cancel quit
		if (!current_view_->quitWriteAll())
			break;
		current_view_->close();
		break;

	case LFUN_LYX_QUIT:
		// quitting is triggered by the gui code
		// (leaving the event loop).
		current_view_->message(from_utf8(N_("Exiting.")));
		if (current_view_->quitWriteAll())
			closeAllViews();
		break;

	case LFUN_SCREEN_FONT_UPDATE: {
		// handle the screen font changes.
		font_loader_.update();
		// Backup current_view_
		GuiView * view = current_view_;
		// Set current_view_ to zero to forbid GuiWorkArea::redraw()
		// to skip the refresh.
		current_view_ = 0;
		BufferList::iterator it = theBufferList().begin();
		BufferList::iterator const end = theBufferList().end();
		for (; it != end; ++it)
			(*it)->changed();
		// Restore current_view_
		current_view_ = view;
		break;
	}

	default:
		// Notify the caller that the action has not been dispatched.
		return false;
	}

	// The action has been dispatched.
	return true;
}


void GuiApplication::resetGui()
{
	map<int, GuiView *>::iterator it;
	for (it = views_.begin(); it != views_.end(); ++it)
		it->second->resetDialogs();

	dispatch(FuncRequest(LFUN_SCREEN_FONT_UPDATE));
}


static void updateIds(map<int, GuiView *> const & stdmap, vector<int> & ids)
{
	ids.clear();
	map<int, GuiView *>::const_iterator it;
	for (it = stdmap.begin(); it != stdmap.end(); ++it)
		ids.push_back(it->first);
}


void GuiApplication::createView(QString const & geometry_arg)
{
	updateIds(views_, view_ids_);
	int id = 0;
	while (views_.find(id) != views_.end())
		id++;
	views_[id] = new GuiView(id);
	updateIds(views_, view_ids_);

	GuiView * view  = views_[id];
	theLyXFunc().setLyXView(view);

	view->show();
	if (!geometry_arg.isEmpty()) {
#ifdef Q_WS_WIN
		int x, y;
		int w, h;
		QRegExp re( "[=]*(?:([0-9]+)[xX]([0-9]+)){0,1}[ ]*(?:([+-][0-9]*)([+-][0-9]*)){0,1}" );
		re.indexIn(geometry_arg);
		w = re.cap(1).toInt();
		h = re.cap(2).toInt();
		x = re.cap(3).toInt();
		y = re.cap(4).toInt();
		view->setGeometry(x, y, w, h);
#endif
	}
	view->setFocus();

	setCurrentView(*view);
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


QString const GuiApplication::romanFontName()
{
	QFont font;
	font.setKerning(false);
	font.setStyleHint(QFont::Serif);
	font.setFamily("serif");

	return QFontInfo(font).family();
}


QString const GuiApplication::sansFontName()
{
	QFont font;
	font.setKerning(false);
	font.setStyleHint(QFont::SansSerif);
	font.setFamily("sans");

	return QFontInfo(font).family();
}


QString const GuiApplication::typewriterFontName()
{
	QFont font;
	font.setKerning(false);
	font.setStyleHint(QFont::TypeWriter);
	font.setFamily("monospace");

	return QFontInfo(font).family();
}


bool GuiApplication::event(QEvent * e)
{
	switch(e->type()) {
	case QEvent::FileOpen: {
		// Open a file; this happens only on Mac OS X for now
		QFileOpenEvent * foe = static_cast<QFileOpenEvent *>(e);

		if (!current_view_ || !current_view_->view())
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
			"attempt to save all unsaved documents and exit."
			"\n\nException: ");
		s += from_ascii(e.what());
		Alert::error(_("Software exception Detected"), s);
		LyX::cref().emergencyCleanup();
		QApplication::exit(1);
	}
	catch (...) {
		docstring s = _("LyX has caught some really weird exception, it will "
			"now attempt to save all unsaved documents and exit.");
		Alert::error(_("Software exception Detected"), s);
		LyX::cref().emergencyCleanup();
		QApplication::exit(1);
	}

	return false;
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
	if (sm.allowsInteraction() && !current_view_->quitWriteAll())
 		sm.cancel();
}


void GuiApplication::addMenuTranslator()
{
	installTranslator(new MenuTranslator(this));
}


bool GuiApplication::unregisterView(int id)
{
	updateIds(views_, view_ids_);
	BOOST_ASSERT(views_.find(id) != views_.end());
	BOOST_ASSERT(views_[id]);

	map<int, GuiView *>::iterator it;
	for (it = views_.begin(); it != views_.end(); ++it) {
		if (it->first == id) {
			views_.erase(id);
			break;
		}
	}
	updateIds(views_, view_ids_);
	return true;
}


bool GuiApplication::closeAllViews()
{
	updateIds(views_, view_ids_);
	if (views_.empty()) {
		// quit in CloseEvent will not be triggert
		qApp->quit();
		return true;
	}

	map<int, GuiView*> const cmap = views_;
	map<int, GuiView*>::const_iterator it;
	for (it = cmap.begin(); it != cmap.end(); ++it) {
		// TODO: return false when close event was ignored
		//       e.g. quitWriteAll()->'Cancel'
		//       maybe we need something like 'bool closeView()'
		it->second->close();
		// unregisterd by the CloseEvent
	}

	views_.clear();
	view_ids_.clear();
	return true;
}


GuiView & GuiApplication::view(int id) const
{
	BOOST_ASSERT(views_.find(id) != views_.end());
	return *views_.find(id)->second;
}


void GuiApplication::hideDialogs(string const & name, Inset * inset) const
{
	vector<int>::const_iterator it = view_ids_.begin();
	vector<int>::const_iterator const end = view_ids_.end();
	for (; it != end; ++it)
		view(*it).hideDialog(name, inset);
}


Buffer const * GuiApplication::updateInset(Inset const * inset) const
{
	Buffer const * buffer_ = 0;
	vector<int>::const_iterator it = view_ids_.begin();
	vector<int>::const_iterator const end = view_ids_.end();
	for (; it != end; ++it) {
		Buffer const * ptr = view(*it).updateInset(inset);
		if (ptr)
			buffer_ = ptr;
	}
	return buffer_;
}


////////////////////////////////////////////////////////////////////////
// X11 specific stuff goes here...
#ifdef Q_WS_X11
bool GuiApplication::x11EventFilter(XEvent * xev)
{
	if (!current_view_)
		return false;

	switch (xev->type) {
	case SelectionRequest: {
		if (xev->xselectionrequest.selection != XA_PRIMARY)
			break;
		LYXERR(Debug::GUI, "X requested selection.");
		BufferView * bv = current_view_->view();
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
		BufferView * bv = current_view_->view();
		if (bv)
			bv->clearSelection();
		break;
	}
	}
	return false;
}
#endif

} // namespace frontend


////////////////////////////////////////////////////////////////////
//
// Font stuff
//
////////////////////////////////////////////////////////////////////

frontend::FontLoader & theFontLoader()
{
	static frontend::NoGuiFontLoader no_gui_font_loader;

	if (!use_gui)
		return no_gui_font_loader;

	BOOST_ASSERT(frontend::guiApp);
	return frontend::guiApp->fontLoader();
}


frontend::FontMetrics const & theFontMetrics(Font const & f)
{
	return theFontMetrics(f.fontInfo());
}


frontend::FontMetrics const & theFontMetrics(FontInfo const & f)
{
	static frontend::NoGuiFontMetrics no_gui_font_metrics;

	if (!use_gui)
		return no_gui_font_metrics;

	BOOST_ASSERT(frontend::guiApp);
	return frontend::guiApp->fontLoader().metrics(f);
}


frontend::Clipboard & theClipboard()
{
	BOOST_ASSERT(frontend::guiApp);
	return frontend::guiApp->clipboard();
}


frontend::Selection & theSelection()
{
	BOOST_ASSERT(frontend::guiApp);
	return frontend::guiApp->selection();
}

} // namespace lyx

#include "GuiApplication_moc.cpp"
