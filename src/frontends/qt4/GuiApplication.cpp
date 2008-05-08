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
#include "GuiKeySymbol.h"
#include "GuiView.h"

#include "frontends/alert.h"
#include "frontends/Application.h"
#include "frontends/FontLoader.h"
#include "frontends/FontMetrics.h"

#include "Buffer.h"
#include "BufferList.h"
#include "BufferView.h"
#include "Font.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "Language.h"
#include "LyX.h"
#include "LyXFunc.h"
#include "LyXRC.h"
#include "Session.h"
#include "version.h"

#include "support/lassert.h"
#include "support/debug.h"
#include "support/ExceptionMessage.h"
#include "support/FileName.h"
#include "support/ForkedCalls.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/os.h"
#include "support/Package.h"

#include <QClipboard>
#include <QEventLoop>
#include <QFileOpenEvent>
#include <QLocale>
#include <QLibraryInfo>
#include <QMenuBar>
#include <QPixmapCache>
#include <QRegExp>
#include <QSessionManager>
#include <QSocketNotifier>
#include <QStandardItemModel>
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

using namespace std;
using namespace lyx::support;

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

class GlobalMenuBar : public QMenuBar
{
public:
	///
	GlobalMenuBar() : QMenuBar(0) {}
	
	///
	bool event(QEvent * e)
	{
		if (e->type() == QEvent::ShortcutOverride) {
			//	    && activeWindow() == 0) {
			QKeyEvent * ke = static_cast<QKeyEvent*>(e);
			KeySymbol sym;
			setKeySymbol(&sym, ke);
			theLyXFunc().processKeySym(sym, q_key_state(ke->modifiers()));
			e->accept();
			return true;
		}
		return false;
	}
};

///////////////////////////////////////////////////////////////
// You can find more platform specific stuff
// at the end of this file...
///////////////////////////////////////////////////////////////


GuiApplication * guiApp;


GuiApplication::GuiApplication(int & argc, char ** argv)
	: QApplication(argc, argv), Application(), current_view_(0), global_menubar_(0)
{
	QString app_name = "LyX";
	QCoreApplication::setOrganizationName(app_name);
	QCoreApplication::setOrganizationDomain("lyx.org");
	QCoreApplication::setApplicationName(app_name + "-" + lyx_version);

	// FIXME: quitOnLastWindowClosed is true by default. We should have a
	// lyxrc setting for this in order to let the application stay resident.
	// But then we need some kind of dock icon, at least on Windows.
	/*
	if (lyxrc.quit_on_last_window_closed)
		setQuitOnLastWindowClosed(false);
	*/
#ifdef Q_WS_MACX
	// FIXME: Do we need a lyxrc setting for this on Mac? This behaviour
	// seems to be the default case for applications like LyX.
	setQuitOnLastWindowClosed(false);
#endif
	
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
			<< language_name);
	} else
		LYXERR(Debug::GUI, "Could not find  Qt translations for locale "
			<< language_name);

#ifdef Q_WS_MACX
	// This allows to translate the strings that appear in the LyX menu.
	addMenuTranslator();
#endif
	connect(this, SIGNAL(lastWindowClosed()), this, SLOT(onLastWindowClosed()));

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

	general_timer_.setInterval(500);
	connect(&general_timer_, SIGNAL(timeout()),
		this, SLOT(handleRegularEvents()));
	general_timer_.start();

	
#ifdef Q_WS_MACX
	if (global_menubar_ == 0) {
		// Create the global default menubar which is shown for the dialogs
		// and if no GuiView is visible.
		global_menubar_ = new GlobalMenuBar();
	}
#endif
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
		current_view_->close();
		break;

	case LFUN_LYX_QUIT:
		// quitting is triggered by the gui code
		// (leaving the event loop).
		if (current_view_)
			current_view_->message(from_utf8(N_("Exiting.")));
		if (closeAllViews())
			quit();
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

	case LFUN_BUFFER_NEW:
		if (viewCount() == 0
		    || (!lyxrc.open_buffers_in_tabs && current_view_->buffer() != 0)) {
			createView(QString(), false); // keep hidden
			current_view_->newDocument(to_utf8(cmd.argument()), false);
			current_view_->show();
			setActiveWindow(current_view_);
		} else
			current_view_->newDocument(to_utf8(cmd.argument()), false);
		break;

	case LFUN_BUFFER_NEW_TEMPLATE:
		if (viewCount() == 0 
		    || (!lyxrc.open_buffers_in_tabs && current_view_->buffer() != 0)) {
			createView();
			current_view_->newDocument(to_utf8(cmd.argument()), true);
			if (!current_view_->buffer())
				current_view_->close();
		} else
			current_view_->newDocument(to_utf8(cmd.argument()), true);
		break;

	case LFUN_FILE_OPEN:
		if (viewCount() == 0
		    || (!lyxrc.open_buffers_in_tabs && current_view_->buffer() != 0)) {
			createView();
			current_view_->openDocument(to_utf8(cmd.argument()));
			if (!current_view_->buffer())
				current_view_->close();
		} else
			current_view_->openDocument(to_utf8(cmd.argument()));
		break;

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


