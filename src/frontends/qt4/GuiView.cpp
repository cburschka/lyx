/**
 * \file GuiView.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon
 * \author Abdelrazak Younes
 * \author Peter Kümmel
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiView.h"

#include "Dialog.h"
#include "FileDialog.h"
#include "GuiApplication.h"
#include "GuiCompleter.h"
#include "GuiWorkArea.h"
#include "GuiKeySymbol.h"
#include "GuiToolbar.h"
#include "GuiToolbars.h"
#include "Menus.h"
#include "TocModel.h"

#include "qt_helpers.h"

#include "frontends/alert.h"

#include "buffer_funcs.h"
#include "Buffer.h"
#include "BufferList.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Converter.h"
#include "Cursor.h"
#include "Encoding.h"
#include "ErrorList.h"
#include "Format.h"
#include "FuncStatus.h"
#include "FuncRequest.h"
#include "Intl.h"
#include "Layout.h"
#include "Lexer.h"
#include "LyXFunc.h"
#include "LyX.h"
#include "LyXRC.h"
#include "LyXVC.h"
#include "Paragraph.h"
#include "TextClass.h"
#include "Text.h"
#include "ToolbarBackend.h"
#include "version.h"

#include "support/lassert.h"
#include "support/debug.h"
#include "support/FileName.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/ForkedCalls.h"
#include "support/lstrings.h"
#include "support/os.h"
#include "support/Package.h"
#include "support/Timeout.h"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDebug>
#include <QDesktopWidget>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QList>
#include <QMenu>
#include <QMenuBar>
#include <QPainter>
#include <QPixmap>
#include <QPoint>
#include <QPushButton>
#include <QSettings>
#include <QShowEvent>
#include <QSplitter>
#include <QStackedWidget>
#include <QStatusBar>
#include <QTimer>
#include <QToolBar>
#include <QUrl>
#include <QScrollBar>

#include <boost/bind.hpp>

#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

using namespace std;
using namespace lyx::support;

namespace lyx {
namespace frontend {

namespace {

class BackgroundWidget : public QWidget
{
public:
	BackgroundWidget()
	{
		LYXERR(Debug::GUI, "show banner: " << lyxrc.show_banner);
		/// The text to be written on top of the pixmap
		QString const text = lyx_version ?
			qt_("version ") + lyx_version : qt_("unknown version");
		splash_ = QPixmap(":/images/banner.png");

		QPainter pain(&splash_);
		pain.setPen(QColor(0, 0, 0));
		QFont font;
		// The font used to display the version info
		font.setStyleHint(QFont::SansSerif);
		font.setWeight(QFont::Bold);
		font.setPointSize(int(toqstr(lyxrc.font_sizes[FONT_SIZE_LARGE]).toDouble()));
		pain.setFont(font);
		pain.drawText(190, 225, text);
	}

	void paintEvent(QPaintEvent *)
	{
		int x = (width() - splash_.width()) / 2;
		int y = (height() - splash_.height()) / 2;
		QPainter pain(this);
		pain.drawPixmap(x, y, splash_);
	}

private:
	QPixmap splash_;
};

} // namespace anon


typedef boost::shared_ptr<Dialog> DialogPtr;

struct GuiView::GuiViewPrivate
{
	GuiViewPrivate()
		: current_work_area_(0), layout_(0), autosave_timeout_(5000),
		in_show_(false)
	{
		// hardcode here the platform specific icon size
		smallIconSize = 14;	// scaling problems
		normalIconSize = 20;	// ok, default
		bigIconSize = 26;		// better for some math icons

		splitter_ = new QSplitter;
		bg_widget_ = new BackgroundWidget;
		stack_widget_ = new QStackedWidget;
		stack_widget_->addWidget(bg_widget_);
		stack_widget_->addWidget(splitter_);
		setBackground();
	}

	~GuiViewPrivate()
	{
		delete splitter_;
		delete bg_widget_;
		delete stack_widget_;
		delete toolbars_;
	}

	QMenu * toolBarPopup(GuiView * parent)
	{
		// FIXME: translation
		QMenu * menu = new QMenu(parent);
		QActionGroup * iconSizeGroup = new QActionGroup(parent);

		QAction * smallIcons = new QAction(iconSizeGroup);
		smallIcons->setText(qt_("Small-sized icons"));
		smallIcons->setCheckable(true);
		QObject::connect(smallIcons, SIGNAL(triggered()),
			parent, SLOT(smallSizedIcons()));
		menu->addAction(smallIcons);

		QAction * normalIcons = new QAction(iconSizeGroup);
		normalIcons->setText(qt_("Normal-sized icons"));
		normalIcons->setCheckable(true);
		QObject::connect(normalIcons, SIGNAL(triggered()),
			parent, SLOT(normalSizedIcons()));
		menu->addAction(normalIcons);

		QAction * bigIcons = new QAction(iconSizeGroup);
		bigIcons->setText(qt_("Big-sized icons"));
		bigIcons->setCheckable(true);
		QObject::connect(bigIcons, SIGNAL(triggered()),
			parent, SLOT(bigSizedIcons()));
		menu->addAction(bigIcons);

		unsigned int cur = parent->iconSize().width();
		if ( cur == parent->d.smallIconSize)
			smallIcons->setChecked(true);
		else if (cur == parent->d.normalIconSize)
			normalIcons->setChecked(true);
		else if (cur == parent->d.bigIconSize)
			bigIcons->setChecked(true);

		return menu;
	}

	void setBackground()
	{
		stack_widget_->setCurrentWidget(bg_widget_);
		bg_widget_->setUpdatesEnabled(true);
	}

	TabWorkArea * tabWorkArea(int i)
	{
		return dynamic_cast<TabWorkArea *>(splitter_->widget(i));
	}

	TabWorkArea * currentTabWorkArea()
	{
		if (splitter_->count() == 1)
			// The first TabWorkArea is always the first one, if any.
			return tabWorkArea(0);

		for (int i = 0; i != splitter_->count(); ++i) {
			TabWorkArea * twa = tabWorkArea(i);
			if (current_work_area_ == twa->currentWorkArea())
				return twa;
		}

		// None has the focus so we just take the first one.
		return tabWorkArea(0);
	}

public:
	GuiWorkArea * current_work_area_;
	QSplitter * splitter_;
	QStackedWidget * stack_widget_;
	BackgroundWidget * bg_widget_;
	/// view's toolbars
	GuiToolbars * toolbars_;
	/// The main layout box.
	/** 
	 * \warning Don't Delete! The layout box is actually owned by
	 * whichever toolbar contains it. All the GuiView class needs is a
	 * means of accessing it.
	 *
	 * FIXME: replace that with a proper model so that we are not limited
	 * to only one dialog.
	 */
	GuiLayoutBox * layout_;

	///
	map<string, Inset *> open_insets_;

	///
	map<string, DialogPtr> dialogs_;

	unsigned int smallIconSize;
	unsigned int normalIconSize;
	unsigned int bigIconSize;
	///
	QTimer statusbar_timer_;
	/// auto-saving of buffers
	Timeout autosave_timeout_;
	/// flag against a race condition due to multiclicks, see bug #1119
	bool in_show_;

	///
	TocModels toc_models_;
};


GuiView::GuiView(int id)
	: d(*new GuiViewPrivate), id_(id)
{
	// GuiToolbars *must* be initialised before the menu bar.
	d.toolbars_ = new GuiToolbars(*this);

	// set ourself as the current view. This is needed for the menu bar
	// filling, at least for the static special menu item on Mac. Otherwise
	// they are greyed out.
	theLyXFunc().setLyXView(this);
	
	// Fill up the menu bar.
	guiApp->menus().fillMenuBar(menuBar(), this, true);

	setCentralWidget(d.stack_widget_);

	// Start autosave timer
	if (lyxrc.autosave) {
		d.autosave_timeout_.timeout.connect(boost::bind(&GuiView::autoSave, this));
		d.autosave_timeout_.setTimeout(lyxrc.autosave * 1000);
		d.autosave_timeout_.start();
	}
	connect(&d.statusbar_timer_, SIGNAL(timeout()),
		this, SLOT(clearMessage()));

	// We don't want to keep the window in memory if it is closed.
	setAttribute(Qt::WA_DeleteOnClose, true);

#if (!defined(Q_WS_WIN) && !defined(Q_WS_MACX))
	// assign an icon to main form. We do not do it under Qt/Win or Qt/Mac,
	// since the icon is provided in the application bundle.
	setWindowIcon(QPixmap(":/images/lyx.png"));
#endif

	// For Drag&Drop.
	setAcceptDrops(true);

	statusBar()->setSizeGripEnabled(true);

	// Forbid too small unresizable window because it can happen
	// with some window manager under X11.
	setMinimumSize(300, 200);

	if (!lyxrc.allow_geometry_session)
		// No session handling, default to a sane size.
		setGeometry(50, 50, 690, 510);

	// Now take care of session management.
	QSettings settings;
	QString const key = "view-" + QString::number(id_);
#ifdef Q_WS_X11
	QPoint pos = settings.value(key + "/pos", QPoint(50, 50)).toPoint();
	QSize size = settings.value(key + "/size", QSize(690, 510)).toSize();
	resize(size);
	move(pos);
#else
	if (!restoreGeometry(settings.value(key + "/geometry").toByteArray()))
		setGeometry(50, 50, 690, 510);
#endif
	setIconSize(settings.value(key + "/icon_size").toSize());
}


