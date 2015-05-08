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
#include "DispatchResult.h"
#include "FileDialog.h"
#include "FontLoader.h"
#include "GuiApplication.h"
#include "GuiCommandBuffer.h"
#include "GuiCompleter.h"
#include "GuiKeySymbol.h"
#include "GuiToc.h"
#include "GuiToolbar.h"
#include "GuiWorkArea.h"
#include "GuiProgress.h"
#include "LayoutBox.h"
#include "Menus.h"
#include "TocModel.h"

#include "qt_helpers.h"

#include "frontends/alert.h"
#include "frontends/KeySymbol.h"

#include "buffer_funcs.h"
#include "Buffer.h"
#include "BufferList.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Compare.h"
#include "Converter.h"
#include "Cursor.h"
#include "CutAndPaste.h"
#include "Encoding.h"
#include "ErrorList.h"
#include "Format.h"
#include "FuncStatus.h"
#include "FuncRequest.h"
#include "Intl.h"
#include "Layout.h"
#include "Lexer.h"
#include "LyXAction.h"
#include "LyX.h"
#include "LyXRC.h"
#include "LyXVC.h"
#include "Paragraph.h"
#include "SpellChecker.h"
#include "Session.h"
#include "TexRow.h"
#include "TextClass.h"
#include "Text.h"
#include "Toolbars.h"
#include "version.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/ExceptionMessage.h"
#include "support/FileName.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/filetools.h"
#include "support/ForkedCalls.h"
#include "support/lassert.h"
#include "support/lstrings.h"
#include "support/os.h"
#include "support/Package.h"
#include "support/PathChanger.h"
#include "support/Systemcall.h"
#include "support/Timeout.h"
#include "support/ProgressInterface.h"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDebug>
#include <QDesktopWidget>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFuture>
#include <QFutureWatcher>
#include <QLabel>
#include <QList>
#include <QMenu>
#include <QMenuBar>
#include <QMimeData>
#include <QMovie>
#include <QPainter>
#include <QPixmap>
#include <QPixmapCache>
#include <QPoint>
#include <QPushButton>
#include <QScrollBar>
#include <QSettings>
#include <QShowEvent>
#include <QSplitter>
#include <QStackedWidget>
#include <QStatusBar>
#include <QtConcurrentRun>
#include <QTime>
#include <QTimer>
#include <QToolBar>
#include <QUrl>



// sync with GuiAlert.cpp
#define EXPORT_in_THREAD 1


#include "support/bind.h"

#include <sstream>

#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif


using namespace std;
using namespace lyx::support;

namespace lyx {

using support::addExtension;
using support::changeExtension;
using support::removeExtension;

namespace frontend {

namespace {

class BackgroundWidget : public QWidget
{
public:
	BackgroundWidget()
	{
		LYXERR(Debug::GUI, "show banner: " << lyxrc.show_banner);
		if (!lyxrc.show_banner)
			return;
		/// The text to be written on top of the pixmap
		QString const text = lyx_version ?
			qt_("version ") + lyx_version : qt_("unknown version");
		splash_ = getPixmap("images/", "banner", "svgz,png");

		QPainter pain(&splash_);
		pain.setPen(QColor(0, 0, 0));
		double const multiplier = splashPixelRatio() / pixelRatio();
		int const size = static_cast<int>(toqstr(lyxrc.font_sizes[FONT_SIZE_LARGE]).toDouble() * multiplier);
		int const x = static_cast<int>(190 * multiplier);
		int const y = static_cast<int>(225 * multiplier);
		LYXERR(Debug::GUI,
			"widget pixel ratio: " << pixelRatio() <<
			" splash pixel ratio: " << splashPixelRatio() <<
			" version text size,position: " << size << "@" << x << "+" << y);
		QFont font;
		// The font used to display the version info
		font.setStyleHint(QFont::SansSerif);
		font.setWeight(QFont::Bold);
		font.setPointSize(size);
		pain.setFont(font);
		pain.drawText(x, y, text);
		setFocusPolicy(Qt::StrongFocus);
	}

	void paintEvent(QPaintEvent *)
	{
		int const w = static_cast<int>(splash_.width() / splashPixelRatio());
		int const h = static_cast<int>(splash_.height() / splashPixelRatio());
		int const x = (width() - w) / 2;
		int const y = (height() - h) / 2;
		LYXERR(Debug::GUI,
			"widget pixel ratio: " << pixelRatio() <<
			" splash pixel ratio: " << splashPixelRatio() <<
			" paint pixmap: " << w << "x" << h << "@" << x << "+" << y);
		QPainter pain(this);
		pain.drawPixmap(x, y, w, h, splash_);
	}

	void keyPressEvent(QKeyEvent * ev)
	{
		KeySymbol sym;
		setKeySymbol(&sym, ev);
		if (sym.isOK()) {
			guiApp->processKeySym(sym, q_key_state(ev->modifiers()));
			ev->accept();
		} else {
			ev->ignore();
		}
	}

private:
	QPixmap splash_;

	/// Current ratio between physical pixels and device-independent pixels
	double pixelRatio() const {
#if QT_VERSION >= 0x050000
		return devicePixelRatio();
#else
		return 1.0;
#endif
	}

	/// Ratio between physical pixels and device-independent pixels of splash image
	double splashPixelRatio() const {
#if QT_VERSION >= 0x050000
		return splash_.devicePixelRatio();
#else
		return 1.0;
#endif
	}
};


/// Toolbar store providing access to individual toolbars by name.
typedef map<string, GuiToolbar *> ToolbarMap;

typedef shared_ptr<Dialog> DialogPtr;

} // namespace anon


struct GuiView::GuiViewPrivate
{
	GuiViewPrivate(GuiView * gv)
		: gv_(gv), current_work_area_(0), current_main_work_area_(0),
		layout_(0), autosave_timeout_(5000),
		in_show_(false)
	{
		// hardcode here the platform specific icon size
		smallIconSize = 16;  // scaling problems
		normalIconSize = 20; // ok, default if iconsize.png is missing
		bigIconSize = 26;	// better for some math icons
		hugeIconSize = 32;	// better for hires displays
		giantIconSize = 48;

		// if it exists, use width of iconsize.png as normal size
		QString const dir = toqstr(addPath("images", lyxrc.icon_set));
		FileName const fn = lyx::libFileSearch(dir, "iconsize.png");
		if (!fn.empty()) {
			QImage image(toqstr(fn.absFileName()));
			if (image.width() < int(smallIconSize))
				normalIconSize = smallIconSize;
			else if (image.width() > int(giantIconSize))
				normalIconSize = giantIconSize;
			else
				normalIconSize = image.width();
		}

		splitter_ = new QSplitter;
		bg_widget_ = new BackgroundWidget;
		stack_widget_ = new QStackedWidget;
		stack_widget_->addWidget(bg_widget_);
		stack_widget_->addWidget(splitter_);
		setBackground();

		// TODO cleanup, remove the singleton, handle multiple Windows?
		progress_ = ProgressInterface::instance();
		if (!dynamic_cast<GuiProgress*>(progress_)) {
			progress_ = new GuiProgress;  // TODO who deletes it
			ProgressInterface::setInstance(progress_);
		}
		QObject::connect(
				dynamic_cast<GuiProgress*>(progress_),
				SIGNAL(updateStatusBarMessage(QString const&)),
				gv, SLOT(updateStatusBarMessage(QString const&)));
		QObject::connect(
				dynamic_cast<GuiProgress*>(progress_),
				SIGNAL(clearMessageText()),
				gv, SLOT(clearMessageText()));
	}

	~GuiViewPrivate()
	{
		delete splitter_;
		delete bg_widget_;
		delete stack_widget_;
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

		QAction * hugeIcons = new QAction(iconSizeGroup);
		hugeIcons->setText(qt_("Huge-sized icons"));
		hugeIcons->setCheckable(true);
		QObject::connect(hugeIcons, SIGNAL(triggered()),
			parent, SLOT(hugeSizedIcons()));
		menu->addAction(hugeIcons);

		QAction * giantIcons = new QAction(iconSizeGroup);
		giantIcons->setText(qt_("Giant-sized icons"));
		giantIcons->setCheckable(true);
		QObject::connect(giantIcons, SIGNAL(triggered()),
			parent, SLOT(giantSizedIcons()));
		menu->addAction(giantIcons);

		unsigned int cur = parent->iconSize().width();
		if ( cur == parent->d.smallIconSize)
			smallIcons->setChecked(true);
		else if (cur == parent->d.normalIconSize)
			normalIcons->setChecked(true);
		else if (cur == parent->d.bigIconSize)
			bigIcons->setChecked(true);
		else if (cur == parent->d.hugeIconSize)
			hugeIcons->setChecked(true);
		else if (cur == parent->d.giantIconSize)
			giantIcons->setChecked(true);

		return menu;
	}

	void setBackground()
	{
		stack_widget_->setCurrentWidget(bg_widget_);
		bg_widget_->setUpdatesEnabled(true);
		bg_widget_->setFocus();
	}

	int tabWorkAreaCount()
	{
		return splitter_->count();
	}

	TabWorkArea * tabWorkArea(int i)
	{
		return dynamic_cast<TabWorkArea *>(splitter_->widget(i));
	}

	TabWorkArea * currentTabWorkArea()
	{
		int areas = tabWorkAreaCount();
		if (areas == 1)
			// The first TabWorkArea is always the first one, if any.
			return tabWorkArea(0);

		for (int i = 0; i != areas;  ++i) {
			TabWorkArea * twa = tabWorkArea(i);
			if (current_main_work_area_ == twa->currentWorkArea())
				return twa;
		}

		// None has the focus so we just take the first one.
		return tabWorkArea(0);
	}

	int countWorkAreasOf(Buffer & buf)
	{
		int areas = tabWorkAreaCount();
		int count = 0;
		for (int i = 0; i != areas;  ++i) {
			TabWorkArea * twa = tabWorkArea(i);
			if (twa->workArea(buf))
				++count;
		}
		return count;
	}

	void setPreviewFuture(QFuture<Buffer::ExportStatus> const & f)
	{
		if (processing_thread_watcher_.isRunning()) {
			// we prefer to cancel this preview in order to keep a snappy
			// interface.
			return;
		}
		processing_thread_watcher_.setFuture(f);
	}

public:
	GuiView * gv_;
	GuiWorkArea * current_work_area_;
	GuiWorkArea * current_main_work_area_;
	QSplitter * splitter_;
	QStackedWidget * stack_widget_;
	BackgroundWidget * bg_widget_;
	/// view's toolbars
	ToolbarMap toolbars_;
	ProgressInterface* progress_;
	/// The main layout box.
	/**
	 * \warning Don't Delete! The layout box is actually owned by
	 * whichever toolbar contains it. All the GuiView class needs is a
	 * means of accessing it.
	 *
	 * FIXME: replace that with a proper model so that we are not limited
	 * to only one dialog.
	 */
	LayoutBox * layout_;

	///
	map<string, DialogPtr> dialogs_;

	unsigned int smallIconSize;
	unsigned int normalIconSize;
	unsigned int bigIconSize;
	unsigned int hugeIconSize;
	unsigned int giantIconSize;
	///
	QTimer statusbar_timer_;
	/// auto-saving of buffers
	Timeout autosave_timeout_;
	/// flag against a race condition due to multiclicks, see bug #1119
	bool in_show_;

	///
	TocModels toc_models_;

	///
	QFutureWatcher<docstring> autosave_watcher_;
	QFutureWatcher<Buffer::ExportStatus> processing_thread_watcher_;
	///
	string last_export_format;
	string processing_format;

	static QSet<Buffer const *> busyBuffers;
	static Buffer::ExportStatus previewAndDestroy(Buffer const * orig, Buffer * buffer, string const & format);
	static Buffer::ExportStatus exportAndDestroy(Buffer const * orig, Buffer * buffer, string const & format);
	static Buffer::ExportStatus compileAndDestroy(Buffer const * orig, Buffer * buffer, string const & format);
	static docstring autosaveAndDestroy(Buffer const * orig, Buffer * buffer);

	template<class T>
	static Buffer::ExportStatus runAndDestroy(const T& func, Buffer const * orig, Buffer * buffer, string const & format);

	// TODO syncFunc/previewFunc: use bind
	bool asyncBufferProcessing(string const & argument,
				   Buffer const * used_buffer,
				   docstring const & msg,
				   Buffer::ExportStatus (*asyncFunc)(Buffer const *, Buffer *, string const &),
				   Buffer::ExportStatus (Buffer::*syncFunc)(string const &, bool) const,
				   Buffer::ExportStatus (Buffer::*previewFunc)(string const &) const);

