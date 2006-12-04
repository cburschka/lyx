/**
 * \file GuiWorkArea.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiWorkArea.h"

#include "GuiApplication.h"
#include "QLPainter.h"
#include "QLyXKeySym.h"
#include "qt_helpers.h"

#include "gettext.h"
#include "LyXView.h"

#include "BufferView.h"
#include "rowpainter.h"
#include "debug.h"
#include "funcrequest.h"
#include "LColor.h"
#include "version.h"
#include "lyxrc.h"

#include "support/filetools.h" // LibFileSearch
#include "support/os.h"

#include "graphics/GraphicsImage.h"
#include "graphics/GraphicsLoader.h"

#include <QLayout>
#include <QMainWindow>
#include <QMimeData>
#include <QUrl>
#include <QDragEnterEvent>
#include <QPainter>
#include <QScrollBar>

#include <boost/bind.hpp>
#include <boost/current_function.hpp>

#ifdef Q_WS_WIN
int const CursorWidth = 2;
#else
int const CursorWidth = 1;
#endif


using std::endl;
using std::string;

namespace os = lyx::support::os;


namespace lyx {

using support::FileName;

/// return the LyX key state from Qt's
static key_modifier::state q_key_state(Qt::KeyboardModifiers state)
{
	key_modifier::state k = key_modifier::none;
	if (state & Qt::ControlModifier)
		k |= key_modifier::ctrl;
	if (state & Qt::ShiftModifier)
		k |= key_modifier::shift;
	if (state & Qt::AltModifier || state & Qt::MetaModifier)
		k |= key_modifier::alt;
	return k;
}


/// return the LyX mouse button state from Qt's
static mouse_button::state q_button_state(Qt::MouseButton button)
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
mouse_button::state q_motion_state(Qt::MouseButtons state)
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


namespace frontend {

class CursorWidget {
public:
	CursorWidget() {}

	void draw(QPainter & painter)
	{
		// FIXME: do something depending on the cursor shape.
		if (show_ && rect_.isValid())
			painter.fillRect(rect_, color_);
	}

	void update(int x, int y, int h, CursorShape shape)
	{
		color_ = guiApp->colorCache().get(LColor::cursor);
		rect_ = QRect(x, y, CursorWidth, h);
		shape_ = shape;
	}

	void show(bool set_show = true) { show_ = set_show; }
	void hide() { show_ = false; }

	QRect const & rect() { return rect_; }

private:
	///
	CursorShape shape_;
	///
	bool show_;
	///
	QColor color_;
	///
	QRect rect_;
};


// This is a 'heartbeat' generating synthetic mouse move events when the
// cursor is at the top or bottom edge of the viewport. One scroll per 0.2 s
SyntheticMouseEvent::SyntheticMouseEvent()
	: timeout(200), restart_timeout(true),
	  x_old(-1), y_old(-1), scrollbar_value_old(-1.0)
{}


GuiWorkArea::GuiWorkArea(int w, int h, int id, LyXView & lyx_view)
	: WorkArea(id, lyx_view)
{
	cursor_ = new frontend::CursorWidget();
	cursor_->hide();

	setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setAcceptDrops(true);
	setMouseTracking(true);
	setMinimumSize(100, 70);

	viewport()->setAutoFillBackground(false);
	viewport()->setAttribute(Qt::WA_OpaquePaintEvent);
	setFocusPolicy(Qt::WheelFocus);

	viewport()->setCursor(Qt::IBeamCursor);

	resize(w, h);

	synthetic_mouse_event_.timeout.timeout.connect(
		boost::bind(&GuiWorkArea::generateSyntheticMouseEvent,
			    this));

	// Initialize the vertical Scroll Bar
	QObject::connect(verticalScrollBar(), SIGNAL(actionTriggered(int)),
		this, SLOT(adjustViewWithScrollBar(int)));

	// PageStep only depends on the viewport height.
	verticalScrollBar()->setPageStep(viewport()->height());

	lyxerr[Debug::GUI] << BOOST_CURRENT_FUNCTION
		<< "\n Area width\t" << width()
		<< "\n Area height\t" << height()
		<< "\n viewport width\t" << viewport()->width()
		<< "\n viewport height\t" << viewport()->height()
		<< endl;

	// Enables input methods for asian languages.
	// Must be set when creating custom text editing widgets.
	setAttribute(Qt::WA_InputMethodEnabled, true);
}


void GuiWorkArea::setScrollbarParams(int h, int scroll_pos, int scroll_line_step)
{
	if (verticalScrollBarPolicy() != Qt::ScrollBarAlwaysOn)
		setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

	verticalScrollBar()->setTracking(false);

	// do what cursor movement does (some grey)
	h += height() / 4;
	int scroll_max_ = std::max(0, h - height());

	verticalScrollBar()->setRange(0, scroll_max_);
	verticalScrollBar()->setSliderPosition(scroll_pos);
	verticalScrollBar()->setSingleStep(scroll_line_step);
	verticalScrollBar()->setValue(scroll_pos);

	verticalScrollBar()->setTracking(true);
}


void GuiWorkArea::adjustViewWithScrollBar(int)
{
	scrollBufferView(verticalScrollBar()->sliderPosition());
}


void GuiWorkArea::dragEnterEvent(QDragEnterEvent * event)
{
	if (event->mimeData()->hasUrls())
		event->accept();
	/// \todo Ask lyx-devel is this is enough:
	/// if (event->mimeData()->hasFormat("text/plain"))
	///	event->acceptProposedAction();
}


void GuiWorkArea::dropEvent(QDropEvent* event)
{
	QList<QUrl> files = event->mimeData()->urls();
	if (files.isEmpty())
		return;

	lyxerr[Debug::GUI] << "GuiWorkArea::dropEvent: got URIs!" << endl;
	for (int i = 0; i!=files.size(); ++i) {
		string const file = os::internal_path(fromqstr(files.at(i).toLocalFile()));
		if (!file.empty())
			dispatch(FuncRequest(LFUN_FILE_OPEN, file));
	}
}


void GuiWorkArea::focusInEvent(QFocusEvent * /*event*/)
{
	// No need to do anything if we didn't change views...
	if (theApp() == 0 || &lyx_view_ == &theApp()->currentView())
		return;

	theApp()->setCurrentView(lyx_view_);

	// FIXME: it would be better to send a signal "newBuffer()"
	// in BufferList that could be connected to the different tabbars.
	lyx_view_.updateTab();

	startBlinkingCursor();

	//FIXME: Use case: Two windows share the same buffer.
	// The first window is resize. This modify the inner Buffer
	// structure because Paragraph has a notion of line break and
	// thus line width (this is very bad!).
	// When switching to the other window which does not have the
	// same size, LyX crashes because the line break is not adapted
	// the this BufferView width.
	// The following line fix the crash by resizing the BufferView 
	// on a focusInEvent(). That is not a good fix but it is a fix
	// nevertheless. The bad side effect is that when the two
	// BufferViews show the same portion of the Buffer, the second 
	// BufferView will show the same line breaks as the first one;
	// even though those line breaks are not adapted to the second
	// BufferView width... such is life!
	resizeBufferView();
}


