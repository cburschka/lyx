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
#include "QKeySymbol.h"
#include "QLMenubar.h"
#include "QLToolbar.h"
#include "QCommandBuffer.h"
#include "qt_helpers.h"

#include "frontends/Application.h"
#include "frontends/Gui.h"
#include "frontends/WorkArea.h"

#include "support/filetools.h"
#include "support/convert.h"
#include "support/lstrings.h"

#include "BufferView.h"
#include "BufferList.h"
#include "debug.h"
#include "FuncRequest.h"
#include "callback.h"
#include "LyXRC.h"
#include "LyX.h"
#include "Session.h"
#include "LyXFunc.h"
#include "MenuBackend.h"
#include "Buffer.h"
#include "BufferList.h"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QPixmap>
#include <QStatusBar>
#include <QToolBar>
#include <QTabBar>
#include <QDesktopWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>


#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

using std::endl;
using std::string;
using std::vector;

namespace lyx {

using support::FileName;
using support::libFileSearch;
using support::makeDisplayPath;

namespace frontend {

namespace {

int const statusbar_timer_value = 3000;

class TabWidget : public QWidget
{
	QHBoxLayout* hlayout;
public:
	QTabBar* tabbar;
	QPushButton* closeTabButton;

	void hideTabsIfNecessary()
	{
		if (tabbar->count() > 1) {
			tabbar->show();
			closeTabButton->show();
		} else {
			tabbar->hide();
			closeTabButton->hide();
		}
	}

	TabWidget(QWidget* w, bool topTabBar)
	{
		closeTabButton = new QPushButton(this);
		FileName const file = support::libFileSearch("images", "closetab", "xpm");
		if (!file.empty()) {
			QPixmap pm(toqstr(file.absFilename()));
			closeTabButton->setIcon(QIcon(pm));
			closeTabButton->setMaximumSize(pm.size());
			closeTabButton->setFlat(true);
		} else {
			closeTabButton->setText("Close");
		}
		closeTabButton->setCursor(Qt::ArrowCursor);
		closeTabButton->setToolTip(tr("Close tab"));
		closeTabButton->setEnabled(true);

		tabbar = new QTabBar;
#if QT_VERSION >= 0x040200
		tabbar->setUsesScrollButtons(true);
#endif
		hlayout = new QHBoxLayout;
		QVBoxLayout* vlayout = new QVBoxLayout;
		hlayout->addWidget(tabbar);
		hlayout->addWidget(closeTabButton);
		if (topTabBar) {
			vlayout->addLayout(hlayout);
			vlayout->addWidget(w);
		} else {
			tabbar->setShape(QTabBar::RoundedSouth);
			vlayout->addWidget(w);
			vlayout->addLayout(hlayout);
		}
		vlayout->setMargin(0);
		vlayout->setSpacing(0);
		hlayout->setMargin(0);
		setLayout(vlayout);
		hideTabsIfNecessary();
	}

	void clearTabbar()
	{
		for (int i = tabbar->count() - 1; i >= 0; --i)
			tabbar->removeTab(i);
	}
};

} // namespace anon


struct GuiView::GuiViewPrivate
{
	vector<string> tabnames;
	string cur_title;

	TabWidget* tabWidget;

	int posx_offset;
	int posy_offset;

	GuiViewPrivate() : tabWidget(0), posx_offset(0), posy_offset(0)
	{}

	unsigned int smallIconSize;
	unsigned int normalIconSize;
	unsigned int bigIconSize;
	// static needed by "New Window"
	static unsigned int lastIconSize;

	QMenu* toolBarPopup(GuiView *parent)
	{
		// FIXME: translation
		QMenu* menu = new QMenu(parent);
		QActionGroup *iconSizeGroup = new QActionGroup(parent);

		QAction *smallIcons = new QAction(iconSizeGroup);
		smallIcons->setText(qt_("Small-sized icons"));
		smallIcons->setCheckable(true);
		QObject::connect(smallIcons, SIGNAL(triggered()), parent, SLOT(smallSizedIcons()));
		menu->addAction(smallIcons);

		QAction *normalIcons = new QAction(iconSizeGroup);
		normalIcons->setText(qt_("Normal-sized icons"));
		normalIcons->setCheckable(true);
		QObject::connect(normalIcons, SIGNAL(triggered()), parent, SLOT(normalSizedIcons()));
		menu->addAction(normalIcons);

		QAction *bigIcons = new QAction(iconSizeGroup);
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
};


unsigned int GuiView::GuiViewPrivate::lastIconSize = 0;


GuiView::GuiView(int id)
	: QMainWindow(), LyXView(id), commandbuffer_(0), quitting_by_menu_(false),
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
	//  assign an icon to main form. We do not do it under Qt/Mac,
	//  since the icon is provided in the application bundle.
	FileName const iconname = libFileSearch("images", "lyx", "xpm");
	if (!iconname.empty())
		setWindowIcon(QPixmap(toqstr(iconname.absFilename())));
#endif
}


GuiView::~GuiView()
{
	menubar_.reset();
	delete &d;
}


void GuiView::close()
{
	quitting_by_menu_ = true;
	QMainWindow::close();
	quitting_by_menu_ = false;
}


void GuiView::setFocus()
{
	BOOST_ASSERT(work_area_);
	static_cast<GuiWorkArea *>(work_area_)->setFocus();
}


QMenu* GuiView::createPopupMenu()
{
	return d.toolBarPopup(this);
}


void GuiView::init()
{
	menubar_.reset(new QLMenubar(this, menubackend));
	QObject::connect(menuBar(), SIGNAL(triggered(QAction *)),
		this, SLOT(updateMenu(QAction *)));

	getToolbars().init();

	statusBar()->setSizeGripEnabled(true);

	QObject::connect(&statusbar_timer_, SIGNAL(timeout()),
		this, SLOT(update_view_state_qt()));

	BOOST_ASSERT(work_area_);
	if (!work_area_->bufferView().buffer() && !theBufferList().empty())
		setBuffer(theBufferList().first());

	// make sure the buttons are disabled if needed
	updateToolbars();
	updateLayoutChoice();
	updateMenubar();
}


void GuiView::closeEvent(QCloseEvent * close_event)
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