GuiView::~GuiView()
{
	if (guiApp->currentView() == this)
		guiApp->setCurrentView(0);
	theLyXFunc().setLyXView(0);
	
	delete &d;
}


TocModels & GuiView::tocModels()
{
	return d.toc_models_;
}


void GuiView::setFocus()
{
	if (d.current_work_area_)
		d.current_work_area_->setFocus();
	else
		QWidget::setFocus();
}


QMenu * GuiView::createPopupMenu()
{
	return d.toolBarPopup(this);
}


void GuiView::showEvent(QShowEvent * e)
{
	LYXERR(Debug::GUI, "Passed Geometry "
		<< size().height() << "x" << size().width()
		<< "+" << pos().x() << "+" << pos().y());

	if (d.splitter_->count() == 0)
		// No work area, switch to the background widget.
		d.setBackground();

	QMainWindow::showEvent(e);
}


void GuiView::closeEvent(QCloseEvent * close_event)
{
	// it can happen that this event arrives without selecting the view,
	// e.g. when clicking the close button on a background window.
	theLyXFunc().setLyXView(this);

	while (Buffer * b = buffer()) {
		if (b->parent()) {
			// This is a child document, just close the tab after saving
			// but keep the file loaded.
			if (!saveBuffer(*b)) {
				close_event->ignore();
				return;
			}
			removeWorkArea(d.current_work_area_);
			continue;
		}

		QVector<int> const ids = guiApp->viewIds();
		for (int i = 0; i != ids.size(); ++i) {
			if (id_ == ids[i])
				continue;
			if (guiApp->view(ids[i]).workArea(*b)) {
				// FIXME 1: should we put an alert box here that the buffer
				// is viewed elsewhere?
				// FIXME 2: should we try to save this buffer in any case?
				//saveBuffer(b);

				// This buffer is also opened in another view, so
				// but close the associated work area nevertheless.
				removeWorkArea(d.current_work_area_);
				// but don't close it.
				b = 0;
				break;
			}
		}
		if (b && !closeBuffer(*b, true)) {
			close_event->ignore();
			return;
		}
	}

	// Make sure that no LFUN use this close to be closed View.
	theLyXFunc().setLyXView(0);

	// Save toolbars configuration
	if (isFullScreen()) {
		d.toolbars_->toggleFullScreen(!isFullScreen());
		updateDialogs();
	}

	// Make sure the timer time out will not trigger a statusbar update.
	d.statusbar_timer_.stop();

	// Saving fullscreen requires additional tweaks in the toolbar code.
	// It wouldn't also work under linux natively.
	if (lyxrc.allow_geometry_session && !isFullScreen()) {
		QSettings settings;
		QString const key = "view-" + QString::number(id_);
#ifdef Q_WS_X11
		settings.setValue(key + "/pos", pos());
		settings.setValue(key + "/size", size());
#else
		settings.setValue(key + "/geometry", saveGeometry());
#endif
		settings.setValue(key + "/icon_size", iconSize());
		d.toolbars_->saveToolbarInfo();
		// Now take care of all other dialogs:
		map<string, DialogPtr>::const_iterator it = d.dialogs_.begin();
		for (; it!= d.dialogs_.end(); ++it)
			it->second->saveSession();
	}

	guiApp->unregisterView(this);
	close_event->accept();
}


void GuiView::dragEnterEvent(QDragEnterEvent * event)
{
	if (event->mimeData()->hasUrls())
		event->accept();
	/// \todo Ask lyx-devel is this is enough:
	/// if (event->mimeData()->hasFormat("text/plain"))
	///	event->acceptProposedAction();
}


void GuiView::dropEvent(QDropEvent* event)
{
	QList<QUrl> files = event->mimeData()->urls();
	if (files.isEmpty())
		return;

	LYXERR(Debug::GUI, "GuiView::dropEvent: got URLs!");
	for (int i = 0; i != files.size(); ++i) {
		string const file = os::internal_path(fromqstr(
			files.at(i).toLocalFile()));
		if (!file.empty())
			lyx::dispatch(FuncRequest(LFUN_FILE_OPEN, file));
	}
}


void GuiView::message(docstring const & str)
{
	if (ForkedProcess::iAmAChild())
		return;

	statusBar()->showMessage(toqstr(str));
	d.statusbar_timer_.stop();
	d.statusbar_timer_.start(3000);
}


void GuiView::smallSizedIcons()
{
	setIconSize(QSize(d.smallIconSize, d.smallIconSize));
}


void GuiView::normalSizedIcons()
{
	setIconSize(QSize(d.normalIconSize, d.normalIconSize));
}


void GuiView::bigSizedIcons()
{
	setIconSize(QSize(d.bigIconSize, d.bigIconSize));
}


void GuiView::clearMessage()
{
	if (!hasFocus())
		return;
	theLyXFunc().setLyXView(this);
	statusBar()->showMessage(toqstr(theLyXFunc().viewStatusMessage()));
	d.statusbar_timer_.stop();
}


void GuiView::updateWindowTitle(GuiWorkArea * wa)
{
	if (wa != d.current_work_area_)
		return;
	setWindowTitle(qt_("LyX: ") + wa->windowTitle());
	setWindowIconText(wa->windowIconText());
}


void GuiView::on_currentWorkAreaChanged(GuiWorkArea * wa)
{
	disconnectBuffer();
	disconnectBufferView();
	connectBufferView(wa->bufferView());
	connectBuffer(wa->bufferView().buffer());
	d.current_work_area_ = wa;
	QObject::connect(wa, SIGNAL(titleChanged(GuiWorkArea *)),
		this, SLOT(updateWindowTitle(GuiWorkArea *)));
	updateWindowTitle(wa);

	structureChanged();

	// The document settings needs to be reinitialised.
	updateDialog("document", "");

	// Buffer-dependent dialogs must be updated. This is done here because
	// some dialogs require buffer()->text.
	updateDialogs();
}


void GuiView::on_lastWorkAreaRemoved()
{
#ifdef Q_WS_MACX
	// On Mac close the view if there is no Tab open anymore,
	// but only if no splitter is visible
	if (!lyxrc.open_buffers_in_tabs && d.splitter_->count() == 1) {
		TabWorkArea * twa = qobject_cast<TabWorkArea *>(d.splitter_->widget(0));
		if (twa && twa->count() == 0) {
			// close the view, as no tab is open anymore
			QTimer::singleShot(0, this, SLOT(close()));
		}
	}
#else
	structureChanged();
	// The document settings needs to be reinitialised.
	updateDialog("document", "");
	updateDialogs();
#endif
}


void GuiView::updateStatusBar()
{
	// let the user see the explicit message
	if (d.statusbar_timer_.isActive())
		return;

	theLyXFunc().setLyXView(this);
	statusBar()->showMessage(toqstr(theLyXFunc().viewStatusMessage()));
}


bool GuiView::hasFocus() const
{
	return qApp->activeWindow() == this;
}


