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

#include "GuiImplementation.h"
#include "GuiWorkArea.h"
#include "GuiKeySymbol.h"
#include "GuiMenubar.h"
#include "GuiToolbar.h"
#include "GuiToolbars.h"
#include "qt_helpers.h"

#include "frontends/Application.h"
#include "frontends/Dialogs.h"
#include "frontends/Gui.h"
#include "frontends/WorkArea.h"

#include "support/filetools.h"
#include "support/convert.h"
#include "support/lstrings.h"
#include "support/os.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "BufferList.h"
#include "debug.h"
#include "FuncRequest.h"
#include "Layout.h"
#include "LyX.h"
#include "LyXFunc.h"
#include "LyXRC.h"
#include "MenuBackend.h"
#include "Paragraph.h"
#include "Session.h"
#include "version.h"

#include <boost/current_function.hpp>

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDesktopWidget>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QList>
#include <QMenu>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QStackedWidget>
#include <QStatusBar>
#include <QToolBar>
#include <QUrl>

using std::endl;
using std::string;
using std::vector;

namespace lyx {

using support::FileName;
using support::libFileSearch;
using support::makeDisplayPath;

extern bool quitting;

namespace frontend {

namespace {

int const statusbar_timer_value = 3000;

class BackgroundWidget : public QWidget
{
public:
	BackgroundWidget(QString const & file, QString const & text)
	{
		splash_ = new QPixmap(file);
		if (!splash_) {
			lyxerr << "could not load splash screen: '" << fromqstr(file) << "'" << endl;
			return;
		}

		QPainter pain(splash_);
		pain.setPen(QColor(255, 255, 0));
		QFont font;
		// The font used to display the version info
		font.setStyleHint(QFont::SansSerif);
		font.setWeight(QFont::Bold);
		font.setPointSize(convert<int>(lyxrc.font_sizes[Font::SIZE_LARGE]));
		pain.setFont(font);
		pain.drawText(260, 270, text);
	}

	void paintEvent(QPaintEvent *)
	{
		if (!splash_)
			return;

		int x = (width() - splash_->width()) / 2;
		int y = (height() - splash_->height()) / 2;
		QPainter pain(this);
		pain.drawPixmap(x, y, *splash_);
	}

private:
	QPixmap * splash_;
};

};


struct GuiViewBase::GuiViewPrivate
{
	string cur_title;

	int posx_offset;
	int posy_offset;

	TabWorkArea * tab_widget_;
	QStackedWidget * stack_widget_;
	BackgroundWidget * bg_widget_;
	/// view's menubar
	GuiMenubar * menubar_;
	/// view's toolbars
	GuiToolbars * toolbars_;
	///
	docstring current_layout;

	GuiViewPrivate() : posx_offset(0), posy_offset(0) {}

	unsigned int smallIconSize;
	unsigned int normalIconSize;
	unsigned int bigIconSize;
	// static needed by "New Window"
	static unsigned int lastIconSize;

