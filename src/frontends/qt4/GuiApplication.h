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
#include "GuiFontLoader.h"
#include "GuiClipboard.h"
#include "GuiSelection.h"

#include "frontends/Application.h"

#include <QObject>
#include <QApplication>
#include <QTranslator>

#include <map>

class QSessionManager;

namespace lyx {

class BufferView;

namespace frontend {

class GuiView;
class LyXView;
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
	///
	virtual ~GuiApplication();

	/// Method inherited from \c Application class
	//@{
	virtual Clipboard & clipboard();
	virtual Selection & selection();
	virtual FontLoader & fontLoader() { return font_loader_; }
	virtual int exec();
	virtual void exit(int status);
	virtual bool event(QEvent * e);
	void syncEvents();
	virtual std::string const romanFontName();
	virtual std::string const sansFontName();
	virtual std::string const typewriterFontName();
	virtual bool getRgbColor(ColorCode col, RGBColor & rgbcol);
	virtual std::string const hexName(ColorCode col);
	virtual void updateColor(ColorCode col);
	virtual void registerSocketCallback(int fd, SocketCallback func);
	void unregisterSocketCallback(int fd);
	LyXView & createView(std::string const & geometry_arg);
	//@}

	/// Methods inherited from \c QApplication class
	//@{
	bool notify(QObject * receiver, QEvent * event);
	void commitData(QSessionManager & sm);
	//@}

	///
	ColorCache & colorCache() { return color_cache_; }
	///
	GuiFontLoader & guiFontLoader() { return font_loader_; }


	virtual int createRegisteredView();
	virtual bool closeAllViews();
	virtual bool unregisterView(int id);

	virtual LyXView & view(int id) const;
	///
	virtual void hideDialogs(std::string const & name, Inset * inset) const;
	///
	virtual Buffer const * updateInset(Inset const * inset) const;

private Q_SLOTS:
	///
	void execBatchCommands();
	///
	void socketDataReceived(int fd);

private:
	///
	GuiClipboard clipboard_;
	///
	GuiSelection selection_;
	///
	GuiFontLoader font_loader_;
	///
	ColorCache color_cache_;
	///
	QTranslator qt_trans_;
	///
	std::map<int, SocketNotifier *> socket_notifiers_;

#ifdef Q_WS_X11
public:
	bool x11EventFilter(XEvent * ev);
#endif
	/// A translator suitable for the entries in the LyX menu.
	/// Only needed with Qt/Mac.
	void addMenuTranslator();

	/// Multiple views container.
	/**
	* Warning: This must not be a smart pointer as the destruction of the
	* object is handled by Qt when the view is closed
	* \sa Qt::WA_DeleteOnClose attribute.
	*/
	std::map<int, GuiView *> views_;
}; // GuiApplication

extern GuiApplication * guiApp;

} // namespace frontend
} // namespace lyx

#endif // GUIAPPLICATION_H