bool GuiView::event(QEvent * e)
{
	switch (e->type())
	{
	// Useful debug code:
	//case QEvent::ActivationChange:
	//case QEvent::WindowDeactivate:
	//case QEvent::Paint:
	//case QEvent::Enter:
	//case QEvent::Leave:
	//case QEvent::HoverEnter:
	//case QEvent::HoverLeave:
	//case QEvent::HoverMove:
	//case QEvent::StatusTip:
	//case QEvent::DragEnter:
	//case QEvent::DragLeave:
	//case QEvent::Drop:
	//	break;

	case QEvent::WindowActivate: {
		if (this == guiApp->currentView()) {
			setFocus();
			return QMainWindow::event(e);
		}
		guiApp->setCurrentView(this);
		if (d.current_work_area_) {
			BufferView & bv = d.current_work_area_->bufferView();
			connectBufferView(bv);
			connectBuffer(bv.buffer());
			// The document structure, name and dialogs might have
			// changed in another view.
			structureChanged();
			// The document settings needs to be reinitialised.
			updateDialog("document", "");
			updateDialogs();
		} else {
			setWindowTitle(qt_("LyX"));
			setWindowIconText(qt_("LyX"));
		}
		setFocus();
		return QMainWindow::event(e);
	}

	case QEvent::ShortcutOverride: {

		if (isFullScreen() && menuBar()->isHidden()) {
			QKeyEvent * ke = static_cast<QKeyEvent*>(e);
			// FIXME: we should also try to detect special LyX shortcut such as
			// Alt-P and Alt-M. Right now there is a hack in
			// GuiWorkArea::processKeySym() that hides again the menubar for
			// those cases.
			if (ke->modifiers() & Qt::AltModifier && ke->key() != Qt::Key_Alt)
				menuBar()->show();
			return QMainWindow::event(e);
		}

		if (d.current_work_area_)
			// Nothing special to do.
			return QMainWindow::event(e);

		QKeyEvent * ke = static_cast<QKeyEvent*>(e);
		// Let Qt handle menu access and the Tab keys to navigate keys to navigate
		// between controls.
		if (ke->modifiers() & Qt::AltModifier || ke->key() == Qt::Key_Tab 
			|| ke->key() == Qt::Key_Backtab)
			return QMainWindow::event(e);

		// Allow processing of shortcuts that are allowed even when no Buffer
		// is viewed.
		theLyXFunc().setLyXView(this);
		KeySymbol sym;
		setKeySymbol(&sym, ke);
		theLyXFunc().processKeySym(sym, q_key_state(ke->modifiers()));
		e->accept();
		return true;
	}

	default:
		return QMainWindow::event(e);
	}
}


bool GuiView::focusNextPrevChild(bool /*next*/)
{
	setFocus();
	return true;
}


void GuiView::setBusy(bool busy)
{
	if (d.current_work_area_) {
		d.current_work_area_->setUpdatesEnabled(!busy);
		if (busy)
			d.current_work_area_->stopBlinkingCursor();
		else
			d.current_work_area_->startBlinkingCursor();
	}

	if (busy)
		QApplication::setOverrideCursor(Qt::WaitCursor);
	else
		QApplication::restoreOverrideCursor();
}


GuiToolbar * GuiView::makeToolbar(ToolbarInfo const & tbinfo, bool newline)
{
	GuiToolbar * toolBar = new GuiToolbar(tbinfo, *this);

	if (tbinfo.flags & ToolbarInfo::TOP) {
		if (newline)
			addToolBarBreak(Qt::TopToolBarArea);
		addToolBar(Qt::TopToolBarArea, toolBar);
	}

	if (tbinfo.flags & ToolbarInfo::BOTTOM) {
// Qt < 4.2.2 cannot handle ToolBarBreak on non-TOP dock.
#if (QT_VERSION >= 0x040202)
		if (newline)
			addToolBarBreak(Qt::BottomToolBarArea);
#endif
		addToolBar(Qt::BottomToolBarArea, toolBar);
	}

	if (tbinfo.flags & ToolbarInfo::LEFT) {
// Qt < 4.2.2 cannot handle ToolBarBreak on non-TOP dock.
#if (QT_VERSION >= 0x040202)
		if (newline)
			addToolBarBreak(Qt::LeftToolBarArea);
#endif
		addToolBar(Qt::LeftToolBarArea, toolBar);
	}

	if (tbinfo.flags & ToolbarInfo::RIGHT) {
// Qt < 4.2.2 cannot handle ToolBarBreak on non-TOP dock.
#if (QT_VERSION >= 0x040202)
		if (newline)
			addToolBarBreak(Qt::RightToolBarArea);
#endif
		addToolBar(Qt::RightToolBarArea, toolBar);
	}

	// The following does not work so I cannot restore to exact toolbar location
	/*
	ToolbarSection::ToolbarInfo & tbinfo = LyX::ref().session().toolbars().load(tbinfo.name);
	toolBar->move(tbinfo.posx, tbinfo.posy);
	*/

	return toolBar;
}


GuiWorkArea * GuiView::workArea(Buffer & buffer)
{
	if (TabWorkArea * twa = d.currentTabWorkArea())
		return twa->workArea(buffer);
	return 0;
}


GuiWorkArea * GuiView::addWorkArea(Buffer & buffer)
{
	// Automatically create a TabWorkArea if there are none yet.
	TabWorkArea * tab_widget = d.splitter_->count() 
		? d.currentTabWorkArea() : addTabWorkArea();
	return tab_widget->addWorkArea(buffer, *this);
}


TabWorkArea * GuiView::addTabWorkArea()
{
	TabWorkArea * twa = new TabWorkArea;
	QObject::connect(twa, SIGNAL(currentWorkAreaChanged(GuiWorkArea *)),
		this, SLOT(on_currentWorkAreaChanged(GuiWorkArea *)));
	QObject::connect(twa, SIGNAL(lastWorkAreaRemoved()),
			 this, SLOT(on_lastWorkAreaRemoved()));

	d.splitter_->addWidget(twa);
	d.stack_widget_->setCurrentWidget(d.splitter_);
	return twa;
}


GuiWorkArea const * GuiView::currentWorkArea() const
{
	return d.current_work_area_;
}


void GuiView::setCurrentWorkArea(GuiWorkArea * wa)
{
	LASSERT(wa, /**/);
	d.current_work_area_ = wa;
	for (int i = 0; i != d.splitter_->count(); ++i) {
		if (d.tabWorkArea(i)->setCurrentWorkArea(wa))
			return;
	}
}


void GuiView::removeWorkArea(GuiWorkArea * wa)
{
	LASSERT(wa, /**/);
	if (wa == d.current_work_area_) {
		disconnectBuffer();
		disconnectBufferView();
		d.current_work_area_ = 0;
	}

	for (int i = 0; i != d.splitter_->count(); ++i) {
		TabWorkArea * twa = d.tabWorkArea(i);
		if (!twa->removeWorkArea(wa))
			// Not found in this tab group.
			continue;

		// We found and removed the GuiWorkArea.
		if (!twa->count()) {
			// No more WorkAreas in this tab group, so delete it.
			delete twa;
			break;
		}

		if (d.current_work_area_)
			// This means that we are not closing the current GuiWorkArea;
			break;

		// Switch to the next GuiWorkArea in the found TabWorkArea.
		d.current_work_area_ = twa->currentWorkArea();
		break;
	}

	if (d.splitter_->count() == 0)
		// No more work area, switch to the background widget.
		d.setBackground();
}


void GuiView::setLayoutDialog(GuiLayoutBox * layout)
{
	d.layout_ = layout;
}


void GuiView::updateLayoutList()
{
	if (d.layout_)
		d.layout_->updateContents(false);
}


void GuiView::updateToolbars()
{
	if (d.current_work_area_) {
		bool const math =
			d.current_work_area_->bufferView().cursor().inMathed();
		bool const table =
			lyx::getStatus(FuncRequest(LFUN_LAYOUT_TABULAR)).enabled();
		bool const review =
			lyx::getStatus(FuncRequest(LFUN_CHANGES_TRACK)).enabled() &&
			lyx::getStatus(FuncRequest(LFUN_CHANGES_TRACK)).onoff(true);
		bool const mathmacrotemplate =
			lyx::getStatus(FuncRequest(LFUN_IN_MATHMACROTEMPLATE)).enabled();

		d.toolbars_->update(math, table, review, mathmacrotemplate);
	} else
		d.toolbars_->update(false, false, false, false);
}


Buffer * GuiView::buffer()
{
	if (d.current_work_area_)
		return &d.current_work_area_->bufferView().buffer();
	return 0;
}


Buffer const * GuiView::buffer() const
{
	if (d.current_work_area_)
		return &d.current_work_area_->bufferView().buffer();
	return 0;
}


