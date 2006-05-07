/**
 * \file QWorkArea.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include <boost/current_function.hpp>

#include "QWorkArea.h"
#include "QLPainter.h"
#include "QLyXKeySym.h"

#include "lcolorcache.h"
#include "qt_helpers.h"
#include "BufferView.h"
#include "debug.h"
#include "funcrequest.h"
#include "LColor.h"
#include "support/os.h"

#include <QApplication>
#include <QClipboard>
#include <QLayout>
#include <QMainWindow>
#include <QMimeData>
#include <QUrl>
#include <QDragEnterEvent>
#include <QPixmap>
#include <QPainter>
#include <QScrollBar>

#include <boost/bind.hpp>

///////////////////////////////////////////////////////////////
// Specific stuff
#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

#ifdef Q_WS_MACX
#include <Carbon/Carbon.h>
#include <support/lstrings.h>
using lyx::support::subst;
#endif

// You can find other qt-immodule, X11 and MACX specific stuff
// at the end of this file...
///////////////////////////////////////////////////////////////

using std::endl;
using std::string;

namespace os = lyx::support::os;

namespace {

QWorkArea * wa_ptr = 0;

/// return the LyX key state from Qt's
key_modifier::state q_key_state(Qt::ButtonState state)
{
	key_modifier::state k = key_modifier::none;
	if (state & Qt::ControlModifier)
		k |= key_modifier::ctrl;
	if (state & Qt::ShiftModifier)
		k |= key_modifier::shift;
	if (state & Qt::AltModifier)
		k |= key_modifier::alt;
	return k;
}


/// return the LyX mouse button state from Qt's
mouse_button::state q_button_state(Qt::ButtonState button)
{
	mouse_button::state b = mouse_button::none;
	switch (button) {
		case Qt::LeftButton:
			b = mouse_button::button1;
			break;
		case Qt::MidButton:
			b = mouse_button::button2;
			break;
		case Qt::RightButton:
			b = mouse_button::button3;
			break;
		default:
			break;
	}
	return b;
}


/// return the LyX mouse button state from Qt's
mouse_button::state q_motion_state(Qt::ButtonState state)
{
	mouse_button::state b = mouse_button::none;
	if (state & Qt::LeftButton)
		b |= mouse_button::button1;
	if (state & Qt::MidButton)
		b |= mouse_button::button2;
	if (state & Qt::RightButton)
		b |= mouse_button::button3;
	return b;
}

} // namespace anon

// This is a 'heartbeat' generating synthetic mouse move events when the
// cursor is at the top or bottom edge of the viewport. One scroll per 0.2 s
SyntheticMouseEvent::SyntheticMouseEvent()
	: timeout(200), restart_timeout(true),
	  x_old(-1), y_old(-1), scrollbar_value_old(-1.0)
{}


QWorkArea::QWorkArea(LyXView & owner, int w, int h)
    : QAbstractScrollArea(qApp->mainWidget()), WorkArea(), view_(owner), painter_(this)
{
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	(static_cast<QMainWindow*>(qApp->mainWidget()))->setCentralWidget(this);

	setAcceptDrops(true);

	setMinimumSize(100, 70);

	viewport()->setAutoFillBackground(false);
	viewport()->setAttribute(Qt::WA_OpaquePaintEvent);

	viewport()->setFocusPolicy(Qt::WheelFocus);
	viewport()->setFocus();
	setFocusPolicy(Qt::WheelFocus);

	viewport()->setCursor(Qt::IBeamCursor);

	resize(w, h);
	show();
	workWidth_ = w;
	workHeight_ = h;

	synthetic_mouse_event_.timeout.timeout.connect(
		boost::bind(&QWorkArea::generateSyntheticMouseEvent,
			    this));

	// Initialize the vertical Scroll Bar
	QObject::connect(verticalScrollBar(), SIGNAL(actionTriggered(int)),
		this, SLOT(adjustViewWithScrollBar(int)));

	// PageStep only depends on the viewport height.
	verticalScrollBar()->setPageStep(workHeight_);

	lyxerr[Debug::GUI] << BOOST_CURRENT_FUNCTION
		<< "\n Area width\t" << width()
		<< "\n Area height\t" << height()
		<< "\n viewport width\t" << viewport()->width()
		<< "\n viewport height\t" << viewport()->height()
		<< endl;

/*
	// This is the keyboard buffering stuff...
	// I don't see any need for this under windows. The keyboard is reactive
    // enough...

	if ( !QObject::connect(&step_timer_, SIGNAL(timeout()),
		this, SLOT(keyeventTimeout())) )
			lyxerr[Debug::GUI] << "ERROR: keyeventTimeout cannot connect!" << endl;

	// Start the timer, one-shot.
	step_timer_.start(50, true);
*/

	///////////////////////////////////////////////////////////////////////
	// Specific stuff goes here...

