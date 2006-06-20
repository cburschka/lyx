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

#include <QApplication>

///////////////////////////////////////////////////////////////
// Specific stuff

#ifdef Q_WS_MACX
#include <Carbon/Carbon.h>
#endif
///////////////////////////////////////////////////////////////

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

	void connect(GuiWorkArea * work_area);

private:
	///
	GuiWorkArea * work_area_;

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

#endif // LYX_APPLICATION_H
