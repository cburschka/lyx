/**
 * \file gtk/GuiApplication.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GTK_APPLICATION_H
#define GTK_APPLICATION_H

#include "GuiClipboard.h"
#include "GuiImplementation.h"
#include "GuiSelection.h"
#include "xftFontLoader.h"

#include "frontends/Application.h"

#include <gtkmm.h>

///////////////////////////////////////////////////////////////

class BufferView;

namespace lyx {
namespace frontend {

class GuiWorkArea;

/// The Gtk main application class
/**
There should be only one instance of this class. No Gtk object
initialisation should be done before the instanciation of this class.

\todo The work areas handling could be moved to a base virtual class
comon to all frontends.
*/
class GuiApplication : public Gtk::Main, public Application
{
public:
	GuiApplication(int & argc, char ** argv);

	/// Method inherited from \c Application class
	//@{
	virtual Clipboard& clipboard();
	virtual Selection& selection();
	virtual int const exec();
	virtual Gui & gui() { return gui_; }
	virtual void exit(int status);
	//@}

	///
	xftFontLoader & fontLoader() { return font_loader_; }

private:
	///
	GuiImplementation gui_;
	///
	GuiClipboard clipboard_;
	///
	GuiSelection selection_;
	///
	xftFontLoader font_loader_;
}; // GuiApplication

} // namespace frontend
} // namespace lyx

extern lyx::frontend::GuiApplication * guiApp;


#endif // GTK_APPLICATION_H
