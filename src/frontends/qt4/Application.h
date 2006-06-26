/**
 * \file qt4/Application.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_APPLICATION_H
#define LYX_APPLICATION_H

#include "GuiImplementation.h"
#include "FontLoader.h"

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
class Application : public QApplication
{
public:
	Application(int & argc, char ** argv);

	//
	Gui & gui() { return gui_; }
	///
	FontLoader & fontLoader() { return font_loader_; }
	///
	void setBufferView(BufferView * buffer_view);

private:
	///
	BufferView * buffer_view_;

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
}; // Application

} // namespace frontend
} // namespace lyx

extern lyx::frontend::Application * theApp;


#endif // LYX_APPLICATION_H
