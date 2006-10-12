/**
 * \file qt3/GuiApplication.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QT3_APPLICATION_H
#define QT3_APPLICATION_H

#include "frontends/Application.h"

#include "qfont_loader.h"
#include "GuiClipboard.h"
#include "GuiImplementation.h"
#include "GuiSelection.h"

#include <qapplication.h>

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
	virtual FontLoader & fontLoader() { return font_loader_; }
	virtual int const exec();
	virtual Gui & gui() { return gui_; }
	virtual void exit(int status);
	virtual std::string const romanFontName();
	virtual std::string const sansFontName();
	virtual std::string const typewriterFontName();
	virtual bool getRgbColor(LColor_color col, lyx::RGBColor & rgbcol);
	virtual std::string const hexName(LColor_color col);
	virtual void updateColor(LColor_color col);
	virtual void registerSocketCallback(
		int fd, boost::function<void()> func);
	virtual void unregisterSocketCallback(int fd);
	//@}

	///
private:
	///
	GuiImplementation gui_;
	///
	GuiClipboard clipboard_;
	///
	GuiSelection selection_;
	///
	GuiFontLoader font_loader_;

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


#endif // QT3_APPLICATION_H
