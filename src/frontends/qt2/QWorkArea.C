/**
 * \file QWorkArea.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "debug.h"
#include "LyXView.h"
#include "version.h" // lyx_version

#include "support/filetools.h" // LibFileSearch
#include "support/lstrings.h"
#include "support/LAssert.h"

#include "QWorkArea.h"
#include "qt_helpers.h"

#include <qapplication.h>
#include <qevent.h>
#include <qdragobject.h>
#include <qpainter.h>
#include <qmainwindow.h>
#include <qlayout.h>
#include <qclipboard.h>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

#ifdef Q_OS_MAC
#include <Carbon/Carbon.h>
#endif

#include <cmath>
#include <cctype>

using std::endl;
using std::abs;
using std::hex;


namespace {
QWorkArea const * wa_ptr = 0;
}

QWorkArea::QWorkArea(int, int, int, int)
	: WorkArea(), QWidget(qApp->mainWidget()), painter_(*this)
{
	scrollbar_ = new QScrollBar(QScrollBar::Vertical, this);
	content_ = new QContentPane(this);

	(static_cast<QMainWindow*>(qApp->mainWidget()))->setCentralWidget(this);

	setFocusProxy(content_);
	setAcceptDrops(true);

	content_->show();

	content_->setBackgroundColor(toqstr(lcolor.getX11Name(LColor::background)));

	QHBoxLayout * vl = new QHBoxLayout(this);
	vl->addWidget(content_, 5);
	vl->addWidget(scrollbar_, 0);

#ifdef Q_OS_MAC
	wa_ptr = this;
#endif
	show();
}


QWorkArea::~QWorkArea()
{
}


void QWorkArea::setScrollbarParams(int h, int pos, int line_h)
{
	// do what cursor movement does (some grey)
	h += height() / 4;

	int max = std::max(0, h - height());

	scrollbar_->setRange(0, max);
	scrollbar_->setValue(pos);
	scrollbar_->setLineStep(line_h);
	scrollbar_->setPageStep(height());
}


#ifdef Q_WS_X11
bool lyxX11EventFilter(XEvent * xev)
{
	switch (xev->type) {
	case SelectionRequest:
		lyxerr[Debug::GUI] << "X requested selection." << endl;
		if (wa_ptr)
			wa_ptr->selectionRequested();
		break;
	case SelectionClear:
		lyxerr[Debug::GUI] << "Lost selection." << endl;
		if (wa_ptr)
			wa_ptr->selectionLost();
		break;
	}
	return false;
}
#endif

#ifdef Q_OS_MAC
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
}

pascal OSErr handleOpenDocuments(const AppleEvent* inEvent, 
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
					wa_ptr->viewDispatch(
						FuncRequest(LFUN_FILE_OPEN, 
							    fromqstr(s_arg)));
					break;
				}
			}
		} // for ...
	}
	AEDisposeDesc(&documentList);
	return err;
}
#endif  // Q_OS_MAC

void QWorkArea::haveSelection(bool own) const
{
	wa_ptr = this;

#if QT_VERSION >= 300
	if (!QApplication::clipboard()->supportsSelection())
		return;

	if (own) {
		QApplication::clipboard()->setSelectionMode(true);
		QApplication::clipboard()->setText(QString());
	}
	// We don't need to do anything if own = false, as this case is
	// handled by QT.
#endif
}


string const QWorkArea::getClipboard() const
{
#if QT_VERSION >= 300
	QApplication::clipboard()->setSelectionMode(true);
#endif
	QString str = QApplication::clipboard()->text();
	if (str.isNull())
		return string();
	return fromqstr(str);
}


void QWorkArea::putClipboard(string const & str) const
{
#if QT_VERSION >= 300
	QApplication::clipboard()->setSelectionMode(true);
#endif
	QApplication::clipboard()->setText(toqstr(str));
}


void QWorkArea::dragEnterEvent(QDragEnterEvent * event)
{
	event->accept(QUriDrag::canDecode(event));
}


void QWorkArea::dropEvent(QDropEvent* event)
{
	QStringList files;

	if (QUriDrag::decodeLocalFiles(event, files)) {
		lyxerr[Debug::GUI] << "QWorkArea::dropEvent: got URIs!"
				   << endl;
		for (QStringList::Iterator i = files.begin();
		     i!=files.end(); ++i)
			viewDispatch(FuncRequest(LFUN_FILE_OPEN, fromqstr(*i)));
//			lyxerr << "got file: " << fromqstr(*i) << endl;
	}
}
