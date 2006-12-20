/**
 * \file GuiView.C
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
#include "bufferlist.h"
#include "debug.h"
#include "funcrequest.h"
#include "lyx_cb.h"
#include "lyxrc.h"
#include "lyx_main.h"
#include "session.h"
#include "lyxfunc.h"
#include "MenuBackend.h"
#include "buffer.h"
#include "bufferlist.h"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QPixmap>
#include <QStatusBar>
#include <QToolBar>
#include <QTabBar>
#include <QDesktopWidget>
#include <QVBoxLayout>

#include <boost/bind.hpp>

using std::endl;
using std::string;
using std::vector;

namespace lyx {

using support::FileName;
using support::onlyFilename;
using support::subst;
using support::libFileSearch;

namespace frontend {

namespace {

int const statusbar_timer_value = 3000;

} // namespace anon


class WidgetWithTabBar : public QWidget
{
public:
	QTabBar* tabbar;
	WidgetWithTabBar(QWidget* w)
	{
		tabbar = new QTabBar;
		QVBoxLayout* l = new QVBoxLayout;
		l->addWidget(tabbar);
		l->addWidget(w);
		l->setMargin(0);
		setLayout(l);
	}
};


struct GuiView::GuiViewPrivate
{
	typedef std::map<int, FuncRequest> FuncMap;
	typedef std::pair<int, FuncRequest> FuncMapPair;
	typedef std::map<string, QString> NameMap;
	typedef std::pair<string, QString> NameMapPair;

	FuncMap funcmap;
	NameMap namemap;
	WidgetWithTabBar* wt;

	int posx_offset;
	int posy_offset;

	GuiViewPrivate() : wt(0), posx_offset(0), posy_offset(0)
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
		smallIcons->setText("Small sized icons");
		smallIcons->setCheckable(true);
		QObject::connect(smallIcons, SIGNAL(triggered()), parent, SLOT(smallSizedIcons()));
		menu->addAction(smallIcons);

		QAction *normalIcons = new QAction(iconSizeGroup);
		normalIcons->setText("Normal sized icons");
		normalIcons->setCheckable(true);
		QObject::connect(normalIcons, SIGNAL(triggered()), parent, SLOT(normalSizedIcons()));
		menu->addAction(normalIcons);


		QAction *bigIcons = new QAction(iconSizeGroup);
		bigIcons->setText("Big sized icons");
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

	statusBar()->setSizeGripEnabled(false);

	QObject::connect(&statusbar_timer_, SIGNAL(timeout()),
		this, SLOT(update_view_state_qt()));

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
	if (!quitting_by_menu_) {
		if (!theBufferList().quitWriteAll()) {
			close_event->ignore();
			return;
		}
	}
	if (view()->buffer()) {
		// save cursor position for opened files to .lyx/session
		LyX::ref().session().lastFilePos().save(
			FileName(buffer()->fileName()),
			boost::tie(view()->cursor().pit(),
			view()->cursor().pos()));
	}
	theApp()->gui().unregisterView(id());	
	if (theApp()->gui().viewIds().empty())
	{
		// this is the place where we leave the frontend.
		// it is the only point at which we start quitting.
		saveGeometry();
		close_event->accept();
		// quit the event loop
		qApp->quit();
	}
	close_event->accept();
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
#ifdef Q_WS_WIN
	QRect geometry = normalGeometry();
#else
	updateFloatingGeometry();
	QRect geometry = floatingGeometry_;
#endif

	// save windows size and position
	Session & session = LyX::ref().session();
	session.sessionInfo().save("WindowWidth", convert<string>(geometry.width()));
	session.sessionInfo().save("WindowHeight", convert<string>(geometry.height()));
	session.sessionInfo().save("WindowIsMaximized", (isMaximized() ? "yes" : "no"));
	session.sessionInfo().save("IconSizeXY", convert<string>(iconSize().width()));
	if (lyxrc.geometry_xysaved) {
		session.sessionInfo().save("WindowPosX", convert<string>(geometry.x() + d.posx_offset));
		session.sessionInfo().save("WindowPosY", convert<string>(geometry.y() + d.posy_offset));
	}
	getToolbars().saveToolbarInfo();
}
						  

void GuiView::setGeometry(unsigned int width,
								  unsigned int height,
								  int posx, int posy,
								  bool maximize,
								  unsigned int iconSizeXY,
								  const std::string & geometryArg)
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
			// if there are ever startup positioning problems 
			// on a virtual desktop then check the 6 lines below
			// http://doc.trolltech.com/4.2/qdesktopwidget.html 
			QDesktopWidget& dw = *qApp->desktop();
			QRect desk = dw.availableGeometry(dw.primaryScreen());
			(posx >= desk.width() ? posx = 50 : true);
			(posy >= desk.height()? posy = 50 : true);
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

		if (maximize)
			setWindowState(Qt::WindowMaximized);
	}
	else
	{
		// FIXME: move this code into parse_geometry() (lyx_main.C)
#ifdef Q_WS_WIN
		int x, y;
		int w, h;
		QRegExp re( "[=]*(?:([0-9]+)[xX]([0-9]+)){0,1}[ ]*(?:([+-][0-9]*)([+-][0-9]*)){0,1}" );
		re.indexIn( toqstr(geometryArg.c_str()));
		w = re.cap( 1 ).toInt();
		h = re.cap( 2 ).toInt();
		x = re.cap( 3 ).toInt();
		y = re.cap( 4 ).toInt();
		QWidget::setGeometry( x, y, w, h );
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
			if (!maximize) {
				d.posx_offset = posx - geometry().x();
				d.posy_offset = posy - geometry().y();
			}
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
	statusBar()->showMessage(toqstr(theLyXFunc().viewStatusMessage()));
	statusbar_timer_.stop();
}


void GuiView::initTab(QWidget* workarea)
{
	d.wt = new WidgetWithTabBar(workarea);
	setCentralWidget(d.wt);
	QObject::connect(d.wt->tabbar, SIGNAL(currentChanged(int)),
			this, SLOT(currentTabChanged(int)));
}


void GuiView::updateTab()
{
	QTabBar& tb = *d.wt->tabbar;

	// update when all  is done
	tb.blockSignals(true);

	typedef std::vector<string> Strings;
	Strings const names = theBufferList().getFileNames();
	size_t n_size = names.size();

	Strings addtab;
	// show tabs only when there is more 
	// than one file opened
	if (n_size > 1)
	{
		for (size_t i = 0; i != n_size; i++) 
			if (d.namemap.find(names[i]) == d.namemap.end())
				addtab.push_back(names.at(i));
	}

	for(size_t i = 0; i<addtab.size(); i++)
	{
		QString tab_name = lyx::toqstr(onlyFilename(addtab.at(i))); 
		d.namemap.insert(GuiViewPrivate::NameMapPair(addtab.at(i), tab_name));
		tb.addTab(tab_name);
	}

	// check if all names showed by the tabs
	// are also in the current bufferlist
	Strings removetab;
	bool notall = true;
	if (n_size < 2)
		notall = false;
	std::map<string, QString>::iterator tabit = d.namemap.begin();
	for (;tabit != d.namemap.end(); ++tabit)
	{
		bool found = false;
		for (size_t i = 0; i != n_size; i++) 
			if (tabit->first == names.at(i) && notall)
				found = true;
		if (!found)
			removetab.push_back(tabit->first);
	}
	

	// remove tabs
	for(size_t i = 0; i<removetab.size(); i++)
	{
		if (d.namemap.find(removetab.at(i)) != d.namemap.end())
		{
			tabit = d.namemap.find(removetab.at(i));
			for (int i = 0; i < tb.count(); i++)
				if (tb.tabText(i) == tabit->second)
				{
					tb.removeTab(i);
					break;
				}
			d.namemap.erase(tabit);
		}
	}

	// rebuild func map
	if (removetab.size() > 0 || addtab.size() > 0)
	{
		d.funcmap.clear();
		tabit = d.namemap.begin();
		for (;tabit != d.namemap.end(); ++tabit)
		{
			QTabBar& tb = *d.wt->tabbar;
			for (int i = 0; i < tb.count(); i++)
			{
				if (tb.tabText(i) == tabit->second)
				{
					FuncRequest func(LFUN_BUFFER_SWITCH, tabit->first);
					d.funcmap.insert(GuiViewPrivate::FuncMapPair(i, func));
					break;
				}
			}
		}
	}

	// set current tab
	if (view()->buffer()) 
	{
		string cur_title = view()->buffer()->fileName();
		if (d.namemap.find(cur_title) != d.namemap.end())
		{
			QString tabname = d.namemap.find(cur_title)->second;
			for (int i = 0; i < tb.count(); i++)
				if (tb.tabText(i) == tabname)
				{
					tb.setCurrentIndex(i);
					break;
				}
		}
	}

	tb.blockSignals(false);
	d.wt->update();
}


void GuiView::currentTabChanged (int index)
{
	std::map<int, FuncRequest>::const_iterator it = d.funcmap.find(index);
	if (it != d.funcmap.end())
		activated(it->second);
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


void  GuiView::updateFloatingGeometry()
{
	if (!isMaximized())
		floatingGeometry_ = QRect(x(), y(), width(), height());
}


void GuiView::resizeEvent(QResizeEvent *)
{
	updateFloatingGeometry();
}


void GuiView::moveEvent(QMoveEvent *)
{
	updateFloatingGeometry();
}


void GuiView::show()
{
	QMainWindow::setWindowTitle(qt_("LyX"));
	QMainWindow::show();
	updateFloatingGeometry();
}


void GuiView::busy(bool yes)
{
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


Toolbars::ToolbarPtr GuiView::makeToolbar(ToolbarBackend::Toolbar const & tbb)
{
	QLToolbar * Tb = new QLToolbar(tbb, *this);
	//static QLToolbar * lastTb = 0;

	if (tbb.flags & ToolbarBackend::TOP) {
			addToolBar(Qt::TopToolBarArea, Tb);
			addToolBarBreak(Qt::TopToolBarArea);
	}
	if (tbb.flags & ToolbarBackend::BOTTOM) {
		addToolBar(Qt::BottomToolBarArea, Tb);
		/*
		// Qt bug:
		// http://www.trolltech.com/developer/task-tracker/index_html?id=137015&method=entry
		// Doesn't work because the toolbar will evtl. be hidden.
		if (lastTb)
			insertToolBarBreak(lastTb);
		lastTb = Tb;
		*/
	}
	if (tbb.flags & ToolbarBackend::LEFT) {
		addToolBar(Qt::LeftToolBarArea, Tb);
	}
	if (tbb.flags & ToolbarBackend::RIGHT) {
		addToolBar(Qt::RightToolBarArea, Tb);
	}

	return Toolbars::ToolbarPtr(Tb);
}

} // namespace frontend
} // namespace lyx

#include "GuiView_moc.cpp"
