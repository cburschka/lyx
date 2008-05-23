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

#include "ColorCache.h"
#include "GuiClipboard.h"
#include "GuiImage.h"
#include "GuiKeySymbol.h"
#include "GuiSelection.h"
#include "GuiView.h"
#include "Menus.h"
#include "qt_helpers.h"

#include "frontends/alert.h"
#include "frontends/Application.h"
#include "frontends/FontLoader.h"
#include "frontends/FontMetrics.h"

#include "Buffer.h"
#include "BufferList.h"
#include "BufferView.h"
#include "Color.h"
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
#include "support/foreach.h"
#include "support/ForkedCalls.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/os.h"
#include "support/Package.h"

#ifdef Q_WS_MACX
#include "support/linkback/LinkBackProxy.h"
#endif

#include <QClipboard>
#include <QEventLoop>
#include <QFileOpenEvent>
#include <QHash>
#include <QLocale>
#include <QLibraryInfo>
#include <QMacPasteboardMime>
#include <QMenuBar>
#include <QMimeData>
#include <QPixmapCache>
#include <QRegExp>
#include <QSessionManager>
#include <QSocketNotifier>
#include <QSortFilterProxyModel>
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

#ifdef Q_WS_WIN
#include <QList>
#include <QWindowsMime>
#if defined(Q_CYGWIN_WIN) || defined(Q_CC_MINGW)
#include <wtypes.h>
#endif
#include <objidl.h>
#endif // Q_WS_WIN

#include <boost/bind.hpp>

#include <exception>

using namespace std;
using namespace lyx::support;

// FIXME: These strings are also used in GuiClipboard.cpp.
static char const * const lyx_mime_type = "application/x-lyx";
static char const * const pdf_mime_type = "application/pdf";
static char const * const emf_mime_type = "image/x-emf";
static char const * const wmf_mime_type = "image/x-wmf";

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

#ifdef Q_WS_MACX
// QMacPasteboardMimeGraphics can only be compiled on Mac.

class QMacPasteboardMimeGraphics : public QMacPasteboardMime
{
public:
	QMacPasteboardMimeGraphics()
		: QMacPasteboardMime(MIME_QT_CONVERTOR|MIME_ALL)
	{}

	QString convertorName() { return "Graphics"; }

	QString flavorFor(QString const & mime)
	{
		LYXERR(Debug::ACTION, "flavorFor " << mime);
		if (mime == QLatin1String(pdf_mime_type))
			return QLatin1String("com.adobe.pdf");
		return QString();
	}

	QString mimeFor(QString flav)
	{
		LYXERR(Debug::ACTION, "mimeFor " << flav);
		if (flav == QLatin1String("com.adobe.pdf"))
			return QLatin1String(pdf_mime_type);
		return QString();
	}

	bool canConvert(QString const & mime, QString flav)
	{ return mimeFor(flav) == mime; }

	QVariant convertToMime(QString const & mime, QList<QByteArray> data, QString flav)
	{
		if(data.count() > 1)
			qWarning("QMacPasteboardMimeGraphics: Cannot handle multiple member data");
		return data.first();
	}

	QList<QByteArray> convertFromMime(QString const & mime, QVariant data, QString flav)
	{
		QList<QByteArray> ret;
		ret.append(data.toByteArray());
		return ret;
	}
};
#endif

///////////////////////////////////////////////////////////////
// You can find more platform specific stuff
// at the end of this file...
///////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
// Windows specific stuff goes here...

#ifdef Q_WS_WIN
// QWindowsMimeMetafile can only be compiled on Windows.

static FORMATETC cfFromMime(QString const & mimetype)
{
	FORMATETC formatetc;
	if (mimetype == emf_mime_type) {
		formatetc.cfFormat = CF_ENHMETAFILE;
		formatetc.tymed = TYMED_ENHMF;
	} else if (mimetype == wmf_mime_type) {
		formatetc.cfFormat = CF_METAFILEPICT;
		formatetc.tymed = TYMED_MFPICT;
	}
	formatetc.ptd = 0;
	formatetc.dwAspect = DVASPECT_CONTENT;
	formatetc.lindex = -1;
	return formatetc;
}


class QWindowsMimeMetafile : public QWindowsMime {
public:
	QWindowsMimeMetafile() {}

	bool canConvertFromMime(FORMATETC const & formatetc,
		QMimeData const * mimedata) const
	{
		return false;
	}