void GuiView::setBuffer(Buffer * newBuffer)
{
	LASSERT(newBuffer, /**/);
	setBusy(true);

	GuiWorkArea * wa = workArea(*newBuffer);
	if (wa == 0) {
		updateLabels(*newBuffer->masterBuffer());
		wa = addWorkArea(*newBuffer);
	} else {
		//Disconnect the old buffer...there's no new one.
		disconnectBuffer();
	}
	connectBuffer(*newBuffer);
	connectBufferView(wa->bufferView());
	setCurrentWorkArea(wa);

	setBusy(false);
}


void GuiView::connectBuffer(Buffer & buf)
{
	buf.setGuiDelegate(this);
}


void GuiView::disconnectBuffer()
{
	if (d.current_work_area_)
		d.current_work_area_->bufferView().setGuiDelegate(0);
}


void GuiView::connectBufferView(BufferView & bv)
{
	bv.setGuiDelegate(this);
}


void GuiView::disconnectBufferView()
{
	if (d.current_work_area_)
		d.current_work_area_->bufferView().setGuiDelegate(0);
}


void GuiView::errors(string const & error_type)
{
	ErrorList & el = buffer()->errorList(error_type);
	if (!el.empty())
		showDialog("errorlist", error_type);
}


void GuiView::structureChanged()
{
	d.toc_models_.reset(view());
	// Navigator needs more than a simple update in this case. It needs to be
	// rebuilt.
	updateDialog("toc", "");
}


void GuiView::updateDialog(string const & name, string const & data)
{
	if (!isDialogVisible(name))
		return;

	map<string, DialogPtr>::const_iterator it = d.dialogs_.find(name);
	if (it == d.dialogs_.end())
		return;

	Dialog * const dialog = it->second.get();
	if (dialog->isVisibleView())
		dialog->initialiseParams(data);
}


BufferView * GuiView::view()
{
	return d.current_work_area_ ? &d.current_work_area_->bufferView() : 0;
}


void GuiView::autoSave()
{
	LYXERR(Debug::INFO, "Running autoSave()");

	if (buffer())
		view()->buffer().autoSave();
}


void GuiView::resetAutosaveTimers()
{
	if (lyxrc.autosave)
		d.autosave_timeout_.restart();
}


bool GuiView::getStatus(FuncRequest const & cmd, FuncStatus & flag)
{
	bool enable = true;
	Buffer * buf = buffer();

	/* In LyX/Mac, when a dialog is open, the menus of the
	   application can still be accessed without giving focus to
	   the main window. In this case, we want to disable the menu
	   entries that are buffer-related.

	   Note that this code is not perfect, as bug 1941 attests:
	   http://bugzilla.lyx.org/show_bug.cgi?id=1941#c4
	*/
	if (cmd.origin == FuncRequest::MENU && !hasFocus())
		buf = 0;

	switch(cmd.action) {
	case LFUN_BUFFER_WRITE:
		enable = buf && (buf->isUnnamed() || !buf->isClean());
		break;

	case LFUN_BUFFER_WRITE_AS:
		enable = buf;
		break;

	case LFUN_SPLIT_VIEW:
		enable = buf;
		break;

	case LFUN_CLOSE_TAB_GROUP:
		enable = d.currentTabWorkArea();
		break;

	case LFUN_TOOLBAR_TOGGLE:
		flag.setOnOff(d.toolbars_->visible(cmd.getArg(0)));
		break;

	case LFUN_UI_TOGGLE:
		flag.setOnOff(isFullScreen());
		break;

	case LFUN_DIALOG_TOGGLE:
		flag.setOnOff(isDialogVisible(cmd.getArg(0)));
		// fall through to set "enable"
	case LFUN_DIALOG_SHOW: {
		string const name = cmd.getArg(0);
		if (!buf)
			enable = name == "aboutlyx"
				|| name == "file" //FIXME: should be removed.
				|| name == "prefs"
				|| name == "texinfo";
		else if (name == "print")
			enable = buf->isExportable("dvi")
				&& lyxrc.print_command != "none";
		else if (name == "character") {
			if (!view())
				enable = false;
			else {
				InsetCode ic = view()->cursor().inset().lyxCode();
				enable = ic != ERT_CODE && ic != LISTINGS_CODE;
			}
		}
		else if (name == "symbols") {
			if (!view() || view()->cursor().inMathed())
				enable = false;
			else {
				InsetCode ic = view()->cursor().inset().lyxCode();
				enable = ic != ERT_CODE && ic != LISTINGS_CODE;
			}
		}
		else if (name == "latexlog")
			enable = FileName(buf->logName()).isReadableFile();
		else if (name == "spellchecker")
#if defined (USE_ASPELL) || defined (USE_ISPELL) || defined (USE_PSPELL)
			enable = !buf->isReadonly();
#else
			enable = false;
#endif
		else if (name == "vclog")
			enable = buf->lyxvc().inUse();
		break;
	}

	case LFUN_DIALOG_UPDATE: {
		string const name = cmd.getArg(0);
		if (!buf)
			enable = name == "prefs";
		break;
	}

	case LFUN_INSET_APPLY: {
		string const name = cmd.getArg(0);
		Inset * inset = getOpenInset(name);
		if (inset) {
			FuncRequest fr(LFUN_INSET_MODIFY, cmd.argument());
			FuncStatus fs;
			if (!inset->getStatus(view()->cursor(), fr, fs)) {
				// Every inset is supposed to handle this
				LASSERT(false, /**/);
			}
			flag |= fs;
		} else {
			FuncRequest fr(LFUN_INSET_INSERT, cmd.argument());
			flag |= lyx::getStatus(fr);
		}
		enable = flag.enabled();
		break;
	}

	case LFUN_COMPLETION_INLINE:
		if (!d.current_work_area_
		    || !d.current_work_area_->completer().inlinePossible(view()->cursor()))
		    enable = false;
		break;

	case LFUN_COMPLETION_POPUP:
		if (!d.current_work_area_
		    || !d.current_work_area_->completer().popupPossible(view()->cursor()))
		    enable = false;
		break;

	case LFUN_COMPLETION_COMPLETE:
		if (!d.current_work_area_
			|| !d.current_work_area_->completer().inlinePossible(view()->cursor()))
		    enable = false;
		break;

	default:
		return false;
	}

	if (!enable)
		flag.enabled(false);

	return true;
}


static FileName selectTemplateFile()
{
	FileDialog dlg(qt_("Select template file"));
	dlg.setButton1(qt_("Documents|#o#O"), toqstr(lyxrc.document_path));
	dlg.setButton1(qt_("Templates|#T#t"), toqstr(lyxrc.template_path));

	FileDialog::Result result = dlg.open(toqstr(lyxrc.template_path),
			     QStringList(qt_("LyX Documents (*.lyx)")));

	if (result.first == FileDialog::Later)
		return FileName();
	if (result.second.isEmpty())
		return FileName();
	return FileName(fromqstr(result.second));
}


Buffer * GuiView::loadDocument(FileName const & filename, bool tolastfiles)
{
	setBusy(true);

	Buffer * newBuffer = checkAndLoadLyXFile(filename);

	if (!newBuffer) {
		message(_("Document not loaded."));
		setBusy(false);
		return 0;
	}
	
	setBuffer(newBuffer);

	// scroll to the position when the file was last closed
	if (lyxrc.use_lastfilepos) {
		LastFilePosSection::FilePos filepos =
			LyX::ref().session().lastFilePos().load(filename);
		view()->moveToPosition(filepos.pit, filepos.pos, 0, 0);
	}

	if (tolastfiles)
		LyX::ref().session().lastFiles().add(filename);

	setBusy(false);
	return newBuffer;
}


void GuiView::openDocument(string const & fname)
{
	string initpath = lyxrc.document_path;

	if (buffer()) {
		string const trypath = buffer()->filePath();
		// If directory is writeable, use this as default.
		if (FileName(trypath).isDirWritable())
			initpath = trypath;
	}

	string filename;

	if (fname.empty()) {
		FileDialog dlg(qt_("Select document to open"), LFUN_FILE_OPEN);
		dlg.setButton1(qt_("Documents|#o#O"), toqstr(lyxrc.document_path));
		dlg.setButton2(qt_("Examples|#E#e"),
				toqstr(addPath(package().system_support().absFilename(), "examples")));

		FileDialog::Result result =
			dlg.open(toqstr(initpath), QStringList(qt_("LyX Documents (*.lyx)")));

		if (result.first == FileDialog::Later)
			return;

		filename = fromqstr(result.second);

		// check selected filename
		if (filename.empty()) {
			message(_("Canceled."));
			return;
		}
	} else
		filename = fname;

	// get absolute path of file and add ".lyx" to the filename if
	// necessary. 
	FileName const fullname = 
			fileSearch(string(), filename, "lyx", support::may_not_exist);
	if (!fullname.empty())
		filename = fullname.absFilename();

	// if the file doesn't exist, let the user create one
	if (!fullname.exists()) {
		// the user specifically chose this name. Believe him.
		Buffer * const b = newFile(filename, string(), true);
		if (b)
			setBuffer(b);
		return;
	}

	docstring const disp_fn = makeDisplayPath(filename);
	message(bformat(_("Opening document %1$s..."), disp_fn));

	docstring str2;
	Buffer * buf = loadDocument(fullname);
	if (buf) {
		updateLabels(*buf);
		
		setBuffer(buf);
		buf->errors("Parse");
		str2 = bformat(_("Document %1$s opened."), disp_fn);
	} else {
		str2 = bformat(_("Could not open document %1$s"), disp_fn);
	}
	message(str2);
}