#if USE_INPUT_METHODS
	// to make qt-immodule work
	setInputMethodEnabled(true);
#endif

#ifdef Q_WS_X11
	// doubleClickInterval() is 400 ms on X11 witch is just too long.
	// On Windows and Mac OS X, the operating system's value is used.
	// On Microsoft Windows, calling this function sets the double
	// click interval for all applications. So we don't!
	QApplication::setDoubleClickInterval(300);
#endif

#ifdef Q_WS_MACX
	wa_ptr = this;
#endif
}

QWorkArea::~QWorkArea()
{
}

void QWorkArea::setScrollbarParams(int h, int scroll_pos, int scroll_line_step)
{
	// do what cursor movement does (some grey)
	h += height() / 4;
	int scroll_max_ = std::max(0, h - height());

	verticalScrollBar()->setRange(0, scroll_max_);
	verticalScrollBar()->setSliderPosition(scroll_pos);
	verticalScrollBar()->setLineStep(scroll_line_step);
}

void QWorkArea::adjustViewWithScrollBar(int action)
{
	lyxerr[Debug::GUI] << BOOST_CURRENT_FUNCTION
		<< " verticalScrollBar val=" << verticalScrollBar()->value()
		<< " verticalScrollBar pos=" << verticalScrollBar()->sliderPosition()
		<< " min=" << verticalScrollBar()->minimum()
		<< " max=" << verticalScrollBar()->maximum()
		<< " pagestep=" << verticalScrollBar()->pageStep()
		<< " linestep=" << verticalScrollBar()->lineStep()
		<< endl;

	view_.view()->scrollDocView(verticalScrollBar()->sliderPosition());
}


void QWorkArea::haveSelection(bool own) const
{
	/// \todo ask X11 and MAC devels why this wa_ptr is useful.
	wa_ptr = const_cast<QWorkArea*>(this);

	if (!QApplication::clipboard()->supportsSelection())
		return;

	if (own) {
		QApplication::clipboard()->setText(QString(), QClipboard::Selection);
	}
	// We don't need to do anything if own = false, as this case is
	// handled by QT.
}


string const QWorkArea::getClipboard() const
{
	QString str = QApplication::clipboard()->text(QClipboard::Selection);
	lyxerr[Debug::ACTION] << "getClipboard: " << (const char*) str << endl;
	if (str.isNull())
		return string();
#ifdef Q_WS_MACX
	// The MAC clipboard uses \r for lineendings, and we use \n
	return subst(fromqstr(str), '\r', '\n');
#else
	return fromqstr(str);
#endif
}


void QWorkArea::putClipboard(string const & str) const
{
#ifdef Q_WS_MACX
	// The MAC clipboard uses \r for lineendings, and we use \n
	QApplication::clipboard()->setText(toqstr(subst(str, '\n', '\r')),
					   QClipboard::Selection);
#else
	QApplication::clipboard()->setText(toqstr(str), QClipboard::Selection);
#endif
	lyxerr[Debug::ACTION] << "putClipboard: " << str << endl;
}


void QWorkArea::dragEnterEvent(QDragEnterEvent * event)
{
	if (event->mimeData()->hasUrls())
		event->accept();
	/// \todo Ask lyx-devel is this is enough:
	/// if (event->mimeData()->hasFormat("text/plain"))
	///	event->acceptProposedAction();

}


void QWorkArea::dropEvent(QDropEvent* event)
{
	QList<QUrl> files = event->mimeData()->urls();
	if (files.isEmpty())
		return;

	lyxerr[Debug::GUI] << "QWorkArea::dropEvent: got URIs!" << endl;
	for (int i = 0; i!=files.size(); ++i) {
		string const file = os::internal_path(fromqstr(files.at(i).toString()));
		if (!file.empty())
			view_.view()->workAreaDispatch(FuncRequest(LFUN_FILE_OPEN, file));
	}
}


void QWorkArea::mousePressEvent(QMouseEvent * e)
{
	if (dc_event_.active && dc_event_ == *e) {
		dc_event_.active = false;
		FuncRequest cmd(LFUN_MOUSE_TRIPLE,
			dc_event_.x, dc_event_.y,
			q_button_state(dc_event_.state));
		view_.view()->workAreaDispatch(cmd);
		return;
	}

	FuncRequest const cmd(LFUN_MOUSE_PRESS, e->x(), e->y(),
			      q_button_state(e->button()));
	view_.view()->workAreaDispatch(cmd);
}