	QMenu * toolBarPopup(GuiViewBase * parent)
	{
		// FIXME: translation
		QMenu * menu = new QMenu(parent);
		QActionGroup * iconSizeGroup = new QActionGroup(parent);

		QAction * smallIcons = new QAction(iconSizeGroup);
		smallIcons->setText(qt_("Small-sized icons"));
		smallIcons->setCheckable(true);
		QObject::connect(smallIcons, SIGNAL(triggered()), parent, SLOT(smallSizedIcons()));
		menu->addAction(smallIcons);

		QAction * normalIcons = new QAction(iconSizeGroup);
		normalIcons->setText(qt_("Normal-sized icons"));
		normalIcons->setCheckable(true);
		QObject::connect(normalIcons, SIGNAL(triggered()), parent, SLOT(normalSizedIcons()));
		menu->addAction(normalIcons);

		QAction * bigIcons = new QAction(iconSizeGroup);
		bigIcons->setText(qt_("Big-sized icons"));
		bigIcons->setCheckable(true);
		QObject::connect(bigIcons, SIGNAL(triggered()), parent, SLOT(bigSizedIcons()));
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

	void initBackground()
	{
		bg_widget_ = 0;
		LYXERR(Debug::GUI) << "show banner: " << lyxrc.show_banner << endl;
		/// The text to be written on top of the pixmap
		QString const text = lyx_version ? QString(lyx_version) : qt_("unknown version");
		FileName const file = support::libFileSearch("images", "banner", "png");
		if (file.empty())
			return;

		bg_widget_ = new BackgroundWidget(toqstr(file.absFilename()), text);
	}

	void setBackground()
	{
		if (!bg_widget_)
			return;

		stack_widget_->setCurrentWidget(bg_widget_);
		bg_widget_->setUpdatesEnabled(true);
	}
};


unsigned int GuiViewBase::GuiViewPrivate::lastIconSize = 0;


GuiViewBase::GuiViewBase(int id)
	: QMainWindow(), LyXView(id), quitting_by_menu_(false),
	  d(*new GuiViewPrivate)
{
	// Qt bug? signal lastWindowClosed does not work
	setAttribute(Qt::WA_QuitOnClose, false);
	setAttribute(Qt::WA_DeleteOnClose, true);

	// hardcode here the platform specific icon size
	d.smallIconSize = 14;	// scaling problems
	d.normalIconSize = 20;	// ok, default
	d.bigIconSize = 26;		// better for some math icons

#ifndef Q_WS_MACX
	// assign an icon to main form. We do not do it under Qt/Mac,
	// since the icon is provided in the application bundle.
	FileName const iconname = libFileSearch("images", "lyx", "png");
	if (!iconname.empty())
		setWindowIcon(QPixmap(toqstr(iconname.absFilename())));
#endif

	d.tab_widget_ = new TabWorkArea;
	QObject::connect(d.tab_widget_, SIGNAL(currentWorkAreaChanged(GuiWorkArea *)),
		this, SLOT(on_currentWorkAreaChanged(GuiWorkArea *)));

	d.initBackground();
	if (d.bg_widget_) {
		LYXERR(Debug::GUI) << "stack widget!" << endl;
		d.stack_widget_ = new QStackedWidget;
		d.stack_widget_->addWidget(d.bg_widget_);
		d.stack_widget_->addWidget(d.tab_widget_);
		setCentralWidget(d.stack_widget_);
	} else {
		d.stack_widget_ = 0;
		setCentralWidget(d.tab_widget_);
	}

	// For Drag&Drop.
	setAcceptDrops(true);
}


GuiViewBase::~GuiViewBase()
{
	delete d.menubar_;
	delete d.toolbars_;
	delete &d;
}


void GuiViewBase::close()
{
	quitting_by_menu_ = true;
	while (d.tab_widget_->count()) {
		GuiWorkArea * wa = dynamic_cast<GuiWorkArea *>(d.tab_widget_->widget(0));
		BOOST_ASSERT(wa);
		d.tab_widget_->removeTab(0);
		delete wa;
	}
	QMainWindow::close();
	quitting_by_menu_ = false;
}


void GuiViewBase::setFocus()
{
	if (d.tab_widget_->count())
		d.tab_widget_->currentWidget()->setFocus();
}


QMenu* GuiViewBase::createPopupMenu()
{
	return d.toolBarPopup(this);
}


void GuiViewBase::init()
{
	// GuiToolbars *must* be initialised before GuiMenubar.
	d.toolbars_ = new GuiToolbars(*this);
	// FIXME: GuiToolbars::init() cannot be integrated in the ctor
	// because LyXFunc::getStatus() needs a properly initialized
	// GuiToolbars object (for LFUN_TOOLBAR_TOGGLE).
	d.toolbars_->init();
	d.menubar_ = new GuiMenubar(this, menubackend);

	statusBar()->setSizeGripEnabled(true);

	QObject::connect(&statusbar_timer_, SIGNAL(timeout()),
		this, SLOT(update_view_state_qt()));

	if (d.stack_widget_)
		d.stack_widget_->setCurrentWidget(d.bg_widget_);
}


void GuiViewBase::closeEvent(QCloseEvent * close_event)
{
	// we may have been called through the close window button
	// which bypasses the LFUN machinery.
	if (!quitting_by_menu_ && theApp()->gui().viewIds().size() == 1) {
		if (!theBufferList().quitWriteAll()) {
			close_event->ignore();
			return;
		}
	}

	// Make sure that no LFUN use this close to be closed View.
	theLyXFunc().setLyXView(0);
	// Make sure the timer time out will not trigger a statusbar update.
	statusbar_timer_.stop();

	theApp()->gui().unregisterView(id());
	if (!theApp()->gui().viewIds().empty()) {
		// Just close the window and do nothing else if this is not the
		// last window.
		close_event->accept();
		return;
	}

	quitting = true;

	// this is the place where we leave the frontend.
	// it is the only point at which we start quitting.
	saveGeometry();
	close_event->accept();
	// quit the event loop
	qApp->quit();
}


void GuiViewBase::dragEnterEvent(QDragEnterEvent * event)
{
	if (event->mimeData()->hasUrls())
		event->accept();
	/// \todo Ask lyx-devel is this is enough:
	/// if (event->mimeData()->hasFormat("text/plain"))
	///	event->acceptProposedAction();
}


void GuiViewBase::dropEvent(QDropEvent* event)
{
	QList<QUrl> files = event->mimeData()->urls();
	if (files.isEmpty())
		return;

	LYXERR(Debug::GUI) << BOOST_CURRENT_FUNCTION
		<< " got URLs!" << endl;
	for (int i = 0; i != files.size(); ++i) {
		string const file = support::os::internal_path(fromqstr(
			files.at(i).toLocalFile()));
		if (!file.empty())
			dispatch(FuncRequest(LFUN_FILE_OPEN, file));
	}
}


void GuiViewBase::saveGeometry()
{
	static bool done = false;
	if (done)
		return;
	else
		done = true;

	// FIXME:
	// change the ifdef to 'geometry = normalGeometry();' only
	// when Trolltech has fixed the broken normalGeometry on X11:
	// http://www.trolltech.com/developer/task-tracker/index_html?id=119684+&method=entry
	// Then also the moveEvent, resizeEvent, and the
	// code for floatingGeometry_ can be removed;
	// adjust GuiViewBase::setGeometry()

	QRect normal_geometry;
	int maximized;
#ifdef Q_WS_WIN
	normal_geometry = normalGeometry();
	if (isMaximized()) {
		maximized = CompletelyMaximized;
	} else {
		maximized = NotMaximized;
	}
#else
	normal_geometry = updateFloatingGeometry();

	QDesktopWidget& dw = *qApp->desktop();
	QRect desk = dw.availableGeometry(dw.primaryScreen());
	// Qt bug on Linux: load completely maximized, vert max. save-> frameGeometry().height() is wrong
	if (isMaximized() && desk.width() <= frameGeometry().width() && desk.height() <= frameGeometry().height()) {
		maximized = CompletelyMaximized;
		// maximizing does not work when the window is allready hor. or vert. maximized
		// Tested only on KDE
		int dh = frameGeometry().height() - height();
		if (desk.height() <= normal_geometry.height() + dh)
			normal_geometry.setHeight(normal_geometry.height() - 1);
		int dw = frameGeometry().width() - width();
		if (desk.width() <= normal_geometry.width() + dw)
			normal_geometry.setWidth(normal_geometry.width() - 1);
	} else if (desk.height() <= frameGeometry().height()) {
		maximized = VerticallyMaximized;
	} else if (desk.width() <= frameGeometry().width()) {
		maximized = HorizontallyMaximized;
	} else {
		maximized = NotMaximized;
	}


#endif
	// save windows size and position
	Session & session = LyX::ref().session();
	session.sessionInfo().save("WindowWidth", convert<string>(normal_geometry.width()));
	session.sessionInfo().save("WindowHeight", convert<string>(normal_geometry.height()));
	session.sessionInfo().save("WindowMaximized", convert<string>(maximized));
	session.sessionInfo().save("IconSizeXY", convert<string>(iconSize().width()));
	if (lyxrc.geometry_xysaved) {
		session.sessionInfo().save("WindowPosX", convert<string>(normal_geometry.x() + d.posx_offset));
		session.sessionInfo().save("WindowPosY", convert<string>(normal_geometry.y() + d.posy_offset));
	}
	d.toolbars_->saveToolbarInfo();
}


void GuiViewBase::setGeometry(unsigned int width,
			  unsigned int height,
			  int posx, int posy,
			  int maximized,
			  unsigned int iconSizeXY,
			  const string & geometryArg)
{
	// use last value (not at startup)
	if (d.lastIconSize != 0)
		setIconSize(d.lastIconSize);
	else if (iconSizeXY != 0)
		setIconSize(iconSizeXY);
	else
		setIconSize(d.normalIconSize);

	// only true when the -geometry option was NOT used
	if (width != 0 && height != 0) {
		if (posx != -1 && posy != -1) {
			// if there are startup positioning problems:
			// http://doc.trolltech.com/4.2/qdesktopwidget.html
			QDesktopWidget& dw = *qApp->desktop();
			if (dw.isVirtualDesktop()) {
				if(!dw.geometry().contains(posx, posy)) {
					posx = 50;
					posy = 50;
				}
			} else {
				// Which system doesn't use a virtual desktop?
				// TODO save also last screen number and check if it is still availabe.
			}
#ifdef Q_WS_WIN
			// FIXME: use setGeometry only when Trolltech has fixed the qt4/X11 bug
			QWidget::setGeometry(posx, posy, width, height);
#else
			resize(width, height);
			move(posx, posy);
#endif
		} else {
			resize(width, height);
		}

		// remember original size
		floatingGeometry_ = QRect(posx, posy, width, height);

		if (maximized != NotMaximized) {
			if (maximized == CompletelyMaximized) {
				setWindowState(Qt::WindowMaximized);
			} else {
#ifndef Q_WS_WIN
				// TODO How to set by the window manager?
				//      setWindowState(Qt::WindowVerticallyMaximized);
				//      is not possible
				QDesktopWidget& dw = *qApp->desktop();
				QRect desk = dw.availableGeometry(dw.primaryScreen());
				if (maximized == VerticallyMaximized)
					resize(width, desk.height());
				if (maximized == HorizontallyMaximized)
					resize(desk.width(), height);
#endif
			}
		}
	}
	else
	{
		// FIXME: move this code into parse_geometry() (LyX.cpp)
#ifdef Q_WS_WIN
		int x, y;
		int w, h;
		QRegExp re( "[=]*(?:([0-9]+)[xX]([0-9]+)){0,1}[ ]*(?:([+-][0-9]*)([+-][0-9]*)){0,1}" );
		re.indexIn(toqstr(geometryArg.c_str()));
		w = re.cap(1).toInt();
		h = re.cap(2).toInt();
		x = re.cap(3).toInt();
		y = re.cap(4).toInt();
		QWidget::setGeometry( x, y, w, h );
#else
		// silence warning
		(void)geometryArg;
#endif
	}
	
	d.setBackground();
	
	show();

	// For an unknown reason, the Window title update is not effective for
	// the second windows up until it is shown on screen (Qt bug?).
	updateWindowTitle();

	// after show geometry() has changed (Qt bug?)
	// we compensate the drift when storing the position
	d.posx_offset = 0;
	d.posy_offset = 0;
	if (width != 0 && height != 0)
		if (posx != -1 && posy != -1) {
#ifdef Q_WS_WIN
			d.posx_offset = posx - normalGeometry().x();
			d.posy_offset = posy - normalGeometry().y();
#else
#ifndef Q_WS_MACX
			if (maximized == NotMaximized) {
				d.posx_offset = posx - geometry().x();
				d.posy_offset = posy - geometry().y();
			}
#endif
#endif
		}
}


void GuiViewBase::setWindowTitle(docstring const & t, docstring const & it)
{
	QString title = windowTitle();
	QString new_title = toqstr(t);
	if (title != new_title) {
		QMainWindow::setWindowTitle(new_title);
		QMainWindow::setWindowIconText(toqstr(it));
	}
	if (Buffer const * buf = buffer())
		d.tab_widget_->setTabText(d.tab_widget_->currentIndex(),
			toqstr(makeDisplayPath(buf->fileName(), 30)));
}


void GuiViewBase::message(docstring const & str)
{
	statusBar()->showMessage(toqstr(str));
	statusbar_timer_.stop();
	statusbar_timer_.start(statusbar_timer_value);
}


void GuiViewBase::clearMessage()
{
	update_view_state_qt();
}


void GuiViewBase::setIconSize(unsigned int size)
{
	d.lastIconSize = size;
	QMainWindow::setIconSize(QSize(size, size));
}


void GuiViewBase::smallSizedIcons()
{
	setIconSize(d.smallIconSize);
}


void GuiViewBase::normalSizedIcons()
{
	setIconSize(d.normalIconSize);
}


void GuiViewBase::bigSizedIcons()
{
	setIconSize(d.bigIconSize);
}


void GuiViewBase::update_view_state_qt()
{
	if (!hasFocus())
		return;
	theLyXFunc().setLyXView(this);
	statusBar()->showMessage(toqstr(theLyXFunc().viewStatusMessage()));
	statusbar_timer_.stop();
}


void GuiViewBase::on_currentWorkAreaChanged(GuiWorkArea * wa)
{
	disconnectBuffer();
	disconnectBufferView();
	connectBufferView(wa->bufferView());
	connectBuffer(wa->bufferView().buffer());

	updateToc();
	// Buffer-dependent dialogs should be updated or
	// hidden. This should go here because some dialogs (eg ToC)
	// require bv_->text.
	getDialogs().updateBufferDependent(true);
	updateToolbars();
	updateLayoutChoice();
	updateWindowTitle();
	updateStatusBar();
}


void GuiViewBase::updateStatusBar()
{
	// let the user see the explicit message
	if (statusbar_timer_.isActive())
		return;

	statusBar()->showMessage(toqstr(theLyXFunc().viewStatusMessage()));
}


void GuiViewBase::activated(FuncRequest const & func)
{
	dispatch(func);
}


bool GuiViewBase::hasFocus() const
{
	return qApp->activeWindow() == this;
}


QRect  GuiViewBase::updateFloatingGeometry()
{
	QDesktopWidget& dw = *qApp->desktop();
	QRect desk = dw.availableGeometry(dw.primaryScreen());
	// remember only non-maximized sizes
	if (!isMaximized() && desk.width() > frameGeometry().width() && desk.height() > frameGeometry().height()) {
		floatingGeometry_ = QRect(x(), y(), width(), height());
	}
	return floatingGeometry_;
}


void GuiViewBase::resizeEvent(QResizeEvent *)
{
	updateFloatingGeometry();
}


void GuiViewBase::moveEvent(QMoveEvent *)
{
	updateFloatingGeometry();
}


bool GuiViewBase::event(QEvent * e)
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