// FIXME: clean that
static bool import(GuiView * lv, FileName const & filename,
	string const & format, ErrorList & errorList)
{
	FileName const lyxfile(support::changeExtension(filename.absFilename(), ".lyx"));

	string loader_format;
	vector<string> loaders = theConverters().loaders();
	if (find(loaders.begin(), loaders.end(), format) == loaders.end()) {
		for (vector<string>::const_iterator it = loaders.begin();
		     it != loaders.end(); ++it) {
			if (!theConverters().isReachable(format, *it))
				continue;

			string const tofile =
				support::changeExtension(filename.absFilename(),
				formats.extension(*it));
			if (!theConverters().convert(0, filename, FileName(tofile),
				filename, format, *it, errorList))
				return false;
			loader_format = *it;
			break;
		}
		if (loader_format.empty()) {
			frontend::Alert::error(_("Couldn't import file"),
				     bformat(_("No information for importing the format %1$s."),
					 formats.prettyName(format)));
			return false;
		}
	} else
		loader_format = format;

	if (loader_format == "lyx") {
		Buffer * buf = lv->loadDocument(lyxfile);
		if (!buf)
			return false;
		updateLabels(*buf);
		lv->setBuffer(buf);
		buf->errors("Parse");
	} else {
		Buffer * const b = newFile(lyxfile.absFilename(), string(), true);
		if (!b)
			return false;
		lv->setBuffer(b);
		bool as_paragraphs = loader_format == "textparagraph";
		string filename2 = (loader_format == format) ? filename.absFilename()
			: support::changeExtension(filename.absFilename(),
					  formats.extension(loader_format));
		lv->view()->insertPlaintextFile(FileName(filename2), as_paragraphs);
		theLyXFunc().setLyXView(lv);
		lyx::dispatch(FuncRequest(LFUN_MARK_OFF));
	}

	return true;
}


void GuiView::importDocument(string const & argument)
{
	string format;
	string filename = split(argument, format, ' ');

	LYXERR(Debug::INFO, format << " file: " << filename);

	// need user interaction
	if (filename.empty()) {
		string initpath = lyxrc.document_path;

		Buffer const * buf = buffer();
		if (buf) {
			string const trypath = buf->filePath();
			// If directory is writeable, use this as default.
			if (FileName(trypath).isDirWritable())
				initpath = trypath;
		}

		docstring const text = bformat(_("Select %1$s file to import"),
			formats.prettyName(format));

		FileDialog dlg(toqstr(text), LFUN_BUFFER_IMPORT);
		dlg.setButton1(qt_("Documents|#o#O"), toqstr(lyxrc.document_path));
		dlg.setButton2(qt_("Examples|#E#e"),
			toqstr(addPath(package().system_support().absFilename(), "examples")));

		docstring filter = formats.prettyName(format);
		filter += " (*.";
		// FIXME UNICODE
		filter += from_utf8(formats.extension(format));
		filter += ')';

		FileDialog::Result result =
			dlg.open(toqstr(initpath), fileFilters(toqstr(filter)));

		if (result.first == FileDialog::Later)
			return;

		filename = fromqstr(result.second);

		// check selected filename
		if (filename.empty())
			message(_("Canceled."));
	}

	if (filename.empty())
		return;

	// get absolute path of file
	FileName const fullname(support::makeAbsPath(filename));

	FileName const lyxfile(support::changeExtension(fullname.absFilename(), ".lyx"));

	// Check if the document already is open
	Buffer * buf = theBufferList().getBuffer(lyxfile.absFilename());
	if (buf) {
		setBuffer(buf);
		if (!closeBuffer()) {
			message(_("Canceled."));
			return;
		}
	}

	docstring const displaypath = makeDisplayPath(lyxfile.absFilename(), 30);

	// if the file exists already, and we didn't do
	// -i lyx thefile.lyx, warn
	if (lyxfile.exists() && fullname != lyxfile) {

		docstring text = bformat(_("The document %1$s already exists.\n\n"
			"Do you want to overwrite that document?"), displaypath);
		int const ret = Alert::prompt(_("Overwrite document?"),
			text, 0, 1, _("&Overwrite"), _("&Cancel"));

		if (ret == 1) {
			message(_("Canceled."));
			return;
		}
	}

	message(bformat(_("Importing %1$s..."), displaypath));
	ErrorList errorList;
	if (import(this, fullname, format, errorList))
		message(_("imported."));
	else
		message(_("file not imported!"));

	// FIXME (Abdel 12/08/06): Is there a need to display the error list here?
}


void GuiView::newDocument(string const & filename, bool from_template)
{
	FileName initpath(lyxrc.document_path);
	Buffer * buf = buffer();
	if (buf) {
		FileName const trypath(buf->filePath());
		// If directory is writeable, use this as default.
		if (trypath.isDirWritable())
			initpath = trypath;
	}

	string templatefile = from_template ?
		selectTemplateFile().absFilename() : string();
	Buffer * b;
	if (filename.empty())
		b = newUnnamedFile(templatefile, initpath);
	else
		b = newFile(filename, templatefile, true);

	if (b)
		setBuffer(b);
	// Ensure the cursor is correctly positionned on screen.
	view()->showCursor();
}


void GuiView::insertLyXFile(docstring const & fname)
{
	BufferView * bv = view();
	if (!bv)
		return;

	// FIXME UNICODE
	FileName filename(to_utf8(fname));
	
	if (!filename.empty()) {
		bv->insertLyXFile(filename);
		return;
	}

	// Launch a file browser
	// FIXME UNICODE
	string initpath = lyxrc.document_path;
	string const trypath = bv->buffer().filePath();
	// If directory is writeable, use this as default.
	if (FileName(trypath).isDirWritable())
		initpath = trypath;

	// FIXME UNICODE
	FileDialog dlg(qt_("Select LyX document to insert"), LFUN_FILE_INSERT);
	dlg.setButton1(qt_("Documents|#o#O"), toqstr(lyxrc.document_path));
	dlg.setButton2(qt_("Examples|#E#e"),
		toqstr(addPath(package().system_support().absFilename(),
		"examples")));

	FileDialog::Result result = dlg.open(toqstr(initpath),
			     QStringList(qt_("LyX Documents (*.lyx)")));

	if (result.first == FileDialog::Later)
		return;

	// FIXME UNICODE
	filename.set(fromqstr(result.second));

	// check selected filename
	if (filename.empty()) {
		// emit message signal.
		message(_("Canceled."));
		return;
	}

	bv->insertLyXFile(filename);
}


void GuiView::insertPlaintextFile(docstring const & fname,
	bool asParagraph)
{
	BufferView * bv = view();
	if (!bv)
		return;

	// FIXME UNICODE
	FileName filename(to_utf8(fname));
	
	if (!filename.empty()) {
		bv->insertPlaintextFile(filename, asParagraph);
		return;
	}

	FileDialog dlg(qt_("Select file to insert"), (asParagraph ?
		LFUN_FILE_INSERT_PLAINTEXT_PARA : LFUN_FILE_INSERT_PLAINTEXT));

	FileDialog::Result result = dlg.open(toqstr(bv->buffer().filePath()),
		QStringList());

	if (result.first == FileDialog::Later)
		return;

	// FIXME UNICODE
	filename.set(fromqstr(result.second));

	// check selected filename
	if (filename.empty()) {
		// emit message signal.
		message(_("Canceled."));
		return;
	}

	bv->insertPlaintextFile(filename, asParagraph);
}