void GuiApplication::createView(QString const & geometry_arg, bool autoShow)
{
	// release the keyboard which might have been grabed by the global
	// menubar on Mac to catch shortcuts even without any GuiView.
	if (global_menubar_)
		global_menubar_->releaseKeyboard();

	// create new view
	updateIds(views_, view_ids_);
	int id = 0;
	while (views_.find(id) != views_.end())
		id++;
	GuiView * view = new GuiView(id);
	
	// copy the icon size from old view
	if (viewCount() > 0)
		view->setIconSize(current_view_->iconSize());

	// register view
	views_[id] = view;
	updateIds(views_, view_ids_);

	if (autoShow) {
		view->show();
		setActiveWindow(view);
	}

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
	setCurrentView(view);
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

	language_model_ = new QStandardItemModel(this);
	language_model_->insertColumns(0, 1);
	int current_row;
	Languages::const_iterator it = languages.begin();
	Languages::const_iterator end = languages.end();
	for (; it != end; ++it) {
		current_row = language_model_->rowCount();
		language_model_->insertRows(current_row, 1);
		QModelIndex item = language_model_->index(current_row, 0);
		language_model_->setData(item, qt_(it->second.display()), Qt::DisplayRole);
		language_model_->setData(item, toqstr(it->second.lang()), Qt::UserRole);
	}
}


void GuiApplication::restoreGuiSession()
{
	if (!lyxrc.load_session)
		return;

	Session & session = LyX::ref().session();
	vector<FileName> const & lastopened = session.lastOpened().getfiles();
	// do not add to the lastfile list since these files are restored from
	// last session, and should be already there (regular files), or should
	// not be added at all (help files).
	for_each(lastopened.begin(), lastopened.end(),
		bind(&GuiView::loadDocument, current_view_, _1, false));

	// clear this list to save a few bytes of RAM
	session.lastOpened().clear();
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


void GuiApplication::handleRegularEvents()
{
	ForkedCallsController::handleCompletedProcesses();
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
			LyX::ref().addFileToLoad(fromqstr(foe->file()));
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
	catch (ExceptionMessage const & e) {
		switch(e.type_) { 
		case ErrorException:
			LyX::cref().emergencyCleanup();
			setQuitOnLastWindowClosed(false);
			closeAllViews();
			Alert::error(e.title_, e.details_);
#ifndef NDEBUG
			// Properly crash in debug mode in order to get a useful backtrace.
			abort();
#endif
			// In release mode, try to exit gracefully.
			this->exit(1);

		case BufferException: {
			Buffer * buf = current_view_->buffer();
			docstring details = e.details_ + '\n';
			details += theBufferList().emergencyWrite(buf);
			theBufferList().release(buf);
			details += "\n" + _("The current document was closed.");
			Alert::error(e.title_, details);
			return false;
		}
		case WarningException:
			Alert::warning(e.title_, e.details_);
			return false;
		}
	}
	catch (exception const & e) {
		docstring s = _("LyX has caught an exception, it will now "
			"attempt to save all unsaved documents and exit."
			"\n\nException: ");
		s += from_ascii(e.what());
		Alert::error(_("Software exception Detected"), s);
		LyX::cref().exit(1);
	}
	catch (...) {
		docstring s = _("LyX has caught some really weird exception, it will "
			"now attempt to save all unsaved documents and exit.");
		Alert::error(_("Software exception Detected"), s);
		LyX::cref().exit(1);
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
	return ltrim(fromqstr(color_cache_.get(col).name()), "#");
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
	/// We are changing that to close all wiew one by one.
	/// FIXME: verify if the default implementation is enough now.
	if (sm.allowsInteraction() && !closeAllViews())
 		sm.cancel();
}


void GuiApplication::addMenuTranslator()
{
	installTranslator(new MenuTranslator(this));
}


bool GuiApplication::unregisterView(int id)
{
	updateIds(views_, view_ids_);
	LASSERT(views_.find(id) != views_.end(), /**/);
	LASSERT(views_[id], /**/);

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
	if (views_.empty())
		return true;

	map<int, GuiView*> const cmap = views_;
	map<int, GuiView*>::const_iterator it;
	for (it = cmap.begin(); it != cmap.end(); ++it) {
		if (!it->second->close())
			return false;
	}

	views_.clear();
	view_ids_.clear();
	return true;
}


GuiView & GuiApplication::view(int id) const
{
	LASSERT(views_.find(id) != views_.end(), /**/);
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


void GuiApplication::readMenus(Lexer & lex)
{
	menus().read(lex);
}


bool GuiApplication::searchMenu(FuncRequest const & func,
	vector<docstring> & names) const
{
	return menus().searchMenu(func, names);
}


void GuiApplication::initGlobalMenu()
{
	if (global_menubar_)
		menus().fillMenuBar(global_menubar_, 0, true);
}


void GuiApplication::onLastWindowClosed()
{
	if (global_menubar_)
		global_menubar_->grabKeyboard();
}


////////////////////////////////////////////////////////////////////////
//
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


void hideDialogs(std::string const & name, Inset * inset)
{
	if (theApp())
		theApp()->hideDialogs(name, inset);
}


////////////////////////////////////////////////////////////////////
//
// Font stuff
//
////////////////////////////////////////////////////////////////////

frontend::FontLoader & theFontLoader()
{
	LASSERT(frontend::guiApp, /**/);
	return frontend::guiApp->fontLoader();
}


frontend::FontMetrics const & theFontMetrics(Font const & f)
{
	return theFontMetrics(f.fontInfo());
}


frontend::FontMetrics const & theFontMetrics(FontInfo const & f)
{
	LASSERT(frontend::guiApp, /**/);
	return frontend::guiApp->fontLoader().metrics(f);
}


////////////////////////////////////////////////////////////////////
//
// Misc stuff
//
////////////////////////////////////////////////////////////////////

frontend::Clipboard & theClipboard()
{
	LASSERT(frontend::guiApp, /**/);
	return frontend::guiApp->clipboard();
}


frontend::Selection & theSelection()
{
	LASSERT(frontend::guiApp, /**/);
	return frontend::guiApp->selection();
}

} // namespace lyx

#include "GuiApplication_moc.cpp"
