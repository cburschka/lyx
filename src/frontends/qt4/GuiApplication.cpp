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
#include "Toolbars.h"

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
#include "Lexer.h"
#include "LyX.h"
#include "LyXAction.h"
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
#include "support/lyxalgo.h" // sorted
#include "support/Messages.h"
#include "support/os.h"
#include "support/Package.h"

#ifdef Q_WS_MACX
#include "support/linkback/LinkBackProxy.h"
#endif

#include <queue>

#include <QByteArray>
#include <QClipboard>
#include <QDateTime>
#include <QDir>
#include <QEventLoop>
#include <QFileOpenEvent>
#include <QFileInfo>
#include <QHash>
#include <QIcon>
#include <QImageReader>
#include <QLocale>
#include <QLibraryInfo>
#include <QList>
#include <QMacPasteboardMime>
#include <QMenuBar>
#include <QMimeData>
#include <QObject>
#include <QPixmap>
#include <QPixmapCache>
#include <QRegExp>
#include <QSessionManager>
#include <QSettings>
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
#include <QWindowsMime>
#ifdef Q_CC_GNU
#include <wtypes.h>
#endif
#include <objidl.h>
#endif // Q_WS_WIN

#include <boost/bind.hpp>
#include <boost/crc.hpp>

#include <exception>
#include <vector>

using namespace std;
using namespace lyx::support;


static void initializeResources()
{
	static bool initialized = false;
	if (!initialized) {
		Q_INIT_RESOURCE(Resources); 
		initialized = true;
	}
}