bool GuiView::renameBuffer(Buffer & b, docstring const & newname)
{
	FileName fname = b.fileName();
	FileName const oldname = fname;

	if (!newname.empty()) {
		// FIXME UNICODE
		fname = support::makeAbsPath(to_utf8(newname), oldname.onlyPath().absFilename());
	} else {
		// Switch to this Buffer.
		setBuffer(&b);

		/// No argument? Ask user through dialog.
		// FIXME UNICODE
		FileDialog dlg(qt_("Choose a filename to save document as"),
				   LFUN_BUFFER_WRITE_AS);
		dlg.setButton1(qt_("Documents|#o#O"), toqstr(lyxrc.document_path));
		dlg.setButton2(qt_("Templates|#T#t"), toqstr(lyxrc.template_path));

		if (!isLyXFilename(fname.absFilename()))
			fname.changeExtension(".lyx");

		FileDialog::Result result =
			dlg.save(toqstr(fname.onlyPath().absFilename()),
			       QStringList(qt_("LyX Documents (*.lyx)")),
				     toqstr(fname.onlyFileName()));

		if (result.first == FileDialog::Later)
			return false;

		fname.set(fromqstr(result.second));

		if (fname.empty())
			return false;

		if (!isLyXFilename(fname.absFilename()))
			fname.changeExtension(".lyx");
	}

	if (FileName(fname).exists()) {
		docstring const file = makeDisplayPath(fname.absFilename(), 30);
		docstring text = bformat(_("The document %1$s already "
					   "exists.\n\nDo you want to "
					   "overwrite that document?"), 
					 file);
		int const ret = Alert::prompt(_("Overwrite document?"),
			text, 0, 2, _("&Overwrite"), _("&Rename"), _("&Cancel"));
		switch (ret) {
		case 0: break;
		case 1: return renameBuffer(b, docstring());
		case 2: return false;
		}
	}

	// Ok, change the name of the buffer
	b.setFileName(fname.absFilename());
	b.markDirty();
	bool unnamed = b.isUnnamed();
	b.setUnnamed(false);
	b.saveCheckSum(fname);

	if (!saveBuffer(b)) {
		b.setFileName(oldname.absFilename());
		b.setUnnamed(unnamed);
		b.saveCheckSum(oldname);
		return false;
	}

	return true;
}


bool GuiView::saveBuffer(Buffer & b)
{
	if (b.isUnnamed())
		return renameBuffer(b, docstring());

	if (b.save()) {
		LyX::ref().session().lastFiles().add(b.fileName());
		return true;
	}

	// Switch to this Buffer.
	setBuffer(&b);

	// FIXME: we don't tell the user *WHY* the save failed !!
	docstring const file = makeDisplayPath(b.absFileName(), 30);
	docstring text = bformat(_("The document %1$s could not be saved.\n\n"
				   "Do you want to rename the document and "
				   "try again?"), file);
	int const ret = Alert::prompt(_("Rename and save?"),
		text, 0, 2, _("&Rename"), _("&Retry"), _("&Cancel"));
	switch (ret) {
	case 0:
		if (!renameBuffer(b, docstring()))
			return false;
		break;
	case 1:
		break;
	case 2:
		return false;
	}

	return saveBuffer(b);
}


bool GuiView::closeBuffer()
{
	Buffer * buf = buffer();
	return buf && closeBuffer(*buf);
}


bool GuiView::closeBuffer(Buffer & buf, bool tolastopened)
{
	// goto bookmark to update bookmark pit.
	//FIXME: we should update only the bookmarks related to this buffer!
	for (size_t i = 0; i < LyX::ref().session().bookmarks().size(); ++i)
		theLyXFunc().gotoBookmark(i+1, false, false);

	if (buf.isClean() || buf.paragraphs().empty()) {
		if (buf.masterBuffer() == &buf && tolastopened)
			LyX::ref().session().lastOpened().add(buf.fileName());
		theBufferList().release(&buf);
		return true;
	}
	// Switch to this Buffer.
	setBuffer(&buf);

	docstring file;
	// FIXME: Unicode?
	if (buf.isUnnamed())
		file = from_utf8(buf.fileName().onlyFileName());
	else
		file = buf.fileName().displayName(30);

	// Bring this window to top before asking questions.
	raise();
	activateWindow();

	docstring const text = bformat(_("The document %1$s has unsaved changes."
		"\n\nDo you want to save the document or discard the changes?"), file);
	int const ret = Alert::prompt(_("Save changed document?"),
		text, 0, 2, _("&Save"), _("&Discard"), _("&Cancel"));

	switch (ret) {
	case 0:
		if (!saveBuffer(buf))
			return false;
		break;
	case 1:
		// if we crash after this we could
		// have no autosave file but I guess
		// this is really improbable (Jug)
		removeAutosaveFile(buf.absFileName());
		break;
	case 2:
		return false;
	}

	// save file names to .lyx/session
	// if master/slave are both open, do not save slave since it
	// will be automatically loaded when the master is loaded
	if (buf.masterBuffer() == &buf && tolastopened)
		LyX::ref().session().lastOpened().add(buf.fileName());

	theBufferList().release(&buf);
	return true;
}