	bool canConvertToMime(QString const & mimetype,
		IDataObject * pDataObj) const
	{
		if (mimetype != emf_mime_type && mimetype != wmf_mime_type)
			return false;
		FORMATETC formatetc = cfFromMime(mimetype);
		return pDataObj->QueryGetData(&formatetc) == S_OK;
	}

	bool convertFromMime(FORMATETC const & formatetc,
		const QMimeData * mimedata, STGMEDIUM * pmedium) const
	{
		return false;
	}

	QVariant convertToMime(QString const & mimetype, IDataObject * pDataObj,
		QVariant::Type preferredType) const
	{
		QByteArray data;
		if (!canConvertToMime(mimetype, pDataObj))
			return data;

		FORMATETC formatetc = cfFromMime(mimetype);
		STGMEDIUM s;
		if (pDataObj->GetData(&formatetc, &s) != S_OK)
			return data;

		int dataSize;
		if (s.tymed == TYMED_ENHMF) {
			dataSize = GetEnhMetaFileBits(s.hEnhMetaFile, 0, 0);
			data.resize(dataSize);
			dataSize = GetEnhMetaFileBits(s.hEnhMetaFile, dataSize,
				(LPBYTE)data.data());
		} else if (s.tymed == TYMED_MFPICT) {
			dataSize = GetMetaFileBitsEx((HMETAFILE)s.hMetaFilePict, 0, 0);
			data.resize(dataSize);
			dataSize = GetMetaFileBitsEx((HMETAFILE)s.hMetaFilePict, dataSize,
				(LPBYTE)data.data());
		}
		data.detach();
		ReleaseStgMedium(&s);

		return data;
	}


	QVector<FORMATETC> formatsForMime(QString const & mimeType,
		QMimeData const * mimeData) const
	{
		QVector<FORMATETC> formats;
		formats += cfFromMime(mimeType);
		return formats;
	}

	QString mimeForFormat(FORMATETC const & formatetc) const
	{
		switch (formatetc.cfFormat) {
		case CF_ENHMETAFILE:
			return emf_mime_type; 
		case CF_METAFILEPICT:
			return wmf_mime_type;
		}
		return QString();
	}
};

#endif // Q_WS_WIN

////////////////////////////////////////////////////////////////////////
// GuiApplication::Private definition and implementation.
////////////////////////////////////////////////////////////////////////

struct GuiApplication::Private
{
	Private()
		: language_model_(0), global_menubar_(0)
	{
#ifdef Q_WS_MACX
		// Create the global default menubar which is shown for the dialogs
		// and if no GuiView is visible.
		global_menubar_ = new GlobalMenuBar();
#endif
	}

	///
	QSortFilterProxyModel * language_model_;
	///
	GuiClipboard clipboard_;
	///
	GuiSelection selection_;
	///
	FontLoader font_loader_;
	///
	ColorCache color_cache_;
	///
	QTranslator qt_trans_;
	///
	QHash<int, SocketNotifier *> socket_notifiers_;
	///
	Menus menus_;
	/// this timer is used for any regular events one wants to
	/// perform. at present it is used to check if forked processes
	/// are done.
	QTimer general_timer_;

	/// Multiple views container.
	/**
	* Warning: This must not be a smart pointer as the destruction of the
	* object is handled by Qt when the view is closed
	* \sa Qt::WA_DeleteOnClose attribute.
	*/
	QHash<int, GuiView *> views_;

	/// Only used on mac.
	GlobalMenuBar * global_menubar_;

#ifdef Q_WS_MACX
	/// Linkback mime handler for MacOSX.
	QMacPasteboardMimeGraphics mac_pasteboard_mime_;
#endif

#ifdef Q_WS_WIN
	/// WMF Mime handler for Windows clipboard.
	// FIXME for Windows Vista and Qt4 (see http://bugzilla.lyx.org/show_bug.cgi?id=4846)
	// But this makes LyX crash on exit when LyX is compiled in release mode and if there
	// is something in the clipboard.
	QWindowsMimeMetafile wmf_mime_;
#endif
};


GuiApplication * guiApp;

GuiApplication::~GuiApplication()
{
#ifdef Q_WS_MACX
	closeAllLinkBackLinks();
#endif
	delete d;
}