namespace lyx {

frontend::Application * createApplication(int & argc, char * argv[])
{
#ifndef Q_WS_X11
	// prune -geometry argument(s) by shifting
	// the following ones 2 places down.
	for (int i = 0 ; i < argc ; ++i) {
		if (strcmp(argv[i], "-geometry") == 0) {
			int const remove = (i+1) < argc ? 2 : 1;
			argc -= remove;
			for (int j = i; j < argc; ++j)
				argv[j] = argv[j + remove];
			--i;
		}
	}
#endif
	return new frontend::GuiApplication(argc, argv);
}

namespace frontend {


/// Return the list of loadable formats.
vector<string> loadableImageFormats()
{
	vector<string> fmts;

	QList<QByteArray> qt_formats = QImageReader::supportedImageFormats();

	LYXERR(Debug::GRAPHICS,
		"\nThe image loader can load the following directly:\n");

	if (qt_formats.empty())
		LYXERR(Debug::GRAPHICS, "\nQt4 Problem: No Format available!");

	for (QList<QByteArray>::const_iterator it = qt_formats.begin(); it != qt_formats.end(); ++it) {

		LYXERR(Debug::GRAPHICS, (const char *) *it << ", ");

		string ext = ascii_lowercase((const char *) *it);
		// special case
		if (ext == "jpeg")
			ext = "jpg";
		fmts.push_back(ext);
	}

	return fmts;
}


////////////////////////////////////////////////////////////////////////
// Icon loading support code.
////////////////////////////////////////////////////////////////////////

namespace {

struct PngMap {
	QString key;
	QString value;
};


bool operator<(PngMap const & lhs, PngMap const & rhs)
{
	return lhs.key < rhs.key;
}


class CompareKey {
public:
	CompareKey(QString const & name) : name_(name) {}
	bool operator()(PngMap const & other) const { return other.key == name_; }
private:
	QString const name_;
};


// this must be sorted alphabetically
// Upper case comes before lower case
PngMap sorted_png_map[] = {
	{ "Bumpeq", "bumpeq2" },
	{ "Cap", "cap2" },
	{ "Cup", "cup2" },
	{ "Delta", "delta2" },
	{ "Downarrow", "downarrow2" },
	{ "Gamma", "gamma2" },
	{ "Lambda", "lambda2" },
	{ "Leftarrow", "leftarrow2" },
	{ "Leftrightarrow", "leftrightarrow2" },
	{ "Longleftarrow", "longleftarrow2" },
	{ "Longleftrightarrow", "longleftrightarrow2" },
	{ "Longrightarrow", "longrightarrow2" },
	{ "Omega", "omega2" },
	{ "Phi", "phi2" },
	{ "Pi", "pi2" },
	{ "Psi", "psi2" },
	{ "Rightarrow", "rightarrow2" },
	{ "Sigma", "sigma2" },
	{ "Subset", "subset2" },
	{ "Supset", "supset2" },
	{ "Theta", "theta2" },
	{ "Uparrow", "uparrow2" },
	{ "Updownarrow", "updownarrow2" },
	{ "Upsilon", "upsilon2" },
	{ "Vdash", "vdash3" },
	{ "Vert", "vert2" },
	{ "Xi", "xi2" },
	{ "nLeftarrow", "nleftarrow2" },
	{ "nLeftrightarrow", "nleftrightarrow2" },
	{ "nRightarrow", "nrightarrow2" },
	{ "nVDash", "nvdash3" },
	{ "nvDash", "nvdash2" },
	{ "textrm \\AA", "textrm_AA"},
	{ "textrm \\O", "textrm_O"},
	{ "vDash", "vdash2" }
};

size_t const nr_sorted_png_map = sizeof(sorted_png_map) / sizeof(PngMap);


QString findPng(QString const & name)
{
	PngMap const * const begin = sorted_png_map;
	PngMap const * const end = begin + nr_sorted_png_map;
	LASSERT(sorted(begin, end), /**/);

	PngMap const * const it = find_if(begin, end, CompareKey(name));

	QString png_name;
	if (it != end) {
		png_name = it->value;
	} else {
		png_name = name;
		png_name.replace('_', "underscore");
		png_name.replace(' ', '_');

		// This way we can have "math-delim { }" on the toolbar.
		png_name.replace('(', "lparen");
		png_name.replace(')', "rparen");
		png_name.replace('[', "lbracket");
		png_name.replace(']', "rbracket");
		png_name.replace('{', "lbrace");
		png_name.replace('}', "rbrace");
		png_name.replace('|', "bars");
		png_name.replace(',', "thinspace");
		png_name.replace(':', "mediumspace");
		png_name.replace(';', "thickspace");
		png_name.replace('!', "negthinspace");
	}

	LYXERR(Debug::GUI, "findPng(" << name << ")\n"
		<< "Looking for math PNG called \"" << png_name << '"');
	return png_name;
}

} // namespace anon


QString iconName(FuncRequest const & f, bool unknown)
{
	initializeResources();
	QString name1;
	QString name2;
	QString path;
	switch (f.action) {
	case LFUN_MATH_INSERT:
		if (!f.argument().empty()) {
			path = "math/";
			name1 = findPng(toqstr(f.argument()).mid(1));
		}
		break;
	case LFUN_MATH_DELIM:
	case LFUN_MATH_BIGDELIM:
		path = "math/";
		name1 = findPng(toqstr(f.argument()));
		break;
	case LFUN_CALL:
		path = "commands/";
		name1 = toqstr(f.argument());
		break;
	default:
		name2 = toqstr(lyxaction.getActionName(f.action));
		name1 = name2;

		if (!f.argument().empty()) {
			name1 = name2 + ' ' + toqstr(f.argument());
			name1.replace(' ', '_');
		}
	}

	FileName fname = libFileSearch("images/" + path, name1, "png");
	if (fname.exists())
		return toqstr(fname.absFilename());

	fname = libFileSearch("images/" + path, name2, "png");
	if (fname.exists())
		return toqstr(fname.absFilename());

	path = ":/images/" + path;
	QDir res(path);
	if (!res.exists()) {
		LYXERR0("Directory " << path << " not found in resource!"); 
		return QString();
	}
	name1 += ".png";
	if (res.exists(name1))
		return path + name1;

	name2 += ".png";
	if (res.exists(name2))
		return path + name2;

	LYXERR(Debug::GUI, "Cannot find icon for command \""
			   << lyxaction.getActionName(f.action)
			   << '(' << to_utf8(f.argument()) << ")\"");

	if (unknown)
		return QString(":/images/unknown.png");

	return QString();
}


QIcon getIcon(FuncRequest const & f, bool unknown)
{
	QString icon = iconName(f, unknown);
	if (icon.isEmpty())
		return QIcon();

	//LYXERR(Debug::GUI, "Found icon: " << icon);
	QPixmap pm;
	if (!pm.load(icon)) {
		LYXERR0("Cannot load icon " << icon << " please verify resource system!");
		return QIcon();
	}

	return QIcon(pm);
}


////////////////////////////////////////////////////////////////////////
// LyX server support code.
////////////////////////////////////////////////////////////////////////
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
////////////////////////////////////////////////////////////////////////

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
		if (mime == pdfMimeType())
			return QLatin1String("com.adobe.pdf");
		return QString();
	}

