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
#include "GuiImplementation.h"
#include "GuiSelection.h"

#include "frontends/Application.h"

#include <boost/scoped_ptr.hpp>

#include <QApplication>
#include <QTranslator>

class QSessionManager;

namespace lyx {

class BufferView;
class socket_callback;

namespace frontend {

class GuiWorkArea;
class MenuTranslator;

/// The Qt main application class
/**
There should be only one instance of this class. No Qt object
initialisation should be done before the instanciation of this class.

\todo The work areas handling could be moved to a base virtual class
comon to all frontends.
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
	virtual int const exec();
	virtual Gui & gui() { return gui_; }
	virtual void exit(int status);
	virtual bool event(QEvent * e);
	void syncEvents();
	virtual std::string const romanFontName();
	virtual std::string const sansFontName();
	virtual std::string const typewriterFontName();
	virtual bool getRgbColor(Color_color col, RGBColor & rgbcol);
	virtual std::string const hexName(Color_color col);
	virtual void updateColor(Color_color col);
	virtual void registerSocketCallback(
		int fd, boost::function<void()> func);
	void unregisterSocketCallback(int fd);
	//@}

	/// Methods inherited from \c QApplication class
	//@{
	bool notify(QObject * receiver, QEvent * event);
	void commitData(QSessionManager & sm);
	//@}

	///
	ColorCache & colorCache() { return color_cache_; }
	///
	///
	GuiFontLoader & guiFontLoader() { return font_loader_; }

private Q_SLOTS:
	///
	void execBatchCommands();

private:
	///
	GuiImplementation gui_;
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
	std::map<int, boost::shared_ptr<socket_callback> > socket_callbacks_;

#ifdef Q_WS_X11
public:
	bool x11EventFilter (XEvent * ev);
#endif

	/// A translator suitable for the entries in the LyX menu.
	/// Only needed with Qt/Mac.
	void addMenuTranslator();
	///
	boost::scoped_ptr<MenuTranslator> menu_trans_;
}; // GuiApplication

extern GuiApplication * guiApp;

} // namespace frontend
} // namespace lyx

#endif // GUIAPPLICATION_H