bool GuiView::dispatch(FuncRequest const & cmd)
{
	BufferView * bv = view();
	// By default we won't need any update.
	if (bv)
		bv->cursor().updateFlags(Update::None);
	bool dispatched = true;

	switch(cmd.action) {
		case LFUN_BUFFER_IMPORT:
			importDocument(to_utf8(cmd.argument()));
			break;

		case LFUN_BUFFER_SWITCH:
			setBuffer(theBufferList().getBuffer(to_utf8(cmd.argument())));
			break;

		case LFUN_BUFFER_NEXT:
			setBuffer(theBufferList().next(buffer()));
			break;

		case LFUN_BUFFER_PREVIOUS:
			setBuffer(theBufferList().previous(buffer()));
			break;

		case LFUN_COMMAND_EXECUTE: {
			bool const show_it = cmd.argument() != "off";
			d.toolbars_->showCommandBuffer(show_it);
			break;
		}
		case LFUN_DROP_LAYOUTS_CHOICE:
			if (d.layout_)
				d.layout_->showPopup();
			break;

		case LFUN_MENU_OPEN:
			if (QMenu * menu = guiApp->menus().menu(toqstr(cmd.argument()), *this))
				menu->exec(QCursor::pos());
			break;

		case LFUN_FILE_INSERT:
			insertLyXFile(cmd.argument());
			break;
		case LFUN_FILE_INSERT_PLAINTEXT_PARA:
			insertPlaintextFile(cmd.argument(), true);
			break;

		case LFUN_FILE_INSERT_PLAINTEXT:
			insertPlaintextFile(cmd.argument(), false);
			break;

		case LFUN_BUFFER_WRITE:
			if (bv)
				saveBuffer(bv->buffer());
			break;

		case LFUN_BUFFER_WRITE_AS:
			if (bv)
				renameBuffer(bv->buffer(), cmd.argument());
			break;

		case LFUN_BUFFER_WRITE_ALL: {
			Buffer * first = theBufferList().first();
			if (!first)
				break;
			message(_("Saving all documents..."));
			// We cannot use a for loop as the buffer list cycles.
			Buffer * b = first;
			do {
				if (!b->isClean()) {
					saveBuffer(*b);
					LYXERR(Debug::ACTION, "Saved " << b->absFileName());
				}
				b = theBufferList().next(b);
			} while (b != first); 
			message(_("All documents saved."));
			break;
		}

		case LFUN_TOOLBAR_TOGGLE: {
			string const name = cmd.getArg(0);
			bool const allowauto = cmd.getArg(1) == "allowauto";
			// it is possible to get current toolbar status like this,...
			// but I decide to obey the order of ToolbarBackend::flags
			// and disregard real toolbar status.
			// toolbars_->saveToolbarInfo();
			//
			// toggle state on/off/auto
			d.toolbars_->toggleToolbarState(name, allowauto);
			// update toolbar
			updateToolbars();

			ToolbarInfo * tbi = d.toolbars_->getToolbarInfo(name);
			if (!tbi) {
				message(bformat(_("Unknown toolbar \"%1$s\""), from_utf8(name)));
				break;
			}
			docstring state;
			if (tbi->flags & ToolbarInfo::ON)
				state = _("on");
			else if (tbi->flags & ToolbarInfo::OFF)
				state = _("off");
			else if (tbi->flags & ToolbarInfo::AUTO)
				state = _("auto");

			message(bformat(_("Toolbar \"%1$s\" state set to %2$s"), 
			                   _(tbi->gui_name), state));
			break;
		}

		case LFUN_DIALOG_UPDATE: {
			string const name = to_utf8(cmd.argument());
			// Can only update a dialog connected to an existing inset
			Inset * inset = getOpenInset(name);
			if (inset) {
				FuncRequest fr(LFUN_INSET_DIALOG_UPDATE, cmd.argument());
				inset->dispatch(view()->cursor(), fr);
			} else if (name == "paragraph") {
				lyx::dispatch(FuncRequest(LFUN_PARAGRAPH_UPDATE));
			} else if (name == "prefs") {
				updateDialog(name, string());
			}
			break;
		}

		case LFUN_DIALOG_TOGGLE: {
			if (isDialogVisible(cmd.getArg(0)))
				dispatch(FuncRequest(LFUN_DIALOG_HIDE, cmd.argument()));
			else
				dispatch(FuncRequest(LFUN_DIALOG_SHOW, cmd.argument()));
			break;
		}

		case LFUN_DIALOG_DISCONNECT_INSET:
			disconnectDialog(to_utf8(cmd.argument()));
			break;

		case LFUN_DIALOG_HIDE: {
			guiApp->hideDialogs(to_utf8(cmd.argument()), 0);
			break;
		}

		case LFUN_DIALOG_SHOW: {
			string const name = cmd.getArg(0);
			string data = trim(to_utf8(cmd.argument()).substr(name.size()));

			if (name == "character") {
				data = freefont2string();
				if (!data.empty())
					showDialog("character", data);
			} else if (name == "latexlog") {
				Buffer::LogType type; 
				string const logfile = buffer()->logName(&type);
				switch (type) {
				case Buffer::latexlog:
					data = "latex ";
					break;
				case Buffer::buildlog:
					data = "literate ";
					break;
				}
				data += Lexer::quoteString(logfile);
				showDialog("log", data);
			} else if (name == "vclog") {
				string const data = "vc " +
					Lexer::quoteString(buffer()->lyxvc().getLogFile());
				showDialog("log", data);
			} else if (name == "symbols") {
				data = bv->cursor().getEncoding()->name();
				if (!data.empty())
					showDialog("symbols", data);
			} else
				showDialog(name, data);
			break;
		}

		case LFUN_INSET_APPLY: {
			view()->cursor().recordUndoFullDocument();
			string const name = cmd.getArg(0);
			Inset * inset = getOpenInset(name);
			if (inset) {
				FuncRequest fr(LFUN_INSET_MODIFY, cmd.argument());
				inset->dispatch(view()->cursor(), fr);
			} else {
				FuncRequest fr(LFUN_INSET_INSERT, cmd.argument());
				lyx::dispatch(fr);
			}
			break;
		}

		case LFUN_UI_TOGGLE:
			lfunUiToggle(cmd);
			// Make sure the keyboard focus stays in the work area.
			setFocus();
			break;

		case LFUN_COMPLETION_INLINE:
			if (d.current_work_area_)
				d.current_work_area_->completer().showInline();
			break;

		case LFUN_SPLIT_VIEW:
			if (Buffer * buf = buffer()) {
				string const orientation = cmd.getArg(0);
				d.splitter_->setOrientation(orientation == "vertical"
					? Qt::Vertical : Qt::Horizontal);
				TabWorkArea * twa = addTabWorkArea();
				GuiWorkArea * wa = twa->addWorkArea(*buf, *this);
				setCurrentWorkArea(wa);
			}
			break;

		case LFUN_CLOSE_TAB_GROUP:
			if (TabWorkArea * twa = d.currentTabWorkArea()) {
				delete twa;
				twa = d.currentTabWorkArea();
				// Switch to the next GuiWorkArea in the found TabWorkArea.
				d.current_work_area_ = twa? twa->currentWorkArea() : 0;
				if (d.splitter_->count() == 0)
					// No more work area, switch to the background widget.
					d.setBackground();
			}
			break;
			
		case LFUN_COMPLETION_POPUP:
			if (d.current_work_area_)
				d.current_work_area_->completer().showPopup();
			break;


		case LFUN_COMPLETION_COMPLETE:
			if (d.current_work_area_)
				d.current_work_area_->completer().tab();
			break;

		default:
			dispatched = false;
			break;
	}

	if (isFullScreen()) {
		if (menuBar()->isVisible())
			menuBar()->hide();
		if (statusBar()->isVisible())
			statusBar()->hide();
	}

	return dispatched;
}


void GuiView::lfunUiToggle(FuncRequest const & cmd)
{
	string const arg = cmd.getArg(0);
	if (arg == "scrollbar") {
		// hide() is of no help
		if (d.current_work_area_->verticalScrollBarPolicy() ==
			Qt::ScrollBarAlwaysOff)

			d.current_work_area_->setVerticalScrollBarPolicy(
				Qt::ScrollBarAsNeeded);
		else
			d.current_work_area_->setVerticalScrollBarPolicy(
				Qt::ScrollBarAlwaysOff);
		return;
	}
	if (arg == "statusbar") {
		statusBar()->setVisible(!statusBar()->isVisible());
		return;
	}
	if (arg == "menubar") {
		menuBar()->setVisible(!menuBar()->isVisible());
		return;
	}
#if QT_VERSION >= 0x040300
	if (arg == "frame") {
		int l, t, r, b;
		getContentsMargins(&l, &t, &r, &b);
		//are the frames in default state?
		d.current_work_area_->setFrameStyle(QFrame::NoFrame);
		if (l == 0) {
			setContentsMargins(-2, -2, -2, -2);
		} else {
			setContentsMargins(0, 0, 0, 0);
		}
		return;
	}
#endif
	if (arg != "fullscreen") {
		message(bformat("LFUN_UI_TOGGLE " + _("%1$s unknown command!"), from_utf8(arg)));
		return;
	}

	if (lyxrc.full_screen_toolbars)
		d.toolbars_->toggleFullScreen(!isFullScreen());

	if (isFullScreen()) {
		for (int i = 0; i != d.splitter_->count(); ++i)
			d.tabWorkArea(i)->setFullScreen(false);
#if QT_VERSION >= 0x040300
		setContentsMargins(0, 0, 0, 0);
#endif
		setWindowState(windowState() ^ Qt::WindowFullScreen);
		menuBar()->show();
		statusBar()->show();
	} else {
		for (int i = 0; i != d.splitter_->count(); ++i)
			d.tabWorkArea(i)->setFullScreen(true);
#if QT_VERSION >= 0x040300
		setContentsMargins(-2, -2, -2, -2);
#endif
		setWindowState(windowState() ^ Qt::WindowFullScreen);
		statusBar()->hide();
		menuBar()->hide();
	}
}


Buffer const * GuiView::updateInset(Inset const * inset)
{
	if (!d.current_work_area_)
		return 0;

	if (inset)
		d.current_work_area_->scheduleRedraw();

	return &d.current_work_area_->bufferView().buffer();
}


void GuiView::restartCursor()
{
	/* When we move around, or type, it's nice to be able to see
	 * the cursor immediately after the keypress.
	 */
	if (d.current_work_area_)
		d.current_work_area_->startBlinkingCursor();

	// Take this occasion to update the other GUI elements.
	updateDialogs();
}


void GuiView::updateCompletion(Cursor & cur, bool start, bool keep)
{
	if (d.current_work_area_)
		d.current_work_area_->completer().updateVisibility(cur, start, keep);
}

namespace {

// This list should be kept in sync with the list of insets in
// src/insets/Inset.cpp.  I.e., if a dialog goes with an inset, the
// dialog should have the same name as the inset.

char const * const dialognames[] = {
"aboutlyx", "bibitem", "bibtex", "box", "branch", "changes", "character",
"citation", "document", "errorlist", "ert", "external", "file",
"findreplace", "float", "graphics", "include", "index", "nomenclature", "label", "log",
"mathdelimiter", "mathmatrix", "note", "paragraph", "prefs", "print", 
"ref", "sendto", "space", "spellchecker", "symbols", "tabular", "tabularcreate",

#ifdef HAVE_LIBAIKSAURUS
"thesaurus",
#endif

"texinfo", "toc", "href", "view-source", "vspace", "wrap", "listings" };

char const * const * const end_dialognames =
	dialognames + (sizeof(dialognames) / sizeof(char *));

class cmpCStr {
public:
	cmpCStr(char const * name) : name_(name) {}
	bool operator()(char const * other) {
		return strcmp(other, name_) == 0;
	}
private:
	char const * name_;
};


bool isValidName(string const & name)
{
	return find_if(dialognames, end_dialognames,
			    cmpCStr(name.c_str())) != end_dialognames;
}

} // namespace anon


