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

#include "ColorCache.h"
#include "FontLoader.h"
#include "GuiClipboard.h"
#include "GuiSelection.h"
#include "Menus.h"

#include "frontends/Application.h"

#include <QObject>
#include <QApplication>
#include <QTimer>
#include <QTranslator>

#include <map>
#include <vector>

class QSessionManager;
class QAbstractItemModel;
class QSortFilterProxyModel;

namespace lyx {

class BufferView;

namespace frontend {

class GuiView;
class LyXView;
class GlobalMenuBar;
class GuiWorkArea;
class SocketNotifier;

/// The Qt main application class
/**
There should be only one instance of this class. No Qt object
initialisation should be done before the instanciation of this class.
*/
class GuiApplication : public QApplication, public Application
{
	Q_OBJECT

public:
	GuiApplication(int & argc, char ** argv);
	~GuiApplication();

	/// Method inherited from \c Application class
	//@{
	virtual FuncStatus getStatus(FuncRequest const &);
	virtual bool dispatch(FuncRequest const &);
	virtual void resetGui();
	void restoreGuiSession();
	virtual Clipboard & clipboard();
	virtual Selection & selection();
	virtual FontLoader & fontLoader() { return font_loader_; }
	virtual int exec();
	virtual void exit(int status);
	virtual bool event(QEvent * e);
	virtual bool getRgbColor(ColorCode col, RGBColor & rgbcol);
	virtual std::string const hexName(ColorCode col);
	virtual void updateColor(ColorCode col);
	virtual void readMenus(Lexer & lex);
	virtual void initGlobalMenu();
	virtual void registerSocketCallback(int fd, SocketCallback func);
	void unregisterSocketCallback(int fd);
	bool searchMenu(FuncRequest const & func, std::vector<docstring> & names) const;
	//@}

	Menus const & menus() const { return menus_; }
	Menus & menus() { return menus_; }
	/// Methods inherited from \c QApplication class
	//@{
	bool notify(QObject * receiver, QEvent * event);
	void commitData(QSessionManager & sm);
#ifdef Q_WS_X11
	bool x11EventFilter(XEvent * ev);
#endif
	//@}

	/// Create the main window with given geometry settings.
	/// \param geometry_arg: only for Windows platform.
	void createView(QString const & geometry_arg = QString(),
		bool autoShow = true);
	///
	GuiView const * currentView() const { return current_view_; }
	///
	GuiView * currentView() { return current_view_; }
	///
	void setCurrentView(GuiView * view) { current_view_ = view; }
	///
	virtual size_t viewCount() const { return view_ids_.size(); }
	///
	std::vector<int> const & viewIds() { return view_ids_; }

	///
	ColorCache & colorCache() { return color_cache_; }

	QAbstractItemModel * languageModel();

	/// return a suitable serif font name.
	virtual QString const romanFontName();

	/// return a suitable sans serif font name.
	virtual QString const sansFontName();

	/// return a suitable monospaced font name.
	virtual QString const typewriterFontName();
	///
	virtual bool unregisterView(int id);
	///
	virtual GuiView & view(int id) const;
	///
	virtual void hideDialogs(std::string const & name, Inset * inset) const;
	///
	virtual Buffer const * updateInset(Inset const * inset) const;

private Q_SLOTS:
	///
	void execBatchCommands();
	///
	void socketDataReceived(int fd);
	/// events to be triggered by general_timer_ should go here
	void handleRegularEvents();
	///
	void onLastWindowClosed();
	
private:
	///
	bool closeAllViews();
	///
	GuiClipboard clipboard_;
	///
	GuiSelection selection_;
	///
	FontLoader font_loader_;
	///
	ColorCache color_cache_;
	///
	QSortFilterProxyModel * language_model_;
	///
	QTranslator qt_trans_;
	///
	std::map<int, SocketNotifier *> socket_notifiers_;
	///
	Menus menus_;
	/// this timer is used for any regular events one wants to
	/// perform. at present it is used to check if forked processes
	/// are done.
	QTimer general_timer_;

	/// Multiple views container.
	/**
	* Warning: This must not be a smart pointer as the destruction of the
	* object is handled by Qt when the view is closed
	* \sa Qt::WA_DeleteOnClose attribute.
	*/
	std::map<int, GuiView *> views_;
	///
	std::vector<int> view_ids_;
	/// This LyXView is the one receiving Clipboard and Selection
	/// events
	GuiView * current_view_;
	///
	struct Private;
	Private * const d;
}; // GuiApplication

extern GuiApplication * guiApp;

} // namespace frontend
} // namespace lyx

#endif // GUIAPPLICATION_H