void QWorkArea::mouseReleaseEvent(QMouseEvent * e)
{
	if (synthetic_mouse_event_.timeout.running())
		synthetic_mouse_event_.timeout.stop();

	FuncRequest const cmd(LFUN_MOUSE_RELEASE, e->x(), e->y(),
			      q_button_state(e->button()));
	view_.view()->workAreaDispatch(cmd);
}


void QWorkArea::mouseMoveEvent(QMouseEvent * e)
{
	FuncRequest cmd(LFUN_MOUSE_MOTION, e->x(), e->y(),
			      q_motion_state(e->state()));

	// If we're above or below the work area...
	if (e->y() <= 20 || e->y() >= viewport()->height() - 20) {
		// Make sure only a synthetic event can cause a page scroll,
		// so they come at a steady rate:
		if (e->y() <= 20)
			// _Force_ a scroll up:
			cmd.y = -40;
		else
			cmd.y = viewport()->height();
		// Store the event, to be handled when the timeout expires.
		synthetic_mouse_event_.cmd = cmd;

		if (synthetic_mouse_event_.timeout.running())
			// Discard the event. Note that it _may_ be handled
			// when the timeout expires if
			// synthetic_mouse_event_.cmd has not been overwritten.
			// Ie, when the timeout expires, we handle the
			// most recent event but discard all others that
			// occurred after the one used to start the timeout
			// in the first place.
			return;
		else {
			synthetic_mouse_event_.restart_timeout = true;
			synthetic_mouse_event_.timeout.start();
			// Fall through to handle this event...
		}

	} else if (synthetic_mouse_event_.timeout.running()) {
		// Store the event, to be possibly handled when the timeout
		// expires.
		// Once the timeout has expired, normal control is returned
		// to mouseMoveEvent (restart_timeout = false).
		// This results in a much smoother 'feel' when moving the
		// mouse back into the work area.
		synthetic_mouse_event_.cmd = cmd;
		synthetic_mouse_event_.restart_timeout = false;
		return;
	}

	// Has anything changed on-screen since the last QMouseEvent
	// was received?
	double const scrollbar_value = verticalScrollBar()->value();
	if (e->x() != synthetic_mouse_event_.x_old ||
	    e->y() != synthetic_mouse_event_.y_old ||
	    scrollbar_value != synthetic_mouse_event_.scrollbar_value_old) {
		// Yes it has. Store the params used to check this.
		synthetic_mouse_event_.x_old = e->x();
		synthetic_mouse_event_.y_old = e->y();
		synthetic_mouse_event_.scrollbar_value_old = scrollbar_value;

		// ... and dispatch the event to the LyX core.
		view_.view()->workAreaDispatch(cmd);
	}
}


void QWorkArea::wheelEvent(QWheelEvent * e)
{
	// Wheel rotation by one notch results in a delta() of 120 (see
	// documentation of QWheelEvent)
	int const lines = QApplication::wheelScrollLines() * e->delta() / 120;
	verticalScrollBar()->setValue(verticalScrollBar()->value() -
			lines *  verticalScrollBar()->lineStep());
	adjustViewWithScrollBar();
}


void QWorkArea::generateSyntheticMouseEvent()
{
	// Set things off to generate the _next_ 'pseudo' event.
	if (synthetic_mouse_event_.restart_timeout)
		synthetic_mouse_event_.timeout.start();

	// Has anything changed on-screen since the last timeout signal
	// was received?
	double const scrollbar_value = verticalScrollBar()->value();
	if (scrollbar_value != synthetic_mouse_event_.scrollbar_value_old) {
		// Yes it has. Store the params used to check this.
		synthetic_mouse_event_.scrollbar_value_old = scrollbar_value;

		// ... and dispatch the event to the LyX core.
		view_.view()->workAreaDispatch(synthetic_mouse_event_.cmd);
	}
}

void QWorkArea::keyPressEvent(QKeyEvent * e)
{
	lyxerr[Debug::KEY] << BOOST_CURRENT_FUNCTION
		<< " count=" << e->count()
		<< " text=" << (const char *) e->text()
		<< " isAutoRepeat=" << e->isAutoRepeat()
		<< " key=" << e->key()
		<< endl;

//	keyeventQueue_.push(boost::shared_ptr<QKeyEvent>(new QKeyEvent(*e)));

    boost::shared_ptr<QLyXKeySym> sym(new QLyXKeySym);
    sym->set(e);
    view_.view()->workAreaKeyPress(sym, q_key_state(e->state()));

}