void GuiWorkArea::focusOutEvent(QFocusEvent * /*event*/)
{
	// No need to do anything if we didn't change views...
	if (&lyx_view_ == &theApp()->currentView())
		return;

	stopBlinkingCursor();
}


void GuiWorkArea::mousePressEvent(QMouseEvent * e)
{
	if (dc_event_.active && dc_event_ == *e) {
		dc_event_.active = false;
		FuncRequest cmd(LFUN_MOUSE_TRIPLE,
			dc_event_.x, dc_event_.y,
			q_button_state(dc_event_.state));
		dispatch(cmd);
		return;
	}

	FuncRequest const cmd(LFUN_MOUSE_PRESS, e->x(), e->y(),
			      q_button_state(e->button()));
	dispatch(cmd);
}


void GuiWorkArea::mouseReleaseEvent(QMouseEvent * e)
{
	if (synthetic_mouse_event_.timeout.running())
		synthetic_mouse_event_.timeout.stop();

	FuncRequest const cmd(LFUN_MOUSE_RELEASE, e->x(), e->y(),
			      q_button_state(e->button()));
	dispatch(cmd);
}


void GuiWorkArea::mouseMoveEvent(QMouseEvent * e)
{
	FuncRequest cmd(LFUN_MOUSE_MOTION, e->x(), e->y(),
			      q_motion_state(e->buttons()));

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
		dispatch(cmd);
	}
}


void GuiWorkArea::wheelEvent(QWheelEvent * e)
{
	// Wheel rotation by one notch results in a delta() of 120 (see
	// documentation of QWheelEvent)
	int const lines = qApp->wheelScrollLines() * e->delta() / 120;
	verticalScrollBar()->setValue(verticalScrollBar()->value() -
			lines *  verticalScrollBar()->singleStep());
	adjustViewWithScrollBar();
}


void GuiWorkArea::generateSyntheticMouseEvent()
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
		dispatch(synthetic_mouse_event_.cmd);
	}
}