	if (view()->buffer()) {
		// save cursor position for opened files to .lyx/session
		// only bottom (whole doc) level pit and pos is saved.
		LyX::ref().session().lastFilePos().save(
			FileName(buffer()->fileName()),
			boost::tie(view()->cursor().bottom().pit(),
			view()->cursor().bottom().pos()));
	}

	// this is the place where we leave the frontend.
	// it is the only point at which we start quitting.
	saveGeometry();
	close_event->accept();
	// quit the event loop
	qApp->quit();
}


void GuiView::saveGeometry()
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
	// adjust GuiView::setGeometry()

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
	getToolbars().saveToolbarInfo();
}


void GuiView::setGeometry(unsigned int width,
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


void GuiView::updateMenu(QAction * /*action*/)
{
	menubar_->update();
}


void GuiView::setWindowTitle(docstring const & t, docstring const & it)
{
	QString title = windowTitle();
	QString new_title = toqstr(t);
	if (title != new_title) {
		QMainWindow::setWindowTitle(new_title);
		QMainWindow::setWindowIconText(toqstr(it));
	}
}


void GuiView::addCommandBuffer(QToolBar * toolbar)
{
	commandbuffer_ = new QCommandBuffer(this, *controlcommand_);
	focus_command_buffer.connect(boost::bind(&GuiView::focus_command_widget, this));
	toolbar->addWidget(commandbuffer_);
}


void GuiView::message(docstring const & str)
{
	statusBar()->showMessage(toqstr(str));
	statusbar_timer_.stop();
	statusbar_timer_.start(statusbar_timer_value);
}


void GuiView::clearMessage()
{
	update_view_state_qt();
}


void GuiView::setIconSize(unsigned int size)
{
	d.lastIconSize = size;
	QMainWindow::setIconSize(QSize(size, size));
}


void GuiView::smallSizedIcons()
{
	setIconSize(d.smallIconSize);
}


void GuiView::normalSizedIcons()
{
	setIconSize(d.normalIconSize);
}


void GuiView::bigSizedIcons()
{
	setIconSize(d.bigIconSize);
}


void GuiView::focus_command_widget()
{
	if (commandbuffer_)
		commandbuffer_->focus_command();
}


void GuiView::update_view_state_qt()
{
	if (!hasFocus())
		return;
	theLyXFunc().setLyXView(this);
	statusBar()->showMessage(toqstr(theLyXFunc().viewStatusMessage()));
	statusbar_timer_.stop();
}


void GuiView::initTab(QWidget* workarea)
{
	// construct the TabWidget with 'false' to have the tabbar at the bottom
	d.tabWidget = new TabWidget(workarea, true);
	setCentralWidget(d.tabWidget);
	QObject::connect(d.tabWidget->tabbar, SIGNAL(currentChanged(int)),
			this, SLOT(currentTabChanged(int)));
	QObject::connect(d.tabWidget->closeTabButton, SIGNAL(clicked()),
			this, SLOT(closeCurrentTab()));
}


void GuiView::updateTab()
{
	std::vector<string> const & names = theBufferList().getFileNames();

	string cur_title;
	if (view()->buffer()) {
		cur_title = view()->buffer()->fileName();
	}

	// avoid unnecessary tabbar rebuild:
	// check if something has changed
	if (d.tabnames == names && d.cur_title == cur_title)
		return;
	d.tabnames = names;
	d.cur_title = cur_title;

	QTabBar & tabbar = *d.tabWidget->tabbar;

	// update when all is done
	tabbar.blockSignals(true);

	// remove all tab bars
	d.tabWidget->clearTabbar();

	// rebuild tabbar and function map from scratch
	if (names.size() > 1) {
		for(size_t i = 0; i < names.size(); i++) {
			tabbar.addTab(toqstr(makeDisplayPath(names[i], 30)));
			// set current tab
			if (names[i] == cur_title)
				tabbar.setCurrentIndex(i);
		}
	}
	tabbar.blockSignals(false);
	d.tabWidget->hideTabsIfNecessary();
}


void GuiView::closeCurrentTab()
{
	dispatch(FuncRequest(LFUN_BUFFER_CLOSE));
}


void GuiView::currentTabChanged(int i)
{
	BOOST_ASSERT(i >= 0 && size_type(i) < d.tabnames.size());
	dispatch(FuncRequest(LFUN_BUFFER_SWITCH, d.tabnames[i]));
}


void GuiView::updateStatusBar()
{
	// let the user see the explicit message
	if (statusbar_timer_.isActive())
		return;

	statusBar()->showMessage(toqstr(theLyXFunc().viewStatusMessage()));
}


void GuiView::activated(FuncRequest const & func)
{
	dispatch(func);
}


bool GuiView::hasFocus() const
{
	return qApp->activeWindow() == this;
}


QRect  GuiView::updateFloatingGeometry()
{
	QDesktopWidget& dw = *qApp->desktop();
	QRect desk = dw.availableGeometry(dw.primaryScreen());
	// remember only non-maximized sizes
	if (!isMaximized() && desk.width() > frameGeometry().width() && desk.height() > frameGeometry().height()) {
		floatingGeometry_ = QRect(x(), y(), width(), height());
	}
	return floatingGeometry_;
}


void GuiView::resizeEvent(QResizeEvent *)
{
	updateFloatingGeometry();
}


void GuiView::moveEvent(QMoveEvent *)
{
	updateFloatingGeometry();
}


bool GuiView::event(QEvent * e)
{
	// Useful debug code:
	/*
	switch (e->type())
	{
	case QEvent::WindowActivate:
	case QEvent::ActivationChange:
	case QEvent::WindowDeactivate:
	case QEvent::Paint:
	case QEvent::Enter:
	case QEvent::Leave:
	case QEvent::HoverEnter:
	case QEvent::HoverLeave:
	case QEvent::HoverMove:
	case QEvent::StatusTip:
		break;
	default:
	*/

	if (e->type() == QEvent::ShortcutOverride) {
		QKeyEvent * ke = static_cast<QKeyEvent*>(e);
		if (ke->key() == Qt::Key_Tab || ke->key() == Qt::Key_Backtab) {
			boost::shared_ptr<QKeySymbol> sym(new QKeySymbol);
			sym->set(ke);
			BOOST_ASSERT(work_area_);
			work_area_->processKeySym(sym, key_modifier::none);
			e->accept();
			return true;
		}
	}
	//} for the debug switch above.

	return QMainWindow::event(e);
}


bool GuiView::focusNextPrevChild(bool /*next*/)
{
	setFocus();
	return true;
}


void GuiView::show()
{
	QMainWindow::setWindowTitle(qt_("LyX"));
	QMainWindow::show();
	updateFloatingGeometry();
}


void GuiView::busy(bool yes)
{
	BOOST_ASSERT(work_area_);
	static_cast<GuiWorkArea *>(work_area_)->setUpdatesEnabled(!yes);

	if (yes) {
		work_area_->stopBlinkingCursor();
		QApplication::setOverrideCursor(Qt::WaitCursor);
	}
	else {
		work_area_->startBlinkingCursor();
		QApplication::restoreOverrideCursor();
	}
}


Toolbars::ToolbarPtr GuiView::makeToolbar(ToolbarInfo const & tbinfo, bool newline)
{
	QLToolbar * Tb = new QLToolbar(tbinfo, *this);

	if (tbinfo.flags & ToolbarInfo::TOP) {
		if (newline)
			addToolBarBreak(Qt::TopToolBarArea);
		addToolBar(Qt::TopToolBarArea, Tb);
	}

	if (tbinfo.flags & ToolbarInfo::BOTTOM) {
// Qt < 4.2.2 cannot handle ToolBarBreak on non-TOP dock.
#if (QT_VERSION >= 0x040202)
		if (newline)
			addToolBarBreak(Qt::BottomToolBarArea);
#endif
		addToolBar(Qt::BottomToolBarArea, Tb);
	}

	if (tbinfo.flags & ToolbarInfo::LEFT) {
// Qt < 4.2.2 cannot handle ToolBarBreak on non-TOP dock.
#if (QT_VERSION >= 0x040202)
		if (newline)
			addToolBarBreak(Qt::LeftToolBarArea);
#endif
		addToolBar(Qt::LeftToolBarArea, Tb);
	}

	if (tbinfo.flags & ToolbarInfo::RIGHT) {
// Qt < 4.2.2 cannot handle ToolBarBreak on non-TOP dock.
#if (QT_VERSION >= 0x040202)
		if (newline)
			addToolBarBreak(Qt::RightToolBarArea);
#endif
		addToolBar(Qt::RightToolBarArea, Tb);
	}

	// The following does not work so I cannot restore to exact toolbar location
	/*
	ToolbarSection::ToolbarInfo & tbinfo = LyX::ref().session().toolbars().load(tbinfo.name);
	Tb->move(tbinfo.posx, tbinfo.posy);
	*/

	return Toolbars::ToolbarPtr(Tb);
}

} // namespace frontend
} // namespace lyx

#include "GuiView_moc.cpp"