GuiApplication::GuiApplication(int & argc, char ** argv)
	: QApplication(argc, argv),	current_view_(0), d(new GuiApplication::Private)
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
	if (d->qt_trans_.load(language_name,
		QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
	{
		installTranslator(&d->qt_trans_);
		// even if the language calls for RtL, don't do that
		setLayoutDirection(Qt::LeftToRight);
		LYXERR(Debug::GUI, "Successfully installed Qt translations for locale "
			<< language_name);
	} else
		LYXERR(Debug::GUI, "Could not find  Qt translations for locale "
			<< language_name);

#ifdef Q_WS_MACX
	// This allows to translate the strings that appear in the LyX menu.
	/// A translator suitable for the entries in the LyX menu.
	/// Only needed with Qt/Mac.
	installTranslator(new MenuTranslator(this));
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

	d->general_timer_.setInterval(500);
	connect(&d->general_timer_, SIGNAL(timeout()),
		this, SLOT(handleRegularEvents()));
	d->general_timer_.start();
}


bool GuiApplication::getStatus(FuncRequest const & cmd, FuncStatus & flag) const
{
	bool enable = true;

	switch(cmd.action) {

	case LFUN_WINDOW_CLOSE:
		enable = d->views_.size() > 0;
		break;

	case LFUN_BUFFER_NEW:
	case LFUN_BUFFER_NEW_TEMPLATE:
	case LFUN_FILE_OPEN:
	case LFUN_SCREEN_FONT_UPDATE:
	case LFUN_SET_COLOR:
	case LFUN_WINDOW_NEW:
	case LFUN_LYX_QUIT:
		enable = true;
		break;

	default:
		return false;
	}

	if (!enable)
		flag.enabled(false);

	return true;
}

	
bool GuiApplication::dispatch(FuncRequest const & cmd)
{
	switch (cmd.action) {

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
		d->font_loader_.update();
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
		if (d->views_.empty()
		    || (!lyxrc.open_buffers_in_tabs && current_view_->buffer() != 0)) {
			createView(QString(), false); // keep hidden
			current_view_->newDocument(to_utf8(cmd.argument()), false);
			current_view_->show();
			setActiveWindow(current_view_);
		} else {
			current_view_->newDocument(to_utf8(cmd.argument()), false);
		}
		break;

	case LFUN_BUFFER_NEW_TEMPLATE:
		if (d->views_.empty()
		    || (!lyxrc.open_buffers_in_tabs && current_view_->buffer() != 0)) {
			createView();
			current_view_->newDocument(to_utf8(cmd.argument()), true);
			if (!current_view_->buffer())
				current_view_->close();
		} else {
			current_view_->newDocument(to_utf8(cmd.argument()), true);
		}
		break;

	case LFUN_FILE_OPEN:
		if (d->views_.empty()
		    || (!lyxrc.open_buffers_in_tabs && current_view_->buffer() != 0)) {
			createView();
			current_view_->openDocument(to_utf8(cmd.argument()));
			if (!current_view_->buffer())
				current_view_->close();
		} else
			current_view_->openDocument(to_utf8(cmd.argument()));
		break;

	case LFUN_SET_COLOR: {
		string lyx_name;
		string const x11_name = split(to_utf8(cmd.argument()), lyx_name, ' ');
		if (lyx_name.empty() || x11_name.empty()) {
			current_view_->message(
				_("Syntax: set-color <lyx_name> <x11_name>"));
			break;
		}

		string const graphicsbg = lcolor.getLyXName(Color_graphicsbg);
		bool const graphicsbg_changed = lyx_name == graphicsbg
			&& x11_name != graphicsbg;
		if (graphicsbg_changed) {
			// FIXME: The graphics cache no longer has a changeDisplay method.
#if 0
			graphics::GCache::get().changeDisplay(true);
#endif
		}

		if (!lcolor.setColor(lyx_name, x11_name)) {
			current_view_->message(
					bformat(_("Set-color \"%1$s\" failed "
							       "- color is undefined or "
							       "may not be redefined"),
								   from_utf8(lyx_name)));
			break;
		}
		// Make sure we don't keep old colors in cache.
		d->color_cache_.clear();
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
	QHash<int, GuiView *>::iterator it;
	for (it = d->views_.begin(); it != d->views_.end(); ++it)
		(*it)->resetDialogs();

	dispatch(FuncRequest(LFUN_SCREEN_FONT_UPDATE));
}


void GuiApplication::createView(QString const & geometry_arg, bool autoShow)
{
	// release the keyboard which might have been grabed by the global
	// menubar on Mac to catch shortcuts even without any GuiView.
	if (d->global_menubar_)
		d->global_menubar_->releaseKeyboard();

	// create new view
	int id = 0;
	while (d->views_.find(id) != d->views_.end())
		id++;
	GuiView * view = new GuiView(id);
	
	// copy the icon size from old view
	if (current_view_)
		view->setIconSize(current_view_->iconSize());

	// register view
	d->views_[id] = view;

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
	return d->clipboard_;
}


Selection & GuiApplication::selection()
{
	return d->selection_;
}


FontLoader & GuiApplication::fontLoader() 
{
	return d->font_loader_;
}


Menus const & GuiApplication::menus() const 
{
	return d->menus_;
}


Menus & GuiApplication::menus()
{
	return d->menus_; 
}


QList<int> GuiApplication::viewIds() const
{
	return d->views_.keys();
}


ColorCache & GuiApplication::colorCache()
{
	return d->color_cache_;
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
	// init the global menubar on Mac. This must be done after the session
	// was recovered to know the "last files".
	if (d->global_menubar_)
		d->menus_.fillMenuBar(d->global_menubar_, 0, true);

	LyX::ref().execBatchCommands();
}

QAbstractItemModel * GuiApplication::languageModel()
{
	if (d->language_model_)
		return d->language_model_;

	QStandardItemModel * lang_model = new QStandardItemModel(this);
	lang_model->insertColumns(0, 1);
	int current_row;
	Languages::const_iterator it = languages.begin();
	Languages::const_iterator end = languages.end();
	for (; it != end; ++it) {
		current_row = lang_model->rowCount();
		lang_model->insertRows(current_row, 1);
		QModelIndex item = lang_model->index(current_row, 0);
		lang_model->setData(item, qt_(it->second.display()), Qt::DisplayRole);
		lang_model->setData(item, toqstr(it->second.lang()), Qt::UserRole);
	}
	d->language_model_ = new QSortFilterProxyModel(this);
	d->language_model_->setSourceModel(lang_model);
#if QT_VERSION >= 0x040300
	d->language_model_->setSortLocaleAware(true);
#endif
	return d->language_model_;
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
	QColor const & qcol = d->color_cache_.get(col);
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
	return ltrim(fromqstr(d->color_cache_.get(col).name()), "#");
}


void GuiApplication::registerSocketCallback(int fd, SocketCallback func)
{
	SocketNotifier * sn = new SocketNotifier(this, fd, func);
	d->socket_notifiers_[fd] = sn;
	connect(sn, SIGNAL(activated(int)), this, SLOT(socketDataReceived(int)));
}


void GuiApplication::socketDataReceived(int fd)
{
	d->socket_notifiers_[fd]->func_();
}


void GuiApplication::unregisterSocketCallback(int fd)
{
	d->socket_notifiers_.take(fd)->setEnabled(false);
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


void GuiApplication::unregisterView(GuiView * gv)
{
	LASSERT(d->views_[gv->id()] == gv, /**/);
	d->views_.remove(gv->id());
	if (current_view_ == gv) {
		current_view_ = 0;
		theLyXFunc().setLyXView(0);
	}
}


bool GuiApplication::closeAllViews()
{
	if (d->views_.empty())
		return true;

	QList<GuiView *> views = d->views_.values();
	foreach (GuiView * view, views) {
		if (!view->close())
			return false;
	}

	d->views_.clear();
	return true;
}


GuiView & GuiApplication::view(int id) const
{
	LASSERT(d->views_.contains(id), /**/);
	return *d->views_.value(id);
}


void GuiApplication::hideDialogs(string const & name, Inset * inset) const
{
	QList<GuiView *> views = d->views_.values();
	foreach (GuiView * view, views)
		view->hideDialog(name, inset);
}


Buffer const * GuiApplication::updateInset(Inset const * inset) const
{
	Buffer const * buffer_ = 0;
	QHash<int, GuiView *>::iterator end = d->views_.end();
	for (QHash<int, GuiView *>::iterator it = d->views_.begin(); it != end; ++it) {
		if (Buffer const * ptr = (*it)->updateInset(inset))
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


void GuiApplication::onLastWindowClosed()
{
	if (d->global_menubar_)
		d->global_menubar_->grabKeyboard();
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
				d->selection_.put(sel);
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