void GuiView::resetDialogs()
{
	// Make sure that no LFUN uses any LyXView.
	theLyXFunc().setLyXView(0);
	// FIXME: the "math panels" toolbar takes an awful lot of time to
	// initialise so we don't do that for the time being.
	//d.toolbars_->init();
	guiApp->menus().fillMenuBar(menuBar(), this);
	if (d.layout_)
		d.layout_->updateContents(true);
	// Now update controls with current buffer.
	theLyXFunc().setLyXView(this);
	restartCursor();
}


Dialog * GuiView::find_or_build(string const & name)
{
	if (!isValidName(name))
		return 0;

	map<string, DialogPtr>::iterator it = d.dialogs_.find(name);

	if (it != d.dialogs_.end())
		return it->second.get();

	Dialog * dialog = build(name);
	d.dialogs_[name].reset(dialog);
	if (lyxrc.allow_geometry_session)
		dialog->restoreSession();
	return dialog;
}


void GuiView::showDialog(string const & name, string const & data,
	Inset * inset)
{
	if (d.in_show_)
		return;

	d.in_show_ = true;
	Dialog * dialog = find_or_build(name);
	if (dialog) {
		dialog->showData(data);
		if (inset)
			d.open_insets_[name] = inset;
	}
	d.in_show_ = false;
}


bool GuiView::isDialogVisible(string const & name) const
{
	map<string, DialogPtr>::const_iterator it = d.dialogs_.find(name);
	if (it == d.dialogs_.end())
		return false;
	return it->second.get()->isVisibleView();
}


void GuiView::hideDialog(string const & name, Inset * inset)
{
	map<string, DialogPtr>::const_iterator it = d.dialogs_.find(name);
	if (it == d.dialogs_.end())
		return;

	if (inset && inset != getOpenInset(name))
		return;

	Dialog * const dialog = it->second.get();
	if (dialog->isVisibleView())
		dialog->hideView();
	d.open_insets_[name] = 0;
}


void GuiView::disconnectDialog(string const & name)
{
	if (!isValidName(name))
		return;

	if (d.open_insets_.find(name) != d.open_insets_.end())
		d.open_insets_[name] = 0;
}


Inset * GuiView::getOpenInset(string const & name) const
{
	if (!isValidName(name))
		return 0;

	map<string, Inset *>::const_iterator it = d.open_insets_.find(name);
	return it == d.open_insets_.end() ? 0 : it->second;
}


void GuiView::hideAll() const
{
	map<string, DialogPtr>::const_iterator it  = d.dialogs_.begin();
	map<string, DialogPtr>::const_iterator end = d.dialogs_.end();

	for(; it != end; ++it)
		it->second->hideView();
}


void GuiView::updateDialogs()
{
	map<string, DialogPtr>::const_iterator it  = d.dialogs_.begin();
	map<string, DialogPtr>::const_iterator end = d.dialogs_.end();

	for(; it != end; ++it) {
		Dialog * dialog = it->second.get();
		if (dialog && dialog->isVisibleView())
			dialog->checkStatus();
	}
	updateToolbars();
	updateLayoutList();
	updateStatusBar();
}


// will be replaced by a proper factory...
Dialog * createGuiAbout(GuiView & lv);
Dialog * createGuiBibitem(GuiView & lv);
Dialog * createGuiBibtex(GuiView & lv);
Dialog * createGuiBox(GuiView & lv);
Dialog * createGuiBranch(GuiView & lv);
Dialog * createGuiChanges(GuiView & lv);
Dialog * createGuiCharacter(GuiView & lv);
Dialog * createGuiCitation(GuiView & lv);
Dialog * createGuiDelimiter(GuiView & lv);
Dialog * createGuiDocument(GuiView & lv);
Dialog * createGuiErrorList(GuiView & lv);
Dialog * createGuiERT(GuiView & lv);
Dialog * createGuiExternal(GuiView & lv);
Dialog * createGuiFloat(GuiView & lv);
Dialog * createGuiGraphics(GuiView & lv);
Dialog * createGuiHSpace(GuiView & lv);
Dialog * createGuiInclude(GuiView & lv);
Dialog * createGuiLabel(GuiView & lv);
Dialog * createGuiListings(GuiView & lv);
Dialog * createGuiLog(GuiView & lv);
Dialog * createGuiMathMatrix(GuiView & lv);
Dialog * createGuiNomenclature(GuiView & lv);
Dialog * createGuiNote(GuiView & lv);
Dialog * createGuiParagraph(GuiView & lv);
Dialog * createGuiPreferences(GuiView & lv);
Dialog * createGuiPrint(GuiView & lv);
Dialog * createGuiRef(GuiView & lv);
Dialog * createGuiSearch(GuiView & lv);
Dialog * createGuiSendTo(GuiView & lv);
Dialog * createGuiShowFile(GuiView & lv);
Dialog * createGuiSpellchecker(GuiView & lv);
Dialog * createGuiSymbols(GuiView & lv);
Dialog * createGuiTabularCreate(GuiView & lv);
Dialog * createGuiTabular(GuiView & lv);
Dialog * createGuiTexInfo(GuiView & lv);
Dialog * createGuiToc(GuiView & lv);
Dialog * createGuiThesaurus(GuiView & lv);
Dialog * createGuiHyperlink(GuiView & lv);
Dialog * createGuiVSpace(GuiView & lv);
Dialog * createGuiViewSource(GuiView & lv);
Dialog * createGuiWrap(GuiView & lv);


Dialog * GuiView::build(string const & name)
{
	LASSERT(isValidName(name), /**/);

	if (name == "aboutlyx")
		return createGuiAbout(*this);
	if (name == "bibitem")
		return createGuiBibitem(*this);
	if (name == "bibtex")
		return createGuiBibtex(*this);
	if (name == "box")
		return createGuiBox(*this);
	if (name == "branch")
		return createGuiBranch(*this);
	if (name == "changes")
		return createGuiChanges(*this);
	if (name == "character")
		return createGuiCharacter(*this);
	if (name == "citation")
		return createGuiCitation(*this);
	if (name == "document")
		return createGuiDocument(*this);
	if (name == "errorlist")
		return createGuiErrorList(*this);
	if (name == "ert")
		return createGuiERT(*this);
	if (name == "external")
		return createGuiExternal(*this);
	if (name == "file")
		return createGuiShowFile(*this);
	if (name == "findreplace")
		return createGuiSearch(*this);
	if (name == "float")
		return createGuiFloat(*this);
	if (name == "graphics")
		return createGuiGraphics(*this);
	if (name == "include")
		return createGuiInclude(*this);
	if (name == "nomenclature")
		return createGuiNomenclature(*this);
	if (name == "label")
		return createGuiLabel(*this);
	if (name == "log")
		return createGuiLog(*this);
	if (name == "view-source")
		return createGuiViewSource(*this);
	if (name == "mathdelimiter")
		return createGuiDelimiter(*this);
	if (name == "mathmatrix")
		return createGuiMathMatrix(*this);
	if (name == "note")
		return createGuiNote(*this);
	if (name == "paragraph")
		return createGuiParagraph(*this);
	if (name == "prefs")
		return createGuiPreferences(*this);
	if (name == "print")
		return createGuiPrint(*this);
	if (name == "ref")
		return createGuiRef(*this);
	if (name == "sendto")
		return createGuiSendTo(*this);
	if (name == "space")
		return createGuiHSpace(*this);
	if (name == "spellchecker")
		return createGuiSpellchecker(*this);
	if (name == "symbols")
		return createGuiSymbols(*this);
	if (name == "tabular")
		return createGuiTabular(*this);
	if (name == "tabularcreate")
		return createGuiTabularCreate(*this);
	if (name == "texinfo")
		return createGuiTexInfo(*this);
#ifdef HAVE_LIBAIKSAURUS
	if (name == "thesaurus")
		return createGuiThesaurus(*this);
#endif
	if (name == "toc")
		return createGuiToc(*this);
	if (name == "href")
		return createGuiHyperlink(*this);
	if (name == "vspace")
		return createGuiVSpace(*this);
	if (name == "wrap")
		return createGuiWrap(*this);
	if (name == "listings")
		return createGuiListings(*this);

	return 0;
}


} // namespace frontend
} // namespace lyx

#include "GuiView_moc.cpp"
