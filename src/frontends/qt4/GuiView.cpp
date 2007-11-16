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
#include "Dialogs.h"
#include "Gui.h"

#include "qt_helpers.h"

#include "frontends/Application.h"
#include "frontends/Dialogs.h"
#include "frontends/Gui.h"

#include "support/filetools.h"
#include "support/convert.h"
#include "support/lstrings.h"
#include "support/os.h"

#include "buffer_funcs.h"
#include "Buffer.h"
#include "BufferList.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Cursor.h"
#include "debug.h"
#include "ErrorList.h"
#include "FuncRequest.h"
#include "gettext.h"
#include "Intl.h"
#include "Layout.h"
#include "LyXFunc.h"
#include "LyX.h"
#include "LyXRC.h"
#include "MenuBackend.h"
#include "Paragraph.h"
#include "TextClass.h"
#include "Text.h"
#include "ToolbarBackend.h"
#include "version.h"

#include "support/lstrings.h"
#include "support/filetools.h" // OnlyFilename()
#include "support/Timeout.h"

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
#include <QPoint>
#include <QPushButton>
#include <QSettings>
#include <QShowEvent>
#include <QSplitter>
#include <QStackedWidget>
#include <QStatusBar>
#include <QToolBar>
#include <QUrl>

#include <boost/bind.hpp>
#include <boost/current_function.hpp>

#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

using std::endl;
using std::string;
using std::vector;

namespace lyx {

extern bool quitting;

namespace frontend {

using support::bformat;
using support::FileName;
using support::makeDisplayPath;
using support::onlyFilename;

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
		font.setPointSize(convert<int>(lyxrc.font_sizes[FONT_SIZE_LARGE]));
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

} // namespace anon


struct GuiView::GuiViewPrivate
{
	GuiViewPrivate()
		: current_work_area_(0), posx_offset(0), posy_offset(0)
	{}

	~GuiViewPrivate()
	{
		delete splitter_;
		delete bg_widget_;
		delete stack_widget_;
		delete menubar_;
		delete toolbars_;
	}

	unsigned int smallIconSize;
	unsigned int normalIconSize;
	unsigned int bigIconSize;
	// static needed by "New Window"
	static unsigned int lastIconSize;

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

	void initBackground()
	{
		LYXERR(Debug::GUI, "show banner: " << lyxrc.show_banner);
		/// The text to be written on top of the pixmap
		QString const text = lyx_version ? QString(lyx_version) : qt_("unknown version");
		bg_widget_ = new BackgroundWidget(":/images/banner.png", text);
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

		TabWorkArea * tab_widget = 0;
		for (int i = 0; i != splitter_->count(); ++i) {
			QWidget * w = splitter_->widget(i);
			if (!w->hasFocus())
				continue;
			tab_widget = dynamic_cast<TabWorkArea *>(w);
			if (tab_widget)
				break;
		}

		return tab_widget;
	}

public:
	///
	string cur_title;

	GuiWorkArea * current_work_area_;
	int posx_offset;
	int posy_offset;