	case QEvent::WindowActivate:
		theApp()->setCurrentView(*this);
		if (d.tab_widget_->count()) {
			GuiWorkArea * wa = dynamic_cast<GuiWorkArea *>(
				d.tab_widget_->currentWidget());
			BOOST_ASSERT(wa);
			BufferView & bv = wa->bufferView();
			connectBufferView(bv);
			connectBuffer(bv.buffer());
			// The document structure, name and dialogs might have
			// changed in another view.
			updateToc();
			updateWindowTitle();
			getDialogs().updateBufferDependent(true);
		}
		return QMainWindow::event(e);

	case QEvent::ShortcutOverride: {
		QKeyEvent * ke = static_cast<QKeyEvent*>(e);
		if (d.tab_widget_->count() == 0) {
			theLyXFunc().setLyXView(this);
			KeySymbol sym;
			setKeySymbol(&sym, ke);
			theLyXFunc().processKeySym(sym, q_key_state(ke->modifiers()));
			e->accept();
			return true;
		}
		if (ke->key() == Qt::Key_Tab || ke->key() == Qt::Key_Backtab) {
			KeySymbol sym;
			setKeySymbol(&sym, ke);
			currentWorkArea()->processKeySym(sym, NoModifier);
			e->accept();
			return true;
		}
	}
	default:
		return QMainWindow::event(e);
	}
}


bool GuiViewBase::focusNextPrevChild(bool /*next*/)
{
	setFocus();
	return true;
}


void GuiViewBase::showView()
{
	QMainWindow::setWindowTitle(qt_("LyX"));
	QMainWindow::show();
	updateFloatingGeometry();
}


void GuiViewBase::busy(bool yes)
{
	if (d.tab_widget_->count()) {
		GuiWorkArea * wa = dynamic_cast<GuiWorkArea *>(d.tab_widget_->currentWidget());
		BOOST_ASSERT(wa);
		wa->setUpdatesEnabled(!yes);
		if (yes)
			wa->stopBlinkingCursor();
		else
			wa->startBlinkingCursor();
	}

	if (yes)
		QApplication::setOverrideCursor(Qt::WaitCursor);
	else
		QApplication::restoreOverrideCursor();
}


GuiToolbar * GuiViewBase::makeToolbar(ToolbarInfo const & tbinfo, bool newline)
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


WorkArea * GuiViewBase::workArea(Buffer & buffer)
{
	for (int i = 0; i != d.tab_widget_->count(); ++i) {
		GuiWorkArea * wa = dynamic_cast<GuiWorkArea *>(d.tab_widget_->widget(i));
		BOOST_ASSERT(wa);
		if (&wa->bufferView().buffer() == &buffer)
			return wa;
	}
	return 0;
}


WorkArea * GuiViewBase::addWorkArea(Buffer & buffer)
{
	GuiWorkArea * wa = new GuiWorkArea(buffer, *this);
	wa->setUpdatesEnabled(false);
	d.tab_widget_->addTab(wa, toqstr(makeDisplayPath(buffer.fileName(), 30)));
	wa->bufferView().updateMetrics(false);
	if (d.stack_widget_)
		d.stack_widget_->setCurrentWidget(d.tab_widget_);
	// Hide tabbar if there's only one tab.
	d.tab_widget_->showBar(d.tab_widget_->count() > 1);
	return wa;
}


WorkArea * GuiViewBase::currentWorkArea()
{
	if (d.tab_widget_->count() == 0)
		return 0;
	BOOST_ASSERT(dynamic_cast<GuiWorkArea *>(d.tab_widget_->currentWidget()));
	return dynamic_cast<GuiWorkArea *>(d.tab_widget_->currentWidget());
}


WorkArea const * GuiViewBase::currentWorkArea() const
{
	if (d.tab_widget_->count() == 0)
		return 0;
	BOOST_ASSERT(dynamic_cast<GuiWorkArea const *>(d.tab_widget_->currentWidget()));
	return dynamic_cast<GuiWorkArea const *>(d.tab_widget_->currentWidget());
}


void GuiViewBase::setCurrentWorkArea(WorkArea * work_area)
{
	BOOST_ASSERT(work_area);

	// Changing work area can result from opening a file so
	// update the toc in any case.
	updateToc();

	GuiWorkArea * wa = dynamic_cast<GuiWorkArea *>(work_area);
	BOOST_ASSERT(wa);
	if (wa != d.tab_widget_->currentWidget())
		// Switch to the work area.
		d.tab_widget_->setCurrentWidget(wa);
	else
		// Make sure the work area is up to date.
		d.tab_widget_->on_currentTabChanged(d.tab_widget_->currentIndex());
	wa->setFocus();
}


void GuiViewBase::removeWorkArea(WorkArea * work_area)
{
	BOOST_ASSERT(work_area);
	if (work_area == currentWorkArea()) {
		disconnectBuffer();
		disconnectBufferView();
	}

	// removing a work area often results from closing a file so
	// update the toc in any case.
	updateToc();

	GuiWorkArea * gwa = dynamic_cast<GuiWorkArea *>(work_area);
	gwa->setUpdatesEnabled(false);
	BOOST_ASSERT(gwa);
	int index = d.tab_widget_->indexOf(gwa);
	d.tab_widget_->removeTab(index);

	delete gwa;

	if (d.tab_widget_->count()) {
		// make sure the next work area is enabled.
		d.tab_widget_->currentWidget()->setUpdatesEnabled(true);
		// Hide tabbar if there's only one tab.
		d.tab_widget_->showBar(d.tab_widget_->count() > 1);
		return;
	}

	getDialogs().hideBufferDependent();
	if (d.stack_widget_) {
		// No more work area, switch to the background widget.
		d.setBackground();
	}
}


void GuiViewBase::showMiniBuffer(bool visible)
{
	d.toolbars_->showCommandBuffer(visible);
}


void GuiViewBase::openMenu(docstring const & name)
{
	d.menubar_->openByName(toqstr(name));
}


void GuiViewBase::openLayoutList()
{
	d.toolbars_->openLayoutList();
}


void GuiViewBase::updateLayoutChoice()
{
	// Don't show any layouts without a buffer
	if (!buffer()) {
		d.toolbars_->clearLayoutList();
		return;
	}

	// Update the layout display
	if (d.toolbars_->updateLayoutList(buffer()->params().getTextClassPtr())) {
		d.current_layout = buffer()->params().getTextClass().defaultLayoutName();
	}

	docstring const & layout = currentWorkArea()->bufferView().cursor().
		innerParagraph().layout()->name();

	if (layout != d.current_layout) {
		d.toolbars_->setLayout(layout);
		d.current_layout = layout;
	}
}


bool GuiViewBase::isToolbarVisible(std::string const & id)
{
	return d.toolbars_->visible(id);
}

void GuiViewBase::updateToolbars()
{
	WorkArea * wa = currentWorkArea();
	if (wa) {
		bool const math =
			wa->bufferView().cursor().inMathed();
		bool const table =
			lyx::getStatus(FuncRequest(LFUN_LAYOUT_TABULAR)).enabled();
		bool const review =
			lyx::getStatus(FuncRequest(LFUN_CHANGES_TRACK)).enabled() &&
			lyx::getStatus(FuncRequest(LFUN_CHANGES_TRACK)).onoff(true);

		d.toolbars_->update(math, table, review);
	} else
		d.toolbars_->update(false, false, false);

	// update read-only status of open dialogs.
	getDialogs().checkStatus();
}


ToolbarInfo * GuiViewBase::getToolbarInfo(string const & name)
{
	return d.toolbars_->getToolbarInfo(name);
}


void GuiViewBase::toggleToolbarState(string const & name, bool allowauto)
{
	// it is possible to get current toolbar status like this,...
	// but I decide to obey the order of ToolbarBackend::flags
	// and disregard real toolbar status.
	// toolbars_->saveToolbarInfo();
	//
	// toggle state on/off/auto
	d.toolbars_->toggleToolbarState(name, allowauto);
	// update toolbar
	updateToolbars();
}


} // namespace frontend
} // namespace lyx

#include "GuiView_moc.cpp"