// This is not used for now...
void QWorkArea::keyeventTimeout()
{
	bool handle_autos = true;

	while (!keyeventQueue_.empty()) {
		boost::shared_ptr<QKeyEvent> ev = keyeventQueue_.front();

		// We never handle more than one auto repeated
		// char in a list of queued up events.
		if (!handle_autos && ev->isAutoRepeat()) {
			keyeventQueue_.pop();
			continue;
		}

                boost::shared_ptr<QLyXKeySym> sym(new QLyXKeySym);
		sym->set(ev.get());

                lyxerr[Debug::GUI] << BOOST_CURRENT_FUNCTION
                                   << " count=" << ev->count()
                                   << " text=" << (const char *) ev->text()
                                   << " isAutoRepeat=" << ev->isAutoRepeat()
                                   << " key=" << ev->key()
                                   << endl;

                view_.view()->workAreaKeyPress(sym, q_key_state(ev->state()));
		keyeventQueue_.pop();

		handle_autos = false;
	}

	// Restart the timer.
	step_timer_.start(25, true);
}


void QWorkArea::mouseDoubleClickEvent(QMouseEvent * e)
{
	dc_event_ = double_click(e);

	if (!dc_event_.active)
		return;

	dc_event_.active = false;

	FuncRequest cmd(LFUN_MOUSE_DOUBLE,
		dc_event_.x, dc_event_.y,
		q_button_state(dc_event_.state));
	view_.view()->workAreaDispatch(cmd);
}


void QWorkArea::resizeEvent(QResizeEvent * resizeEvent)
{
	workWidth_ = viewport()->width();
	workHeight_ = viewport()->height();

	verticalScrollBar()->setPageStep(viewport()->height());

	screen_device_ = QPixmap(viewport()->width(), viewport()->height());
	paint_device_ = QImage(viewport()->width(), viewport()->height(), QImage::Format_RGB32);

	view_.view()->workAreaResize();

	/*
	lyxerr[Debug::GUI] << BOOST_CURRENT_FUNCTION
		<< "\n QWidget width\t" << this->QWidget::width()
		<< "\n QWidget height\t" << this->QWidget::height()
		<< "\n viewport width\t" << viewport()->width()
		<< "\n viewport height\t" << viewport()->height()
		<< "\n QResizeEvent rect left\t" << rect().left()
		<< "\n QResizeEvent rect right\t" << rect().right()
		<< endl;
		*/
}

void QWorkArea::update(int x, int y, int w, int h)
{
	//screen_device_.fromImage(paint_device_);
	QPainter q(&screen_device_);
	q.drawImage(x, y, paint_device_.copy(x, y, w, h));

	viewport()->update(x, y, w, h);
}

void QWorkArea::paintEvent(QPaintEvent * e)
{
	/*
	lyxerr[Debug::GUI] << BOOST_CURRENT_FUNCTION
		<< "\n QWidget width\t" << this->width()
		<< "\n QWidget height\t" << this->height()
		<< "\n viewport width\t" << viewport()->width()
		<< "\n viewport height\t" << viewport()->height()
		<< "\n pixmap width\t" << pixmap_->width()
		<< "\n pixmap height\t" << pixmap_->height()
		<< "\n QPaintEvent x\t" << e->rect().x()
		<< "\n QPaintEvent y\t" << e->rect().y()
		<< "\n QPaintEvent w\t" << e->rect().width()
		<< "\n QPaintEvent h\t" << e->rect().height()
		<< endl;
	*/
	QPainter q(viewport());
	q.drawPixmap(e->rect(), screen_device_, e->rect());
}


QPixmap QWorkArea::copyScreen(int x, int y, int w, int h) const
{
	return screen_device_.copy(x, y, w, h);
}

void QWorkArea::drawScreen(int x, int y, QPixmap pixmap)
{
	QPainter q(&screen_device_);
	q.drawPixmap(x, y, pixmap);
	viewport()->update(x, y, pixmap.width(), pixmap.height());
}


///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
// Specific stuff

////////////////////////////////////////////////////////////////////////
// qt-immodule specific stuff goes here...

#if USE_INPUT_METHODS
// to make qt-immodule work

void QWorkArea::inputMethodEvent(QInputMethodEvent * e)
{
	QString const text = e->text();
	if (!text.isEmpty()) {
		int key = 0;
		// needed to make math superscript work on some systems
		// ideally, such special coding should not be necessary
		if (text == "^")
			key = Qt::Key_AsciiCircum;
		QKeyEvent ev(QEvent::KeyPress, key, *text.ascii(), 0, text);
		keyPressEvent(&ev);
	}
	e->accept();
}
#endif


////////////////////////////////////////////////////////////////////////
// X11 specific stuff goes here...

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