	QSplitter * splitter_;
	QStackedWidget * stack_widget_;
	BackgroundWidget * bg_widget_;
	/// view's menubar
	GuiMenubar * menubar_;
	/// view's toolbars
	GuiToolbars * toolbars_;
	///
	docstring current_layout;
};


unsigned int GuiView::GuiViewPrivate::lastIconSize = 0;


GuiView::GuiView(int id)
	: QMainWindow(), LyXView(id),
	  d(*new GuiViewPrivate),
		quitting_by_menu_(false),
	 	autosave_timeout_(new Timeout(5000)),
	  dialogs_(new Dialogs(*this))
{
	// Start autosave timer
	if (lyxrc.autosave) {
		autosave_timeout_->timeout.connect(boost::bind(&GuiView::autoSave, this));
		autosave_timeout_->setTimeout(lyxrc.autosave * 1000);
		autosave_timeout_->start();
	}

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
	setWindowIcon(QPixmap(":/images/lyx.png"));
#endif


	d.splitter_ = new QSplitter;

	d.initBackground();
	LYXERR(Debug::GUI, "stack widget!");
	d.stack_widget_ = new QStackedWidget;
	d.stack_widget_->addWidget(d.bg_widget_);
	d.stack_widget_->addWidget(d.splitter_);
	setCentralWidget(d.stack_widget_);

	// For Drag&Drop.
	setAcceptDrops(true);
}


GuiView::~GuiView()
{
	delete dialogs_;
	delete autosave_timeout_;
	delete &d;
}


void GuiView::close()
{
	quitting_by_menu_ = true;
	d.current_work_area_ = 0;
	for (int i = 0; i != d.splitter_->count(); ++i) {
		TabWorkArea * twa = d.tabWorkArea(i);
		if (twa)
			twa->closeAll();
	}
	QMainWindow::close();
	quitting_by_menu_ = false;
}


void GuiView::setFocus()
{
	if (d.current_work_area_)
		d.current_work_area_->setFocus();
	else
		QWidget::setFocus();
}


QMenu* GuiView::createPopupMenu()
{
	return d.toolBarPopup(this);
}


void GuiView::init()
{
	setMinimumSize(300, 200);
	// GuiToolbars *must* be initialised before GuiMenubar.
	d.toolbars_ = new GuiToolbars(*this);
	// FIXME: GuiToolbars::init() cannot be integrated in the ctor
	// because LyXFunc::getStatus() needs a properly initialized
	// GuiToolbars object (for LFUN_TOOLBAR_TOGGLE).
	d.toolbars_->init();
	d.menubar_ = new GuiMenubar(this, menubackend);

	statusBar()->setSizeGripEnabled(true);

	QObject::connect(&statusbar_timer_, SIGNAL(timeout()),
		this, SLOT(clearMessage()));

	d.setBackground();

	if (!lyxrc.allow_geometry_session)
		setGeometry(50, 50, 690, 510);

	// Now take care of session management.
	QSettings settings;
	QString const key = "view-" + QString::number(id());
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

	if (lyxrc.allow_geometry_session) {
		QSettings settings;
		QString const key = "view-" + QString::number(id());
#ifdef Q_WS_X11
		settings.setValue(key + "/pos", pos());
		settings.setValue(key + "/size", size());
#else
		settings.setValue(key + "/geometry", saveGeometry());
#endif
		settings.setValue(key + "/icon_size", iconSize());
		d.toolbars_->saveToolbarInfo();
	}

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
	close_event->accept();
	// quit the event loop
	qApp->quit();
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

	LYXERR(Debug::GUI, BOOST_CURRENT_FUNCTION << " got URLs!");
	for (int i = 0; i != files.size(); ++i) {
		string const file = support::os::internal_path(fromqstr(
			files.at(i).toLocalFile()));
		if (!file.empty())
			dispatch(FuncRequest(LFUN_FILE_OPEN, file));
	}
}


void GuiView::message(docstring const & str)
{
	statusBar()->showMessage(toqstr(str));
	statusbar_timer_.stop();
	statusbar_timer_.start(statusbar_timer_value);
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
	statusbar_timer_.stop();
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

	updateToc();
	// Buffer-dependent dialogs should be updated or
	// hidden. This should go here because some dialogs (eg ToC)
	// require bv_->text.
	dialogs_->updateBufferDependent(true);
	updateToolbars();
	updateLayoutChoice(false);
	updateStatusBar();
}


void GuiView::updateStatusBar()
{
	// let the user see the explicit message
	if (statusbar_timer_.isActive())
		return;

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
		theApp()->setCurrentView(*this);
		if (d.current_work_area_) {
			BufferView & bv = d.current_work_area_->bufferView();
			connectBufferView(bv);
			connectBuffer(bv.buffer());
			// The document structure, name and dialogs might have
			// changed in another view.
			dialogs_->updateBufferDependent(true);
		} else {
			setWindowTitle(qt_("LyX"));
			setWindowIconText(qt_("LyX"));
		}
		return QMainWindow::event(e);
	}
	case QEvent::ShortcutOverride: {
		QKeyEvent * ke = static_cast<QKeyEvent*>(e);
		if (!d.current_work_area_) {
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
			d.current_work_area_->processKeySym(sym, NoModifier);
			e->accept();
			return true;
		}
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


void GuiView::setBusy(bool yes)
{
	if (d.current_work_area_) {
		d.current_work_area_->setUpdatesEnabled(!yes);
		if (yes)
			d.current_work_area_->stopBlinkingCursor();
		else
			d.current_work_area_->startBlinkingCursor();
	}

	if (yes)
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
	for (int i = 0; i != d.splitter_->count(); ++i) {
		GuiWorkArea * wa = d.tabWorkArea(i)->workArea(buffer);
		if (wa)
			return wa;
	}
	return 0;
}


GuiWorkArea * GuiView::addWorkArea(Buffer & buffer)
{
	GuiWorkArea * wa = new GuiWorkArea(buffer, *this);
	wa->setUpdatesEnabled(false);

	// Automatically create a TabWorkArea if there are none yet.
	if (!d.splitter_->count())
		addTabWorkArea();

	TabWorkArea * tab_widget = d.currentTabWorkArea();
	tab_widget->addTab(wa, wa->windowTitle());
	QObject::connect(wa, SIGNAL(titleChanged(GuiWorkArea *)),
		tab_widget, SLOT(updateTabText(GuiWorkArea *)));

	wa->bufferView().updateMetrics();

	// Hide tabbar if there's only one tab.
	tab_widget->showBar(tab_widget->count() > 1);
	return wa;
}


void GuiView::addTabWorkArea()
{
	TabWorkArea * twa = new TabWorkArea;
	QObject::connect(twa, SIGNAL(currentWorkAreaChanged(GuiWorkArea *)),
		this, SLOT(on_currentWorkAreaChanged(GuiWorkArea *)));
	d.splitter_->addWidget(twa);
	d.stack_widget_->setCurrentWidget(d.splitter_);
}


GuiWorkArea const * GuiView::currentWorkArea() const
{
	return d.current_work_area_;
}


void GuiView::setCurrentWorkArea(GuiWorkArea * work_area)
{
	BOOST_ASSERT(work_area);

	// Changing work area can result from opening a file so
	// update the toc in any case.
	updateToc();

	GuiWorkArea * wa = static_cast<GuiWorkArea *>(work_area);
	d.current_work_area_ = wa;
	for (int i = 0; i != d.splitter_->count(); ++i) {
		if (d.tabWorkArea(i)->setCurrentWorkArea(wa))
			return;
	}
}


void GuiView::removeWorkArea(GuiWorkArea * work_area)
{
	BOOST_ASSERT(work_area);
	GuiWorkArea * gwa = static_cast<GuiWorkArea *>(work_area);
	if (gwa == d.current_work_area_) {
		disconnectBuffer();
		disconnectBufferView();
		dialogs_->hideBufferDependent();
		d.current_work_area_ = 0;
	}

	// removing a work area often results from closing a file so
	// update the toc in any case.
	updateToc();

	for (int i = 0; i != d.splitter_->count(); ++i) {
		TabWorkArea * twa = d.tabWorkArea(i);
		if (!twa->removeWorkArea(gwa))
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


void GuiView::updateLayoutChoice(bool force)
{
	// Don't show any layouts without a buffer
	if (!buffer()) {
		d.toolbars_->clearLayoutList();
		return;
	}

	// Update the layout display
	if (d.toolbars_->updateLayoutList(buffer()->params().getTextClassPtr(), force)) {
		d.current_layout = buffer()->params().getTextClass().defaultLayoutName();
	}

	docstring const & layout = d.current_work_area_->bufferView().cursor().
		innerParagraph().layout()->name();

	if (layout != d.current_layout) {
		d.toolbars_->setLayout(layout);
		d.current_layout = layout;
	}
}


bool GuiView::isToolbarVisible(std::string const & id)
{
	return d.toolbars_->visible(id);
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

		d.toolbars_->update(math, table, review);
	} else
		d.toolbars_->update(false, false, false);

	// update read-only status of open dialogs.
	dialogs_->checkStatus();
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
	BOOST_ASSERT(newBuffer);
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


Buffer * GuiView::loadLyXFile(FileName const & filename, bool tolastfiles)
{
	setBusy(true);

	Buffer * newBuffer = checkAndLoadLyXFile(filename);

	if (!newBuffer) {
		message(_("Document not loaded."));
		updateStatusBar();
		setBusy(false);
		return 0;
	}

	GuiWorkArea * wa = workArea(*newBuffer);
	if (wa == 0)
		wa = addWorkArea(*newBuffer);

	// scroll to the position when the file was last closed
	if (lyxrc.use_lastfilepos) {
		LastFilePosSection::FilePos filepos =
			LyX::ref().session().lastFilePos().load(filename);
		// if successfully move to pit (returned par_id is not zero),
		// update metrics and reset font
		wa->bufferView().moveToPosition(filepos.pit, filepos.pos, 0, 0);
	}

	if (tolastfiles)
		LyX::ref().session().lastFiles().add(filename);

	setBusy(false);
	return newBuffer;
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
		dialogs_->show("errorlist", error_type);
}


void GuiView::showDialog(string const & name)
{
	dialogs_->show(name);
}


void GuiView::showDialogWithData(string const & name, string const & data)
{
	dialogs_->show(name, data);
}


void GuiView::showInsetDialog(string const & name, string const & data,
		Inset * inset)
{
	dialogs_->show(name, data, inset);
}


void GuiView::updateDialog(string const & name, string const & data)
{
	if (dialogs_->visible(name))
		dialogs_->update(name, data);
}


BufferView * GuiView::view()
{
	return d.current_work_area_ ? &d.current_work_area_->bufferView() : 0;
}


void GuiView::updateToc()
{
	updateDialog("toc", "");
}


void GuiView::updateEmbeddedFiles()
{
	updateDialog("embedding", "");
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
		autosave_timeout_->restart();
}


void GuiView::dispatch(FuncRequest const & cmd)
{
	switch(cmd.action) {
		case LFUN_BUFFER_SWITCH:
			setBuffer(theBufferList().getBuffer(to_utf8(cmd.argument())));
			break;

		case LFUN_COMMAND_EXECUTE: {
			bool const show_it = cmd.argument() != "off";
			d.toolbars_->showCommandBuffer(show_it);
			break;
		}
		case LFUN_DROP_LAYOUTS_CHOICE:
			d.toolbars_->openLayoutList();
			break;

		case LFUN_MENU_OPEN:
			d.menubar_->openByName(toqstr(cmd.argument()));
			break;

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

		default:
			theLyXFunc().setLyXView(this);
			lyx::dispatch(cmd);
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
}

} // namespace frontend
} // namespace lyx

#include "GuiView_moc.cpp"