void GuiWorkArea::keyPressEvent(QKeyEvent * e)
{
	lyxerr[Debug::KEY] << BOOST_CURRENT_FUNCTION
		<< " count=" << e->count()
		<< " text=" << fromqstr(e->text())
		<< " isAutoRepeat=" << e->isAutoRepeat()
		<< " key=" << e->key()
		<< endl;

	boost::shared_ptr<QLyXKeySym> sym(new QLyXKeySym);
	sym->set(e);
	processKeySym(sym, q_key_state(e->modifiers()));
}


void GuiWorkArea::mouseDoubleClickEvent(QMouseEvent * e)
{
	dc_event_ = double_click(e);

	if (!dc_event_.active)
		return;

	dc_event_.active = false;

	FuncRequest cmd(LFUN_MOUSE_DOUBLE,
		dc_event_.x, dc_event_.y,
		q_button_state(dc_event_.state));
	dispatch(cmd);
}


void GuiWorkArea::resizeEvent(QResizeEvent * ev)
{
	stopBlinkingCursor();
	screen_ = QPixmap(ev->size().width(), ev->size().height());
	verticalScrollBar()->setPageStep(viewport()->height());
	QAbstractScrollArea::resizeEvent(ev);
	resizeBufferView();
	startBlinkingCursor();
}


void GuiWorkArea::update(int x, int y, int w, int h)
{
	viewport()->repaint(x, y, w, h);
}


void GuiWorkArea::doGreyOut(QLPainter & pain)
{
	setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	pain.fillRectangle(0, 0, width(), height(),
		LColor::bottomarea);

	//if (!lyxrc.show_banner)
	//	return;
	lyxerr[Debug::GUI] << "show banner: " << lyxrc.show_banner << endl;
	/// The text to be written on top of the pixmap
	QString const text = lyx_version ? QString(lyx_version) : qt_("unknown version");
	FileName const file = support::libFileSearch("images", "banner", "ppm");
	if (file.empty())
		return;

	QPixmap pm(toqstr(file.absFilename()));
	if (!pm) {
		lyxerr << "could not load splash screen: '" << file << "'" << endl;
		return;
	}

	QFont font;
	// The font used to display the version info
	font.setStyleHint(QFont::SansSerif);
	font.setWeight(QFont::Bold);
	font.setPointSize(LyXFont::SIZE_NORMAL);

	int const w = pm.width();
	int const h = pm.height();

	int x = (width() - w) / 2;
	int y = (height() - h) / 2;

	pain.drawPixmap(x, y, pm);

	x += 260;
	y += 265;

	pain.setPen(QColor(255, 255, 0));
	pain.setFont(font);
	pain.drawText(x, y, text);
}


void GuiWorkArea::paintEvent(QPaintEvent * ev)
{
	QRect const rc = ev->rect(); 
	lyxerr[Debug::PAINTING] << "paintEvent begin: x: " << rc.x()
		<< " y: " << rc.y()
		<< " w: " << rc.width()
		<< " h: " << rc.height() << endl;

	QPainter pain(viewport());
	pain.drawPixmap(rc, screen_, rc);
	cursor_->draw(pain);
}


void GuiWorkArea::expose(int x, int y, int w, int h)
{
	QLPainter pain(&screen_);

	if (greyed_out_) {
		lyxerr[Debug::GUI] << "splash screen requested" << endl;
		doGreyOut(pain);
		verticalScrollBar()->hide();
		update(0, 0, width(), height());
		return;
	}

	verticalScrollBar()->show();
	paintText(*buffer_view_, pain);
	update(x, y, w, h);
}


void GuiWorkArea::showCursor(int x, int y, int h, CursorShape shape)
{
	cursor_->update(x, y, h, shape);
	cursor_->show();
	viewport()->update(cursor_->rect());
}


void GuiWorkArea::removeCursor()
{
	cursor_->hide();
	//if (!qApp->focusWidget())
		viewport()->update(cursor_->rect());
}


void GuiWorkArea::inputMethodEvent(QInputMethodEvent * e)
{
	QString const & text = e->commitString();
	if (!text.isEmpty()) {

		lyxerr[Debug::KEY] << BOOST_CURRENT_FUNCTION
			<< " preeditString =" << fromqstr(e->preeditString())
			<< " commitString  =" << fromqstr(e->commitString())
			<< endl;

		int key = 0;
		// needed to make math superscript work on some systems
		// ideally, such special coding should not be necessary
		if (text == "^")
			key = Qt::Key_AsciiCircum;
		// FIXME: Needs for investigation, this key is not really used,
		// the ctor below just check if key is different from 0.
		QKeyEvent ev(QEvent::KeyPress, key, Qt::NoModifier, text);
		keyPressEvent(&ev);
	}
	e->accept();
}

} // namespace frontend
} // namespace lyx

#include "GuiWorkArea_moc.cpp"
