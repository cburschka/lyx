/**
 * \file qt4/Application.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include "GuiWorkArea.h"
#include "Application.h"

#include "qt_helpers.h"
#include "BufferView.h"
#include "debug.h"
#include "support/lstrings.h"

#include <QApplication>
#include <QEventLoop>
#include <QTranslator>
#include <QTextCodec>
#include <QClipboard>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

using lyx::support::subst;

using std::string;
using std::endl;

///////////////////////////////////////////////////////////////
// You can find other X11 and MACX specific stuff
// at the end of this file...
///////////////////////////////////////////////////////////////

namespace lyx {
namespace frontend {

Application::Application(int & argc, char ** argv)
	: QApplication(argc, argv), buffer_view_(0)
{
#ifdef Q_WS_X11
	// doubleClickInterval() is 400 ms on X11 witch is just too long.
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
}


void Application::setBufferView(BufferView * buffer_view)
{
	buffer_view_ = buffer_view;
}


////////////////////////////////////////////////////////////////////////
// X11 specific stuff goes here...
#ifdef Q_WS_X11
bool Application::x11EventFilter(XEvent * xev)
{
	switch (xev->type) {
	case SelectionRequest:
		lyxerr[Debug::GUI] << "X requested selection." << endl;
		if (buffer_view_)
			buffer_view_->selectionRequested();
		break;
	case SelectionClear:
		lyxerr[Debug::GUI] << "Lost selection." << endl;
		if (buffer_view_)
			buffer_view_->selectionLost();
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

OSErr Application::handleOpenDocuments(const AppleEvent* inEvent,
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
//					buffer_view_->workAreaDispatch(
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

bool Application::macEventFilter(EventRef event)
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
