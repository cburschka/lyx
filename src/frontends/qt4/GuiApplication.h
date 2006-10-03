/**
 * \file qt4/GuiApplication.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QT4_APPLICATION_H
#define QT4_APPLICATION_H

#include "ColorCache.h"
#include "FontLoader.h"
#include "GuiClipboard.h"
#include "GuiImplementation.h"
#include "GuiSelection.h"

#include "frontends/Application.h"

#include <QApplication>

///////////////////////////////////////////////////////////////
// Specific stuff

#ifdef Q_WS_MACX
#include <Carbon/Carbon.h>
#endif
///////////////////////////////////////////////////////////////

class BufferView;

namespace lyx {
namespace frontend {

class GuiWorkArea;

/// The Qt main application class
/**
There should be only one instance of this class. No Qt object
initialisation should be done before the instanciation of this class.

\todo The work areas handling could be moved to a base virtual class
comon to all frontends.
*/
class GuiApplication : public QApplication, public Application
{
public:
	GuiApplication(int & argc, char ** argv);
	///
	virtual ~GuiApplication() {}

	/// Method inherited from \c Application class
	//@{
	virtual Clipboard& clipboard();
	virtual Selection& selection();
	virtual int const exec();
	virtual Gui & gui() { return gui_; }
	virtual void exit(int status);
	virtual std::string const romanFontName();
	virtual std::string const sansFontName();
	virtual std::string const typewriterFontName();
	//@}

	///
	ColorCache & colorCache() { return color_cache_; }
	///
	FontLoader & fontLoader() { return font_loader_; }

private:
	///
	GuiImplementation gui_;
	///
	GuiClipboard clipboard_;
	///
	GuiSelection selection_;
	///
	FontLoader font_loader_;
	///
	ColorCache color_cache_;

#ifdef Q_WS_X11
public:
	bool x11EventFilter (XEvent * ev);
#endif

#ifdef Q_WS_MACX
public:
	bool macEventFilter(EventRef event);
private:
//	static OSStatus	handleOpenDocuments(
	static pascal OSErr	handleOpenDocuments(
		const AppleEvent* inEvent, AppleEvent*, long);
#endif
}; // GuiApplication

} // namespace frontend
} // namespace lyx

extern lyx::frontend::GuiApplication * guiApp;


#endif // QT4_APPLICATION_H
