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
#include "ColorSet.h"
#include "GuiClipboard.h"
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
#include "BufferParams.h"
#include "BufferView.h"
#include "CmdDef.h"
#include "Color.h"
#include "CutAndPaste.h"
#include "ErrorList.h"
#include "Font.h"
#include "FuncRequest.h"
#include "FuncStatus.h"
#include "GuiWorkArea.h"
#include "Intl.h"
#include "KeyMap.h"
#include "Language.h"
#include "LaTeXPackages.h"
#include "Lexer.h"
#include "LyX.h"
#include "LyXAction.h"
#include "LyXRC.h"
#include "Paragraph.h"
#include "Server.h"
#include "Session.h"
#include "SpellChecker.h"
#include "Thesaurus.h"
#include "version.h"

#include "insets/InsetText.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/ExceptionMessage.h"
#include "support/FileName.h"
#include "support/filetools.h"
#include "support/foreach.h"
#include "support/ForkedCalls.h"
#include "support/gettext.h"
#include "support/lassert.h"
#include "support/lstrings.h"
#include "support/lyxalgo.h" // sorted
#include "support/textutils.h"
#include "support/Messages.h"
#include "support/os.h"
#include "support/Package.h"
#include "support/TempFile.h"

#ifdef Q_OS_MAC
#include "support/AppleScript.h"
#include "support/linkback/LinkBackProxy.h"
#endif

#include <queue>

#include <QByteArray>
#include <QClipboard>
#include <QDateTime>
#include <QDesktopWidget>
#include <QDir>
#include <QEvent>
#include <QEventLoop>
#include <QFileOpenEvent>
#include <QFileInfo>
#include <QHash>
#include <QIcon>
#include <QImageReader>
#include <QKeyEvent>
#include <QLocale>
#include <QLibraryInfo>
#include <QList>
#include <QMenuBar>
#include <QMimeData>
#include <QObject>
#include <QPixmap>
#include <QPixmapCache>
#include <QRegExp>
#include <QSessionManager>
#include <QSettings>
#include <QShowEvent>
#include <QSocketNotifier>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTextCodec>
#include <QTimer>
#include <QTranslator>
#include <QThreadPool>
#include <QWidget>

#ifdef Q_WS_X11
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#undef CursorShape
#undef None
#elif defined(QPA_XCB)
#include <xcb/xcb.h>
#endif

#if (QT_VERSION < 0x050000) || (QT_VERSION >= 0x050400)
#if defined(Q_OS_WIN) || defined(Q_CYGWIN_WIN)
#if (QT_VERSION < 0x050000)
#include <QWindowsMime>
#define QWINDOWSMIME QWindowsMime
#else
#include <QWinMime>
#define QWINDOWSMIME QWinMime
#endif
#ifdef Q_CC_GNU
#include <wtypes.h>
#endif
#include <objidl.h>
#endif
#endif

#ifdef Q_OS_MAC
#include <QMacPasteboardMime>
#endif // Q_OS_MAC

#include "support/bind.h"
#include <boost/crc.hpp>

#include <exception>
#include <sstream>
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
#if !defined(Q_WS_X11) && !defined(QPA_XCB)
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
	frontend::GuiApplication * guiApp = new frontend::GuiApplication(argc, argv);
	// I'd rather do that in the constructor, but I do not think that
	// the palette is accessible there.
	guiApp->colorCache().setPalette(guiApp->palette());
	return guiApp;
}


