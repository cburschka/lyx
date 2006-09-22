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

#include "GuiImplementation.h"
#include "FontLoader.h"

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

	/// Method inherited from \c Application class
	//@{
	virtual int const exec();
	virtual Gui & gui() { return gui_; }
	virtual void exit(int status);
	//@}

	///
	FontLoader & fontLoader() { return font_loader_; }

	///
	LyXView & createView(unsigned int width, unsigned int height,
		int posx, int posy, bool maximize);

private:
	///
	GuiImplementation gui_;

	///
	FontLoader font_loader_;

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
