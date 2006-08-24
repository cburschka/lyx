/**
 * \file QWorkArea.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "frontends/LyXView.h"
#include "BufferView.h"

#include "QWorkArea.h"

#include "lcolorcache.h"
#include "qt_helpers.h"

#include "debug.h"
#include "funcrequest.h"
#include "LColor.h"
#include "support/os.h"

#include <qapplication.h>
#include <qclipboard.h>
#include <qdragobject.h>
#include <qlayout.h>
#include <qmainwindow.h>
#include <qpainter.h>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

#ifdef Q_WS_MACX
#include <Carbon/Carbon.h>
#endif

using std::endl;
using std::string;

namespace os = lyx::support::os;

namespace {
lyx::frontend::QWorkArea * wa_ptr = 0;
}

namespace lyx {
namespace frontend {

QWorkArea::QWorkArea(LyXView & owner, int, int)
	: QWidget(qApp->mainWidget()), owner_(owner), painter_(*this)
{
	scrollbar_ = new QScrollBar(QScrollBar::Vertical, this);
	content_ = new QContentPane(this);

	(static_cast<QMainWindow*>(qApp->mainWidget()))->setCentralWidget(this);

	setFocusProxy(content_);
	setAcceptDrops(true);

	content_->show();

	// It is said that this helps reduce flicker
	content_->setBackgroundMode(NoBackground);

	QHBoxLayout * vl = new QHBoxLayout(this);
	vl->addWidget(content_, 5);
	vl->addWidget(scrollbar_, 0);

#ifdef Q_WS_MACX
	wa_ptr = this;
#endif
	show();
}


QWorkArea::~QWorkArea()
{
}


void QWorkArea::setScrollbarParams(int h, int pos, int line_h)
{
	scrollbar_->setTracking(false);
	// do what cursor movement does (some grey)
	h += height() / 4;

	int max = std::max(0, h - height());

	scrollbar_->setRange(0, max);
	content_->trackScrollbar(false);
	scrollbar_->setValue(pos);
	content_->trackScrollbar(true);
	scrollbar_->setLineStep(line_h);
	scrollbar_->setPageStep(height());

	scrollbar_->setTracking(true);
}

} // namespace frontend
} // namespace lyx


#ifdef Q_WS_X11
bool lyxX11EventFilter(XEvent * xev)
{
	switch (xev->type) {
	case SelectionRequest:
		lyxerr[Debug::GUI] << "X requested selection." << endl;
		if (wa_ptr)
			wa_ptr->view().view()->selectionRequested();
		break;
	case SelectionClear:
		lyxerr[Debug::GUI] << "Lost selection." << endl;
		if (wa_ptr)
			wa_ptr->view().view()->selectionLost();
		break;
	}
	return false;
}
#endif

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
					wa_ptr->view().view()->workAreaDispatch(
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
#endif  // Q_WS_MACX


namespace lyx {
namespace frontend {

void QWorkArea::haveSelection(bool own)
{
	wa_ptr = const_cast<QWorkArea*>(this);

	if (!QApplication::clipboard()->supportsSelection())
		return;

	if (own) {
		QApplication::clipboard()->setSelectionMode(true);
		QApplication::clipboard()->setText(QString());
	}
	// We don't need to do anything if own = false, as this case is
	// handled by QT.
}


void QWorkArea::dragEnterEvent(QDragEnterEvent * event)
{
	event->accept(QUriDrag::canDecode(event));
}


void QWorkArea::dropEvent(QDropEvent * event)
{
	QStringList files;

	if (QUriDrag::decodeLocalFiles(event, files)) {
		lyxerr[Debug::GUI] << "QWorkArea::dropEvent: got URIs!"
				   << endl;
		for (QStringList::Iterator i = files.begin();
		     i != files.end(); ++i) {
			string const file = os::internal_path(fromqstr(*i));
			view().view()->workAreaDispatch(FuncRequest(LFUN_FILE_OPEN, file));
		}
	}
}

} // namespace frontend
} // namespace lyx
