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
#include "frontends/Gui.h"
#include "frontends/Selection.h"

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
