/**
 * \file qt4/GuiApplication.C
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
#include "QLImage.h"
#include "socket_callback.h"

#include "frontends/LyXView.h"

#include "graphics/LoaderQueue.h"

#include "support/lstrings.h"
#include "support/os.h"
#include "support/package.h"

#include "BufferView.h"
#include "Color.h"
#include "debug.h"
#include "funcrequest.h"
#include "lyx_main.h"
#include "lyxfunc.h"
#include "lyxrc.h"

#include <QApplication>
#include <QClipboard>
#include <QEventLoop>
#include <QLocale>
#include <QLibraryInfo>
#include <QTextCodec>
#include <QTranslator>
#include <QWidget>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

#include <boost/bind.hpp>

using std::string;
using std::endl;

// in QLyXKeySym.C
extern void initEncodings();

///////////////////////////////////////////////////////////////
// You can find other X11 and MACX specific stuff
// at the end of this file...
///////////////////////////////////////////////////////////////

namespace {

int getDPI()
{
	QWidget w;
	return int(0.5 * (w.logicalDpiX() + w.logicalDpiY()));
}

} // namespace anon


namespace lyx {

frontend::Application * createApplication(int & argc, char * argv[])
{
	return new frontend::GuiApplication(argc, argv);
}


namespace frontend {

GuiApplication * guiApp;


GuiApplication::~GuiApplication()
{
	socket_callbacks_.clear();
}


GuiApplication::GuiApplication(int & argc, char ** argv)
	: QApplication(argc, argv), Application(argc, argv)
{
#ifdef Q_WS_X11
	// doubleClickInterval() is 400 ms on X11 which is just too long.
	// On Windows and Mac OS X, the operating system's value is used.
	// On Microsoft Windows, calling this function sets the double
	// click interval for all applications. So we don't!
	QApplication::setDoubleClickInterval(300);
#endif

#ifdef Q_WS_MACX
	AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments,
			      NewAEEventHandlerUPP(handleOpenDocuments),
			      0, false);
#endif

	// install translation file for Qt built-in dialogs
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

	/* Qt Docs:
	void QApplication::lastWindowClosed ()   [signal]
	This signal is emitted from QApplication::exec() when the last visible 
	primary window ... is closed.
	This feature be turned off by setting quitOnLastWindowClosed to false.
	*/
	//setQuitOnLastWindowClosed(false);
	// this connect should not be necessary: 
	// we rely on a Qt bug on Windows and maybe Linux
	QObject::connect(this, SIGNAL(lastWindowClosed()),
		this, SLOT(quitLyX()));

	guiApp = this;
}


void GuiApplication::quitLyX()
{
	theLyXFunc().setLyXView(0);

	// trigger LFUN_LYX_QUIT instead of QApplication::quit() directly
	// since LFUN_LYX_QUIT may have more cleanup stuff
	dispatch(FuncRequest(LFUN_LYX_QUIT, "force"));
}


Clipboard& GuiApplication::clipboard()
{
	return clipboard_;
}


Selection& GuiApplication::selection()
{
	return selection_;
}


int const GuiApplication::exec()
{
	return QApplication::exec();
}


void GuiApplication::exit(int status)
{
	QApplication::exit(status);
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


void GuiApplication::syncEvents()
{
	// This is the ONLY place where processEvents may be called.
	// During screen update/ redraw, this method is disabled to
	// prevent keyboard events being handed to the LyX core, where
	// they could cause re-entrant calls to screen update.
	processEvents(QEventLoop::ExcludeUserInputEvents);
}


bool GuiApplication::getRgbColor(LColor_color col,
	lyx::RGBColor & rgbcol)
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


string const GuiApplication::hexName(LColor_color col)
{
	return lyx::support::ltrim(fromqstr(color_cache_.get(col).name()), "#");
}


void GuiApplication::updateColor(LColor_color)
{
	// FIXME: Bleh, can't we just clear them all at once ?
	color_cache_.clear();
}


void GuiApplication::registerSocketCallback(int fd, boost::function<void()> func)
{
	socket_callbacks_[fd] =
		boost::shared_ptr<socket_callback>(new socket_callback(fd, func));
}


void GuiApplication::unregisterSocketCallback(int fd)
{
	socket_callbacks_.erase(fd);
}

////////////////////////////////////////////////////////////////////////
// X11 specific stuff goes here...
#ifdef Q_WS_X11
bool GuiApplication::x11EventFilter(XEvent * xev)
{
	BufferView * bv = currentView().view();

	switch (xev->type) {
	case SelectionRequest:
		lyxerr[Debug::GUI] << "X requested selection." << endl;
		if (bv) {
			lyx::docstring const sel = bv->requestSelection();
			if (!sel.empty())
				selection_.put(sel);
		}
		break;
	case SelectionClear:
		lyxerr[Debug::GUI] << "Lost selection." << endl;
		if (bv)
			bv->clearSelection();
		break;
	}
	return false;
}
#endif


////////////////////////////////////////////////////////////////////////
// Mac OSX specific stuff goes here...

#ifdef Q_WS_MACX
namespace{

OSErr checkAppleEventForMissingParams(const AppleEvent& theAppleEvent)
 {
	DescType returnedType;
	Size actualSize;
	OSErr err = AEGetAttributePtr(&theAppleEvent, keyMissedKeywordAttr,
				      typeWildCard, &returnedType, nil, 0,
				      &actualSize);
	switch (err) {
	case errAEDescNotFound:
		return noErr;
	case noErr:
		return errAEEventNotHandled;
	default:
		return err;
	}
 }

} // namespace

OSErr GuiApplication::handleOpenDocuments(const AppleEvent* inEvent,
				       AppleEvent* /*reply*/, long /*refCon*/)
{
	QString s_arg;
	AEDescList documentList;
	OSErr err = AEGetParamDesc(inEvent, keyDirectObject, typeAEList,
				   &documentList);
	if (err != noErr)
		return err;

	err = checkAppleEventForMissingParams(*inEvent);
	if (err == noErr) {
		long documentCount;
		err = AECountItems(&documentList, &documentCount);
		for (long documentIndex = 1;
		     err == noErr && documentIndex <= documentCount;
		     documentIndex++) {
			DescType returnedType;
			Size actualSize;
			AEKeyword keyword;
			FSRef ref;
			char qstr_buf[1024];
			err = AESizeOfNthItem(&documentList, documentIndex,
					      &returnedType, &actualSize);
			if (err == noErr) {
				err = AEGetNthPtr(&documentList, documentIndex,
						  typeFSRef, &keyword,
						  &returnedType, (Ptr)&ref,
						  sizeof(FSRef), &actualSize);
				if (err == noErr) {
					FSRefMakePath(&ref, (UInt8*)qstr_buf,
						      1024);
					s_arg=QString::fromUtf8(qstr_buf);
//					bv->workAreaDispatch(
//						FuncRequest(LFUN_FILE_OPEN,
//							    fromqstr(s_arg)));
					break;
				}
			}
		} // for ...
	}
	AEDisposeDesc(&documentList);

	return err;
}

bool GuiApplication::macEventFilter(EventRef event)
{
	if (GetEventClass(event) == kEventClassAppleEvent) {
		EventRecord eventrec;
		ConvertEventRefToEventRecord(event, &eventrec);
		AEProcessAppleEvent(&eventrec);

		return false;
	}
	return false;
}

#endif  // Q_WS_MACX

} // namespace frontend
} // namespace lyx

#include "GuiApplication_moc.cpp"