	QString mimeFor(QString flav)
	{
		LYXERR(Debug::ACTION, "mimeFor " << flav);
		if (flav == QLatin1String("com.adobe.pdf"))
			return pdfMimeType();
		return QString();
	}

	bool canConvert(QString const & mime, QString flav)
	{
		return mimeFor(flav) == mime;
	}

	QVariant convertToMime(QString const & /*mime*/, QList<QByteArray> data,
		QString /*flav*/)
	{
		if(data.count() > 1)
			qWarning("QMacPasteboardMimeGraphics: Cannot handle multiple member data");
		return data.first();
	}

	QList<QByteArray> convertFromMime(QString const & /*mime*/,
		QVariant data, QString /*flav*/)
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
	if (mimetype == emfMimeType()) {
		formatetc.cfFormat = CF_ENHMETAFILE;
		formatetc.tymed = TYMED_ENHMF;
	} else if (mimetype == wmfMimeType()) {
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
		if (mimetype != emfMimeType() && mimetype != wmfMimeType())
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
		if (mimeType== emfMimeType() || mimeType== wmfMimeType())
			formats += cfFromMime(mimeType);
		return formats;
	}

	QString mimeForFormat(FORMATETC const & formatetc) const
	{
		switch (formatetc.cfFormat) {
		case CF_ENHMETAFILE:
			return emfMimeType(); 
		case CF_METAFILEPICT:
			return wmfMimeType();
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
	Private(): language_model_(0), global_menubar_(0) {}

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
	///
	/// The global instance
	Toolbars toolbars_;

	/// this timer is used for any regular events one wants to
	/// perform. at present it is used to check if forked processes
	/// are done.
	QTimer general_timer_;

	/// delayed FuncRequests
	std::queue<FuncRequest> func_request_queue_;

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
	/// \warning: see comment in ~GuiApplication and in bug 4846.
	QWindowsMimeMetafile wmf_mime_;
#endif
};


GuiApplication * guiApp;

GuiApplication::~GuiApplication()
{
#ifdef Q_WS_MACX
	closeAllLinkBackLinks();
#endif
	// FIXME: Work around bug 4846 for Windows Vista and Qt4
	// (see http://bugzilla.lyx.org/show_bug.cgi?id=4846)
	// If the clipboard is not cleared, LyX crashes on exit when it is
	// compiled in release mode and if there is something in the clipboard.
	// This is related to QWindowsMimeMetafile which is apparently not 
	// properly destroyed.
	qApp->clipboard()->clear(QClipboard::Clipboard);
	delete d;
}


GuiApplication::GuiApplication(int & argc, char ** argv)
	: QApplication(argc, argv),	current_view_(0), d(new GuiApplication::Private)
{
	QString app_name = "LyX";
	QCoreApplication::setOrganizationName(app_name);
	QCoreApplication::setOrganizationDomain("lyx.org");
	QCoreApplication::setApplicationName(app_name + "-" + lyx_version);

	// Install translator for GUI elements.
	installTranslator(&d->qt_trans_);

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

	// This allows to translate the strings that appear in the LyX menu.
	/// A translator suitable for the entries in the LyX menu.
	/// Only needed with Qt/Mac.
	installTranslator(new MenuTranslator(this));
#endif
	
#ifdef Q_WS_X11
	// doubleClickInterval() is 400 ms on X11 which is just too long.
	// On Windows and Mac OS X, the operating system's value is used.
	// On Microsoft Windows, calling this function sets the double
	// click interval for all applications. So we don't!
	QApplication::setDoubleClickInterval(300);
#endif

	connect(this, SIGNAL(lastWindowClosed()), this, SLOT(onLastWindowClosed()));

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


void GuiApplication::clearSession()
{
	QSettings settings;
	settings.clear();
}


docstring GuiApplication::iconName(FuncRequest const & f, bool unknown)
{
	return qstring_to_ucs4(lyx::frontend::iconName(f, unknown));
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
		flag.setEnabled(false);

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
		for (size_t i = 0; i < theSession().bookmarks().size(); ++i)
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
			string const fname = to_utf8(cmd.argument());
			// We want the ui session to be saved per document and not per
			// window number. The filename crc is a good enough identifier.
			boost::crc_32_type crc;
			crc = for_each(fname.begin(), fname.end(), crc);
			createView(crc.checksum());
			current_view_->openDocument(fname);
			if (current_view_ && !current_view_->buffer())
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


void GuiApplication::dispatchDelayed(FuncRequest const & func)
{
	d->func_request_queue_.push(func);
	QTimer::singleShot(0, this, SLOT(processFuncRequestQueue()));
}


void GuiApplication::resetGui()
{
	// Set the language defined by the user.
	setGuiLanguage();

	// Read menus
	if (!readUIFile(toqstr(lyxrc.ui_file)))
		// Gives some error box here.
		return;

	if (d->global_menubar_)
		d->menus_.fillMenuBar(d->global_menubar_, 0, false);

	QHash<int, GuiView *>::iterator it;
	for (it = d->views_.begin(); it != d->views_.end(); ++it) {
		GuiView * gv = *it;
		gv->setLayoutDirection(layoutDirection());
		gv->resetDialogs();
	}

	dispatch(FuncRequest(LFUN_SCREEN_FONT_UPDATE));
}


void GuiApplication::createView(int view_id)
{
	createView(QString(), true, view_id);
}


void GuiApplication::createView(QString const & geometry_arg, bool autoShow,
	int view_id)
{
	// release the keyboard which might have been grabed by the global
	// menubar on Mac to catch shortcuts even without any GuiView.
	if (d->global_menubar_)
		d->global_menubar_->releaseKeyboard();

	// create new view
	int id = view_id;
	if (id == 0) {
		while (d->views_.find(id) != d->views_.end())
			id++;
	}
	LYXERR(Debug::GUI, "About to create new window with ID " << id);
	GuiView * view = new GuiView(id);
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


Toolbars const & GuiApplication::toolbars() const 
{
	return d->toolbars_;
}


Toolbars & GuiApplication::toolbars()
{
	return d->toolbars_; 
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
	// asynchronously handle batch commands. This event will be in
	// the event queue in front of other asynchronous events. Hence,
	// we can assume in the latter that the gui is setup already.
	QTimer::singleShot(0, this, SLOT(execBatchCommands()));

	return QApplication::exec();
}


void GuiApplication::exit(int status)
{
	QApplication::exit(status);
}


void GuiApplication::setGuiLanguage()
{
	// Set the language defined by the user.
	setRcGuiLanguage();

	QString const default_language = toqstr(Messages::defaultLanguage());
	LYXERR(Debug::LOCALE, "Tring to set default locale to: " << default_language);
	QLocale const default_locale(default_language);
	QLocale::setDefault(default_locale);

	// install translation file for Qt built-in dialogs
	QString const language_name = QString("qt_") + default_locale.name();

	// language_name can be short (e.g. qt_zh) or long (e.g. qt_zh_CN). 
	// Short-named translator can be loaded from a long name, but not the
	// opposite. Therefore, long name should be used without truncation.
	// c.f. http://doc.trolltech.com/4.1/qtranslator.html#load
	if (!d->qt_trans_.load(language_name,
			QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
		LYXERR(Debug::LOCALE, "Could not find  Qt translations for locale "
			<< language_name);
	} else {
		LYXERR(Debug::LOCALE, "Successfully installed Qt translations for locale "
			<< language_name);
	}

	switch (default_locale.language()) {
	case QLocale::Arabic :
	case QLocale::Hebrew :
	case QLocale::Persian :
	case QLocale::Urdu :
        setLayoutDirection(Qt::RightToLeft);
		break;
	default:
        setLayoutDirection(Qt::LeftToRight);
	}
}


void GuiApplication::processFuncRequestQueue()
{
	while (!d->func_request_queue_.empty()) {
		lyx::dispatch(d->func_request_queue_.back());
		d->func_request_queue_.pop();
	}
}


void GuiApplication::execBatchCommands()
{
	setGuiLanguage();

	// Read menus
	if (!readUIFile(toqstr(lyxrc.ui_file)))
		// Gives some error box here.
		return;

#ifdef Q_WS_MACX
	// Create the global default menubar which is shown for the dialogs
	// and if no GuiView is visible.
	// This must be done after the session was recovered to know the "last files".
	d->global_menubar_ = new GlobalMenuBar();
	d->menus_.fillMenuBar(d->global_menubar_, 0, true);
#endif

	lyx::execBatchCommands();
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

	Session & session = theSession();
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
		// Open a file; this happens only on Mac OS X for now.
		//
		// We do this asynchronously because on startup the batch
		// commands are not executed here yet and the gui is not ready
		// therefore.
		QFileOpenEvent * foe = static_cast<QFileOpenEvent *>(e);
		dispatchDelayed(FuncRequest(LFUN_FILE_OPEN, qstring_to_ucs4(foe->file())));
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
			emergencyCleanup();
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
		lyx_exit(1);
	}
	catch (...) {
		docstring s = _("LyX has caught some really weird exception, it will "
			"now attempt to save all unsaved documents and exit.");
		Alert::error(_("Software exception Detected"), s);
		lyx_exit(1);
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


bool GuiApplication::searchMenu(FuncRequest const & func,
	docstring_list & names) const
{
	return d->menus_.searchMenu(func, names);
}


bool GuiApplication::readUIFile(QString const & name, bool include)
{
	enum {
		ui_menuset = 1,
		ui_toolbars,
		ui_toolbarset,
		ui_include,
		ui_last
	};

	LexerKeyword uitags[] = {
		{ "include", ui_include },
		{ "menuset", ui_menuset },
		{ "toolbars", ui_toolbars },
		{ "toolbarset", ui_toolbarset }
	};

	LYXERR(Debug::INIT, "About to read " << name << "...");

	FileName ui_path;
	if (include) {
		ui_path = libFileSearch("ui", name, "inc");
		if (ui_path.empty())
			ui_path = libFileSearch("ui", changeExtension(name, "inc"));
	} else {
		ui_path = libFileSearch("ui", name, "ui");
	}

	if (ui_path.empty()) {
		LYXERR(Debug::INIT, "Could not find " << name);
		Alert::warning(_("Could not find UI definition file"),
			       bformat(_("Error while reading the configuration file\n%1$s.\n"
				   "Please check your installation."), qstring_to_ucs4(name)));
		return false;
	}


	// Ensure that a file is read only once (prevents include loops)
	static QStringList uifiles;
	QString const uifile = toqstr(ui_path.absFilename());
	if (uifiles.contains(uifile)) {
		if (!include) {
			// We are reading again the top uifile so reset the safeguard:
			uifiles.clear();
			d->menus_.reset();
			d->toolbars_.reset();
		} else {
			LYXERR(Debug::INIT, "UI file '" << name << "' has been read already. "
				<< "Is this an include loop?");
			return false;
		}
	}
	uifiles.push_back(uifile);

	LYXERR(Debug::INIT, "Found " << name << " in " << ui_path);

	Lexer lex(uitags);
	lex.setFile(ui_path);
	if (!lex.isOK()) {
		lyxerr << "Unable to set LyXLeX for ui file: " << ui_path
		       << endl;
	}

	if (lyxerr.debugging(Debug::PARSER))
		lex.printTable(lyxerr);

	while (lex.isOK()) {
		switch (lex.lex()) {
		case ui_include: {
			lex.next(true);
			QString const file = toqstr(lex.getString());
			if (!readUIFile(file, true))
				return false;
			break;
		}
		case ui_menuset:
			d->menus_.read(lex);
			break;

		case ui_toolbarset:
			d->toolbars_.readToolbars(lex);
			break;

		case ui_toolbars:
			d->toolbars_.readToolbarSettings(lex);
			break;

		default:
			if (!rtrim(lex.getString()).empty())
				lex.printError("LyX::ReadUIFile: "
					       "Unknown menu tag: `$$Token'");
			break;
		}
	}

	if (include)
		return true;

	QSettings settings;
	settings.beginGroup("ui_files");
	bool touched = false;
	for (int i = 0; i != uifiles.size(); ++i) {
		QFileInfo fi(uifiles[i]);
		QDateTime const date_value = fi.lastModified();
		QString const name_key = QString::number(i);
		if (!settings.contains(name_key)
		 || settings.value(name_key).toString() != uifiles[i]
		 || settings.value(name_key + "/date").toDateTime() != date_value) {
			touched = true;
			settings.setValue(name_key, uifiles[i]);
			settings.setValue(name_key + "/date", date_value);
		}
	}
	settings.endGroup();
	if (touched)
		settings.remove("views");

	return true;
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
		LYXERR(Debug::SELECTION, "X requested selection.");
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
		LYXERR(Debug::SELECTION, "Lost selection.");
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
