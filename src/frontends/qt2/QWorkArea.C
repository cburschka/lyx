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


#include "QWorkArea.h"
#include "debug.h"
#include "lfuns.h"
#include "qt_helpers.h"
#include "lcolorcache.h"

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

#include <cctype>

using std::endl;

QWorkArea::QWorkArea(int, int, int, int)
	: WorkArea(), QWidget(qApp->mainWidget()), painter_(*this)
{
	scrollbar_ = new QScrollBar(QScrollBar::Vertical, this);
	content_ = new QContentPane(this);

	(static_cast<QMainWindow*>(qApp->mainWidget()))->setCentralWidget(this);

	setFocusProxy(content_);
	setAcceptDrops(true);

	content_->show();

	content_->setBackgroundColor(lcolorcache.get(LColor::background));

	QHBoxLayout * vl = new QHBoxLayout(this);
	vl->addWidget(content_, 5);
	vl->addWidget(scrollbar_, 0);

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

namespace {
QWorkArea const * wa_ptr = 0;
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
			dispatch(FuncRequest(LFUN_FILE_OPEN, fromqstr(*i)));
	}
}