void setLocale()
{
	QLocale theLocale;
	string code;
	if (lyxrc.gui_language == "auto") {
		theLocale = QLocale::system();
		code = fromqstr(theLocale.name());
	} else {
		Language const * l = languages.getLanguage(lyxrc.gui_language);
		code = l ? l->code() : "C";
		theLocale = QLocale(toqstr(code));
	}
	// Qt tries to outsmart us and transforms en_US to C.
	Messages::guiLanguage((code == "C") ? "en_US" : code);
	QLocale::setDefault(theLocale);
	setlocale(LC_NUMERIC, "C");
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
//
// Icon loading support code
//
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
// Please don't change the formatting, this list is parsed by
// development/tools/generate_symbols_images.py.
PngMap sorted_png_map[] = {
	{ "Arrownot", "arrownot2"},
	{ "Arrowvert", "arrowvert2"},
	{ "Bowtie", "bowtie2" },
	{ "Box", "box2" },
	{ "Bumpeq", "bumpeq2" },
	{ "CIRCLE", "circle3" },
	{ "Cap", "cap2" },
	{ "CheckedBox", "checkedbox2" },
	{ "Circle", "circle2" },
	{ "Colonapprox", "colonapprox2" },
	{ "Coloneq", "coloneq2" },
	{ "Coloneqq", "coloneqq2" },
	{ "Colonsim", "colonsim2" },
	{ "Cup", "cup2" },
	{ "DOWNarrow", "downarrow3" },
	{ "Delta", "delta2" },
	{ "Diamond", "diamond2" },
	{ "Doteq", "doteq2" },
	{ "Downarrow", "downarrow2" },
	{ "Eqcolon", "eqcolon2" },
	{ "Eqqcolon", "eqqcolon2" },
	{ "Gamma", "gamma2" },
	{ "Join", "join2" },
	{ "LEFTCIRCLE", "leftcircle3" },
	{ "LEFTarrow", "leftarrow3" },
	{ "LEFTcircle", "leftcircle4" },
	{ "LHD", "lhd2" },
	{ "Lambda", "lambda2" },
	{ "Lbag", "lbag2"},
	{ "Leftarrow", "leftarrow2" },
	{ "Leftcircle", "leftcircle2" },
	{ "Leftrightarrow", "leftrightarrow2" },
	{ "Longarrownot", "longarrownot2"},
	{ "Longleftarrow", "longleftarrow2" },
	{ "Longleftrightarrow", "longleftrightarrow2" },
	{ "Longmapsfrom", "longmapsfrom2"},
	{ "Longmapsto", "longmapsto2"},
	{ "Longrightarrow", "longrightarrow2" },
	{ "Mapsfrom", "mapsfrom2"},
	{ "Mapsfromchar", "mapsfromchar2"},
	{ "Mapsto", "mapsto2"},
	{ "Mapstochar", "mapstochar2"},
	{ "Omega", "omega2" },
	{ "Phi", "phi2" },
	{ "Pi", "pi2" },
	{ "Psi", "psi2" },
	{ "RHD", "rhd2" },
	{ "RIGHTCIRCLE", "rightcircle3" },
	{ "RIGHTarrow", "rightarrow3" },
	{ "RIGHTcircle", "rightcircle4" },
	{ "Rbag", "rbag2"},
	{ "Rightarrow", "rightarrow2" },
	{ "Rightcircle", "rightcircle2" },
	{ "Sigma", "sigma2" },
	{ "Square", "square2" },
	{ "Subset", "subset2" },
	{ "Supset", "supset2" },
	{ "Theta", "theta2" },
	{ "Thorn", "thorn2" },
	{ "UParrow", "uparrow3" },
	{ "Uparrow", "uparrow2" },
	{ "Updownarrow", "updownarrow2" },
	{ "Upsilon", "upsilon2" },
	{ "Vdash", "vdash3" },
	{ "Vert", "vert2" },
	{ "XBox", "xbox3" },
	{ "Xbox", "xbox2" },
	{ "Xi", "xi2" },
	{ "lVert", "vert2" },
	{ "lvert", "vert" },
	{ "nLeftarrow", "nleftarrow2" },
	{ "nLeftrightarrow", "nleftrightarrow2" },
	{ "nRightarrow", "nrightarrow2" },
	{ "nVDash", "nvdash3" },
	{ "nVdash", "nvdash4" },
	{ "nvDash", "nvdash2" },
	{ "rVert", "vert2" },
	{ "rvert", "vert" },
	{ "textrm \\AA", "textrm_AA"},
	{ "textrm \\O", "textrm_O"},
	{ "vDash", "vdash2" },
	{ "varDelta", "vardelta2" },
	{ "varGamma", "vargamma2" },
	{ "varLambda", "varlambda2" },
	{ "varOmega", "varomega2" },
	{ "varPhi", "varphi2" },
	{ "varPi", "varpi2" },
	{ "varPsi", "varpsi2" },
	{ "varSigma", "varsigma2" },
	{ "varTheta", "vartheta2" },
	{ "varUpsilon", "varupsilon2" },
	{ "varXi", "varxi2" }
};


size_t const nr_sorted_png_map = sizeof(sorted_png_map) / sizeof(PngMap);

// This list specifies which system's theme icon is related to which lyx
// command. It was based on:
// http://standards.freedesktop.org/icon-naming-spec/icon-naming-spec-latest.html
// this must be sorted alphabetically
// Upper case comes before lower case
PngMap sorted_theme_icon_map[] = {
	{ "bookmark-goto 0", "go-jump" },
	{ "buffer-new", "document-new" },
	{ "buffer-print", "document-print" },
	{ "buffer-write", "document-save" },
	{ "buffer-write-as", "document-save-as" },
	{ "buffer-zoom-in", "zoom-in" },
	{ "buffer-zoom-out", "zoom-out" },
	{ "copy", "edit-copy" },
	{ "cut", "edit-cut" },
	{ "depth-decrement", "format-indent-less" },
	{ "depth-increment", "format-indent-more" },
	{ "dialog-show print", "document-print" },
	{ "dialog-show spellchecker", "tools-check-spelling" },
	{ "dialog-show-new-inset graphics", "insert-image" },
	{ "dialog-toggle findreplaceadv", "edit-find-replace" },
	{ "file-open", "document-open" },
	{ "font-bold", "format-text-bold" },
	{ "font-ital", "format-text-italic" },
	{ "font-strikeout", "format-text-strikethrough" },
	{ "font-underline", "format-text-underline" },
	{ "lyx-quit", "application-exit" },
	{ "paste", "edit-paste" },
	{ "redo", "edit-redo" },
	{ "undo", "edit-undo" },
	{ "window-close", "window-close" },
	{ "window-new", "window-new" }
};

size_t const nr_sorted_theme_icon_map = sizeof(sorted_theme_icon_map) / sizeof(PngMap);


QString findPng(QString const & name)
{
	PngMap const * const begin = sorted_png_map;
	PngMap const * const end = begin + nr_sorted_png_map;
	LATTEST(sorted(begin, end));

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


QString themeIconName(QString const & action)
{
	PngMap const * const begin = sorted_theme_icon_map;
	PngMap const * const end = begin + nr_sorted_theme_icon_map;
	LASSERT(sorted(begin, end), /**/);

	PngMap const * const it = find_if(begin, end, CompareKey(action));

	if (it != end)
		return it->value;
	return QString();
}


QString iconName(FuncRequest const & f, bool unknown)
{
	initializeResources();
	QString name1;
	QString name2;
	QString path;
	switch (f.action()) {
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
	case LFUN_COMMAND_ALTERNATIVES: {
		// use the first of the alternative commands
		docstring firstcom;
		docstring dummy = split(f.argument(), firstcom, ';');
		name1 = toqstr(firstcom);
		// FIXME: we should rename the icons to tabular-xxx instead of
		// "tabular-feature-xxx"
		name1.replace("inset-modify tabular", "tabular-feature");
		name1.replace(' ', '_');
		break;
	}
	case LFUN_INSET_MODIFY: {
		// FIXME: we should rename the icons to tabular-xxx instead of
		// "tabular-feature-xxx" and generalize this naming to all
		// insets, not to tabular using ones.
		string inset_name;
		string const command = split(to_utf8(f.argument()), inset_name, ' ');
		if (insetCode(inset_name) == TABULAR_CODE) {
			name1 = "tabular-feature "+ toqstr(command);
			name1.replace(' ', '_');
			break;
		}
	}
	default:
		name2 = toqstr(lyxaction.getActionName(f.action()));
		name1 = name2;

		if (!f.argument().empty()) {
			name1 = name2 + ' ' + toqstr(f.argument());
			name1.replace(' ', '_');
			name1.replace('\\', "backslash");
		}
	}

	QStringList imagedirs;
	imagedirs << "images/" << "images/ipa/";
	search_mode mode = theGuiApp()->imageSearchMode();
	for (int i = 0; i < imagedirs.size(); ++i) {
		QString imagedir = imagedirs.at(i) + path;
		FileName fname = imageLibFileSearch(imagedir, name1, "svgz,png", mode);
		if (fname.exists())
			return toqstr(fname.absFileName());

		fname = imageLibFileSearch(imagedir, name2, "svgz,png", mode);
		if (fname.exists())
			return toqstr(fname.absFileName());
	}

	path = ":/images/" + path;
	QDir res(path);
	if (!res.exists()) {
		LYXERR0("Directory " << path << " not found in resource!");
		return QString();
	}
	if (res.exists(name1 + ".svgz"))
		return path + name1 + ".svgz";
	else if (res.exists(name1 + ".png"))
		return path + name1 + ".png";

	if (res.exists(name2 + ".svgz"))
		return path + name2 + ".svgz";
	else if (res.exists(name2 + ".png"))
		return path + name2 + ".png";

	LYXERR(Debug::GUI, "Cannot find icon with filename "
			   << "\"" << name1 << ".{svgz,png}\""
			   << " or filename "
			   << "\"" << name2 << ".{svgz,png}\""
			   << " for command \""
			   << lyxaction.getActionName(f.action())
			   << '(' << to_utf8(f.argument()) << ")\"");

	if (unknown) {
		QString imagedir = "images/";
		FileName fname = imageLibFileSearch(imagedir, "unknown", "svgz,png", mode);
		if (fname.exists())
			return toqstr(fname.absFileName());
		return QString(":/images/unknown.png");
	}

	return QString();
}


bool getPixmap(QPixmap & pixmap, QString const & path)
{
	if (pixmap.load(path)) {
#if QT_VERSION >= 0x050000
		if (path.endsWith(".svgz") || path.endsWith(".svg") ) {
			GuiApplication const * guiApp = theGuiApp();
			if (guiApp != 0) {
				pixmap.setDevicePixelRatio(guiApp->pixelRatio());
			}
		}
#endif
		return true;
	}
	return false;
}


QPixmap getPixmap(QString const & path, QString const & name, QString const & ext)
{
	QString imagedir = path;
	FileName fname = imageLibFileSearch(imagedir, name, ext, theGuiApp()->imageSearchMode());
	QString fpath = toqstr(fname.absFileName());
	QPixmap pixmap = QPixmap();

	if (getPixmap(pixmap, fpath)) {
		return pixmap;
	}
	
	QStringList exts = ext.split(",");
	fpath = ":/" + path + name + ".";
	for (int i = 0; i < exts.size(); ++i) {
		if (getPixmap(pixmap, fpath + exts.at(i))) {
			return pixmap;
		}
	}

	bool const list = ext.contains(",");
	LYXERR0("Cannot load pixmap \""
		<< path << name << "." << (list ? "{" : "") << ext
		<< (list ? "}" : "") << "\", please verify resource system!");

	return QPixmap();
}


QIcon getIcon(FuncRequest const & f, bool unknown)
{
#if (QT_VERSION >= 0x040600)
	if (lyxrc.use_system_theme_icons) {
		QString action = toqstr(lyxaction.getActionName(f.action()));
		if (!f.argument().empty())
			action += " " + toqstr(f.argument());
		QString const theme_icon = themeIconName(action);
		if (QIcon::hasThemeIcon(theme_icon))
			return QIcon::fromTheme(theme_icon);
	}
#endif

	QString icon = iconName(f, unknown);
	if (icon.isEmpty())
		return QIcon();

	//LYXERR(Debug::GUI, "Found icon: " << icon);
	QPixmap pixmap = QPixmap();
	if (!getPixmap(pixmap,icon)) {
		LYXERR0("Cannot load icon " << icon << " please verify resource system!");
		return QIcon();
	}

	return QIcon(pixmap);
}


////////////////////////////////////////////////////////////////////////
//
// LyX server support code.
//
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
//
// Mac specific stuff goes here...
//
////////////////////////////////////////////////////////////////////////

class MenuTranslator : public QTranslator
{
public:
	MenuTranslator(QObject * parent)
		: QTranslator(parent)
	{}

#if QT_VERSION >= 0x050000
	virtual QString translate(const char * /* context */,
		const char *sourceText,
		const char * /* disambiguation */ = 0, int /* n */ = -1) const
#else
	QString translate(const char * /*context*/,
	  const char * sourceText,
	  const char * /*comment*/ = 0) const
#endif
	{
		string const s = sourceText;
		if (s == N_("About %1")	|| s == N_("Preferences")
				|| s == N_("Reconfigure") || s == N_("Quit %1"))
			return qt_(s);
		else
			return QString();
	}
};

#ifdef Q_OS_MAC
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
//
// You can find more platform specific stuff at the end of this file...
//
///////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
// Windows specific stuff goes here...

#if (QT_VERSION < 0x050000) || (QT_VERSION >= 0x050400)
#if defined(Q_OS_WIN) || defined(Q_CYGWIN_WIN)
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


class QWindowsMimeMetafile : public QWINDOWSMIME {
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


	QVector<FORMATETC> formatsForMime(QString const & mimetype,
		QMimeData const * mimedata) const
	{
		QVector<FORMATETC> formats;
		if (mimetype == emfMimeType() || mimetype == wmfMimeType())
			formats += cfFromMime(mimetype);
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

#endif
#endif


/// Allows to check whether ESC was pressed during a long operation
class KeyChecker : public QObject {
private:
	bool pressed_;
	bool started_;
public:
	KeyChecker() : pressed_(false), started_(false) {}

	void start() {
		QCoreApplication::instance()->installEventFilter(this);
		pressed_ = false;
		started_ = true;
	}
	void stop() {
		QCoreApplication::instance()->removeEventFilter(this);
		started_ = false;
	}
	bool pressed() {
		QCoreApplication::processEvents();
		return pressed_;
	}
	bool started() const {
		return started_;
	}
	bool eventFilter(QObject *obj, QEvent *event) {
		LYXERR(Debug::ACTION, "Event Type: " << event->type());
		switch (event->type()) {
		case QEvent::Show:
		case QEvent::Hide:
		case QEvent::Resize:
		case QEvent::UpdateRequest:
		case QEvent::CursorChange:
		case QEvent::ActionChanged:
		case QEvent::EnabledChange:
		case QEvent::SockAct:
		case QEvent::Timer:
		case QEvent::Paint:
		case QEvent::ToolTipChange:
		case QEvent::LayoutRequest:
		case QEvent::MetaCall:
			return QObject::eventFilter(obj, event);
		default:
			// FIXME Blocking all these events is a bad idea.
			QKeyEvent *keyEvent = dynamic_cast<QKeyEvent*>(event);
			if (keyEvent && keyEvent->key() == Qt::Key_Escape)
				pressed_ = true;
			return true;
		}
	}
};


////////////////////////////////////////////////////////////////////////
// GuiApplication::Private definition and implementation.
////////////////////////////////////////////////////////////////////////

struct GuiApplication::Private
{
	Private(): language_model_(0), meta_fake_bit(NoModifier),
		global_menubar_(0)
	{
	#if (QT_VERSION < 0x050000) || (QT_VERSION >= 0x050400)
	#if defined(Q_OS_WIN) || defined(Q_CYGWIN_WIN)
		/// WMF Mime handler for Windows clipboard.
		wmf_mime_ = new QWindowsMimeMetafile;
	#endif
	#endif
		initKeySequences(&theTopLevelKeymap());
	}

	void initKeySequences(KeyMap * kb)
	{
		keyseq = KeySequence(kb, kb);
		cancel_meta_seq = KeySequence(kb, kb);
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
	///
	/// The global instance
	Toolbars toolbars_;

	/// this timer is used for any regular events one wants to
	/// perform. at present it is used to check if forked processes
	/// are done.
	QTimer general_timer_;

	/// delayed FuncRequests
	std::queue<FuncRequest> func_request_queue_;

	///
	KeySequence keyseq;
	///
	KeySequence cancel_meta_seq;
	///
	KeyModifier meta_fake_bit;

	/// The result of last dispatch action
	DispatchResult dispatch_result_;

	/// Multiple views container.
	/**
	* Warning: This must not be a smart pointer as the destruction of the
	* object is handled by Qt when the view is closed
	* \sa Qt::WA_DeleteOnClose attribute.
	*/
	QHash<int, GuiView *> views_;

	/// Only used on mac.
	QMenuBar * global_menubar_;

#ifdef Q_OS_MAC
	/// Linkback mime handler for MacOSX.
	QMacPasteboardMimeGraphics mac_pasteboard_mime_;
#endif

#if (QT_VERSION < 0x050000) || (QT_VERSION >= 0x050400)
#if defined(Q_OS_WIN) || defined(Q_CYGWIN_WIN)
	/// WMF Mime handler for Windows clipboard.
	QWindowsMimeMetafile * wmf_mime_;
#endif
#endif

	/// Allows to check whether ESC was pressed during a long operation
	KeyChecker key_checker_;
};


GuiApplication * guiApp;

GuiApplication::~GuiApplication()
{
#ifdef Q_OS_MAC
	closeAllLinkBackLinks();
#endif
	delete d;
}


GuiApplication::GuiApplication(int & argc, char ** argv)
	: QApplication(argc, argv), current_view_(0),
	  d(new GuiApplication::Private)
{
	QString app_name = "LyX";
	QCoreApplication::setOrganizationName(app_name);
	QCoreApplication::setOrganizationDomain("lyx.org");
	QCoreApplication::setApplicationName(lyx_package);

	qsrand(QDateTime::currentDateTime().toTime_t());

	// Install translator for GUI elements.
	installTranslator(&d->qt_trans_);

#ifdef QPA_XCB
	// Enable reception of XCB events.
	installNativeEventFilter(this);
#endif

	// FIXME: quitOnLastWindowClosed is true by default. We should have a
	// lyxrc setting for this in order to let the application stay resident.
	// But then we need some kind of dock icon, at least on Windows.
	/*
	if (lyxrc.quit_on_last_window_closed)
		setQuitOnLastWindowClosed(false);
	*/
#ifdef Q_OS_MAC
	// FIXME: Do we need a lyxrc setting for this on Mac? This behaviour
	// seems to be the default case for applications like LyX.
	setQuitOnLastWindowClosed(false);
	// This allows to translate the strings that appear in the LyX menu.
	/// A translator suitable for the entries in the LyX menu.
	/// Only needed with Qt/Mac.
	installTranslator(new MenuTranslator(this));
	///
	setupApplescript();
#endif

#if defined(Q_WS_X11) || defined(QPA_XCB)
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

	// maxThreadCount() defaults in general to 2 on single or dual-processor.
	// This is clearly not enough in a time where we use threads for
	// document preview and/or export. 20 should be OK.
	QThreadPool::globalInstance()->setMaxThreadCount(20);
}


GuiApplication * theGuiApp()
{
	return dynamic_cast<GuiApplication *>(theApp());
}


double GuiApplication::pixelRatio() const
{
#if QT_VERSION >= 0x050000
	return devicePixelRatio();
#else
	return 1.0;
#endif
}


void GuiApplication::clearSession()
{
	QSettings settings;
	settings.clear();
}


docstring Application::iconName(FuncRequest const & f, bool unknown)
{
	return qstring_to_ucs4(lyx::frontend::iconName(f, unknown));
}


docstring Application::mathIcon(docstring const & c)
{
	return qstring_to_ucs4(findPng(toqstr(c)));
}


FuncStatus GuiApplication::getStatus(FuncRequest const & cmd) const
{
	FuncStatus status;

	BufferView * bv = 0;
	BufferView * doc_bv = 0;

	if (cmd.action() == LFUN_NOACTION) {
		status.message(from_utf8(N_("Nothing to do")));
		status.setEnabled(false);
	}

	else if (cmd.action() == LFUN_UNKNOWN_ACTION) {
		status.setUnknown(true);
		status.message(from_utf8(N_("Unknown action")));
		status.setEnabled(false);
	}

	// Does the GuiApplication know something?
	else if (getStatus(cmd, status)) { }

	// If we do not have a GuiView, then other functions are disabled
	else if (!current_view_)
		status.setEnabled(false);

	// Does the GuiView know something?
	else if (current_view_->getStatus(cmd, status))	{ }

	// In LyX/Mac, when a dialog is open, the menus of the
	// application can still be accessed without giving focus to
	// the main window. In this case, we want to disable the menu
	// entries that are buffer or view-related.
	//FIXME: Abdel (09/02/10) This has very bad effect on Linux, don't know why...
	/*
	else if (cmd.origin() == FuncRequest::MENU && !current_view_->hasFocus())
		status.setEnabled(false);
	*/

	// If we do not have a BufferView, then other functions are disabled
	else if (!(bv = current_view_->currentBufferView()))
		status.setEnabled(false);

	// Does the current BufferView know something?
	else if (bv->getStatus(cmd, status)) { }

	// Does the current Buffer know something?
	else if (bv->buffer().getStatus(cmd, status)) { }

	// If we do not have a document BufferView, different from the
	// current BufferView, then other functions are disabled
	else if (!(doc_bv = current_view_->documentBufferView()) || doc_bv == bv)
		status.setEnabled(false);

	// Does the document Buffer know something?
	else if (doc_bv->buffer().getStatus(cmd, status)) { }

	else {
		LYXERR(Debug::ACTION, "LFUN not handled in getStatus(): " << cmd);
		status.message(from_utf8(N_("Command not handled")));
		status.setEnabled(false);
	}

	// the default error message if we disable the command
	if (!status.enabled() && status.message().empty())
		status.message(from_utf8(N_("Command disabled")));

	return status;
}


bool GuiApplication::getStatus(FuncRequest const & cmd, FuncStatus & flag) const
{
	// I would really like to avoid having this switch and rather try to
	// encode this in the function itself.
	// -- And I'd rather let an inset decide which LFUNs it is willing
	// to handle (Andre')
	bool enable = true;
	switch (cmd.action()) {

	// This could be used for the no-GUI version. The GUI version is handled in
	// GuiView::getStatus(). See above.
	/*
	case LFUN_BUFFER_WRITE:
	case LFUN_BUFFER_WRITE_AS: {
		Buffer * b = theBufferList().getBuffer(FileName(cmd.getArg(0)));
		enable = b && (b->isUnnamed() || !b->isClean());
		break;
	}
	*/

	case LFUN_BOOKMARK_GOTO: {
		const unsigned int num = convert<unsigned int>(to_utf8(cmd.argument()));
		enable = theSession().bookmarks().isValid(num);
		break;
	}

	case LFUN_BOOKMARK_CLEAR:
		enable = theSession().bookmarks().hasValid();
		break;

	// this one is difficult to get right. As a half-baked
	// solution, we consider only the first action of the sequence
	case LFUN_COMMAND_SEQUENCE: {
		// argument contains ';'-terminated commands
		string const firstcmd = token(to_utf8(cmd.argument()), ';', 0);
		FuncRequest func(lyxaction.lookupFunc(firstcmd));
		func.setOrigin(cmd.origin());
		flag = getStatus(func);
		break;
	}

	// we want to check if at least one of these is enabled
	case LFUN_COMMAND_ALTERNATIVES: {
		// argument contains ';'-terminated commands
		string arg = to_utf8(cmd.argument());
		while (!arg.empty()) {
			string first;
			arg = split(arg, first, ';');
			FuncRequest func(lyxaction.lookupFunc(first));
			func.setOrigin(cmd.origin());
			flag = getStatus(func);
			// if this one is enabled, the whole thing is
			if (flag.enabled())
				break;
		}
		break;
	}

	case LFUN_CALL: {
		FuncRequest func;
		string name = to_utf8(cmd.argument());
		if (theTopLevelCmdDef().lock(name, func)) {
			func.setOrigin(cmd.origin());
			flag = getStatus(func);
			theTopLevelCmdDef().release(name);
		} else {
			// catch recursion or unknown command
			// definition. all operations until the
			// recursion or unknown command definition
			// occurs are performed, so set the state to
			// enabled
			enable = true;
		}
		break;
	}

	case LFUN_CURSOR_FOLLOWS_SCROLLBAR_TOGGLE:
	case LFUN_REPEAT:
	case LFUN_PREFERENCES_SAVE:
	case LFUN_BUFFER_SAVE_AS_DEFAULT:
	case LFUN_DEBUG_LEVEL_SET:
		// these are handled in our dispatch()
		break;

	case LFUN_WINDOW_CLOSE:
		enable = d->views_.size() > 0;
		break;

	case LFUN_BUFFER_NEW:
	case LFUN_BUFFER_NEW_TEMPLATE:
	case LFUN_FILE_OPEN:
	case LFUN_HELP_OPEN:
	case LFUN_SCREEN_FONT_UPDATE:
	case LFUN_SET_COLOR:
	case LFUN_WINDOW_NEW:
	case LFUN_LYX_QUIT:
	case LFUN_LYXRC_APPLY:
	case LFUN_COMMAND_PREFIX:
	case LFUN_CANCEL:
	case LFUN_META_PREFIX:
	case LFUN_RECONFIGURE:
	case LFUN_SERVER_GET_FILENAME:
	case LFUN_SERVER_NOTIFY:
		enable = true;
		break;

	case LFUN_BUFFER_FORALL: {
		if (theBufferList().empty()) {
			flag.message(from_utf8(N_("Command not allowed without a buffer open")));
			flag.setEnabled(false);
			break;
		}

		FuncRequest const cmdToPass = lyxaction.lookupFunc(cmd.getLongArg(0));
		if (cmdToPass.action() == LFUN_UNKNOWN_ACTION) {
			flag.message(from_utf8(N_("the <LFUN-COMMAND> argument of buffer-forall is not valid")));
			flag.setEnabled(false);
		}
		break;
	}

	case LFUN_DIALOG_SHOW: {
		string const name = cmd.getArg(0);
		return name == "aboutlyx"
			|| name == "prefs"
			|| name == "texinfo"
			|| name == "progress"
			|| name == "compare";
	}

	default:
		return false;
	}

	if (!enable)
		flag.setEnabled(false);
	return true;
}

/// make a post-dispatch status message
static docstring makeDispatchMessage(docstring const & msg,
				     FuncRequest const & cmd)
{
	const bool verbose = (cmd.origin() == FuncRequest::MENU
			      || cmd.origin() == FuncRequest::TOOLBAR
			      || cmd.origin() == FuncRequest::COMMANDBUFFER);

	if (cmd.action() == LFUN_SELF_INSERT || !verbose) {
		LYXERR(Debug::ACTION, "dispatch msg is " << msg);
		return msg;
	}

	docstring dispatch_msg = msg;
	if (!dispatch_msg.empty())
		dispatch_msg += ' ';

	docstring comname = from_utf8(lyxaction.getActionName(cmd.action()));

	bool argsadded = false;

	if (!cmd.argument().empty()) {
		if (cmd.action() != LFUN_UNKNOWN_ACTION) {
			comname += ' ' + cmd.argument();
			argsadded = true;
		}
	}
	docstring const shortcuts = theTopLevelKeymap().
		printBindings(cmd, KeySequence::ForGui);

	if (!shortcuts.empty())
		comname += ": " + shortcuts;
	else if (!argsadded && !cmd.argument().empty())
		comname += ' ' + cmd.argument();

	if (!comname.empty()) {
		comname = rtrim(comname);
		dispatch_msg += '(' + rtrim(comname) + ')';
	}
	LYXERR(Debug::ACTION, "verbose dispatch msg " << to_utf8(dispatch_msg));
	return dispatch_msg;
}


DispatchResult const & GuiApplication::dispatch(FuncRequest const & cmd)
{
	Buffer * buffer = 0;
	if (current_view_ && current_view_->currentBufferView()) {
		current_view_->currentBufferView()->cursor().saveBeforeDispatchPosXY();
		buffer = &current_view_->currentBufferView()->buffer();
		if (buffer)
			buffer->undo().beginUndoGroup();
	}

	DispatchResult dr;
	// redraw the screen at the end (first of the two drawing steps).
	// This is done unless explicitly requested otherwise
	dr.screenUpdate(Update::FitCursor);
	dispatch(cmd, dr);
	updateCurrentView(cmd, dr);

	// the buffer may have been closed by one action
	if (theBufferList().isLoaded(buffer))
		buffer->undo().endUndoGroup();

	d->dispatch_result_ = dr;
	return d->dispatch_result_;
}


void GuiApplication::updateCurrentView(FuncRequest const & cmd, DispatchResult & dr)
{
	if (!current_view_)
		return;

	BufferView * bv = current_view_->currentBufferView();
	if (bv) {
		if (dr.needBufferUpdate()) {
			bv->cursor().clearBufferUpdate();
			bv->buffer().updateBuffer();
		}
		// BufferView::update() updates the ViewMetricsInfo and
		// also initializes the position cache for all insets in
		// (at least partially) visible top-level paragraphs.
		// We will redraw the screen only if needed.
		bv->processUpdateFlags(dr.screenUpdate());

		// Do we have a selection?
		theSelection().haveSelection(bv->cursor().selection());

		// update gui
		current_view_->restartCursor();
	}
	if (dr.needMessageUpdate()) {
		// Some messages may already be translated, so we cannot use _()
		current_view_->message(makeDispatchMessage(
				translateIfPossible(dr.message()), cmd));
	}
}


void GuiApplication::gotoBookmark(unsigned int idx, bool openFile,
	bool switchToBuffer)
{
	if (!theSession().bookmarks().isValid(idx))
		return;
	BookmarksSection::Bookmark const & bm =
		theSession().bookmarks().bookmark(idx);
	LASSERT(!bm.filename.empty(), return);
	string const file = bm.filename.absFileName();
	// if the file is not opened, open it.
	if (!theBufferList().exists(bm.filename)) {
		if (openFile)
			dispatch(FuncRequest(LFUN_FILE_OPEN, file));
		else
			return;
	}
	// open may fail, so we need to test it again
	if (!theBufferList().exists(bm.filename))
		return;

	// bm can be changed when saving
	BookmarksSection::Bookmark tmp = bm;

	// Special case idx == 0 used for back-from-back jump navigation
	if (idx == 0)
		dispatch(FuncRequest(LFUN_BOOKMARK_SAVE, "0"));

	// if the current buffer is not that one, switch to it.
	BufferView * doc_bv = current_view_ ?
		current_view_->documentBufferView() : 0;
	Cursor const old = doc_bv->cursor();
	if (!doc_bv || doc_bv->buffer().fileName() != tmp.filename) {
		if (switchToBuffer) {
			dispatch(FuncRequest(LFUN_BUFFER_SWITCH, file));
			if (!current_view_)
				return;
			doc_bv = current_view_->documentBufferView();
		} else
			return;
	}

	// moveToPosition try paragraph id first and then paragraph (pit, pos).
	if (!doc_bv->moveToPosition(
			tmp.bottom_pit, tmp.bottom_pos, tmp.top_id, tmp.top_pos))
		return;

	Cursor & cur = doc_bv->cursor();
	if (cur != old)
		notifyCursorLeavesOrEnters(old, cur);

	// bm changed
	if (idx == 0)
		return;

	// Cursor jump succeeded!
	pit_type new_pit = cur.pit();
	pos_type new_pos = cur.pos();
	int new_id = cur.paragraph().id();

	// if bottom_pit, bottom_pos or top_id has been changed, update bookmark
	// see http://www.lyx.org/trac/ticket/3092
	if (bm.bottom_pit != new_pit || bm.bottom_pos != new_pos
		|| bm.top_id != new_id) {
		const_cast<BookmarksSection::Bookmark &>(bm).updatePos(
			new_pit, new_pos, new_id);
	}
}

// This function runs "configure" and then rereads lyx.defaults to
// reconfigure the automatic settings.
void GuiApplication::reconfigure(string const & option)
{
	// emit message signal.
	if (current_view_)
		current_view_->message(_("Running configure..."));

	// Run configure in user lyx directory
	string const lock_file = package().getConfigureLockName();
	int fd = fileLock(lock_file.c_str());
	int const ret = package().reconfigureUserLyXDir(option);
	// emit message signal.
	if (current_view_)
		current_view_->message(_("Reloading configuration..."));
	lyxrc.read(libFileSearch(QString(), "lyxrc.defaults"), false);
	// Re-read packages.lst
	LaTeXPackages::getAvailable();
	fileUnlock(fd, lock_file.c_str());

	if (ret)
		Alert::information(_("System reconfiguration failed"),
			   _("The system reconfiguration has failed.\n"
				  "Default textclass is used but LyX may\n"
				  "not be able to work properly.\n"
				  "Please reconfigure again if needed."));
	else
		Alert::information(_("System reconfigured"),
			   _("The system has been reconfigured.\n"
			     "You need to restart LyX to make use of any\n"
			     "updated document class specifications."));
}

void GuiApplication::validateCurrentView()
{
	if (!d->views_.empty() && !current_view_) {
		// currently at least one view exists but no view has the focus.
		// choose the last view to make it current.
		// a view without any open document is preferred.
		GuiView * candidate = 0;
		QHash<int, GuiView *>::const_iterator it = d->views_.begin();
		QHash<int, GuiView *>::const_iterator end = d->views_.end();
		for (; it != end; ++it) {
			candidate = *it;
			if (!candidate->documentBufferView())
				break;
		}
		setCurrentView(candidate);
	}
}

void GuiApplication::dispatch(FuncRequest const & cmd, DispatchResult & dr)
{
	string const argument = to_utf8(cmd.argument());
	FuncCode const action = cmd.action();

	LYXERR(Debug::ACTION, "cmd: " << cmd);

	// we have not done anything wrong yet.
	dr.setError(false);

	FuncStatus const flag = getStatus(cmd);
	if (!flag.enabled()) {
		// We cannot use this function here
		LYXERR(Debug::ACTION, "action "
		       << lyxaction.getActionName(action)
		       << " [" << action << "] is disabled at this location");
		dr.setMessage(flag.message());
		dr.setError(true);
		dr.dispatched(false);
		dr.screenUpdate(Update::None);
		dr.clearBufferUpdate();
		return;
	};

	if (cmd.origin() == FuncRequest::LYXSERVER) {
		if (current_view_ && current_view_->currentBufferView())
			current_view_->currentBufferView()->cursor().saveBeforeDispatchPosXY();
		// we will also need to redraw the screen at the end
		dr.screenUpdate(Update::FitCursor);
	}

	// Assumes that the action will be dispatched.
	dr.dispatched(true);

	switch (cmd.action()) {

	case LFUN_WINDOW_NEW:
		createView(toqstr(cmd.argument()));
		break;

	case LFUN_WINDOW_CLOSE:
		// update bookmark pit of the current buffer before window close
		for (size_t i = 0; i < theSession().bookmarks().size(); ++i)
			gotoBookmark(i+1, false, false);
		// clear the last opened list, because
		// maybe this will end the session
		theSession().lastOpened().clear();
		// check for valid current_view_
		validateCurrentView();
		if (current_view_)
			current_view_->closeScheduled();
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
		theBufferList().changed(false);
		// Restore current_view_
		current_view_ = view;
		break;
	}

	case LFUN_BUFFER_NEW:
		validateCurrentView();
		if (d->views_.empty()
		   || (!lyxrc.open_buffers_in_tabs && current_view_->documentBufferView() != 0)) {
			createView(QString(), false); // keep hidden
			current_view_->newDocument(to_utf8(cmd.argument()), false);
			current_view_->show();
			setActiveWindow(current_view_);
		} else {
			current_view_->newDocument(to_utf8(cmd.argument()), false);
		}
		break;

	case LFUN_BUFFER_NEW_TEMPLATE:
		validateCurrentView();
		if (d->views_.empty()
		   || (!lyxrc.open_buffers_in_tabs && current_view_->documentBufferView() != 0)) {
			createView();
			current_view_->newDocument(to_utf8(cmd.argument()), true);
			if (!current_view_->documentBufferView())
				current_view_->close();
		} else {
			current_view_->newDocument(to_utf8(cmd.argument()), true);
		}
		break;

	case LFUN_FILE_OPEN: {
		validateCurrentView();
		// FIXME: create a new method shared with LFUN_HELP_OPEN.
		string const fname = to_utf8(cmd.argument());
		bool const is_open = FileName::isAbsolute(fname)
			&& theBufferList().getBuffer(FileName(fname));
		if (d->views_.empty()
		    || (!lyxrc.open_buffers_in_tabs
			&& current_view_->documentBufferView() != 0
			&& !is_open)) {
			// We want the ui session to be saved per document and not per
			// window number. The filename crc is a good enough identifier.
			boost::crc_32_type crc;
			crc = for_each(fname.begin(), fname.end(), crc);
			createView(crc.checksum());
			current_view_->openDocument(fname);
			if (current_view_ && !current_view_->documentBufferView())
				current_view_->close();
		} else
			current_view_->openDocument(fname);
		break;
	}

	case LFUN_HELP_OPEN: {
		// FIXME: create a new method shared with LFUN_FILE_OPEN.
		if (current_view_ == 0)
			createView();
		string const arg = to_utf8(cmd.argument());
		if (arg.empty()) {
			current_view_->message(_("Missing argument"));
			break;
		}
		FileName fname = i18nLibFileSearch("doc", arg, "lyx");
		if (fname.empty())
			fname = i18nLibFileSearch("examples", arg, "lyx");

		if (fname.empty()) {
			lyxerr << "LyX: unable to find documentation file `"
			       << arg << "'. Bad installation?" << endl;
			break;
		}
		current_view_->message(bformat(_("Opening help file %1$s..."),
					       makeDisplayPath(fname.absFileName())));
		Buffer * buf = current_view_->loadDocument(fname, false);

#ifndef DEVEL_VERSION
		if (buf)
			buf->setReadonly(true);
#else
		(void) buf;
#endif
		break;
	}

	case LFUN_SET_COLOR: {
		string lyx_name;
		string const x11_name = split(to_utf8(cmd.argument()), lyx_name, ' ');
		if (lyx_name.empty() || x11_name.empty()) {
			if (current_view_)
				current_view_->message(
					_("Syntax: set-color <lyx_name> <x11_name>"));
			break;
		}

#if 0
		// FIXME: The graphics cache no longer has a changeDisplay method.
		string const graphicsbg = lcolor.getLyXName(Color_graphicsbg);
		bool const graphicsbg_changed =
				lyx_name == graphicsbg && x11_name != graphicsbg;
		if (graphicsbg_changed)
			graphics::GCache::get().changeDisplay(true);
#endif

		if (!lcolor.setColor(lyx_name, x11_name)) {
			if (current_view_)
				current_view_->message(
					bformat(_("Set-color \"%1$s\" failed "
				        "- color is undefined or "
				        "may not be redefined"),
				        from_utf8(lyx_name)));
			break;
		}
		// Make sure we don't keep old colors in cache.
		d->color_cache_.clear();
		// Update the current view
		lyx::dispatch(FuncRequest(LFUN_SCREEN_FONT_UPDATE));
		break;
	}

	case LFUN_LYXRC_APPLY: {
		// reset active key sequences, since the bindings
		// are updated (bug 6064)
		d->keyseq.reset();
		LyXRC const lyxrc_orig = lyxrc;

		istringstream ss(to_utf8(cmd.argument()));
		bool const success = lyxrc.read(ss);

		if (!success) {
			lyxerr << "Warning in LFUN_LYXRC_APPLY!\n"
					<< "Unable to read lyxrc data"
					<< endl;
			break;
		}

		actOnUpdatedPrefs(lyxrc_orig, lyxrc);

		// If the request comes from the minibuffer, then we can't reset
		// the GUI, since that would destory the minibuffer itself and
		// cause a crash, since we are currently in one of the methods of
		// GuiCommandBuffer. See bug #8540.
		if (cmd.origin() != FuncRequest::COMMANDBUFFER)
			resetGui();
		// else
		//   FIXME Unfortunately, that leaves a bug here, since we cannot
		//   reset the GUI in this case. If the changes to lyxrc affected the
		//   UI, then, nothing would happen. This seems fairly unlikely, but
		//   it definitely is a bug.

		break;
	}

	case LFUN_COMMAND_PREFIX:
		dispatch(FuncRequest(LFUN_MESSAGE, d->keyseq.printOptions(true)));
		break;

	case LFUN_CANCEL: {
		d->keyseq.reset();
		d->meta_fake_bit = NoModifier;
		GuiView * gv = currentView();
		if (gv && gv->currentBufferView())
			// cancel any selection
			processFuncRequest(FuncRequest(LFUN_MARK_OFF));
		dr.setMessage(from_ascii(N_("Cancel")));
		break;
	}
	case LFUN_META_PREFIX:
		d->meta_fake_bit = AltModifier;
		dr.setMessage(d->keyseq.print(KeySequence::ForGui));
		break;

	// --- Menus -----------------------------------------------
	case LFUN_RECONFIGURE:
		// argument is any additional parameter to the configure.py command
		reconfigure(to_utf8(cmd.argument()));
		break;

	// --- lyxserver commands ----------------------------
	case LFUN_SERVER_GET_FILENAME: {
		if (current_view_ && current_view_->documentBufferView()) {
			docstring const fname = from_utf8(
				current_view_->documentBufferView()->buffer().absFileName());
			dr.setMessage(fname);
			LYXERR(Debug::INFO, "FNAME[" << fname << ']');
		} else {
			dr.setMessage(docstring());
			LYXERR(Debug::INFO, "No current file for LFUN_SERVER_GET_FILENAME");
		}
		break;
	}

	case LFUN_SERVER_NOTIFY: {
		docstring const dispatch_buffer = d->keyseq.print(KeySequence::Portable);
		dr.setMessage(dispatch_buffer);
		theServer().notifyClient(to_utf8(dispatch_buffer));
		break;
	}

	case LFUN_CURSOR_FOLLOWS_SCROLLBAR_TOGGLE:
		lyxrc.cursor_follows_scrollbar = !lyxrc.cursor_follows_scrollbar;
		break;

	case LFUN_REPEAT: {
		// repeat command
		string countstr;
		string rest = split(argument, countstr, ' ');
		istringstream is(countstr);
		int count = 0;
		is >> count;
		//lyxerr << "repeat: count: " << count << " cmd: " << rest << endl;
		for (int i = 0; i < count; ++i)
			dispatch(lyxaction.lookupFunc(rest));
		break;
	}

	case LFUN_COMMAND_SEQUENCE: {
		// argument contains ';'-terminated commands
		string arg = argument;
		// FIXME: this LFUN should also work without any view.
		Buffer * buffer = (current_view_ && current_view_->documentBufferView())
				  ? &(current_view_->documentBufferView()->buffer()) : 0;
		if (buffer)
			buffer->undo().beginUndoGroup();
		while (!arg.empty()) {
			string first;
			arg = split(arg, first, ';');
			FuncRequest func(lyxaction.lookupFunc(first));
			func.setOrigin(cmd.origin());
			dispatch(func);
		}
		// the buffer may have been closed by one action
		if (theBufferList().isLoaded(buffer))
			buffer->undo().endUndoGroup();
		break;
	}

	case LFUN_BUFFER_FORALL: {
		FuncRequest const funcToRun = lyxaction.lookupFunc(cmd.getLongArg(0));

		map<Buffer *, GuiView *> views_lVisible;
		map<GuiView *, Buffer *> activeBuffers;

		QList<GuiView *> allViews = d->views_.values();

		// this foreach does not modify any buffer. It just collects info on local visibility of buffers
		// and on which buffer is active in each view.
		Buffer * const last = theBufferList().last();
		foreach (GuiView * view, allViews) {
			// all of the buffers might be locally hidden. That is, there is no active buffer.
			if (!view || !view->currentBufferView())
				activeBuffers[view] = 0;
			else
				activeBuffers[view] = &view->currentBufferView()->buffer();

			// find out if each is locally visible or locally hidden.
			// we don't use a for loop as the buffer list cycles.
			Buffer * b = theBufferList().first();
			while (true) {
				bool const locallyVisible = view && view->workArea(*b);
				if (locallyVisible) {
					bool const exists_ = (views_lVisible.find(b) != views_lVisible.end());
					// only need to overwrite/add if we don't already know a buffer is globally
					// visible or we do know but we would prefer to dispatch LFUN from the
					// current view because of cursor position issues.
					if (!exists_ || (exists_ && views_lVisible[b] != current_view_))
						views_lVisible[b] = view;
				}
				if (b == last)
					break;
				b = theBufferList().next(b);
			}
		}

		GuiView * const homeView = currentView();
		Buffer * b = theBufferList().first();
		Buffer * nextBuf = 0;
		int numProcessed = 0;
		while (true) {
			if (b != last)
				nextBuf = theBufferList().next(b); // get next now bc LFUN might close current.

			bool const visible = (views_lVisible.find(b) != views_lVisible.end());
			if (visible) {
				// first change to a view where b is locally visible, preferably current_view_.
				GuiView * const vLv = views_lVisible[b];
				vLv->setBuffer(b);
				lyx::dispatch(funcToRun);
				numProcessed++;
			}
			if (b == last)
				break;
			b = nextBuf;
		}

		// put things back to how they were (if possible).
		foreach (GuiView * view, allViews) {
			Buffer * originalBuf = activeBuffers[view];
			// there might not have been an active buffer in this view or it might have been closed by the LFUN.
			if (theBufferList().isLoaded(originalBuf))
				view->setBuffer(originalBuf);
		}
		homeView->setFocus();

		dr.setMessage(bformat(_("Applied \"%1$s\" to %2$d buffer(s)"), from_utf8(cmd.getLongArg(0)), numProcessed));
		break;
	}

	case LFUN_COMMAND_ALTERNATIVES: {
		// argument contains ';'-terminated commands
		string arg = argument;
		while (!arg.empty()) {
			string first;
			arg = split(arg, first, ';');
			FuncRequest func(lyxaction.lookupFunc(first));
			func.setOrigin(cmd.origin());
			FuncStatus const stat = getStatus(func);
			if (stat.enabled()) {
				dispatch(func);
				break;
			}
		}
		break;
	}

	case LFUN_CALL: {
		FuncRequest func;
		if (theTopLevelCmdDef().lock(argument, func)) {
			func.setOrigin(cmd.origin());
			dispatch(func);
			theTopLevelCmdDef().release(argument);
		} else {
			if (func.action() == LFUN_UNKNOWN_ACTION) {
				// unknown command definition
				lyxerr << "Warning: unknown command definition `"
						<< argument << "'"
						<< endl;
			} else {
				// recursion detected
				lyxerr << "Warning: Recursion in the command definition `"
						<< argument << "' detected"
						<< endl;
			}
		}
		break;
	}

	case LFUN_PREFERENCES_SAVE:
		lyxrc.write(support::makeAbsPath("preferences",
			package().user_support().absFileName()), false);
		break;

	case LFUN_BUFFER_SAVE_AS_DEFAULT: {
		string const fname = addName(addPath(package().user_support().absFileName(),
			"templates/"), "defaults.lyx");
		Buffer defaults(fname);

		istringstream ss(argument);
		Lexer lex;
		lex.setStream(ss);

		// See #9236
		// We need to make sure that, after we recreat the DocumentClass,
		// which we do in readHeader, we apply it to the document itself.
		DocumentClassConstPtr olddc = defaults.params().documentClassPtr();
		int const unknown_tokens = defaults.readHeader(lex);
		DocumentClassConstPtr newdc = defaults.params().documentClassPtr();
		ErrorList el;
		InsetText & theinset = static_cast<InsetText &>(defaults.inset());
		cap::switchBetweenClasses(olddc, newdc, theinset, el);

		if (unknown_tokens != 0) {
			lyxerr << "Warning in LFUN_BUFFER_SAVE_AS_DEFAULT!\n"
			       << unknown_tokens << " unknown token"
			       << (unknown_tokens == 1 ? "" : "s")
			       << endl;
		}

		if (defaults.writeFile(FileName(defaults.absFileName())))
			dr.setMessage(bformat(_("Document defaults saved in %1$s"),
					      makeDisplayPath(fname)));
		else {
			dr.setError(true);
			dr.setMessage(from_ascii(N_("Unable to save document defaults")));
		}
		break;
	}

	case LFUN_BOOKMARK_GOTO:
		// go to bookmark, open unopened file and switch to buffer if necessary
		gotoBookmark(convert<unsigned int>(to_utf8(cmd.argument())), true, true);
		dr.screenUpdate(Update::Force | Update::FitCursor);
		break;

	case LFUN_BOOKMARK_CLEAR:
		theSession().bookmarks().clear();
		break;

	case LFUN_DEBUG_LEVEL_SET:
		lyxerr.setLevel(Debug::value(to_utf8(cmd.argument())));
		break;

	case LFUN_DIALOG_SHOW: {
		string const name = cmd.getArg(0);

		if ( name == "aboutlyx"
			|| name == "prefs"
			|| name == "texinfo"
			|| name == "progress"
			|| name == "compare")
		{
			// work around: on Mac OS the application
			// is not terminated when closing the last view.
			// Create a new one to be able to dispatch the
			// LFUN_DIALOG_SHOW to this view.
			if (current_view_ == 0)
				createView();
		}
		// fall through
	}

	default:
		// The LFUN must be for one of GuiView, BufferView, Buffer or Cursor;
		// let's try that:
		if (current_view_)
			current_view_->dispatch(cmd, dr);
		break;
	}

	if (cmd.origin() == FuncRequest::LYXSERVER)
		updateCurrentView(cmd, dr);
}


docstring GuiApplication::viewStatusMessage()
{
	// When meta-fake key is pressed, show the key sequence so far + "M-".
	if (d->meta_fake_bit != NoModifier)
		return d->keyseq.print(KeySequence::ForGui) + "M-";

	// Else, when a non-complete key sequence is pressed,
	// show the available options.
	if (d->keyseq.length() > 0 && !d->keyseq.deleted())
		return d->keyseq.printOptions(true);

	return docstring();
}


void GuiApplication::handleKeyFunc(FuncCode action)
{
	char_type c = 0;

	if (d->keyseq.length())
		c = 0;
	GuiView * gv = currentView();
	LASSERT(gv && gv->currentBufferView(), return);
	BufferView * bv = gv->currentBufferView();
	bv->getIntl().getTransManager().deadkey(
		c, get_accent(action).accent, bv->cursor().innerText(),
		bv->cursor());
	// Need to clear, in case the minibuffer calls these
	// actions
	d->keyseq.clear();
	// copied verbatim from do_accent_char
	bv->cursor().resetAnchor();
}


void GuiApplication::processKeySym(KeySymbol const & keysym, KeyModifier state)
{
	LYXERR(Debug::KEY, "KeySym is " << keysym.getSymbolName());

	// Do nothing if we have nothing (JMarc)
	if (!keysym.isOK()) {
		LYXERR(Debug::KEY, "Empty kbd action (probably composing)");
		if (current_view_)
			current_view_->restartCursor();
		return;
	}

	if (keysym.isModifier()) {
		if (current_view_)
			current_view_->restartCursor();
		return;
	}

	char_type encoded_last_key = keysym.getUCSEncoded();

	// Do a one-deep top-level lookup for
	// cancel and meta-fake keys. RVDK_PATCH_5
	d->cancel_meta_seq.reset();

	FuncRequest func = d->cancel_meta_seq.addkey(keysym, state);
	LYXERR(Debug::KEY, "action first set to [" << func.action() << ']');

	// When not cancel or meta-fake, do the normal lookup.
	// Note how the meta_fake Mod1 bit is OR-ed in and reset afterwards.
	// Mostly, meta_fake_bit = NoModifier. RVDK_PATCH_5.
	if ((func.action() != LFUN_CANCEL) && (func.action() != LFUN_META_PREFIX)) {
		// remove Caps Lock and Mod2 as a modifiers
		func = d->keyseq.addkey(keysym, (state | d->meta_fake_bit));
		LYXERR(Debug::KEY, "action now set to [" << func.action() << ']');
	}

	// Dont remove this unless you know what you are doing.
	d->meta_fake_bit = NoModifier;

	// Can this happen now ?
	if (func.action() == LFUN_NOACTION)
		func = FuncRequest(LFUN_COMMAND_PREFIX);

	LYXERR(Debug::KEY, " Key [action=" << func.action() << "]["
		<< d->keyseq.print(KeySequence::Portable) << ']');

	// already here we know if it any point in going further
	// why not return already here if action == -1 and
	// num_bytes == 0? (Lgb)

	if (d->keyseq.length() > 1 && current_view_)
		current_view_->message(d->keyseq.print(KeySequence::ForGui));


	// Maybe user can only reach the key via holding down shift.
	// Let's see. But only if shift is the only modifier
	if (func.action() == LFUN_UNKNOWN_ACTION && state == ShiftModifier) {
		LYXERR(Debug::KEY, "Trying without shift");
		func = d->keyseq.addkey(keysym, NoModifier);
		LYXERR(Debug::KEY, "Action now " << func.action());
	}

	if (func.action() == LFUN_UNKNOWN_ACTION) {
		// We didn't match any of the key sequences.
		// See if it's normal insertable text not already
		// covered by a binding
		if (keysym.isText() && d->keyseq.length() == 1) {
			// Non-printable characters (such as ASCII control characters)
			// must not be inserted (#5704)
			if (!isPrintable(encoded_last_key)) {
				LYXERR(Debug::KEY, "Non-printable character! Omitting.");
				current_view_->restartCursor();
				return;
			}
			// If a non-Shift Modifier is used we have a non-bound key sequence
			// (such as Alt+j = j). This should be omitted (#5575).
			// On Windows, AltModifier and ControlModifier are both
			// set when AltGr is pressed. Therefore, in order to not
			// break AltGr-bound symbols (see #5575 for details),
			// unbound Ctrl+Alt key sequences are allowed.
			if ((state & AltModifier || state & ControlModifier || state & MetaModifier)
#if defined(Q_OS_WIN) || defined(Q_CYGWIN_WIN)
			    && !(state & AltModifier && state & ControlModifier)
#endif
			    ) {
				current_view_->message(_("Unknown function."));
				current_view_->restartCursor();
				return;
			}
			// Since all checks above were passed, we now really have text that
			// is to be inserted (e.g., AltGr-bound symbols). Thus change the
			// func to LFUN_SELF_INSERT and thus cause the text to be inserted
			// below.
			LYXERR(Debug::KEY, "isText() is true, inserting.");
			func = FuncRequest(LFUN_SELF_INSERT, FuncRequest::KEYBOARD);
		} else {
			LYXERR(Debug::KEY, "Unknown Action and not isText() -- giving up");
			if (current_view_) {
				current_view_->message(_("Unknown function."));
				current_view_->restartCursor();
			}
			return;
		}
	}

	if (func.action() == LFUN_SELF_INSERT) {
		if (encoded_last_key != 0) {
			docstring const arg(1, encoded_last_key);
			processFuncRequest(FuncRequest(LFUN_SELF_INSERT, arg,
					     FuncRequest::KEYBOARD));
			LYXERR(Debug::KEY, "SelfInsert arg[`" << to_utf8(arg) << "']");
		}
	} else
		processFuncRequest(func);
}


void GuiApplication::processFuncRequest(FuncRequest const & func)
{
	lyx::dispatch(func);
}


void GuiApplication::processFuncRequestAsync(FuncRequest const & func)
{
	addToFuncRequestQueue(func);
	processFuncRequestQueueAsync();
}


void GuiApplication::processFuncRequestQueue()
{
	while (!d->func_request_queue_.empty()) {
		processFuncRequest(d->func_request_queue_.front());
		d->func_request_queue_.pop();
	}
}


void GuiApplication::processFuncRequestQueueAsync()
{
	QTimer::singleShot(0, this, SLOT(slotProcessFuncRequestQueue()));
}


void GuiApplication::addToFuncRequestQueue(FuncRequest const & func)
{
	d->func_request_queue_.push(func);
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
		setCurrentView(gv);
		gv->setLayoutDirection(layoutDirection());
		gv->resetDialogs();
	}

	processFuncRequest(FuncRequest(LFUN_SCREEN_FONT_UPDATE));
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
	while (d->views_.find(id) != d->views_.end())
		id++;

	LYXERR(Debug::GUI, "About to create new window with ID " << id);
	GuiView * view = new GuiView(id);
	// register view
	d->views_[id] = view;

	if (autoShow) {
		view->show();
		setActiveWindow(view);
	}

	if (!geometry_arg.isEmpty()) {
#if defined(Q_OS_WIN) || defined(Q_CYGWIN_WIN)
		int x, y;
		int w, h;
		QChar sx, sy;
		QRegExp re( "[=]*(?:([0-9]+)[xX]([0-9]+)){0,1}[ ]*(?:([+-][0-9]*)){0,1}(?:([+-][0-9]*)){0,1}" );
		re.indexIn(geometry_arg);
		w = re.cap(1).toInt();
		h = re.cap(2).toInt();
		x = re.cap(3).toInt();
		y = re.cap(4).toInt();
		sx = re.cap(3).isEmpty() ? '+' : re.cap(3).at(0);
		sy = re.cap(4).isEmpty() ? '+' : re.cap(4).at(0);
		// Set initial geometry such that we can get the frame size.
		view->setGeometry(x, y, w, h);
		int framewidth = view->geometry().x() - view->x();
		int titleheight = view->geometry().y() - view->y();
		// Negative displacements must be interpreted as distances
		// from the right or bottom screen borders.
		if (sx == '-' || sy == '-') {
			QRect rec = QApplication::desktop()->screenGeometry();
			if (sx == '-')
				x += rec.width() - w - framewidth;
			if (sy == '-')
				y += rec.height() - h - titleheight;
			view->setGeometry(x, y, w, h);
		}
		// Make sure that the left and top frame borders are visible.
		if (view->x() < 0 || view->y() < 0) {
			if (view->x() < 0)
				x = framewidth;
			if (view->y() < 0)
				y = titleheight;
			view->setGeometry(x, y, w, h);
		}
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
	setLocale();
	QLocale theLocale;
	// install translation file for Qt built-in dialogs
	QString const language_name = QString("qt_") + theLocale.name();
	// language_name can be short (e.g. qt_zh) or long (e.g. qt_zh_CN).
	// Short-named translator can be loaded from a long name, but not the
	// opposite. Therefore, long name should be used without truncation.
	// c.f. http://doc.trolltech.com/4.1/qtranslator.html#load
	if (!d->qt_trans_.load(language_name,
			QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
		LYXERR(Debug::LOCALE, "Could not find Qt translations for locale "
			<< language_name);
	} else {
		LYXERR(Debug::LOCALE, "Successfully installed Qt translations for locale "
			<< language_name);
	}

	switch (theLocale.language()) {
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


void GuiApplication::execBatchCommands()
{
	setGuiLanguage();

	// Read menus
	if (!readUIFile(toqstr(lyxrc.ui_file)))
		// Gives some error box here.
		return;

#ifdef Q_OS_MAC
#if QT_VERSION > 0x040600
	setAttribute(Qt::AA_MacDontSwapCtrlAndMeta,lyxrc.mac_dontswap_ctrl_meta);
#endif
#if QT_VERSION > 0x050100
	setAttribute(Qt::AA_UseHighDpiPixmaps,true);
#endif
	// Create the global default menubar which is shown for the dialogs
	// and if no GuiView is visible.
	// This must be done after the session was recovered to know the "last files".
	d->global_menubar_ = new QMenuBar(0);
	d->menus_.fillMenuBar(d->global_menubar_, 0, true);
#endif

	lyx::execBatchCommands();
}


QAbstractItemModel * GuiApplication::languageModel()
{
	if (d->language_model_)
		return d->language_model_;

	QStandardItemModel * lang_model = new QStandardItemModel(this);
	lang_model->insertColumns(0, 3);
	QIcon speller(getPixmap("images/", "dialog-show_spellchecker", "svgz,png"));
	QIcon saurus(getPixmap("images/", "thesaurus-entry", "svgz,png"));
	Languages::const_iterator it = lyx::languages.begin();
	Languages::const_iterator end = lyx::languages.end();
	for (; it != end; ++it) {
		int current_row = lang_model->rowCount();
		lang_model->insertRows(current_row, 1);
		QModelIndex pl_item = lang_model->index(current_row, 0);
		QModelIndex sp_item = lang_model->index(current_row, 1);
		QModelIndex th_item = lang_model->index(current_row, 2);
		lang_model->setData(pl_item, qt_(it->second.display()), Qt::DisplayRole);
		lang_model->setData(pl_item, toqstr(it->second.lang()), Qt::UserRole);
		lang_model->setData(sp_item, qt_(it->second.display()), Qt::DisplayRole);
		lang_model->setData(sp_item, toqstr(it->second.lang()), Qt::UserRole);
		if (theSpellChecker() && theSpellChecker()->hasDictionary(&it->second))
			lang_model->setData(sp_item, speller, Qt::DecorationRole);
		lang_model->setData(th_item, qt_(it->second.display()), Qt::DisplayRole);
		lang_model->setData(th_item, toqstr(it->second.lang()), Qt::UserRole);
		if (thesaurus.thesaurusInstalled(from_ascii(it->second.code())))
			lang_model->setData(th_item, saurus, Qt::DecorationRole);
	}
	d->language_model_ = new QSortFilterProxyModel(this);
	d->language_model_->setSourceModel(lang_model);
	d->language_model_->setSortLocaleAware(true);
	return d->language_model_;
}


void GuiApplication::restoreGuiSession()
{
	if (!lyxrc.load_session)
		return;

	Session & session = theSession();
	LastOpenedSection::LastOpened const & lastopened =
		session.lastOpened().getfiles();

	validateCurrentView();

	FileName active_file;
	// do not add to the lastfile list since these files are restored from
	// last session, and should be already there (regular files), or should
	// not be added at all (help files).
	for (size_t i = 0; i < lastopened.size(); ++i) {
		FileName const & file_name = lastopened[i].file_name;
		if (d->views_.empty() || (!lyxrc.open_buffers_in_tabs
			  && current_view_->documentBufferView() != 0)) {
			boost::crc_32_type crc;
			string const & fname = file_name.absFileName();
			crc = for_each(fname.begin(), fname.end(), crc);
			createView(crc.checksum());
		}
		current_view_->loadDocument(file_name, false);

		if (lastopened[i].active)
			active_file = file_name;
	}

	// Restore last active buffer
	Buffer * buffer = theBufferList().getBuffer(active_file);
	if (buffer && current_view_)
		current_view_->setBuffer(buffer);

	// clear this list to save a few bytes of RAM
	session.lastOpened().clear();
}


QString const GuiApplication::romanFontName()
{
	QFont font;
	font.setStyleHint(QFont::Serif);
	font.setFamily("serif");

	return QFontInfo(font).family();
}


QString const GuiApplication::sansFontName()
{
	QFont font;
	font.setStyleHint(QFont::SansSerif);
	font.setFamily("sans");

	return QFontInfo(font).family();
}


QString const GuiApplication::typewriterFontName()
{
	QFont font;
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
		FuncRequest const fr(LFUN_FILE_OPEN, qstring_to_ucs4(foe->file()));
		processFuncRequestAsync(fr);
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
			if (!current_view_ || !current_view_->documentBufferView())
				return false;
			Buffer * buf = &current_view_->documentBufferView()->buffer();
			docstring details = e.details_ + '\n';
			details += buf->emergencyWrite();
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


bool Application::getRgbColorUncached(ColorCode col, RGBColor & rgbcol)
{
	QColor const qcol(lcolor.getX11Name(col).c_str());
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
	/** The implementation is required to avoid an application exit
	 ** when session state save is triggered by session manager.
	 ** The default implementation sends a close event to all
	 ** visible top level widgets when session managment allows
	 ** interaction.
	 ** We are changing that to check the state of each buffer in all
	 ** views and ask the users what to do if buffers are dirty.
	 ** Furthermore, we save the session state.
	 ** We do NOT close the views here since the user still can cancel
	 ** the logout process (see #9277); also, this would hide LyX from
	 ** an OSes own session handling (application restoration).
	 **/
	#ifdef QT_NO_SESSIONMANAGER
		#ifndef _MSC_VER
			#warning Qt is compiled without session manager
		#else
			#pragma message("warning: Qt is compiled without session manager")
		#endif
		(void) sm;
	#else
		if (sm.allowsInteraction() && !prepareAllViewsForLogout())
			sm.cancel();
		else
			sm.release();
	#endif
}


void GuiApplication::unregisterView(GuiView * gv)
{
	if(d->views_.contains(gv->id()) && d->views_.value(gv->id()) == gv) {
		d->views_.remove(gv->id());
		if (current_view_ == gv)
			current_view_ = 0;
	}
}


bool GuiApplication::closeAllViews()
{
	if (d->views_.empty())
		return true;

	// When a view/window was closed before without quitting LyX, there
	// are already entries in the lastOpened list.
	theSession().lastOpened().clear();

	QList<GuiView *> const views = d->views_.values();
	foreach (GuiView * view, views) {
		if (!view->closeScheduled())
			return false;
	}

	d->views_.clear();
	return true;
}


bool GuiApplication::prepareAllViewsForLogout()
{
	if (d->views_.empty())
		return true;

	QList<GuiView *> const views = d->views_.values();
	foreach (GuiView * view, views) {
		if (!view->prepareAllBuffersForLogout())
			return false;
	}

	return true;
}


GuiView & GuiApplication::view(int id) const
{
	LAPPERR(d->views_.contains(id));
	return *d->views_.value(id);
}


void GuiApplication::hideDialogs(string const & name, Inset * inset) const
{
	QList<GuiView *> const views = d->views_.values();
	foreach (GuiView * view, views)
		view->hideDialog(name, inset);
}


Buffer const * GuiApplication::updateInset(Inset const * inset) const
{
	Buffer const * buffer_ = 0;
	QHash<int, GuiView *>::const_iterator end = d->views_.end();
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


// Ensure that a file is read only once (prevents include loops)
static QStringList uifiles;
// store which ui files define Toolbars
static QStringList toolbar_uifiles;


GuiApplication::ReturnValues GuiApplication::readUIFile(FileName ui_path)
{
	enum {
		ui_menuset = 1,
		ui_toolbars,
		ui_toolbarset,
		ui_include,
		ui_format,
		ui_last
	};

	LexerKeyword uitags[] = {
		{ "format", ui_format },
		{ "include", ui_include },
		{ "menuset", ui_menuset },
		{ "toolbars", ui_toolbars },
		{ "toolbarset", ui_toolbarset }
	};

	Lexer lex(uitags);
	lex.setFile(ui_path);
	if (!lex.isOK()) {
		lyxerr << "Unable to set LyXLeX for ui file: " << ui_path
					 << endl;
	}

	if (lyxerr.debugging(Debug::PARSER))
		lex.printTable(lyxerr);

	bool error = false;
	// format before introduction of format tag
	unsigned int format = 0;
	while (lex.isOK()) {
		int const status = lex.lex();

		// we have to do this check here, outside the switch,
		// because otherwise we would start reading include files,
		// e.g., if the first tag we hit was an include tag.
		if (status == ui_format)
			if (lex.next()) {
				format = lex.getInteger();
				continue;
			}

		// this will trigger unless the first tag we hit is a format
		// tag, with the right format.
		if (format != LFUN_FORMAT)
			return FormatMismatch;

		switch (status) {
		case Lexer::LEX_FEOF:
			continue;

		case ui_include: {
			lex.next(true);
			QString const file = toqstr(lex.getString());
			bool const success = readUIFile(file, true);
			if (!success) {
				LYXERR0("Failed to read inlcuded file: " << fromqstr(file));
				return ReadError;
			}
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
			toolbar_uifiles.push_back(toqstr(ui_path.absFileName()));
			break;

		default:
			if (!rtrim(lex.getString()).empty())
				lex.printError("LyX::ReadUIFile: "
				               "Unknown menu tag: `$$Token'");
			else
				LYXERR0("Error with status: " << status);
			error = true;
			break;
		}

	}
	return (error ? ReadError : ReadOK);
}


bool GuiApplication::readUIFile(QString const & name, bool include)
{
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
		static const QString defaultUIFile = "default";
		LYXERR(Debug::INIT, "Could not find " << name);
		if (include) {
			Alert::warning(_("Could not find UI definition file"),
				bformat(_("Error while reading the included file\n%1$s\n"
					"Please check your installation."), qstring_to_ucs4(name)));
			return false;
		}
		if (name == defaultUIFile) {
			LYXERR(Debug::INIT, "Could not find default UI file!!");
			Alert::warning(_("Could not find default UI file"),
				_("LyX could not find the default UI file!\n"
				  "Please check your installation."));
			return false;
		}
		Alert::warning(_("Could not find UI definition file"),
		bformat(_("Error while reading the configuration file\n%1$s\n"
			"Falling back to default.\n"
			"Please look under Tools>Preferences>User Interface and\n"
			"check which User Interface file you are using."), qstring_to_ucs4(name)));
		return readUIFile(defaultUIFile, false);
	}

	QString const uifile = toqstr(ui_path.absFileName());
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

	ReturnValues retval = readUIFile(ui_path);

	if (retval == FormatMismatch) {
		LYXERR(Debug::FILES, "Converting ui file to format " << LFUN_FORMAT);
		TempFile tmp("convertXXXXXX.ui");
		FileName const tempfile = tmp.name();
		bool const success = prefs2prefs(ui_path, tempfile, true);
		if (!success) {
			LYXERR0("Unable to convert " << ui_path.absFileName() <<
				" to format " << LFUN_FORMAT << ".");
		} else {
			retval = readUIFile(tempfile);
		}
	}

	if (retval != ReadOK) {
		LYXERR0("Unable to read UI file: " << ui_path.absFileName());
		return false;
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
		// if an ui file which defines Toolbars has changed,
		// we have to reset the settings
		if (toolbar_uifiles.contains(uifiles[i])
		 && (!settings.contains(name_key)
		 || settings.value(name_key).toString() != uifiles[i]
		 || settings.value(name_key + "/date").toDateTime() != date_value)) {
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


void GuiApplication::startLongOperation() {
	d->key_checker_.start();
}


bool GuiApplication::longOperationCancelled() {
	return d->key_checker_.pressed();
}


void GuiApplication::stopLongOperation() {
	d->key_checker_.stop();
}


bool GuiApplication::longOperationStarted() {
	return d->key_checker_.started();
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
		BufferView * bv = current_view_->currentBufferView();
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
		BufferView * bv = current_view_->currentBufferView();
		if (bv)
			bv->clearSelection();
		break;
	}
	}
	return false;
}
#elif defined(QPA_XCB)
bool GuiApplication::nativeEventFilter(const QByteArray & eventType,
				       void * message, long *)
{
	if (!current_view_ || eventType != "xcb_generic_event_t")
		return false;

	xcb_generic_event_t * ev = static_cast<xcb_generic_event_t *>(message);

	switch (ev->response_type) {
	case XCB_SELECTION_REQUEST: {
		xcb_selection_request_event_t * srev =
			reinterpret_cast<xcb_selection_request_event_t *>(ev);
		if (srev->selection != XCB_ATOM_PRIMARY)
			break;
		LYXERR(Debug::SELECTION, "X requested selection.");
		BufferView * bv = current_view_->currentBufferView();
		if (bv) {
			docstring const sel = bv->requestSelection();
			if (!sel.empty())
				d->selection_.put(sel);
		}
		break;
	}
	case XCB_SELECTION_CLEAR: {
		xcb_selection_clear_event_t * scev =
			reinterpret_cast<xcb_selection_clear_event_t *>(ev);
		if (scev->selection != XCB_ATOM_PRIMARY)
			break;
		LYXERR(Debug::SELECTION, "Lost selection.");
		BufferView * bv = current_view_->currentBufferView();
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
		frontend::guiApp->hideDialogs(name, inset);
}


////////////////////////////////////////////////////////////////////
//
// Font stuff
//
////////////////////////////////////////////////////////////////////

frontend::FontLoader & theFontLoader()
{
	LAPPERR(frontend::guiApp);
	return frontend::guiApp->fontLoader();
}


frontend::FontMetrics const & theFontMetrics(Font const & f)
{
	return theFontMetrics(f.fontInfo());
}


frontend::FontMetrics const & theFontMetrics(FontInfo const & f)
{
	LAPPERR(frontend::guiApp);
	return frontend::guiApp->fontLoader().metrics(f);
}


////////////////////////////////////////////////////////////////////
//
// Misc stuff
//
////////////////////////////////////////////////////////////////////

frontend::Clipboard & theClipboard()
{
	LAPPERR(frontend::guiApp);
	return frontend::guiApp->clipboard();
}


frontend::Selection & theSelection()
{
	LAPPERR(frontend::guiApp);
	return frontend::guiApp->selection();
}


} // namespace lyx

#include "moc_GuiApplication.cpp"