	QVector<GuiWorkArea*> guiWorkAreas();
};

QSet<Buffer const *> GuiView::GuiViewPrivate::busyBuffers;


GuiView::GuiView(int id)
	: d(*new GuiViewPrivate(this)), id_(id), closing_(false), busy_(0)
{
	// GuiToolbars *must* be initialised before the menu bar.
	normalSizedIcons(); // at least on Mac the default is 32 otherwise, which is huge
	constructToolbars();

	// set ourself as the current view. This is needed for the menu bar
	// filling, at least for the static special menu item on Mac. Otherwise
	// they are greyed out.
	guiApp->setCurrentView(this);

	// Fill up the menu bar.
	guiApp->menus().fillMenuBar(menuBar(), this, true);

	setCentralWidget(d.stack_widget_);

	// Start autosave timer
	if (lyxrc.autosave) {
		d.autosave_timeout_.timeout.connect(bind(&GuiView::autoSave, this));
		d.autosave_timeout_.setTimeout(lyxrc.autosave * 1000);
		d.autosave_timeout_.start();
	}
	connect(&d.statusbar_timer_, SIGNAL(timeout()),
		this, SLOT(clearMessage()));

	// We don't want to keep the window in memory if it is closed.
	setAttribute(Qt::WA_DeleteOnClose, true);

#if !(defined(Q_OS_WIN) || defined(Q_CYGWIN_WIN)) && !defined(Q_OS_MAC)
	// QIcon::fromTheme was introduced in Qt 4.6
#if (QT_VERSION >= 0x040600)
	// assign an icon to main form. We do not do it under Qt/Win or Qt/Mac,
	// since the icon is provided in the application bundle. We use a themed
	// version when available and use the bundled one as fallback.
	setWindowIcon(QIcon::fromTheme("lyx", getPixmap("images/", "lyx", "svg,png")));
#else
	setWindowIcon(getPixmap("images/", "lyx", "svg,png"));
#endif

#endif
	resetWindowTitleAndIconText();

	// use tabbed dock area for multiple docks
	// (such as "source" and "messages")
	setDockOptions(QMainWindow::ForceTabbedDocks);

	// For Drag&Drop.
	setAcceptDrops(true);

	// add busy indicator to statusbar
	QLabel * busylabel = new QLabel(statusBar());
	statusBar()->addPermanentWidget(busylabel);
	search_mode mode = theGuiApp()->imageSearchMode();
	QString fn = toqstr(lyx::libFileSearch("images", "busy", "gif", mode).absFileName());
	QMovie * busyanim = new QMovie(fn, QByteArray(), busylabel);
	busylabel->setMovie(busyanim);
	busyanim->start();
	busylabel->hide();

	connect(&d.processing_thread_watcher_, SIGNAL(started()), 
		busylabel, SLOT(show()));
	connect(&d.processing_thread_watcher_, SIGNAL(finished()), 
		busylabel, SLOT(hide()));

	statusBar()->setSizeGripEnabled(true);
	updateStatusBar();

	connect(&d.autosave_watcher_, SIGNAL(finished()), this,
		SLOT(autoSaveThreadFinished()));

	connect(&d.processing_thread_watcher_, SIGNAL(started()), this,
		SLOT(processingThreadStarted()));
	connect(&d.processing_thread_watcher_, SIGNAL(finished()), this,
		SLOT(processingThreadFinished()));

	connect(this, SIGNAL(triggerShowDialog(QString const &, QString const &, Inset *)),
		SLOT(doShowDialog(QString const &, QString const &, Inset *)));

	// Forbid too small unresizable window because it can happen
	// with some window manager under X11.
	setMinimumSize(300, 200);

	if (lyxrc.allow_geometry_session) {
		// Now take care of session management.
		if (restoreLayout())
			return;
	}

	// no session handling, default to a sane size.
	setGeometry(50, 50, 690, 510);
	initToolbars();

	// clear session data if any.
	QSettings settings;
	settings.remove("views");
}


GuiView::~GuiView()
{
	delete &d;
}


QVector<GuiWorkArea*> GuiView::GuiViewPrivate::guiWorkAreas()
{
	QVector<GuiWorkArea*> areas;
	for (int i = 0; i < tabWorkAreaCount(); i++) {
		TabWorkArea* ta = tabWorkArea(i);
		for (int u = 0; u < ta->count(); u++) {
			areas << ta->workArea(u);
		}
	}
	return areas;
}

static void handleExportStatus(GuiView * view, Buffer::ExportStatus status,
	string const & format)
{
	docstring const fmt = formats.prettyName(format);
	docstring msg;
	switch (status) {
	case Buffer::ExportSuccess:
		msg = bformat(_("Successful export to format: %1$s"), fmt);
		break;
	case Buffer::ExportCancel:
		msg = _("Document export cancelled.");
		break;
	case Buffer::ExportError:
	case Buffer::ExportNoPathToFormat:
	case Buffer::ExportTexPathHasSpaces:
	case Buffer::ExportConverterError:
		msg = bformat(_("Error while exporting format: %1$s"), fmt);
		break;
	case Buffer::PreviewSuccess:
		msg = bformat(_("Successful preview of format: %1$s"), fmt);
		break;
	case Buffer::PreviewError:
		msg = bformat(_("Error while previewing format: %1$s"), fmt);
		break;
	}
	view->message(msg);
}


void GuiView::processingThreadStarted()
{
}


void GuiView::processingThreadFinished()
{
	QFutureWatcher<Buffer::ExportStatus> const * watcher =
		static_cast<QFutureWatcher<Buffer::ExportStatus> const *>(sender());

	Buffer::ExportStatus const status = watcher->result();
	handleExportStatus(this, status, d.processing_format);
	
	updateToolbars();
	BufferView const * const bv = currentBufferView();
	if (bv && !bv->buffer().errorList("Export").empty()) {
		errors("Export");
		return;
	}
	errors(d.last_export_format);
}


void GuiView::autoSaveThreadFinished()
{
	QFutureWatcher<docstring> const * watcher =
		static_cast<QFutureWatcher<docstring> const *>(sender());
	message(watcher->result());
	updateToolbars();
}


void GuiView::saveLayout() const
{
	QSettings settings;
	settings.beginGroup("views");
	settings.beginGroup(QString::number(id_));
#if defined(Q_WS_X11) || defined(QPA_XCB)
	settings.setValue("pos", pos());
	settings.setValue("size", size());
#else
	settings.setValue("geometry", saveGeometry());
#endif
	settings.setValue("layout", saveState(0));
	settings.setValue("icon_size", iconSize());
}


void GuiView::saveUISettings() const
{
	// Save the toolbar private states
	ToolbarMap::iterator end = d.toolbars_.end();
	for (ToolbarMap::iterator it = d.toolbars_.begin(); it != end; ++it)
		it->second->saveSession();
	// Now take care of all other dialogs
	map<string, DialogPtr>::const_iterator it = d.dialogs_.begin();
	for (; it!= d.dialogs_.end(); ++it)
		it->second->saveSession();
}


bool GuiView::restoreLayout()
{
	QSettings settings;
	settings.beginGroup("views");
	settings.beginGroup(QString::number(id_));
	QString const icon_key = "icon_size";
	if (!settings.contains(icon_key))
		return false;

	//code below is skipped when when ~/.config/LyX is (re)created
	QSize icon_size = settings.value(icon_key).toSize();
	// Check whether session size changed.
	if (icon_size.width() != int(d.smallIconSize) &&
	    icon_size.width() != int(d.normalIconSize) &&
	    icon_size.width() != int(d.bigIconSize) &&
	    icon_size.width() != int(d.hugeIconSize) &&
	    icon_size.width() != int(d.giantIconSize)) {
		icon_size.setWidth(d.normalIconSize);
		icon_size.setHeight(d.normalIconSize);
	}
	setIconSize(icon_size);

#if defined(Q_WS_X11) || defined(QPA_XCB)
	QPoint pos = settings.value("pos", QPoint(50, 50)).toPoint();
	QSize size = settings.value("size", QSize(690, 510)).toSize();
	resize(size);
	move(pos);
#else
	// Work-around for bug #6034: the window ends up in an undetermined
	// state when trying to restore a maximized window when it is
	// already maximized.
	if (!(windowState() & Qt::WindowMaximized))
		if (!restoreGeometry(settings.value("geometry").toByteArray()))
			setGeometry(50, 50, 690, 510);
#endif
	// Make sure layout is correctly oriented.
	setLayoutDirection(qApp->layoutDirection());

	// Allow the toc and view-source dock widget to be restored if needed.
	Dialog * dialog;
	if ((dialog = findOrBuild("toc", true)))
		// see bug 5082. At least setup title and enabled state.
		// Visibility will be adjusted by restoreState below.
		dialog->prepareView();
	if ((dialog = findOrBuild("view-source", true)))
		dialog->prepareView();
	if ((dialog = findOrBuild("progress", true)))
		dialog->prepareView();

	if (!restoreState(settings.value("layout").toByteArray(), 0))
		initToolbars();
	
	// init the toolbars that have not been restored
	Toolbars::Infos::iterator cit = guiApp->toolbars().begin();
	Toolbars::Infos::iterator end = guiApp->toolbars().end();
	for (; cit != end; ++cit) {
		GuiToolbar * tb = toolbar(cit->name);
		if (tb && !tb->isRestored())
			initToolbar(cit->name);
	}

	updateDialogs();
	return true;
}


GuiToolbar * GuiView::toolbar(string const & name)
{
	ToolbarMap::iterator it = d.toolbars_.find(name);
	if (it != d.toolbars_.end())
		return it->second;

	LYXERR(Debug::GUI, "Toolbar::display: no toolbar named " << name);
	return 0;
}


void GuiView::constructToolbars()
{
	ToolbarMap::iterator it = d.toolbars_.begin();
	for (; it != d.toolbars_.end(); ++it)
		delete it->second;
	d.toolbars_.clear();

	// I don't like doing this here, but the standard toolbar
	// destroys this object when it's destroyed itself (vfr)
	d.layout_ = new LayoutBox(*this);
	d.stack_widget_->addWidget(d.layout_);
	d.layout_->move(0,0);

	// extracts the toolbars from the backend
	Toolbars::Infos::iterator cit = guiApp->toolbars().begin();
	Toolbars::Infos::iterator end = guiApp->toolbars().end();
	for (; cit != end; ++cit)
		d.toolbars_[cit->name] =  new GuiToolbar(*cit, *this);
}


void GuiView::initToolbars()
{
	// extracts the toolbars from the backend
	Toolbars::Infos::iterator cit = guiApp->toolbars().begin();
	Toolbars::Infos::iterator end = guiApp->toolbars().end();
	for (; cit != end; ++cit)
		initToolbar(cit->name);
}


void GuiView::initToolbar(string const & name)
{
	GuiToolbar * tb = toolbar(name);
	if (!tb)
		return;
	int const visibility = guiApp->toolbars().defaultVisibility(name);
	bool newline = !(visibility & Toolbars::SAMEROW);
	tb->setVisible(false);
	tb->setVisibility(visibility);

	if (visibility & Toolbars::TOP) {
		if (newline)
			addToolBarBreak(Qt::TopToolBarArea);
		addToolBar(Qt::TopToolBarArea, tb);
	}

	if (visibility & Toolbars::BOTTOM) {
		if (newline)
			addToolBarBreak(Qt::BottomToolBarArea);
		addToolBar(Qt::BottomToolBarArea, tb);
	}

	if (visibility & Toolbars::LEFT) {
		if (newline)
			addToolBarBreak(Qt::LeftToolBarArea);
		addToolBar(Qt::LeftToolBarArea, tb);
	}

	if (visibility & Toolbars::RIGHT) {
		if (newline)
			addToolBarBreak(Qt::RightToolBarArea);
		addToolBar(Qt::RightToolBarArea, tb);
	}

	if (visibility & Toolbars::ON)
		tb->setVisible(true);
}


TocModels & GuiView::tocModels()
{
	return d.toc_models_;
}


void GuiView::setFocus()
{
	LYXERR(Debug::DEBUG, "GuiView::setFocus()" << this);
	QMainWindow::setFocus();
}


bool GuiView::hasFocus() const
{
	if (currentWorkArea())
		return currentWorkArea()->hasFocus();
	if (currentMainWorkArea())
		return currentMainWorkArea()->hasFocus();
	return d.bg_widget_->hasFocus();
}


void GuiView::focusInEvent(QFocusEvent * e)
{
	LYXERR(Debug::DEBUG, "GuiView::focusInEvent()" << this);
	QMainWindow::focusInEvent(e);
	// Make sure guiApp points to the correct view.
	guiApp->setCurrentView(this);
	if (currentWorkArea())
		currentWorkArea()->setFocus();
	else if (currentMainWorkArea())
		currentMainWorkArea()->setFocus();
	else
		d.bg_widget_->setFocus();
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

	updateToolbars();
	QMainWindow::showEvent(e);
}


bool GuiView::closeScheduled()
{
	closing_ = true;
	return close();
}


bool GuiView::prepareAllBuffersForLogout()
{
	Buffer * first = theBufferList().first();
	if (!first)
		return true;

	// First, iterate over all buffers and ask the users if unsaved
	// changes should be saved.
	// We cannot use a for loop as the buffer list cycles.
	Buffer * b = first;
	do {
		if (!saveBufferIfNeeded(const_cast<Buffer &>(*b), false))
			return false;
		b = theBufferList().next(b);
	} while (b != first);

	// Next, save session state
	// When a view/window was closed before without quitting LyX, there
	// are already entries in the lastOpened list.
	theSession().lastOpened().clear();
	writeSession();

	return true;
}


/** Destroy only all tabbed WorkAreas. Destruction of other WorkAreas
 ** is responsibility of the container (e.g., dialog)
 **/
void GuiView::closeEvent(QCloseEvent * close_event)
{
	LYXERR(Debug::DEBUG, "GuiView::closeEvent()");

	if (!GuiViewPrivate::busyBuffers.isEmpty()) {
		Alert::warning(_("Exit LyX"), 
			_("LyX could not be closed because documents are being processed by LyX."));
		close_event->setAccepted(false);
		return;
	}

	// If the user pressed the x (so we didn't call closeView
	// programmatically), we want to clear all existing entries.
	if (!closing_)
		theSession().lastOpened().clear();
	closing_ = true;

	writeSession();

	// it can happen that this event arrives without selecting the view,
	// e.g. when clicking the close button on a background window.
	setFocus();
	if (!closeWorkAreaAll()) {
		closing_ = false;
		close_event->ignore();
		return;
	}

	// Make sure that nothing will use this to be closed View.
	guiApp->unregisterView(this);

	if (isFullScreen()) {
		// Switch off fullscreen before closing.
		toggleFullScreen();
		updateDialogs();
	}

	// Make sure the timer time out will not trigger a statusbar update.
	d.statusbar_timer_.stop();

	// Saving fullscreen requires additional tweaks in the toolbar code.
	// It wouldn't also work under linux natively.
	if (lyxrc.allow_geometry_session) {
		saveLayout();
		saveUISettings();
	}

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


void GuiView::dropEvent(QDropEvent * event)
{
	QList<QUrl> files = event->mimeData()->urls();
	if (files.isEmpty())
		return;

	LYXERR(Debug::GUI, "GuiView::dropEvent: got URLs!");
	for (int i = 0; i != files.size(); ++i) {
		string const file = os::internal_path(fromqstr(
			files.at(i).toLocalFile()));
		if (file.empty())
			continue;

		string const ext = support::getExtension(file);
		vector<const Format *> found_formats;

		// Find all formats that have the correct extension.
		vector<const Format *> const & import_formats
			= theConverters().importableFormats();
		vector<const Format *>::const_iterator it = import_formats.begin();
		for (; it != import_formats.end(); ++it)
			if ((*it)->hasExtension(ext))
				found_formats.push_back(*it);

		FuncRequest cmd;
		if (found_formats.size() >= 1) {
			if (found_formats.size() > 1) {
				//FIXME: show a dialog to choose the correct importable format
				LYXERR(Debug::FILES,
					"Multiple importable formats found, selecting first");
			}
			string const arg = found_formats[0]->name() + " " + file;
			cmd = FuncRequest(LFUN_BUFFER_IMPORT, arg);
		}
		else {
			//FIXME: do we have to explicitly check whether it's a lyx file?
			LYXERR(Debug::FILES,
				"No formats found, trying to open it as a lyx file");
			cmd = FuncRequest(LFUN_FILE_OPEN, file);
		}
		// add the functions to the queue
		guiApp->addToFuncRequestQueue(cmd);
		event->accept();
	}
	// now process the collected functions. We perform the events
	// asynchronously. This prevents potential problems in case the
	// BufferView is closed within an event.
	guiApp->processFuncRequestQueueAsync();
}


void GuiView::message(docstring const & str)
{
	if (ForkedProcess::iAmAChild())
		return;

	// call is moved to GUI-thread by GuiProgress
	d.progress_->appendMessage(toqstr(str));
}


void GuiView::clearMessageText()
{
	message(docstring());
}


void GuiView::updateStatusBarMessage(QString const & str)
{
	statusBar()->showMessage(str);
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


void GuiView::hugeSizedIcons()
{
	setIconSize(QSize(d.hugeIconSize, d.hugeIconSize));
}


void GuiView::giantSizedIcons()
{
	setIconSize(QSize(d.giantIconSize, d.giantIconSize));
}


void GuiView::clearMessage()
{
	// FIXME: This code was introduced in r19643 to fix bug #4123. However,
	// the hasFocus function mostly returns false, even if the focus is on
	// a workarea in this view.
	//if (!hasFocus())
	//	return;
	showMessage();
	d.statusbar_timer_.stop();
}


void GuiView::updateWindowTitle(GuiWorkArea * wa)
{
	if (wa != d.current_work_area_
		|| wa->bufferView().buffer().isInternal())
		return;
	setWindowTitle(qt_("LyX: ") + wa->windowTitle());
	setWindowIconText(wa->windowIconText());
#if (QT_VERSION >= 0x040400)
	// Sets the path for the window: this is used by OSX to 
	// allow a context click on the title bar showing a menu
	// with the path up to the file
	setWindowFilePath(toqstr(wa->bufferView().buffer().absFileName()));
#endif
}


void GuiView::on_currentWorkAreaChanged(GuiWorkArea * wa)
{
	if (d.current_work_area_)
		QObject::disconnect(d.current_work_area_, SIGNAL(busy(bool)),
			this, SLOT(setBusy(bool)));
	disconnectBuffer();
	disconnectBufferView();
	connectBufferView(wa->bufferView());
	connectBuffer(wa->bufferView().buffer());
	d.current_work_area_ = wa;
	QObject::connect(wa, SIGNAL(titleChanged(GuiWorkArea *)),
		this, SLOT(updateWindowTitle(GuiWorkArea *)));
	QObject::connect(wa, SIGNAL(busy(bool)), this, SLOT(setBusy(bool)));
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
	if (closing_)
		// We already are in a close event. Nothing more to do.
		return;

	if (d.splitter_->count() > 1)
		// We have a splitter so don't close anything.
		return;

	// Reset and updates the dialogs.
	d.toc_models_.reset(0);
	updateDialog("document", "");
	updateDialogs();

	resetWindowTitleAndIconText();
	updateStatusBar();

	if (lyxrc.open_buffers_in_tabs)
		// Nothing more to do, the window should stay open.
		return;

	if (guiApp->viewIds().size() > 1) {
		close();
		return;
	}

#ifdef Q_OS_MAC
	// On Mac we also close the last window because the application stay
	// resident in memory. On other platforms we don't close the last
	// window because this would quit the application.
	close();
#endif
}


void GuiView::updateStatusBar()
{
	// let the user see the explicit message
	if (d.statusbar_timer_.isActive())
		return;

	showMessage();
}


void GuiView::showMessage()
{
	if (busy_)
		return;
	QString msg = toqstr(theGuiApp()->viewStatusMessage());
	if (msg.isEmpty()) {
		BufferView const * bv = currentBufferView();
		if (bv)
			msg = toqstr(bv->cursor().currentState());
		else
			msg = qt_("Welcome to LyX!");
	}
	statusBar()->showMessage(msg);
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
		GuiView * old_view = guiApp->currentView();
		if (this == old_view) {
			setFocus();
			return QMainWindow::event(e);
		}
		if (old_view && old_view->currentBufferView()) {
			// save current selection to the selection buffer to allow
			// middle-button paste in this window.
			cap::saveSelection(old_view->currentBufferView()->cursor());
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
			resetWindowTitleAndIconText();
		}
		setFocus();
		return QMainWindow::event(e);
	}

	case QEvent::ShortcutOverride: {
		// See bug 4888
		if (isFullScreen() && menuBar()->isHidden()) {
			QKeyEvent * ke = static_cast<QKeyEvent*>(e);
			// FIXME: we should also try to detect special LyX shortcut such as
			// Alt-P and Alt-M. Right now there is a hack in
			// GuiWorkArea::processKeySym() that hides again the menubar for
			// those cases.
			if (ke->modifiers() & Qt::AltModifier && ke->key() != Qt::Key_Alt) {
				menuBar()->show();
				return QMainWindow::event(e);
			}
		}
		return QMainWindow::event(e);
	}

	default:
		return QMainWindow::event(e);
	}
}

void GuiView::resetWindowTitleAndIconText()
{
	setWindowTitle(qt_("LyX"));
	setWindowIconText(qt_("LyX"));
}

bool GuiView::focusNextPrevChild(bool /*next*/)
{
	setFocus();
	return true;
}


bool GuiView::busy() const
{
	return busy_ > 0;
}


void GuiView::setBusy(bool busy)
{
	bool const busy_before = busy_ > 0;
	busy ? ++busy_ : --busy_;
	if ((busy_ > 0) == busy_before)
		// busy state didn't change
		return;

	if (busy) {
		QApplication::setOverrideCursor(Qt::WaitCursor);
		return;
	}
	QApplication::restoreOverrideCursor();
	updateLayoutList();	
}


double GuiView::pixelRatio() const
{
#if QT_VERSION >= 0x050000
	return devicePixelRatio();
#else
	return 1.0;
#endif
}
	
	
GuiWorkArea * GuiView::workArea(int index)
{
	if (TabWorkArea * twa = d.currentTabWorkArea())
		if (index < twa->count())
			return dynamic_cast<GuiWorkArea *>(twa->widget(index));
	return 0;
}


GuiWorkArea * GuiView::workArea(Buffer & buffer)
{
	if (currentWorkArea()
		&& &currentWorkArea()->bufferView().buffer() == &buffer)
		return (GuiWorkArea *) currentWorkArea();
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


GuiWorkArea * GuiView::currentWorkArea()
{
	return d.current_work_area_;
}


GuiWorkArea const * GuiView::currentMainWorkArea() const
{
	if (!d.currentTabWorkArea())
		return 0;
	return d.currentTabWorkArea()->currentWorkArea();
}


GuiWorkArea * GuiView::currentMainWorkArea()
{
	if (!d.currentTabWorkArea())
		return 0;
	return d.currentTabWorkArea()->currentWorkArea();
}


void GuiView::setCurrentWorkArea(GuiWorkArea * wa)
{
	LYXERR(Debug::DEBUG, "Setting current wa: " << wa << endl);
	if (!wa) {
		d.current_work_area_ = 0;
		d.setBackground();
		return;
	}

	// FIXME: I've no clue why this is here and why it accesses
	//  theGuiApp()->currentView, which might be 0 (bug 6464).
	//  See also 27525 (vfr).
	if (theGuiApp()->currentView() == this
		  && theGuiApp()->currentView()->currentWorkArea() == wa)
		return;

	if (currentBufferView())
		cap::saveSelection(currentBufferView()->cursor());

	theGuiApp()->setCurrentView(this);
	d.current_work_area_ = wa;
	
	// We need to reset this now, because it will need to be
	// right if the tabWorkArea gets reset in the for loop. We
	// will change it back if we aren't in that case.
	GuiWorkArea * const old_cmwa = d.current_main_work_area_;
	d.current_main_work_area_ = wa;

	for (int i = 0; i != d.splitter_->count(); ++i) {
		if (d.tabWorkArea(i)->setCurrentWorkArea(wa)) {
			LYXERR(Debug::DEBUG, "Current wa: " << currentWorkArea() 
				<< ", Current main wa: " << currentMainWorkArea());
			return;
		}
	}
	
	d.current_main_work_area_ = old_cmwa;
	
	LYXERR(Debug::DEBUG, "This is not a tabbed wa");
	on_currentWorkAreaChanged(wa);
	BufferView & bv = wa->bufferView();
	bv.cursor().fixIfBroken();
	bv.updateMetrics();
	wa->setUpdatesEnabled(true);
	LYXERR(Debug::DEBUG, "Current wa: " << currentWorkArea() << ", Current main wa: " << currentMainWorkArea());
}


void GuiView::removeWorkArea(GuiWorkArea * wa)
{
	LASSERT(wa, return);
	if (wa == d.current_work_area_) {
		disconnectBuffer();
		disconnectBufferView();
		d.current_work_area_ = 0;
		d.current_main_work_area_ = 0;
	}

	bool found_twa = false;
	for (int i = 0; i != d.splitter_->count(); ++i) {
		TabWorkArea * twa = d.tabWorkArea(i);
		if (twa->removeWorkArea(wa)) {
			// Found in this tab group, and deleted the GuiWorkArea.
			found_twa = true;
			if (twa->count() != 0) {
				if (d.current_work_area_ == 0)
					// This means that we are closing the current GuiWorkArea, so
					// switch to the next GuiWorkArea in the found TabWorkArea.
					setCurrentWorkArea(twa->currentWorkArea());
			} else {
				// No more WorkAreas in this tab group, so delete it.
				delete twa;
			}
			break;
		}
	}

	// It is not a tabbed work area (i.e., the search work area), so it
	// should be deleted by other means.
	LASSERT(found_twa, return);

	if (d.current_work_area_ == 0) {
		if (d.splitter_->count() != 0) {
			TabWorkArea * twa = d.currentTabWorkArea();
			setCurrentWorkArea(twa->currentWorkArea());
		} else {
			// No more work areas, switch to the background widget.
			setCurrentWorkArea(0);
		}
	}
}


LayoutBox * GuiView::getLayoutDialog() const
{
	return d.layout_;
}


void GuiView::updateLayoutList()
{
	if (d.layout_)
		d.layout_->updateContents(false);
}


void GuiView::updateToolbars()
{
	ToolbarMap::iterator end = d.toolbars_.end();
	if (d.current_work_area_) {
		bool const math =
			d.current_work_area_->bufferView().cursor().inMathed()
			&& !d.current_work_area_->bufferView().cursor().inRegexped();
		bool const table =
			lyx::getStatus(FuncRequest(LFUN_LAYOUT_TABULAR)).enabled();
		bool const review =
			lyx::getStatus(FuncRequest(LFUN_CHANGES_TRACK)).enabled() &&
			lyx::getStatus(FuncRequest(LFUN_CHANGES_TRACK)).onOff(true);
		bool const mathmacrotemplate =
			lyx::getStatus(FuncRequest(LFUN_IN_MATHMACROTEMPLATE)).enabled();
		bool const ipa =
			lyx::getStatus(FuncRequest(LFUN_IN_IPA)).enabled();

		for (ToolbarMap::iterator it = d.toolbars_.begin(); it != end; ++it)
			it->second->update(math, table, review, mathmacrotemplate, ipa);
	} else
		for (ToolbarMap::iterator it = d.toolbars_.begin(); it != end; ++it)
			it->second->update(false, false, false, false, false);
}


void GuiView::setBuffer(Buffer * newBuffer)
{
	LYXERR(Debug::DEBUG, "Setting buffer: " << newBuffer << endl);
	LASSERT(newBuffer, return);
	
	GuiWorkArea * wa = workArea(*newBuffer);
	if (wa == 0) {
		setBusy(true);
		newBuffer->masterBuffer()->updateBuffer();
		setBusy(false);
		wa = addWorkArea(*newBuffer);
		// scroll to the position when the BufferView was last closed
		if (lyxrc.use_lastfilepos) {
			LastFilePosSection::FilePos filepos =
				theSession().lastFilePos().load(newBuffer->fileName());
			wa->bufferView().moveToPosition(filepos.pit, filepos.pos, 0, 0);
		}
	} else {
		//Disconnect the old buffer...there's no new one.
		disconnectBuffer();
	}
	connectBuffer(*newBuffer);
	connectBufferView(wa->bufferView());
	setCurrentWorkArea(wa);
}


void GuiView::connectBuffer(Buffer & buf)
{
	buf.setGuiDelegate(this);
}


void GuiView::disconnectBuffer()
{
	if (d.current_work_area_)
		d.current_work_area_->bufferView().buffer().setGuiDelegate(0);
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


void GuiView::errors(string const & error_type, bool from_master)
{
	BufferView const * const bv = currentBufferView();
	if (!bv)
		return;

#if EXPORT_in_THREAD
	// We are called with from_master == false by default, so we
	// have to figure out whether that is the case or not.
	ErrorList & el = bv->buffer().errorList(error_type);
	if (el.empty()) {
	    el = bv->buffer().masterBuffer()->errorList(error_type);
	    from_master = true;
	}
#else
	ErrorList const & el = from_master ?
		bv->buffer().masterBuffer()->errorList(error_type) :
		bv->buffer().errorList(error_type);
#endif

	if (el.empty())
		return;

	string data = error_type;
	if (from_master)
		data = "from_master|" + error_type;
	showDialog("errorlist", data);
}


void GuiView::updateTocItem(string const & type, DocIterator const & dit)
{
	d.toc_models_.updateItem(toqstr(type), dit);
}


void GuiView::structureChanged()
{
	d.toc_models_.reset(documentBufferView());
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


BufferView * GuiView::documentBufferView()
{
	return currentMainWorkArea()
		? &currentMainWorkArea()->bufferView()
		: 0;
}


BufferView const * GuiView::documentBufferView() const
{
	return currentMainWorkArea()
		? &currentMainWorkArea()->bufferView()
		: 0;
}


BufferView * GuiView::currentBufferView()
{
	return d.current_work_area_ ? &d.current_work_area_->bufferView() : 0;
}


BufferView const * GuiView::currentBufferView() const
{
	return d.current_work_area_ ? &d.current_work_area_->bufferView() : 0;
}


docstring GuiView::GuiViewPrivate::autosaveAndDestroy(
	Buffer const * orig, Buffer * clone)
{
	bool const success = clone->autoSave();
	delete clone;
	busyBuffers.remove(orig);
	return success
		? _("Automatic save done.")
		: _("Automatic save failed!");
}


void GuiView::autoSave()
{
	LYXERR(Debug::INFO, "Running autoSave()");

	Buffer * buffer = documentBufferView()
		? &documentBufferView()->buffer() : 0;
	if (!buffer) {
		resetAutosaveTimers();
		return;
	}

	GuiViewPrivate::busyBuffers.insert(buffer);
	QFuture<docstring> f = QtConcurrent::run(GuiViewPrivate::autosaveAndDestroy,
		buffer, buffer->cloneBufferOnly());
	d.autosave_watcher_.setFuture(f);
	resetAutosaveTimers();
}


void GuiView::resetAutosaveTimers()
{
	if (lyxrc.autosave)
		d.autosave_timeout_.restart();
}


bool GuiView::getStatus(FuncRequest const & cmd, FuncStatus & flag)
{
	bool enable = true;
	Buffer * buf = currentBufferView()
		? &currentBufferView()->buffer() : 0;
	Buffer * doc_buffer = documentBufferView()
		? &(documentBufferView()->buffer()) : 0;

	/* In LyX/Mac, when a dialog is open, the menus of the
	   application can still be accessed without giving focus to
	   the main window. In this case, we want to disable the menu
	   entries that are buffer-related.
	 */
	if (cmd.origin() == FuncRequest::MENU && !hasFocus()) {
		buf = 0;
		doc_buffer = 0;
	}

	// Check whether we need a buffer
	if (!lyxaction.funcHasFlag(cmd.action(), LyXAction::NoBuffer) && !buf) {
		// no, exit directly
		flag.message(from_utf8(N_("Command not allowed with"
					"out any document open")));
		flag.setEnabled(false);
		return true;
	}

	if (cmd.origin() == FuncRequest::TOC) {
		GuiToc * toc = static_cast<GuiToc*>(findOrBuild("toc", false));
		if (!toc || !toc->getStatus(documentBufferView()->cursor(), cmd, flag))
			flag.setEnabled(false);
		return true;
	}

	switch(cmd.action()) {
	case LFUN_BUFFER_IMPORT:
		break;

	case LFUN_MASTER_BUFFER_UPDATE:
	case LFUN_MASTER_BUFFER_VIEW:
		enable = doc_buffer
			&& (doc_buffer->parent() != 0
			    || doc_buffer->hasChildren())
			&& !d.processing_thread_watcher_.isRunning();
		break;

	case LFUN_BUFFER_UPDATE:
	case LFUN_BUFFER_VIEW: {
		if (!doc_buffer || d.processing_thread_watcher_.isRunning()) {
			enable = false;
			break;
		}
		string format = to_utf8(cmd.argument());
		if (cmd.argument().empty())
			format = doc_buffer->params().getDefaultOutputFormat();
		enable = doc_buffer->params().isExportableFormat(format);
		break;
	}

	case LFUN_BUFFER_RELOAD:
		enable = doc_buffer && !doc_buffer->isUnnamed()
			&& doc_buffer->fileName().exists()
			&& (!doc_buffer->isClean()
			   || doc_buffer->isExternallyModified(Buffer::timestamp_method));
		break;

	case LFUN_BUFFER_CHILD_OPEN:
		enable = doc_buffer;
		break;

	case LFUN_BUFFER_WRITE:
		enable = doc_buffer && (doc_buffer->isUnnamed() || !doc_buffer->isClean());
		break;

	//FIXME: This LFUN should be moved to GuiApplication.
	case LFUN_BUFFER_WRITE_ALL: {
		// We enable the command only if there are some modified buffers
		Buffer * first = theBufferList().first();
		enable = false;
		if (!first)
			break;
		Buffer * b = first;
		// We cannot use a for loop as the buffer list is a cycle.
		do {
			if (!b->isClean()) {
				enable = true;
				break;
			}
			b = theBufferList().next(b);
		} while (b != first);
		break;
	}

	case LFUN_BUFFER_WRITE_AS:
	case LFUN_BUFFER_EXPORT_AS:
		enable = doc_buffer;
		break;

	case LFUN_BUFFER_CLOSE:
	case LFUN_VIEW_CLOSE:
		enable = doc_buffer;
		break;

	case LFUN_BUFFER_CLOSE_ALL:
		enable = theBufferList().last() != theBufferList().first();
		break;

	case LFUN_VIEW_SPLIT:
		if (cmd.getArg(0) == "vertical")
			enable = doc_buffer && (d.splitter_->count() == 1 ||
					 d.splitter_->orientation() == Qt::Vertical);
		else
			enable = doc_buffer && (d.splitter_->count() == 1 ||
					 d.splitter_->orientation() == Qt::Horizontal);
		break;

	case LFUN_TAB_GROUP_CLOSE:
		enable = d.tabWorkAreaCount() > 1;
		break;

	case LFUN_TOOLBAR_TOGGLE: {
		string const name = cmd.getArg(0);
		if (GuiToolbar * t = toolbar(name))
			flag.setOnOff(t->isVisible());
		else {
			enable = false;
			docstring const msg =
				bformat(_("Unknown toolbar \"%1$s\""), from_utf8(name));
			flag.message(msg);
		}
		break;
	}

	case LFUN_DROP_LAYOUTS_CHOICE:
		enable = buf;
		break;

	case LFUN_UI_TOGGLE:
		flag.setOnOff(isFullScreen());
		break;

	case LFUN_DIALOG_DISCONNECT_INSET:
		break;

	case LFUN_DIALOG_HIDE:
		// FIXME: should we check if the dialog is shown?
		break;

	case LFUN_DIALOG_TOGGLE:
		flag.setOnOff(isDialogVisible(cmd.getArg(0)));
		// fall through to set "enable"
	case LFUN_DIALOG_SHOW: {
		string const name = cmd.getArg(0);
		if (!doc_buffer)
			enable = name == "aboutlyx"
				|| name == "file" //FIXME: should be removed.
				|| name == "prefs"
				|| name == "texinfo"
				|| name == "progress"
				|| name == "compare";
		else if (name == "print")
			enable = doc_buffer->params().isExportable("dvi")
				&& lyxrc.print_command != "none";
		else if (name == "character" || name == "symbols"
			|| name == "mathdelimiter" || name == "mathmatrix") {
			if (!buf || buf->isReadonly())
				enable = false;
			else {
				Cursor const & cur = currentBufferView()->cursor();
				enable = !(cur.inTexted() && cur.paragraph().isPassThru());
			}
		}
		else if (name == "latexlog")
			enable = FileName(doc_buffer->logName()).isReadableFile();
		else if (name == "spellchecker")
			enable = theSpellChecker() 
				&& !doc_buffer->isReadonly()
				&& !doc_buffer->text().empty();
		else if (name == "vclog")
			enable = doc_buffer->lyxvc().inUse();
		break;
	}

	case LFUN_DIALOG_UPDATE: {
		string const name = cmd.getArg(0);
		if (!buf)
			enable = name == "prefs";
		break;
	}

	case LFUN_COMMAND_EXECUTE:
	case LFUN_MESSAGE:
	case LFUN_MENU_OPEN:
		// Nothing to check.
		break;

	case LFUN_COMPLETION_INLINE:
		if (!d.current_work_area_
			|| !d.current_work_area_->completer().inlinePossible(
			currentBufferView()->cursor()))
			enable = false;
		break;

	case LFUN_COMPLETION_POPUP:
		if (!d.current_work_area_
			|| !d.current_work_area_->completer().popupPossible(
			currentBufferView()->cursor()))
			enable = false;
		break;

	case LFUN_COMPLETE:
		if (!d.current_work_area_
			|| !d.current_work_area_->completer().inlinePossible(
			currentBufferView()->cursor()))
			enable = false;
		break;

	case LFUN_COMPLETION_ACCEPT:
		if (!d.current_work_area_
			|| (!d.current_work_area_->completer().popupVisible()
			&& !d.current_work_area_->completer().inlineVisible()
			&& !d.current_work_area_->completer().completionAvailable()))
			enable = false;
		break;

	case LFUN_COMPLETION_CANCEL:
		if (!d.current_work_area_
			|| (!d.current_work_area_->completer().popupVisible()
			&& !d.current_work_area_->completer().inlineVisible()))
			enable = false;
		break;

	case LFUN_BUFFER_ZOOM_OUT:
		enable = doc_buffer && lyxrc.zoom > 10;
		break;

	case LFUN_BUFFER_ZOOM_IN:
		enable = doc_buffer;
		break;

	case LFUN_BUFFER_MOVE_NEXT:
	case LFUN_BUFFER_MOVE_PREVIOUS:
		// we do not cycle when moving
	case LFUN_BUFFER_NEXT:
	case LFUN_BUFFER_PREVIOUS:
		// because we cycle, it doesn't matter whether on first or last
		enable = (d.currentTabWorkArea()->count() > 1);
		break;
	case LFUN_BUFFER_SWITCH:
		// toggle on the current buffer, but do not toggle off
		// the other ones (is that a good idea?)
		if (doc_buffer
			&& to_utf8(cmd.argument()) == doc_buffer->absFileName())
			flag.setOnOff(true);
		break;

	case LFUN_VC_REGISTER:
		enable = doc_buffer && !doc_buffer->lyxvc().inUse();
		break;
	case LFUN_VC_RENAME:
		enable = doc_buffer && doc_buffer->lyxvc().renameEnabled();
		break;
	case LFUN_VC_COPY:
		enable = doc_buffer && doc_buffer->lyxvc().copyEnabled();
		break;
	case LFUN_VC_CHECK_IN:
		enable = doc_buffer && doc_buffer->lyxvc().checkInEnabled();
		break;
	case LFUN_VC_CHECK_OUT:
		enable = doc_buffer && doc_buffer->lyxvc().checkOutEnabled();
		break;
	case LFUN_VC_LOCKING_TOGGLE:
		enable = doc_buffer && !doc_buffer->isReadonly()
			&& doc_buffer->lyxvc().lockingToggleEnabled();
		flag.setOnOff(enable && doc_buffer->lyxvc().locking());
		break;
	case LFUN_VC_REVERT:
		enable = doc_buffer && doc_buffer->lyxvc().inUse() && !doc_buffer->isReadonly();
		break;
	case LFUN_VC_UNDO_LAST:
		enable = doc_buffer && doc_buffer->lyxvc().undoLastEnabled();
		break;
	case LFUN_VC_REPO_UPDATE:
		enable = doc_buffer && doc_buffer->lyxvc().repoUpdateEnabled();
		break;
	case LFUN_VC_COMMAND: {
		if (cmd.argument().empty())
			enable = false;
		if (!doc_buffer && contains(cmd.getArg(0), 'D'))
			enable = false;
		break;
	}
	case LFUN_VC_COMPARE:
		enable = doc_buffer && doc_buffer->lyxvc().prepareFileRevisionEnabled();
		break;

	case LFUN_SERVER_GOTO_FILE_ROW:
		break;
	case LFUN_FORWARD_SEARCH:
		enable = !(lyxrc.forward_search_dvi.empty() && lyxrc.forward_search_pdf.empty());
		break;

	case LFUN_FILE_INSERT_PLAINTEXT:
	case LFUN_FILE_INSERT_PLAINTEXT_PARA:
		enable = documentBufferView() && documentBufferView()->cursor().inTexted();
		break;

	case LFUN_SPELLING_CONTINUOUSLY:
		flag.setOnOff(lyxrc.spellcheck_continuously);
		break;

	default:
		return false;
	}

	if (!enable)
		flag.setEnabled(false);

	return true;
}


static FileName selectTemplateFile()
{
	FileDialog dlg(qt_("Select template file"));
	dlg.setButton1(qt_("Documents|#o#O"), toqstr(lyxrc.document_path));
	dlg.setButton2(qt_("Templates|#T#t"), toqstr(lyxrc.template_path));

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

	Buffer * newBuffer = 0;
	try {
		newBuffer = checkAndLoadLyXFile(filename);
	} catch (ExceptionMessage const & e) {
		setBusy(false);
		throw(e);
	}
	setBusy(false);

	if (!newBuffer) {
		message(_("Document not loaded."));
		return 0;
	}

	setBuffer(newBuffer);
	newBuffer->errors("Parse");

	if (tolastfiles)
		theSession().lastFiles().add(filename);

	return newBuffer;
}


void GuiView::openDocument(string const & fname)
{
	string initpath = lyxrc.document_path;

	if (documentBufferView()) {
		string const trypath = documentBufferView()->buffer().filePath();
		// If directory is writeable, use this as default.
		if (FileName(trypath).isDirWritable())
			initpath = trypath;
	}

	string filename;

	if (fname.empty()) {
		FileDialog dlg(qt_("Select document to open"));
		dlg.setButton1(qt_("Documents|#o#O"), toqstr(lyxrc.document_path));
		dlg.setButton2(qt_("Examples|#E#e"),
				toqstr(addPath(package().system_support().absFileName(), "examples")));

		QStringList const filter(qt_("LyX Documents (*.lyx)"));
		FileDialog::Result result =
			dlg.open(toqstr(initpath), filter);

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
		filename = fullname.absFileName();

	if (!fullname.onlyPath().isDirectory()) {
		Alert::warning(_("Invalid filename"),
				bformat(_("The directory in the given path\n%1$s\ndoes not exist."),
				from_utf8(fullname.absFileName())));
		return;
	}

	// if the file doesn't exist and isn't already open (bug 6645),
	// let the user create one
	if (!fullname.exists() && !theBufferList().exists(fullname) &&
	    !LyXVC::file_not_found_hook(fullname)) {
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
		str2 = bformat(_("Document %1$s opened."), disp_fn);
		if (buf->lyxvc().inUse())
			str2 += " " + from_utf8(buf->lyxvc().versionString()) +
				" " + _("Version control detected.");
	} else {
		str2 = bformat(_("Could not open document %1$s"), disp_fn);
	}
	message(str2);
}

// FIXME: clean that
static bool import(GuiView * lv, FileName const & filename,
	string const & format, ErrorList & errorList)
{
	FileName const lyxfile(support::changeExtension(filename.absFileName(), ".lyx"));

	string loader_format;
	vector<string> loaders = theConverters().loaders();
	if (find(loaders.begin(), loaders.end(), format) == loaders.end()) {
		vector<string>::const_iterator it = loaders.begin();
		vector<string>::const_iterator en = loaders.end();
		for (; it != en; ++it) {
			if (!theConverters().isReachable(format, *it))
				continue;

			string const tofile =
				support::changeExtension(filename.absFileName(),
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
	} else {
		Buffer * const b = newFile(lyxfile.absFileName(), string(), true);
		if (!b)
			return false;
		lv->setBuffer(b);
		bool as_paragraphs = loader_format == "textparagraph";
		string filename2 = (loader_format == format) ? filename.absFileName()
			: support::changeExtension(filename.absFileName(),
					  formats.extension(loader_format));
		lv->currentBufferView()->insertPlaintextFile(FileName(filename2),
			as_paragraphs);
		guiApp->setCurrentView(lv);
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
		if (documentBufferView()) {
			string const trypath = documentBufferView()->buffer().filePath();
			// If directory is writeable, use this as default.
			if (FileName(trypath).isDirWritable())
				initpath = trypath;
		}

		docstring const text = bformat(_("Select %1$s file to import"),
			formats.prettyName(format));

		FileDialog dlg(toqstr(text));
		dlg.setButton1(qt_("Documents|#o#O"), toqstr(lyxrc.document_path));
		dlg.setButton2(qt_("Examples|#E#e"),
			toqstr(addPath(package().system_support().absFileName(), "examples")));

		docstring filter = formats.prettyName(format);
		filter += " (*.{";
		// FIXME UNICODE
		filter += from_utf8(formats.extensions(format));
		filter += "})";

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

	// Can happen if the user entered a path into the dialog
	// (see bug #7437)
	if (fullname.onlyFileName().empty()) {
		docstring msg = bformat(_("The file name '%1$s' is invalid!\n"
					  "Aborting import."),
					from_utf8(fullname.absFileName()));
		frontend::Alert::error(_("File name error"), msg);
		message(_("Canceled."));
		return;
	}


	FileName const lyxfile(support::changeExtension(fullname.absFileName(), ".lyx"));

	// Check if the document already is open
	Buffer * buf = theBufferList().getBuffer(lyxfile);
	if (buf) {
		setBuffer(buf);
		if (!closeBuffer()) {
			message(_("Canceled."));
			return;
		}
	}

	docstring const displaypath = makeDisplayPath(lyxfile.absFileName(), 30);

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
	if (documentBufferView()) {
		FileName const trypath(documentBufferView()->buffer().filePath());
		// If directory is writeable, use this as default.
		if (trypath.isDirWritable())
			initpath = trypath;
	}

	string templatefile;
	if (from_template) {
		templatefile = selectTemplateFile().absFileName();
		if (templatefile.empty())
			return;
	}

	Buffer * b;
	if (filename.empty())
		b = newUnnamedFile(initpath, to_utf8(_("newfile")), templatefile);
	else
		b = newFile(filename, templatefile, true);

	if (b)
		setBuffer(b);

	// If no new document could be created, it is unsure
	// whether there is a valid BufferView.
	if (currentBufferView())
		// Ensure the cursor is correctly positioned on screen.
		currentBufferView()->showCursor();
}


void GuiView::insertLyXFile(docstring const & fname)
{
	BufferView * bv = documentBufferView();
	if (!bv)
		return;

	// FIXME UNICODE
	FileName filename(to_utf8(fname));
	if (filename.empty()) {
		// Launch a file browser
		// FIXME UNICODE
		string initpath = lyxrc.document_path;
		string const trypath = bv->buffer().filePath();
		// If directory is writeable, use this as default.
		if (FileName(trypath).isDirWritable())
			initpath = trypath;

		// FIXME UNICODE
		FileDialog dlg(qt_("Select LyX document to insert"));
		dlg.setButton1(qt_("Documents|#o#O"), toqstr(lyxrc.document_path));
		dlg.setButton2(qt_("Examples|#E#e"),
			toqstr(addPath(package().system_support().absFileName(),
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
	}

	bv->insertLyXFile(filename);
	bv->buffer().errors("Parse");
}


bool GuiView::renameBuffer(Buffer & b, docstring const & newname, RenameKind kind)
{
	FileName fname = b.fileName();
	FileName const oldname = fname;

	if (!newname.empty()) {
		// FIXME UNICODE
		fname = support::makeAbsPath(to_utf8(newname), oldname.onlyPath().absFileName());
	} else {
		// Switch to this Buffer.
		setBuffer(&b);

		// No argument? Ask user through dialog.
		// FIXME UNICODE
		FileDialog dlg(qt_("Choose a filename to save document as"));
		dlg.setButton1(qt_("Documents|#o#O"), toqstr(lyxrc.document_path));
		dlg.setButton2(qt_("Templates|#T#t"), toqstr(lyxrc.template_path));

		if (!isLyXFileName(fname.absFileName()))
			fname.changeExtension(".lyx");

		FileDialog::Result result =
			dlg.save(toqstr(fname.onlyPath().absFileName()),
				   QStringList(qt_("LyX Documents (*.lyx)")),
					 toqstr(fname.onlyFileName()));

		if (result.first == FileDialog::Later)
			return false;

		fname.set(fromqstr(result.second));

		if (fname.empty())
			return false;

		if (!isLyXFileName(fname.absFileName()))
			fname.changeExtension(".lyx");
	}

	// fname is now the new Buffer location.

	// if there is already a Buffer open with this name, we do not want
	// to have another one. (the second test makes sure we're not just
	// trying to overwrite ourselves, which is fine.)
	if (theBufferList().exists(fname) && fname != oldname
		  && theBufferList().getBuffer(fname) != &b) {
		docstring const text =
			bformat(_("The file\n%1$s\nis already open in your current session.\n"
		            "Please close it before attempting to overwrite it.\n"
		            "Do you want to choose a new filename?"),
			        from_utf8(fname.absFileName()));
		int const ret = Alert::prompt(_("Chosen File Already Open"),
			text, 0, 1, _("&Rename"), _("&Cancel"));
		switch (ret) {
		case 0: return renameBuffer(b, docstring(), kind);
		case 1: return false;
		}
		//return false;
	}

	bool const existsLocal = fname.exists();
	bool const existsInVC = LyXVC::fileInVC(fname);
	if (existsLocal || existsInVC) {
		docstring const file = makeDisplayPath(fname.absFileName(), 30);
		if (kind != LV_WRITE_AS && existsInVC) {
			// renaming to a name that is already in VC
			// would not work
			docstring text = bformat(_("The document %1$s "
					"is already registered.\n\n"
					"Do you want to choose a new name?"),
				file);
			docstring const title = (kind == LV_VC_RENAME) ?
				_("Rename document?") : _("Copy document?");
			docstring const button = (kind == LV_VC_RENAME) ?
				_("&Rename") : _("&Copy");
			int const ret = Alert::prompt(title, text, 0, 1,
				button, _("&Cancel"));
			switch (ret) {
			case 0: return renameBuffer(b, docstring(), kind);
			case 1: return false;
			}
		}

		if (existsLocal) {
			docstring text = bformat(_("The document %1$s "
					"already exists.\n\n"
					"Do you want to overwrite that document?"),
				file);
			int const ret = Alert::prompt(_("Overwrite document?"),
					text, 0, 2, _("&Overwrite"),
					_("&Rename"), _("&Cancel"));
			switch (ret) {
			case 0: break;
			case 1: return renameBuffer(b, docstring(), kind);
			case 2: return false;
			}
		}
	}

	switch (kind) {
	case LV_VC_RENAME: {
		string msg = b.lyxvc().rename(fname);
		if (msg.empty())
			return false;
		message(from_utf8(msg));
		break;
	}
	case LV_VC_COPY: {
		string msg = b.lyxvc().copy(fname);
		if (msg.empty())
			return false;
		message(from_utf8(msg));
		break;
	}
	case LV_WRITE_AS:
		break;
	}
	// LyXVC created the file already in case of LV_VC_RENAME or
	// LV_VC_COPY, but call saveBuffer() nevertheless to get
	// relative paths of included stuff right if we moved e.g. from
	// /a/b.lyx to /a/c/b.lyx.

	bool const saved = saveBuffer(b, fname);
	if (saved)
		b.reload();
	return saved;
}


struct PrettyNameComparator
{
	bool operator()(Format const *first, Format const *second) const {
		return compare_no_case(translateIfPossible(from_ascii(first->prettyname())),
				       translateIfPossible(from_ascii(second->prettyname()))) <= 0;
	}
};


bool GuiView::exportBufferAs(Buffer & b, docstring const & iformat)
{
	FileName fname = b.fileName();

	FileDialog dlg(qt_("Choose a filename to export the document as"));
	dlg.setButton1(qt_("Documents|#o#O"), toqstr(lyxrc.document_path));

	QStringList types;
	QString const anyformat = qt_("Guess from extension (*.*)");
	types << anyformat;
	Formats::const_iterator it = formats.begin();
	vector<Format const *> export_formats;
	for (; it != formats.end(); ++it)
		if (it->documentFormat())
			export_formats.push_back(&(*it));
	PrettyNameComparator cmp;
	sort(export_formats.begin(), export_formats.end(), cmp);
	vector<Format const *>::const_iterator fit = export_formats.begin();
	map<QString, string> fmap;
	QString filter;
	string ext;
	for (; fit != export_formats.end(); ++fit) {
		docstring const loc_prettyname =
			translateIfPossible(from_utf8((*fit)->prettyname()));
		QString const loc_filter = toqstr(bformat(from_ascii("%1$s (*.%2$s)"),
						     loc_prettyname,
						     from_ascii((*fit)->extension())));
		types << loc_filter;
		fmap[loc_filter] = (*fit)->name();
		if (from_ascii((*fit)->name()) == iformat) {
			filter = loc_filter;
			ext = (*fit)->extension();
		}
	}
	string ofname = fname.onlyFileName();
	if (!ext.empty())
		ofname = support::changeExtension(ofname, ext);
	FileDialog::Result result =
		dlg.save(toqstr(fname.onlyPath().absFileName()),
			 types,
			 toqstr(ofname),
			 &filter);
	if (result.first != FileDialog::Chosen)
		return false;

	string fmt_name;
	fname.set(fromqstr(result.second));
	if (filter == anyformat)
		fmt_name = formats.getFormatFromExtension(fname.extension());
	else
		fmt_name = fmap[filter];
	LYXERR(Debug::FILES, "filter=" << fromqstr(filter)
	       << ", fmt_name=" << fmt_name << ", fname=" << fname.absFileName());

	if (fmt_name.empty() || fname.empty())
		return false;

	// fname is now the new Buffer location.
	if (FileName(fname).exists()) {
		docstring const file = makeDisplayPath(fname.absFileName(), 30);
		docstring text = bformat(_("The document %1$s already "
					   "exists.\n\nDo you want to "
					   "overwrite that document?"),
					 file);
		int const ret = Alert::prompt(_("Overwrite document?"),
			text, 0, 2, _("&Overwrite"), _("&Rename"), _("&Cancel"));
		switch (ret) {
		case 0: break;
		case 1: return exportBufferAs(b, from_ascii(fmt_name));
		case 2: return false;
		}
	}

	FuncRequest cmd(LFUN_BUFFER_EXPORT, fmt_name + " " + fname.absFileName());
	DispatchResult dr;
	dispatch(cmd, dr);
	return dr.dispatched();
}


bool GuiView::saveBuffer(Buffer & b)
{
	return saveBuffer(b, FileName());
}


bool GuiView::saveBuffer(Buffer & b, FileName const & fn)
{
	if (workArea(b) && workArea(b)->inDialogMode())
		return true;

	if (fn.empty() && b.isUnnamed())
		return renameBuffer(b, docstring());

	bool const success = (fn.empty() ? b.save() : b.saveAs(fn));
	if (success) {
		theSession().lastFiles().add(b.fileName());
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

	return saveBuffer(b, fn);
}


bool GuiView::hideWorkArea(GuiWorkArea * wa)
{
	return closeWorkArea(wa, false);
}


// We only want to close the buffer if it is not visible in other workareas
// of the same view, nor in other views, and if this is not a child
bool GuiView::closeWorkArea(GuiWorkArea * wa)
{
	Buffer & buf = wa->bufferView().buffer();

	bool last_wa = d.countWorkAreasOf(buf) == 1
		&& !inOtherView(buf) && !buf.parent();

	bool close_buffer = last_wa;

	if (last_wa) {
		if (lyxrc.close_buffer_with_last_view == "yes")
			; // Nothing to do
		else if (lyxrc.close_buffer_with_last_view == "no")
			close_buffer = false;
		else {
			docstring file;
			if (buf.isUnnamed())
				file = from_utf8(buf.fileName().onlyFileName());
			else
				file = buf.fileName().displayName(30);
			docstring const text = bformat(
				_("Last view on document %1$s is being closed.\n"
				  "Would you like to close or hide the document?\n"
				  "\n"
				  "Hidden documents can be displayed back through\n"
				  "the menu: View->Hidden->...\n"
				  "\n"
				  "To remove this question, set your preference in:\n"
				  "  Tools->Preferences->Look&Feel->UserInterface\n"
				), file);
			int ret = Alert::prompt(_("Close or hide document?"),
				text, 0, 1, _("&Close"), _("&Hide"));
			close_buffer = (ret == 0);
		}
	}

	return closeWorkArea(wa, close_buffer);
}


bool GuiView::closeBuffer()
{
	GuiWorkArea * wa = currentMainWorkArea();
	setCurrentWorkArea(wa);
	Buffer & buf = wa->bufferView().buffer();
	return wa && closeWorkArea(wa, !buf.parent());
}


void GuiView::writeSession() const {
	GuiWorkArea const * active_wa = currentMainWorkArea();
	for (int i = 0; i < d.splitter_->count(); ++i) {
		TabWorkArea * twa = d.tabWorkArea(i);
		for (int j = 0; j < twa->count(); ++j) {
			GuiWorkArea * wa = static_cast<GuiWorkArea *>(twa->widget(j));
			Buffer & buf = wa->bufferView().buffer();
			theSession().lastOpened().add(buf.fileName(), wa == active_wa);
		}
	}
}


bool GuiView::closeBufferAll()
{
	// Close the workareas in all other views
	QList<int> const ids = guiApp->viewIds();
	for (int i = 0; i != ids.size(); ++i) {
		if (id_ != ids[i] && !guiApp->view(ids[i]).closeWorkAreaAll())
			return false;
	}

	// Close our own workareas
	if (!closeWorkAreaAll())
		return false;

	// Now close the hidden buffers. We prevent hidden buffers from being
	// dirty, so we can just close them.
	theBufferList().closeAll();
	return true;
}


bool GuiView::closeWorkAreaAll()
{
	setCurrentWorkArea(currentMainWorkArea());

	// We might be in a situation that there is still a tabWorkArea, but
	// there are no tabs anymore. This can happen when we get here after a
	// TabWorkArea::lastWorkAreaRemoved() signal. Therefore we count how
	// many TabWorkArea's have no documents anymore.
	int empty_twa = 0;

	// We have to call count() each time, because it can happen that
	// more than one splitter will disappear in one iteration (bug 5998).
	for (; d.splitter_->count() > empty_twa; ) {
		TabWorkArea * twa = d.tabWorkArea(empty_twa);

		if (twa->count() == 0)
			++empty_twa;
		else {
			setCurrentWorkArea(twa->currentWorkArea());
			if (!closeTabWorkArea(twa))
				return false;
		}
	}
	return true;
}


bool GuiView::closeWorkArea(GuiWorkArea * wa, bool close_buffer)
{
	if (!wa)
		return false;

	Buffer & buf = wa->bufferView().buffer();

	if (close_buffer && GuiViewPrivate::busyBuffers.contains(&buf)) {
		Alert::warning(_("Close document"), 
			_("Document could not be closed because it is being processed by LyX."));
		return false;
	}

	if (close_buffer)
		return closeBuffer(buf);
	else {
		if (!inMultiTabs(wa))
			if (!saveBufferIfNeeded(buf, true))
				return false;
		removeWorkArea(wa);
		return true;
	}
}


bool GuiView::closeBuffer(Buffer & buf)
{
	// If we are in a close_event all children will be closed in some time,
	// so no need to do it here. This will ensure that the children end up
	// in the session file in the correct order. If we close the master
	// buffer, we can close or release the child buffers here too.
	bool success = true;
	if (!closing_) {
		ListOfBuffers clist = buf.getChildren();
		ListOfBuffers::const_iterator it = clist.begin();
		ListOfBuffers::const_iterator const bend = clist.end();
		for (; it != bend; ++it) {
			// If a child is dirty, do not close
			// without user intervention
			//FIXME: should we look in other tabworkareas?
			Buffer * child_buf = *it;
			GuiWorkArea * child_wa = workArea(*child_buf);
			if (child_wa) {
				if (!closeWorkArea(child_wa, true)) {
					success = false;
					break;
				}
			} else
				theBufferList().releaseChild(&buf, child_buf);
		}
	}
	if (success) {
		// goto bookmark to update bookmark pit.
		//FIXME: we should update only the bookmarks related to this buffer!
		LYXERR(Debug::DEBUG, "GuiView::closeBuffer()");
		for (size_t i = 0; i < theSession().bookmarks().size(); ++i)
			guiApp->gotoBookmark(i+1, false, false);

		if (saveBufferIfNeeded(buf, false)) {
			buf.removeAutosaveFile();
			theBufferList().release(&buf);
			return true;
		}
	}
	// open all children again to avoid a crash because of dangling
	// pointers (bug 6603)
	buf.updateBuffer();
	return false;
}


bool GuiView::closeTabWorkArea(TabWorkArea * twa)
{
	while (twa == d.currentTabWorkArea()) {
		twa->setCurrentIndex(twa->count()-1);

		GuiWorkArea * wa = twa->currentWorkArea();
		Buffer & b = wa->bufferView().buffer();

		// We only want to close the buffer if the same buffer is not visible
		// in another view, and if this is not a child and if we are closing
		// a view (not a tabgroup).
		bool const close_buffer =
			!inOtherView(b) && !b.parent() && closing_;

		if (!closeWorkArea(wa, close_buffer))
			return false;
	}
	return true;
}


bool GuiView::saveBufferIfNeeded(Buffer & buf, bool hiding)
{
	if (buf.isClean() || buf.paragraphs().empty())
		return true;

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

	int ret;
	if (hiding && buf.isUnnamed()) {
		docstring const text = bformat(_("The document %1$s has not been "
						 "saved yet.\n\nDo you want to save "
						 "the document?"), file);
		ret = Alert::prompt(_("Save new document?"),
			text, 0, 1, _("&Save"), _("&Cancel"));
		if (ret == 1)
			++ret;
	} else {
		docstring const text = bformat(_("The document %1$s has unsaved changes."
			"\n\nDo you want to save the document or discard the changes?"), file);
		ret = Alert::prompt(_("Save changed document?"),
			text, 0, 2, _("&Save"), _("&Discard"), _("&Cancel"));
	}

	switch (ret) {
	case 0:
		if (!saveBuffer(buf))
			return false;
		break;
	case 1:
		// If we crash after this we could have no autosave file
		// but I guess this is really improbable (Jug).
		// Sometimes improbable things happen:
		// - see bug http://www.lyx.org/trac/ticket/6587 (ps)
		// buf.removeAutosaveFile();
		if (hiding)
			// revert all changes
			reloadBuffer(buf);
		buf.markClean();
		break;
	case 2:
		return false;
	}
	return true;
}


bool GuiView::inMultiTabs(GuiWorkArea * wa)
{
	Buffer & buf = wa->bufferView().buffer();

	for (int i = 0; i != d.splitter_->count(); ++i) {
		GuiWorkArea * wa_ = d.tabWorkArea(i)->workArea(buf);
		if (wa_ && wa_ != wa)
			return true;
	}
	return inOtherView(buf);
}


bool GuiView::inOtherView(Buffer & buf)
{
	QList<int> const ids = guiApp->viewIds();

	for (int i = 0; i != ids.size(); ++i) {
		if (id_ == ids[i])
			continue;

		if (guiApp->view(ids[i]).workArea(buf))
			return true;
	}
	return false;
}


void GuiView::gotoNextOrPreviousBuffer(NextOrPrevious np, bool const move)
{
	if (!documentBufferView())
		return;
	
	if (TabWorkArea * twa = d.currentTabWorkArea()) {
		Buffer * const curbuf = &documentBufferView()->buffer();
		int nwa = twa->count();
		for (int i = 0; i < nwa; ++i) {
			if (&workArea(i)->bufferView().buffer() == curbuf) {
				int next_index;
				if (np == NEXTBUFFER)
					next_index = (i == nwa - 1 ? 0 : i + 1);
				else
					next_index = (i == 0 ? nwa - 1 : i - 1);
				if (move)
					twa->moveTab(i, next_index);
				else
					setBuffer(&workArea(next_index)->bufferView().buffer());
				break;
			}
		}
	}
}


/// make sure the document is saved
static bool ensureBufferClean(Buffer * buffer)
{
	LASSERT(buffer, return false);
	if (buffer->isClean() && !buffer->isUnnamed())
		return true;

	docstring const file = buffer->fileName().displayName(30);
	docstring title;
	docstring text;
	if (!buffer->isUnnamed()) {
		text = bformat(_("The document %1$s has unsaved "
						 "changes.\n\nDo you want to save "
						 "the document?"), file);
		title = _("Save changed document?");

	} else {
		text = bformat(_("The document %1$s has not been "
						 "saved yet.\n\nDo you want to save "
						 "the document?"), file);
		title = _("Save new document?");
	}
	int const ret = Alert::prompt(title, text, 0, 1, _("&Save"), _("&Cancel"));

	if (ret == 0)
		dispatch(FuncRequest(LFUN_BUFFER_WRITE));

	return buffer->isClean() && !buffer->isUnnamed();
}


bool GuiView::reloadBuffer(Buffer & buf)
{
	Buffer::ReadStatus status = buf.reload();
	return status == Buffer::ReadSuccess;
}


void GuiView::checkExternallyModifiedBuffers()
{
	BufferList::iterator bit = theBufferList().begin();
	BufferList::iterator const bend = theBufferList().end();
	for (; bit != bend; ++bit) {
		Buffer * buf = *bit;
		if (buf->fileName().exists()
			&& buf->isExternallyModified(Buffer::checksum_method)) {
			docstring text = bformat(_("Document \n%1$s\n has been externally modified."
					" Reload now? Any local changes will be lost."),
					from_utf8(buf->absFileName()));
			int const ret = Alert::prompt(_("Reload externally changed document?"),
						text, 0, 1, _("&Reload"), _("&Cancel"));
			if (!ret)
				reloadBuffer(*buf);
		}
	}
}


void GuiView::dispatchVC(FuncRequest const & cmd, DispatchResult & dr)
{
	Buffer * buffer = documentBufferView()
		? &(documentBufferView()->buffer()) : 0;

	switch (cmd.action()) {
	case LFUN_VC_REGISTER:
		if (!buffer || !ensureBufferClean(buffer))
			break;
		if (!buffer->lyxvc().inUse()) {
			if (buffer->lyxvc().registrer()) {
				reloadBuffer(*buffer);
				dr.clearMessageUpdate();
			}
		}
		break;

	case LFUN_VC_RENAME:
	case LFUN_VC_COPY: {
		if (!buffer || !ensureBufferClean(buffer))
			break;
		if (buffer->lyxvc().inUse() && !buffer->isReadonly()) {
			if (buffer->lyxvc().isCheckInWithConfirmation()) {
				// Some changes are not yet committed.
				// We test here and not in getStatus(), since
				// this test is expensive.
				string log;
				LyXVC::CommandResult ret =
					buffer->lyxvc().checkIn(log);
				dr.setMessage(log);
				if (ret == LyXVC::ErrorCommand ||
				    ret == LyXVC::VCSuccess)
					reloadBuffer(*buffer);
				if (buffer->lyxvc().isCheckInWithConfirmation()) {
					frontend::Alert::error(
						_("Revision control error."),
						_("Document could not be checked in."));
					break;
				}
			}
			RenameKind const kind = (cmd.action() == LFUN_VC_RENAME) ?
				LV_VC_RENAME : LV_VC_COPY;
			renameBuffer(*buffer, cmd.argument(), kind);
		}
		break;
	}

	case LFUN_VC_CHECK_IN:
		if (!buffer || !ensureBufferClean(buffer))
			break;
		if (buffer->lyxvc().inUse() && !buffer->isReadonly()) {
			string log;
			LyXVC::CommandResult ret = buffer->lyxvc().checkIn(log);
			dr.setMessage(log);
			// Only skip reloading if the checkin was cancelled or
			// an error occurred before the real checkin VCS command
			// was executed, since the VCS might have changed the
			// file even if it could not checkin successfully.
			if (ret == LyXVC::ErrorCommand || ret == LyXVC::VCSuccess)
				reloadBuffer(*buffer);
		}
		break;

	case LFUN_VC_CHECK_OUT:
		if (!buffer || !ensureBufferClean(buffer))
			break;
		if (buffer->lyxvc().inUse()) {
			dr.setMessage(buffer->lyxvc().checkOut());
			reloadBuffer(*buffer);
		}
		break;

	case LFUN_VC_LOCKING_TOGGLE:
		LASSERT(buffer, return);
		if (!ensureBufferClean(buffer) || buffer->isReadonly())
			break;
		if (buffer->lyxvc().inUse()) {
			string res = buffer->lyxvc().lockingToggle();
			if (res.empty()) {
				frontend::Alert::error(_("Revision control error."),
				_("Error when setting the locking property."));
			} else {
				dr.setMessage(res);
				reloadBuffer(*buffer);
			}
		}
		break;

	case LFUN_VC_REVERT:
		LASSERT(buffer, return);
		if (buffer->lyxvc().revert()) {
			reloadBuffer(*buffer);
			dr.clearMessageUpdate();
		}
		break;

	case LFUN_VC_UNDO_LAST:
		LASSERT(buffer, return);
		buffer->lyxvc().undoLast();
		reloadBuffer(*buffer);
		dr.clearMessageUpdate();
		break;

	case LFUN_VC_REPO_UPDATE:
		LASSERT(buffer, return);
		if (ensureBufferClean(buffer)) {
			dr.setMessage(buffer->lyxvc().repoUpdate());
			checkExternallyModifiedBuffers();
		}
		break;

	case LFUN_VC_COMMAND: {
		string flag = cmd.getArg(0);
		if (buffer && contains(flag, 'R') && !ensureBufferClean(buffer))
			break;
		docstring message;
		if (contains(flag, 'M')) {
			if (!Alert::askForText(message, _("LyX VC: Log Message")))
				break;
		}
		string path = cmd.getArg(1);
		if (contains(path, "$$p") && buffer)
			path = subst(path, "$$p", buffer->filePath());
		LYXERR(Debug::LYXVC, "Directory: " << path);
		FileName pp(path);
		if (!pp.isReadableDirectory()) {
			lyxerr << _("Directory is not accessible.") << endl;
			break;
		}
		support::PathChanger p(pp);

		string command = cmd.getArg(2);
		if (command.empty())
			break;
		if (buffer) {
			command = subst(command, "$$i", buffer->absFileName());
			command = subst(command, "$$p", buffer->filePath());
		}
		command = subst(command, "$$m", to_utf8(message));
		LYXERR(Debug::LYXVC, "Command: " << command);
		Systemcall one;
		one.startscript(Systemcall::Wait, command);

		if (!buffer)
			break;
		if (contains(flag, 'I'))
			buffer->markDirty();
		if (contains(flag, 'R'))
			reloadBuffer(*buffer);

		break;
		}

	case LFUN_VC_COMPARE: {

		if (cmd.argument().empty()) {
			lyx::dispatch(FuncRequest(LFUN_DIALOG_SHOW, "comparehistory"));
			break;
		}

		string rev1 = cmd.getArg(0);
		string f1, f2;

		// f1
		if (!buffer->lyxvc().prepareFileRevision(rev1, f1))
			break;

		if (isStrInt(rev1) && convert<int>(rev1) <= 0) {
			f2 = buffer->absFileName();
		} else {
			string rev2 = cmd.getArg(1);
			if (rev2.empty())
				break;
			// f2
			if (!buffer->lyxvc().prepareFileRevision(rev2, f2))
				break;
		}

		LYXERR(Debug::LYXVC, "Launching comparison for fetched revisions:\n" <<
					f1 << "\n"  << f2 << "\n" );
		string par = "compare run " + quoteName(f1) + " " + quoteName(f2);
		lyx::dispatch(FuncRequest(LFUN_DIALOG_SHOW, par));
		break;
	}

	default:
		break;
	}
}


void GuiView::openChildDocument(string const & fname)
{
	LASSERT(documentBufferView(), return);
	Buffer & buffer = documentBufferView()->buffer();
	FileName const filename = support::makeAbsPath(fname, buffer.filePath());
	documentBufferView()->saveBookmark(false);
	Buffer * child = 0;
	if (theBufferList().exists(filename)) {
		child = theBufferList().getBuffer(filename);
		setBuffer(child);
	} else {
		message(bformat(_("Opening child document %1$s..."),
			makeDisplayPath(filename.absFileName())));
		child = loadDocument(filename, false);
	}
	// Set the parent name of the child document.
	// This makes insertion of citations and references in the child work,
	// when the target is in the parent or another child document.
	if (child)
		child->setParent(&buffer);
}


bool GuiView::goToFileRow(string const & argument)
{
	string file_name;
	int row;
	size_t i = argument.find_last_of(' ');
	if (i != string::npos) {
		file_name = os::internal_path(trim(argument.substr(0, i)));
		istringstream is(argument.substr(i + 1));
		is >> row;
		if (is.fail())
			i = string::npos;
	}
	if (i == string::npos) {
		LYXERR0("Wrong argument: " << argument);
		return false;
	}
	Buffer * buf = 0;
	string const abstmp = package().temp_dir().absFileName();
	string const realtmp = package().temp_dir().realPath();
	// We have to use os::path_prefix_is() here, instead of
	// simply prefixIs(), because the file name comes from
	// an external application and may need case adjustment.
	if (os::path_prefix_is(file_name, abstmp, os::CASE_ADJUSTED)
		|| os::path_prefix_is(file_name, realtmp, os::CASE_ADJUSTED)) {
		// Needed by inverse dvi search. If it is a file
		// in tmpdir, call the apropriated function.
		// If tmpdir is a symlink, we may have the real
		// path passed back, so we correct for that.
		if (!prefixIs(file_name, abstmp))
			file_name = subst(file_name, realtmp, abstmp);
		buf = theBufferList().getBufferFromTmp(file_name);
	} else {
		// Must replace extension of the file to be .lyx
		// and get full path
		FileName const s = fileSearch(string(),
						  support::changeExtension(file_name, ".lyx"), "lyx");
		// Either change buffer or load the file
		if (theBufferList().exists(s))
			buf = theBufferList().getBuffer(s);
		else if (s.exists()) {
			buf = loadDocument(s);
			if (!buf)
				return false;
		} else {
			message(bformat(
					_("File does not exist: %1$s"),
					makeDisplayPath(file_name)));
			return false;
		}
	}
	if (!buf) {
		message(bformat(
			_("No buffer for file: %1$s."),
			makeDisplayPath(file_name))
		);
		return false;
	}
	setBuffer(buf);
	documentBufferView()->setCursorFromRow(row);
	return true;
}


template<class T>
Buffer::ExportStatus GuiView::GuiViewPrivate::runAndDestroy(const T& func, Buffer const * orig, Buffer * clone, string const & format)
{
	Buffer::ExportStatus const status = func(format);

	// the cloning operation will have produced a clone of the entire set of
	// documents, starting from the master. so we must delete those.
	Buffer * mbuf = const_cast<Buffer *>(clone->masterBuffer());
	delete mbuf;
	busyBuffers.remove(orig);
	return status;
}


Buffer::ExportStatus GuiView::GuiViewPrivate::compileAndDestroy(Buffer const * orig, Buffer * clone, string const & format)
{
	Buffer::ExportStatus (Buffer::* mem_func)(std::string const &, bool) const = &Buffer::doExport;
	return runAndDestroy(lyx::bind(mem_func, clone, _1, true), orig, clone, format);
}


Buffer::ExportStatus GuiView::GuiViewPrivate::exportAndDestroy(Buffer const * orig, Buffer * clone, string const & format)
{
	Buffer::ExportStatus (Buffer::* mem_func)(std::string const &, bool) const = &Buffer::doExport;
	return runAndDestroy(lyx::bind(mem_func, clone, _1, false), orig, clone, format);
}


Buffer::ExportStatus GuiView::GuiViewPrivate::previewAndDestroy(Buffer const * orig, Buffer * clone, string const & format)
{
	Buffer::ExportStatus (Buffer::* mem_func)(std::string const &) const = &Buffer::preview;
	return runAndDestroy(lyx::bind(mem_func, clone, _1), orig, clone, format);
}


bool GuiView::GuiViewPrivate::asyncBufferProcessing(
			   string const & argument,
			   Buffer const * used_buffer,
			   docstring const & msg,
			   Buffer::ExportStatus (*asyncFunc)(Buffer const *, Buffer *, string const &),
			   Buffer::ExportStatus (Buffer::*syncFunc)(string const &, bool) const,
			   Buffer::ExportStatus (Buffer::*previewFunc)(string const &) const)
{
	if (!used_buffer)
		return false;

	string format = argument;
	if (format.empty())
		format = used_buffer->params().getDefaultOutputFormat();
	processing_format = format;
	if (!msg.empty()) {
		progress_->clearMessages();
		gv_->message(msg);
	}
#if EXPORT_in_THREAD
	GuiViewPrivate::busyBuffers.insert(used_buffer);
	Buffer * cloned_buffer = used_buffer->cloneFromMaster();
	if (!cloned_buffer) {
		Alert::error(_("Export Error"),
		             _("Error cloning the Buffer."));
		return false;
	}
	QFuture<Buffer::ExportStatus> f = QtConcurrent::run(
				asyncFunc,
				used_buffer,
				cloned_buffer,
				format);
	setPreviewFuture(f);
	last_export_format = used_buffer->params().bufferFormat();
	(void) syncFunc;
	(void) previewFunc;
	// We are asynchronous, so we don't know here anything about the success
	return true;
#else
	Buffer::ExportStatus status;
	if (syncFunc) {
		// TODO check here if it breaks exporting with Qt < 4.4
		status = (used_buffer->*syncFunc)(format, true);
	} else if (previewFunc) {
		status = (used_buffer->*previewFunc)(format); 
	} else
		return false;
	handleExportStatus(gv_, status, format);
	(void) asyncFunc;
	return (status == Buffer::ExportSuccess 
			|| status == Buffer::PreviewSuccess);
#endif
}

void GuiView::dispatchToBufferView(FuncRequest const & cmd, DispatchResult & dr)
{
	BufferView * bv = currentBufferView();
	LASSERT(bv, return);

	// Let the current BufferView dispatch its own actions.
	bv->dispatch(cmd, dr);
	if (dr.dispatched())
		return;

	// Try with the document BufferView dispatch if any.
	BufferView * doc_bv = documentBufferView();
	if (doc_bv && doc_bv != bv) {
		doc_bv->dispatch(cmd, dr);
		if (dr.dispatched())
			return;
	}

	// Then let the current Cursor dispatch its own actions.
	bv->cursor().dispatch(cmd);

	// update completion. We do it here and not in
	// processKeySym to avoid another redraw just for a
	// changed inline completion
	if (cmd.origin() == FuncRequest::KEYBOARD) {
		if (cmd.action() == LFUN_SELF_INSERT
			|| (cmd.action() == LFUN_ERT_INSERT && bv->cursor().inMathed()))
			updateCompletion(bv->cursor(), true, true);
		else if (cmd.action() == LFUN_CHAR_DELETE_BACKWARD)
			updateCompletion(bv->cursor(), false, true);
		else
			updateCompletion(bv->cursor(), false, false);
	}

	dr = bv->cursor().result();
}


void GuiView::dispatch(FuncRequest const & cmd, DispatchResult & dr)
{
	BufferView * bv = currentBufferView();
	// By default we won't need any update.
	dr.screenUpdate(Update::None);
	// assume cmd will be dispatched
	dr.dispatched(true);

	Buffer * doc_buffer = documentBufferView()
		? &(documentBufferView()->buffer()) : 0;

	if (cmd.origin() == FuncRequest::TOC) {
		GuiToc * toc = static_cast<GuiToc*>(findOrBuild("toc", false));
		// FIXME: do we need to pass a DispatchResult object here?
		toc->doDispatch(bv->cursor(), cmd);
		return;
	}

	string const argument = to_utf8(cmd.argument());

	switch(cmd.action()) {
		case LFUN_BUFFER_CHILD_OPEN:
			openChildDocument(to_utf8(cmd.argument()));
			break;

		case LFUN_BUFFER_IMPORT:
			importDocument(to_utf8(cmd.argument()));
			break;

		case LFUN_BUFFER_EXPORT: {
			if (!doc_buffer)
				break;
			FileName target_dir = doc_buffer->fileName().onlyPath();
			string const dest = cmd.getArg(1);
			if (!dest.empty() && FileName::isAbsolute(dest))
				target_dir = FileName(support::onlyPath(dest));
			// GCC only sees strfwd.h when building merged
			if (::lyx::operator==(cmd.argument(), "custom")) {
				dispatch(FuncRequest(LFUN_DIALOG_SHOW, "sendto"), dr);
				break;
			}
			if (!target_dir.isDirWritable()) {
				exportBufferAs(*doc_buffer, cmd.argument());
				break;
			}
			/* TODO/Review: Is it a problem to also export the children?
					See the update_unincluded flag */
			d.asyncBufferProcessing(argument,
						doc_buffer,
						_("Exporting ..."),
						&GuiViewPrivate::exportAndDestroy,
						&Buffer::doExport,
						0);
			// TODO Inform user about success
			break;
		}

		case LFUN_BUFFER_EXPORT_AS: {
			LASSERT(doc_buffer, break);
			docstring f = cmd.argument();
			if (f.empty())
				f = from_ascii(doc_buffer->params().getDefaultOutputFormat());
			exportBufferAs(*doc_buffer, f);
			break;
		}

		case LFUN_BUFFER_UPDATE: {
			d.asyncBufferProcessing(argument,
						doc_buffer,
						_("Exporting ..."),
						&GuiViewPrivate::compileAndDestroy,
						&Buffer::doExport,
						0);
			break;
		}
		case LFUN_BUFFER_VIEW: {
			d.asyncBufferProcessing(argument,
						doc_buffer,
						_("Previewing ..."),
						&GuiViewPrivate::previewAndDestroy,
						0,
						&Buffer::preview);
			break;
		}
		case LFUN_MASTER_BUFFER_UPDATE: {
			d.asyncBufferProcessing(argument,
						(doc_buffer ? doc_buffer->masterBuffer() : 0),
						docstring(),
						&GuiViewPrivate::compileAndDestroy,
						&Buffer::doExport,
						0);
			break;
		}
		case LFUN_MASTER_BUFFER_VIEW: {
			d.asyncBufferProcessing(argument,
						(doc_buffer ? doc_buffer->masterBuffer() : 0),
						docstring(),
						&GuiViewPrivate::previewAndDestroy,
						0, &Buffer::preview);
			break;
		}
		case LFUN_BUFFER_SWITCH: {
			string const file_name = to_utf8(cmd.argument());
			if (!FileName::isAbsolute(file_name)) {
				dr.setError(true);
				dr.setMessage(_("Absolute filename expected."));
				break;
			}

			Buffer * buffer = theBufferList().getBuffer(FileName(file_name));
			if (!buffer) {
				dr.setError(true);
				dr.setMessage(_("Document not loaded"));
				break;
			}

			// Do we open or switch to the buffer in this view ?
			if (workArea(*buffer)
				  || lyxrc.open_buffers_in_tabs || !documentBufferView()) {
				setBuffer(buffer);
				break;
			}

			// Look for the buffer in other views
			QList<int> const ids = guiApp->viewIds();
			int i = 0;
			for (; i != ids.size(); ++i) {
				GuiView & gv = guiApp->view(ids[i]);
				if (gv.workArea(*buffer)) {
					gv.activateWindow();
					gv.setBuffer(buffer);
					break;
				}
			}

			// If necessary, open a new window as a last resort
			if (i == ids.size()) {
				lyx::dispatch(FuncRequest(LFUN_WINDOW_NEW));
				lyx::dispatch(cmd);
			}
			break;
		}

		case LFUN_BUFFER_NEXT:
			gotoNextOrPreviousBuffer(NEXTBUFFER, false);
			break;

		case LFUN_BUFFER_MOVE_NEXT:
			gotoNextOrPreviousBuffer(NEXTBUFFER, true);
			break;

		case LFUN_BUFFER_PREVIOUS:
			gotoNextOrPreviousBuffer(PREVBUFFER, false);
			break;

		case LFUN_BUFFER_MOVE_PREVIOUS:
			gotoNextOrPreviousBuffer(PREVBUFFER, true);
			break;

		case LFUN_COMMAND_EXECUTE: {
			bool const show_it = cmd.argument() != "off";
			// FIXME: this is a hack, "minibuffer" should not be
			// hardcoded.
			if (GuiToolbar * t = toolbar("minibuffer")) {
				t->setVisible(show_it);
				if (show_it && t->commandBuffer())
					t->commandBuffer()->setFocus();
			}
			break;
		}
		case LFUN_DROP_LAYOUTS_CHOICE:
			d.layout_->showPopup();
			break;

		case LFUN_MENU_OPEN:
			if (QMenu * menu = guiApp->menus().menu(toqstr(cmd.argument()), *this))
				menu->exec(QCursor::pos());
			break;

		case LFUN_FILE_INSERT:
			insertLyXFile(cmd.argument());
			break;

		case LFUN_FILE_INSERT_PLAINTEXT:
		case LFUN_FILE_INSERT_PLAINTEXT_PARA: {
			string const fname = to_utf8(cmd.argument());
			if (!fname.empty() && !FileName::isAbsolute(fname)) {
				dr.setMessage(_("Absolute filename expected."));
				break;
			}
			
			FileName filename(fname);
			if (fname.empty()) {
				FileDialog dlg(qt_("Select file to insert"));

				FileDialog::Result result = dlg.open(toqstr(bv->buffer().filePath()),
					QStringList(qt_("All Files (*)")));
				
				if (result.first == FileDialog::Later || result.second.isEmpty()) {
					dr.setMessage(_("Canceled."));
					break;
				}

				filename.set(fromqstr(result.second));
			}

			if (bv) {
				FuncRequest const new_cmd(cmd, filename.absoluteFilePath());
				bv->dispatch(new_cmd, dr);
			}
			break;
		}

		case LFUN_BUFFER_RELOAD: {
			LASSERT(doc_buffer, break);

			int ret = 0;
			if (!doc_buffer->isClean()) {
				docstring const file =
					makeDisplayPath(doc_buffer->absFileName(), 20);
				docstring text = bformat(_("Any changes will be lost. "
					"Are you sure you want to revert to the saved version "
					"of the document %1$s?"), file);
				ret = Alert::prompt(_("Revert to saved document?"),
					text, 1, 1, _("&Revert"), _("&Cancel"));
			}

			if (ret == 0) {
				doc_buffer->markClean();
				reloadBuffer(*doc_buffer);
				dr.forceBufferUpdate();
			}
			break;
		}

		case LFUN_BUFFER_WRITE:
			LASSERT(doc_buffer, break);
			saveBuffer(*doc_buffer);
			break;

		case LFUN_BUFFER_WRITE_AS:
			LASSERT(doc_buffer, break);
			renameBuffer(*doc_buffer, cmd.argument());
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
			dr.setMessage(_("All documents saved."));
			break;
		}

		case LFUN_BUFFER_CLOSE:
			closeBuffer();
			break;

		case LFUN_BUFFER_CLOSE_ALL:
			closeBufferAll();
			break;

		case LFUN_TOOLBAR_TOGGLE: {
			string const name = cmd.getArg(0);
			if (GuiToolbar * t = toolbar(name))
				t->toggle();
			break;
		}

		case LFUN_DIALOG_UPDATE: {
			string const name = to_utf8(cmd.argument());
			if (name == "prefs" || name == "document")
				updateDialog(name, string());
			else if (name == "paragraph")
				lyx::dispatch(FuncRequest(LFUN_PARAGRAPH_UPDATE));
			else if (currentBufferView()) {
				Inset * inset = currentBufferView()->editedInset(name);
				// Can only update a dialog connected to an existing inset
				if (inset) {
					// FIXME: get rid of this indirection; GuiView ask the inset
					// if he is kind enough to update itself...
					FuncRequest fr(LFUN_INSET_DIALOG_UPDATE, cmd.argument());
					//FIXME: pass DispatchResult here?
					inset->dispatch(currentBufferView()->cursor(), fr);
				}
			}
			break;
		}

		case LFUN_DIALOG_TOGGLE: {
			FuncCode const func_code = isDialogVisible(cmd.getArg(0))
				? LFUN_DIALOG_HIDE : LFUN_DIALOG_SHOW;
			dispatch(FuncRequest(func_code, cmd.argument()), dr);
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
				string const logfile = doc_buffer->logName(&type);
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
					Lexer::quoteString(doc_buffer->lyxvc().getLogFile());
				showDialog("log", data);
			} else if (name == "symbols") {
				data = bv->cursor().getEncoding()->name();
				if (!data.empty())
					showDialog("symbols", data);
			// bug 5274
			} else if (name == "prefs" && isFullScreen()) {
				lfunUiToggle("fullscreen");
				showDialog("prefs", data);
			} else
				showDialog(name, data);
			break;
		}

		case LFUN_MESSAGE:
			dr.setMessage(cmd.argument());
			break;

		case LFUN_UI_TOGGLE: {
			string arg = cmd.getArg(0);
			if (!lfunUiToggle(arg)) {
				docstring const msg = "ui-toggle " + _("%1$s unknown command!");
				dr.setMessage(bformat(msg, from_utf8(arg)));
			}
			// Make sure the keyboard focus stays in the work area.
			setFocus();
			break;
		}

		case LFUN_VIEW_SPLIT: {
			LASSERT(doc_buffer, break);
			string const orientation = cmd.getArg(0);
			d.splitter_->setOrientation(orientation == "vertical"
				? Qt::Vertical : Qt::Horizontal);
			TabWorkArea * twa = addTabWorkArea();
			GuiWorkArea * wa = twa->addWorkArea(*doc_buffer, *this);
			setCurrentWorkArea(wa);
			break;
		}
		case LFUN_TAB_GROUP_CLOSE:
			if (TabWorkArea * twa = d.currentTabWorkArea()) {
				closeTabWorkArea(twa);
				d.current_work_area_ = 0;
				twa = d.currentTabWorkArea();
				// Switch to the next GuiWorkArea in the found TabWorkArea.
				if (twa) {
					// Make sure the work area is up to date.
					setCurrentWorkArea(twa->currentWorkArea());
				} else {
					setCurrentWorkArea(0);
				}
			}
			break;

		case LFUN_VIEW_CLOSE:
			if (TabWorkArea * twa = d.currentTabWorkArea()) {
				closeWorkArea(twa->currentWorkArea());
				d.current_work_area_ = 0;
				twa = d.currentTabWorkArea();
				// Switch to the next GuiWorkArea in the found TabWorkArea.
				if (twa) {
					// Make sure the work area is up to date.
					setCurrentWorkArea(twa->currentWorkArea());
				} else {
					setCurrentWorkArea(0);
				}
			}
			break;

		case LFUN_COMPLETION_INLINE:
			if (d.current_work_area_)
				d.current_work_area_->completer().showInline();
			break;

		case LFUN_COMPLETION_POPUP:
			if (d.current_work_area_)
				d.current_work_area_->completer().showPopup();
			break;


		case LFUN_COMPLETE:
			if (d.current_work_area_)
				d.current_work_area_->completer().tab();
			break;

		case LFUN_COMPLETION_CANCEL:
			if (d.current_work_area_) {
				if (d.current_work_area_->completer().popupVisible())
					d.current_work_area_->completer().hidePopup();
				else
					d.current_work_area_->completer().hideInline();
			}
			break;

		case LFUN_COMPLETION_ACCEPT:
			if (d.current_work_area_)
				d.current_work_area_->completer().activate();
			break;

		case LFUN_BUFFER_ZOOM_IN:
		case LFUN_BUFFER_ZOOM_OUT:
			if (cmd.argument().empty()) {
				if (cmd.action() == LFUN_BUFFER_ZOOM_IN)
					lyxrc.zoom += 20;
				else
					lyxrc.zoom -= 20;
			} else
				lyxrc.zoom += convert<int>(cmd.argument());

			if (lyxrc.zoom < 10)
				lyxrc.zoom = 10;

			// The global QPixmapCache is used in GuiPainter to cache text
			// painting so we must reset it.
			QPixmapCache::clear();
			guiApp->fontLoader().update();
			lyx::dispatch(FuncRequest(LFUN_SCREEN_FONT_UPDATE));
			break;

		case LFUN_VC_REGISTER:
		case LFUN_VC_RENAME:
		case LFUN_VC_COPY:
		case LFUN_VC_CHECK_IN:
		case LFUN_VC_CHECK_OUT:
		case LFUN_VC_REPO_UPDATE:
		case LFUN_VC_LOCKING_TOGGLE:
		case LFUN_VC_REVERT:
		case LFUN_VC_UNDO_LAST:
		case LFUN_VC_COMMAND:
		case LFUN_VC_COMPARE:
			dispatchVC(cmd, dr);
			break;

		case LFUN_SERVER_GOTO_FILE_ROW:
			goToFileRow(to_utf8(cmd.argument()));
			break;

		case LFUN_FORWARD_SEARCH: {
			Buffer const * doc_master = doc_buffer->masterBuffer();
			FileName const path(doc_master->temppath());
			string const texname = doc_master->isChild(doc_buffer)
				? DocFileName(changeExtension(
					doc_buffer->absFileName(),
						"tex")).mangledFileName()
				: doc_buffer->latexName();
			string const fulltexname = 
				support::makeAbsPath(texname, doc_master->temppath()).absFileName();
			string const mastername =
				removeExtension(doc_master->latexName());
			FileName const dviname(addName(path.absFileName(),
					addExtension(mastername, "dvi")));
			FileName const pdfname(addName(path.absFileName(),
					addExtension(mastername, "pdf")));
			bool const have_dvi = dviname.exists();
			bool const have_pdf = pdfname.exists();
			if (!have_dvi && !have_pdf) {
				dr.setMessage(_("Please, preview the document first."));
				break;
			}
			string outname = dviname.onlyFileName();
			string command = lyxrc.forward_search_dvi;
			if (!have_dvi || (have_pdf &&
			    pdfname.lastModified() > dviname.lastModified())) {
				outname = pdfname.onlyFileName();
				command = lyxrc.forward_search_pdf;
			}

			DocIterator tmpcur = bv->cursor();
			// Leave math first
			while (tmpcur.inMathed())
				tmpcur.pop_back();
			int row = tmpcur.inMathed() ? 0 : doc_buffer->texrow().getRowFromIdPos(
								tmpcur.paragraph().id(), tmpcur.pos());
			LYXERR(Debug::ACTION, "Forward search: row:" << row
				<< " id:" << tmpcur.paragraph().id());
			if (!row || command.empty()) {
				dr.setMessage(_("Couldn't proceed."));
				break;
			}
			string texrow = convert<string>(row);

			command = subst(command, "$$n", texrow);
			command = subst(command, "$$f", fulltexname);
			command = subst(command, "$$t", texname);
			command = subst(command, "$$o", outname);

			PathChanger p(path);
			Systemcall one;
			one.startscript(Systemcall::DontWait, command);
			break;
		}

		case LFUN_SPELLING_CONTINUOUSLY:
			lyxrc.spellcheck_continuously = !lyxrc.spellcheck_continuously;
			dr.screenUpdate(Update::Force | Update::FitCursor);
			break;

		default:
			// The LFUN must be for one of BufferView, Buffer or Cursor;
			// let's try that:
			dispatchToBufferView(cmd, dr);
			break;
	}

	// Part of automatic menu appearance feature.
	if (isFullScreen()) {
		if (menuBar()->isVisible() && lyxrc.full_screen_menubar)
			menuBar()->hide();
	}

	// Need to update bv because many LFUNs here might have destroyed it
	bv = currentBufferView();

	// Clear non-empty selections
        // (e.g. from a "char-forward-select" followed by "char-backward-select")
	if (bv) {
		Cursor & cur = bv->cursor();
		if ((cur.selection() && cur.selBegin() == cur.selEnd())) {
			cur.clearSelection();
		}
	}
}


bool GuiView::lfunUiToggle(string const & ui_component)
{
	if (ui_component == "scrollbar") {
		// hide() is of no help
		if (d.current_work_area_->verticalScrollBarPolicy() ==
			Qt::ScrollBarAlwaysOff)

			d.current_work_area_->setVerticalScrollBarPolicy(
				Qt::ScrollBarAsNeeded);
		else
			d.current_work_area_->setVerticalScrollBarPolicy(
				Qt::ScrollBarAlwaysOff);
	} else if (ui_component == "statusbar") {
		statusBar()->setVisible(!statusBar()->isVisible());
	} else if (ui_component == "menubar") {
		menuBar()->setVisible(!menuBar()->isVisible());
	} else
	if (ui_component == "frame") {
		int l, t, r, b;
		getContentsMargins(&l, &t, &r, &b);
		//are the frames in default state?
		d.current_work_area_->setFrameStyle(QFrame::NoFrame);
		if (l == 0) {
			setContentsMargins(-2, -2, -2, -2);
		} else {
			setContentsMargins(0, 0, 0, 0);
		}
	} else
	if (ui_component == "fullscreen") {
		toggleFullScreen();
	} else
		return false;
	return true;
}


void GuiView::toggleFullScreen()
{
	if (isFullScreen()) {
		for (int i = 0; i != d.splitter_->count(); ++i)
			d.tabWorkArea(i)->setFullScreen(false);
		setContentsMargins(0, 0, 0, 0);
		setWindowState(windowState() ^ Qt::WindowFullScreen);
		restoreLayout();
		menuBar()->show();
		statusBar()->show();
	} else {
		// bug 5274
		hideDialogs("prefs", 0);
		for (int i = 0; i != d.splitter_->count(); ++i)
			d.tabWorkArea(i)->setFullScreen(true);
		setContentsMargins(-2, -2, -2, -2);
		saveLayout();
		setWindowState(windowState() ^ Qt::WindowFullScreen);
		if (lyxrc.full_screen_statusbar)
			statusBar()->hide();
		if (lyxrc.full_screen_menubar)
			menuBar()->hide();
		if (lyxrc.full_screen_toolbars) {
			ToolbarMap::iterator end = d.toolbars_.end();
			for (ToolbarMap::iterator it = d.toolbars_.begin(); it != end; ++it)
				it->second->hide();
		}
	}

	// give dialogs like the TOC a chance to adapt
	updateDialogs();
}


Buffer const * GuiView::updateInset(Inset const * inset)
{
	if (!inset)
		return 0;

	Buffer const * inset_buffer = &(inset->buffer());

	for (int i = 0; i != d.splitter_->count(); ++i) {
		GuiWorkArea * wa = d.tabWorkArea(i)->currentWorkArea();
		if (!wa)
			continue;
		Buffer const * buffer = &(wa->bufferView().buffer());
		if (inset_buffer == buffer)
			wa->scheduleRedraw();
	}
	return inset_buffer;
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
	updateStatusBar();
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
// Changes should be also recorded in LFUN_DIALOG_SHOW doxygen
// docs in LyXAction.cpp.

char const * const dialognames[] = {

"aboutlyx", "bibitem", "bibtex", "box", "branch", "changes", "character",
"citation", "compare", "comparehistory", "document", "errorlist", "ert",
"external", "file", "findreplace", "findreplaceadv", "float", "graphics",
"href", "include", "index", "index_print", "info", "listings", "label", "line",
"log", "mathdelimiter", "mathmatrix", "mathspace", "nomenclature",
"nomencl_print", "note", "paragraph", "phantom", "prefs", "print", "ref",
"sendto", "space", "spellchecker", "symbols", "tabular", "tabularcreate",
"thesaurus", "texinfo", "toc", "view-source", "vspace", "wrap", "progress"};

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
	// Make sure that no LFUN uses any GuiView.
	guiApp->setCurrentView(0);
	saveLayout();
	saveUISettings();
	menuBar()->clear();
	constructToolbars();
	guiApp->menus().fillMenuBar(menuBar(), this, false);
	d.layout_->updateContents(true);
	// Now update controls with current buffer.
	guiApp->setCurrentView(this);
	restoreLayout();
	restartCursor();
}


Dialog * GuiView::findOrBuild(string const & name, bool hide_it)
{
	if (!isValidName(name))
		return 0;

	map<string, DialogPtr>::iterator it = d.dialogs_.find(name);

	if (it != d.dialogs_.end()) {
		if (hide_it)
			it->second->hideView();
		return it->second.get();
	}

	Dialog * dialog = build(name);
	d.dialogs_[name].reset(dialog);
	if (lyxrc.allow_geometry_session)
		dialog->restoreSession();
	if (hide_it)
		dialog->hideView();
	return dialog;
}


void GuiView::showDialog(string const & name, string const & data,
	Inset * inset)
{
	triggerShowDialog(toqstr(name), toqstr(data), inset);
}


void GuiView::doShowDialog(QString const & qname, QString const & qdata,
	Inset * inset)
{
	if (d.in_show_)
		return;

	const string name = fromqstr(qname);
	const string data = fromqstr(qdata);

	d.in_show_ = true;
	try {
		Dialog * dialog = findOrBuild(name, false);
		if (dialog) {
			bool const visible = dialog->isVisibleView();
			dialog->showData(data);
			if (inset && currentBufferView())
				currentBufferView()->editInset(name, inset);
			// We only set the focus to the new dialog if it was not yet
			// visible in order not to change the existing previous behaviour
			if (visible) {
				// activateWindow is needed for floating dockviews
				dialog->asQWidget()->raise();
				dialog->asQWidget()->activateWindow();
				dialog->asQWidget()->setFocus();
			}
		}
	}
	catch (ExceptionMessage const & ex) {
		d.in_show_ = false;
		throw ex;
	}
	d.in_show_ = false;
}


bool GuiView::isDialogVisible(string const & name) const
{
	map<string, DialogPtr>::const_iterator it = d.dialogs_.find(name);
	if (it == d.dialogs_.end())
		return false;
	return it->second.get()->isVisibleView() && !it->second.get()->isClosing();
}


void GuiView::hideDialog(string const & name, Inset * inset)
{
	map<string, DialogPtr>::const_iterator it = d.dialogs_.find(name);
	if (it == d.dialogs_.end())
		return;

	if (inset) {
		if (!currentBufferView())
			return;
		if (inset != currentBufferView()->editedInset(name))
			return;
	}

	Dialog * const dialog = it->second.get();
	if (dialog->isVisibleView())
		dialog->hideView();
	if (currentBufferView())
		currentBufferView()->editInset(name, 0);
}


void GuiView::disconnectDialog(string const & name)
{
	if (!isValidName(name))
		return;
	if (currentBufferView())
		currentBufferView()->editInset(name, 0);
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
		if (dialog) {
			if (dialog->needBufferOpen() && !documentBufferView())
				hideDialog(fromqstr(dialog->name()), 0);
			else if (dialog->isVisibleView())
				dialog->checkStatus();
		}
	}
	updateToolbars();
	updateLayoutList();
}

Dialog * createDialog(GuiView & lv, string const & name);

// will be replaced by a proper factory...
Dialog * createGuiAbout(GuiView & lv);
Dialog * createGuiBibtex(GuiView & lv);
Dialog * createGuiChanges(GuiView & lv);
Dialog * createGuiCharacter(GuiView & lv);
Dialog * createGuiCitation(GuiView & lv);
Dialog * createGuiCompare(GuiView & lv);
Dialog * createGuiCompareHistory(GuiView & lv);
Dialog * createGuiDelimiter(GuiView & lv);
Dialog * createGuiDocument(GuiView & lv);
Dialog * createGuiErrorList(GuiView & lv);
Dialog * createGuiExternal(GuiView & lv);
Dialog * createGuiGraphics(GuiView & lv);
Dialog * createGuiInclude(GuiView & lv);
Dialog * createGuiIndex(GuiView & lv);
Dialog * createGuiListings(GuiView & lv);
Dialog * createGuiLog(GuiView & lv);
Dialog * createGuiMathMatrix(GuiView & lv);
Dialog * createGuiNote(GuiView & lv);
Dialog * createGuiParagraph(GuiView & lv);
Dialog * createGuiPhantom(GuiView & lv);
Dialog * createGuiPreferences(GuiView & lv);
Dialog * createGuiPrint(GuiView & lv);
Dialog * createGuiPrintindex(GuiView & lv);
Dialog * createGuiRef(GuiView & lv);
Dialog * createGuiSearch(GuiView & lv);
Dialog * createGuiSearchAdv(GuiView & lv);
Dialog * createGuiSendTo(GuiView & lv);
Dialog * createGuiShowFile(GuiView & lv);
Dialog * createGuiSpellchecker(GuiView & lv);
Dialog * createGuiSymbols(GuiView & lv);
Dialog * createGuiTabularCreate(GuiView & lv);
Dialog * createGuiTexInfo(GuiView & lv);
Dialog * createGuiToc(GuiView & lv);
Dialog * createGuiThesaurus(GuiView & lv);
Dialog * createGuiViewSource(GuiView & lv);
Dialog * createGuiWrap(GuiView & lv);
Dialog * createGuiProgressView(GuiView & lv);



Dialog * GuiView::build(string const & name)
{
	LASSERT(isValidName(name), return 0);

	Dialog * dialog = createDialog(*this, name);
	if (dialog)
		return dialog;

	if (name == "aboutlyx")
		return createGuiAbout(*this);
	if (name == "bibtex")
		return createGuiBibtex(*this);
	if (name == "changes")
		return createGuiChanges(*this);
	if (name == "character")
		return createGuiCharacter(*this);
	if (name == "citation")
		return createGuiCitation(*this);
	if (name == "compare")
		return createGuiCompare(*this);
	if (name == "comparehistory")
		return createGuiCompareHistory(*this);
	if (name == "document")
		return createGuiDocument(*this);
	if (name == "errorlist")
		return createGuiErrorList(*this);
	if (name == "external")
		return createGuiExternal(*this);
	if (name == "file")
		return createGuiShowFile(*this);
	if (name == "findreplace")
		return createGuiSearch(*this);
	if (name == "findreplaceadv")
		return createGuiSearchAdv(*this);
	if (name == "graphics")
		return createGuiGraphics(*this);
	if (name == "include")
		return createGuiInclude(*this);
	if (name == "index")
		return createGuiIndex(*this);
	if (name == "index_print")
		return createGuiPrintindex(*this);
	if (name == "listings")
		return createGuiListings(*this);
	if (name == "log")
		return createGuiLog(*this);
	if (name == "mathdelimiter")
		return createGuiDelimiter(*this);
	if (name == "mathmatrix")
		return createGuiMathMatrix(*this);
	if (name == "note")
		return createGuiNote(*this);
	if (name == "paragraph")
		return createGuiParagraph(*this);
	if (name == "phantom")
		return createGuiPhantom(*this);
	if (name == "prefs")
		return createGuiPreferences(*this);
	if (name == "print")
		return createGuiPrint(*this);
	if (name == "ref")
		return createGuiRef(*this);
	if (name == "sendto")
		return createGuiSendTo(*this);
	if (name == "spellchecker")
		return createGuiSpellchecker(*this);
	if (name == "symbols")
		return createGuiSymbols(*this);
	if (name == "tabularcreate")
		return createGuiTabularCreate(*this);
	if (name == "texinfo")
		return createGuiTexInfo(*this);
	if (name == "thesaurus")
		return createGuiThesaurus(*this);
	if (name == "toc")
		return createGuiToc(*this);
	if (name == "view-source")
		return createGuiViewSource(*this);
	if (name == "wrap")
		return createGuiWrap(*this);
	if (name == "progress")
		return createGuiProgressView(*this);

	return 0;
}


} // namespace frontend
} // namespace lyx

#include "moc_GuiView.cpp"
