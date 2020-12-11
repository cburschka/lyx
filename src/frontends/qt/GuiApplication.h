/**
 * \file GuiApplication.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIAPPLICATION_H
#define GUIAPPLICATION_H

#include "KeyModifier.h"

#include "frontends/Application.h"
#include "support/filetools.h"

#include <QApplication>
#include <QList>
#ifdef QPA_XCB
#include <QAbstractNativeEventFilter>
#endif

class QAbstractItemModel;
class QIcon;
class QSessionManager;
class QFont;

namespace lyx {

class ColorCache;
class KeySymbol;

namespace support {
class FileName;
}

namespace frontend {

class Clipboard;
class FontLoader;
class GuiView;
class Menus;
class Selection;
class Toolbars;

/// The Qt main application class
/**
There should be only one instance of this class. No Qt object
initialisation should be done before the instantiation of this class.
*/
class GuiApplication : public QApplication, public Application
#ifdef QPA_XCB
		     , public QAbstractNativeEventFilter
#endif
{
	Q_OBJECT

public:
	GuiApplication(int & argc, char ** argv);
	~GuiApplication();

	/// \name Methods inherited from Application class
	//@{
	DispatchResult const & dispatch(FuncRequest const &) override;
	void dispatch(FuncRequest const &, DispatchResult & dr) override;
	FuncStatus getStatus(FuncRequest const & cmd) const override;
	void restoreGuiSession() override;
	Buffer const * updateInset(Inset const * inset) const override;
	int exec() override;
	void exit(int status) override;
	bool event(QEvent * e) override;
	bool getRgbColor(ColorCode col, RGBColor & rgbcol) override;
	std::string const hexName(ColorCode col) override;
	void registerSocketCallback(int fd, SocketCallback func) override;
	void unregisterSocketCallback(int fd) override;
	bool searchMenu(FuncRequest const & func, docstring_list & names) const override;
	bool hasBufferView() const override;
	std::string inputLanguageCode() const override;
	void handleKeyFunc(FuncCode action) override;
	bool unhide(Buffer * buf) override;
	//@}

	///
	bool getStatus(FuncRequest const & cmd, FuncStatus & status) const;
	///
	void hideDialogs(std::string const & name, Inset * inset) const;
	///
	void resetGui();
	/// Return true if current position is RTL of if no document is open and interface if RTL
	bool rtlContext() const;

	///
	Clipboard & clipboard();
	///
	Selection & selection();
	///
	FontLoader & fontLoader();

	///
	Toolbars const & toolbars() const;
	///
	Toolbars & toolbars();
	///
	Menus const & menus() const;
	///
	Menus & menus();

	/// \name Methods inherited from QApplication class
	//@{
	bool notify(QObject * receiver, QEvent * event) override;
	void commitData(QSessionManager & sm);
#ifdef Q_WS_X11
	bool x11EventFilter(XEvent * ev) override;
#elif defined(QPA_XCB)
	virtual bool nativeEventFilter(const QByteArray & eventType, void * message,
	                               long * result) override;
#endif
	//@}

	/// Create the main window with given geometry settings.
	/// \param geometry_arg: only for Windows platform.
	/// \param optional id identifier.
	void createView(QString const & geometry_arg = QString(),
		bool autoShow = true, int id = 0);
	/// FIXME: this method and the one above are quite ugly.
	void createView(int id);
	///
	GuiView const * currentView() const { return current_view_; }
	///
	GuiView * currentView() { return current_view_; }
	///
	void setCurrentView(GuiView * view) { current_view_ = view; }
	///
	QList<int> viewIds() const;

	/// Clear all session information.
	void clearSession();

	///
	ColorCache & colorCache();
	///
	QAbstractItemModel * languageModel();

	/// return a suitable serif font name.
	QString const romanFontName();

	/// return a suitable sans serif font name.
	QString const sansFontName();

	/// return a suitable monospaced font name.
	QString const typewriterFontName();
	QFont const typewriterSystemFont();

	///
	void unregisterView(GuiView * gv);
	///
	GuiView & view(int id) const;

#if (QT_VERSION < 0x050000)
	/// Emulate platformName() for Qt4
	QString platformName() const;
#endif

	/// Current ratio between physical pixels and device-independent pixels
	double pixelRatio() const;

	/// How to load image files
	support::search_mode imageSearchMode() const {
#if QT_VERSION >= 0x050000
		return pixelRatio() > 1 ? support::check_hidpi : support::must_exist;
#else
		return support::must_exist;
#endif
	}

	/// return true if the key is part of a shortcut
	bool queryKeySym(KeySymbol const & key, KeyModifier state) const;
	///
	void processKeySym(KeySymbol const & key, KeyModifier state);
	/// return the status bar state string
	docstring viewStatusMessage();

	/// \name Methods to process FuncRequests
	//@{
	/// process the func request
	void processFuncRequest(FuncRequest const &);
	/// add a func request to the queue and process it asynchronously
	/// \note As a side-effect this will also process the
	/// func requests that were added to the queue before.
	void processFuncRequestAsync(FuncRequest const &);
	/// process the func requests in the queue
	void processFuncRequestQueue();
	/// process the func requests in the queue asynchronously
	void processFuncRequestQueueAsync();
	/// add a func request to the queue for later processing
	void addToFuncRequestQueue(FuncRequest const &);
	//@}

	/// goto a bookmark
	/// openFile: whether or not open a file if the file is not opened
	/// switchToBuffer: whether or not switch to buffer if the buffer is
	///		not the current buffer
	void gotoBookmark(unsigned int idx, bool openFile, bool switchToBuffer);

	/// Start a long operation with some cancel possibility (button or ESC)
	void startLongOperation() override;
	/// This needs to be periodically called to avoid freezing the GUI
	bool longOperationCancelled() override;
	/// Stop the long operation mode (i.e., release the GUI)
	void stopLongOperation() override;
	/// A started long operation is still in progress ?
	bool longOperationStarted() override;
private Q_SLOTS:
	///
	void execBatchCommands();
	///
	void socketDataReceived(int fd);
	/// events to be triggered by Private::general_timer_ should go here
	void handleRegularEvents();
	///
	void onLastWindowClosed();
	///
	void onLocaleChanged();
	///
	void slotProcessFuncRequestQueue() { processFuncRequestQueue(); }

private:
	///
	void validateCurrentView();
	///
	void updateCurrentView(FuncRequest const & cmd, DispatchResult & dr);
	///
	bool closeAllViews();
	/// Things that need to be done when the OSes session manager
	/// requests a log out.
	bool prepareAllViewsForLogout();
	/// read the given ui (menu/toolbar) file
	bool readUIFile(QString const & name, bool include = false);
	///
	enum ReturnValues {
		ReadOK,
		ReadError,
		FormatMismatch
	};
	///
	ReturnValues readUIFile(support::FileName const & ui_path);
	///
	void setGuiLanguage();
	///
	void reconfigure(std::string const & option);

	/// This GuiView is the one receiving Clipboard and Selection
	/// events
	GuiView * current_view_;

	///
	struct Private;
	Private * const d;
}; // GuiApplication

extern GuiApplication * guiApp;


/// \return the pixmap for the given path, name and extension.
/// in case of errors a warning is produced and an empty pixmap is returned.
QPixmap getPixmap(QString const & path, QString const & name, QString const & ext);

/// \return an icon for the given action.
QIcon getIcon(FuncRequest const & f, bool unknown, bool rtl = false);

///
GuiApplication * theGuiApp();

} // namespace frontend
} // namespace lyx

#endif // GUIAPPLICATION_H
