/**
 * \file frontend/Application.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_APPLICATION_H
#define LYX_APPLICATION_H

#include <boost/function.hpp>

#include <string>

namespace lyx {

class BufferView;
class Color_color;
struct RGBColor;

namespace frontend {

class Clipboard;
class FontLoader;
class Gui;
class LyXView;
class Selection;

/// The main application class
/**
There should be only one instance of this class. No Qt object
initialisation should be done before the instanciation of this class.

\todo The work areas handling could be moved to a base virtual class
common to all frontends.

 Model/View/Controller separation in LyX:

 1) The Model: \c Buffer

 The Buffer is the in-memory representation of a LyX file format. The
 Buffer does not (should not) have any information on what part of it
 is represented on screen. There is one unique Buffer per opened LyX
 file.


 2) The Controller: \c BufferView / \c Painter

 The BufferView is a tool used by the view that translates a part of
 the Buffer contents into drawing routines. The BufferView asks each
 inset of the Buffer to draw itself onto the screen using the Painter.
 There can be only one Buffer displayed in a BufferView. While there
 is the possibility to switch Buffer inside the BufferView, the goal
 is to instantiate a new BufferView on each Buffer switch.

 \todo Instantiate a new BufferView on each Buffer switch.

 The \c Painter is just a virtual interface to formalize each kind of
 drawing routines (text, line, rectangle, etc).

 The \c BufferView also contains a Cursor which may or may not be
 visible on screen. The cursor is really just a bookmark to remember
 where the next Buffer insertion/deletion is going to take place.


 3) The View: \c WorkArea (and it's qt4 specialisation GuiWorkArea)

 This contains the real screen area where the drawing is done by the
 Painter. One WorkArea holds one unique \c BufferView. While it could be
 possible that multiple WorkArea share one BufferView, this is not
 possible right now.
 The WorkArea also provide a scrollbar which position is translated
 into scrolling command to the inner \c BufferView.

 The WorkArea use the BufferView to translate each keyboard or mouse
 events into terms that the Buffer can understand:
 - insert/delete char
 - select char
 - etc.


 4) The Window: \c LyXView (and its qt4 specialisation \c GuiView)

 This is a full window containing a menubar, toolbars, a tabbar and a
 WorkArea. One LyXView could in theory contain multiple WorkArea
 (ex: with split window) but this number is limited to one only for
 now. In any case, there would be only one WorkArea that gets the focus
 at a time.

 Now, concerning the TabBar versus TabWidget issue. Right now, there is
 only one WorkArea and the TabBar just used to tell the BufferView inside
 the WorkArea to switch to this another Buffer.

 With a TabWidget, each Tab would own its own \c WorkArea. Clicking on a tab
 would switch a WorkArea instead of a Buffer.
*/
class Application
{
public:
	///
	Application(int & argc, char ** argv);
	///
	virtual ~Application() {}

	///
	virtual Gui & gui() = 0;

	/// Start the main event loop.
	/// The batch command is programmed to be execute once
	/// the event loop is started.
	virtual int const exec() = 0;

	/// Quit running LyX.
	/**
	* This may either quit directly or record the exit status
	* and only stop the event loop.
	*/
	virtual void exit(int status) = 0;

	/**
	* Synchronise all pending events.
	*/
	virtual void syncEvents() = 0;
	///
	virtual Clipboard & clipboard() = 0;
	///
	virtual Selection & selection() = 0;
	///
	virtual FontLoader & fontLoader() = 0;

	/// return a suitable serif font name.
	virtual std::string const romanFontName() = 0;

	/// return a suitable sans serif font name.
	virtual std::string const sansFontName() = 0;

	/// return a suitable monospaced font name.
	virtual std::string const typewriterFontName() = 0;

	/**
	* Given col, fills r, g, b in the range 0-255.
	* The function returns true if successful.
	* It returns false on failure and sets r, g, b to 0.
	*/
	virtual bool getRgbColor(Color_color col, RGBColor & rgbcol) = 0;

	/** Eg, passing Color::black returns "000000",
	*      passing Color::white returns "ffffff".
	*/
	virtual std::string const hexName(Color_color col) = 0;

	/**
	* update an altered GUI color
	*/
	virtual void updateColor(Color_color col) = 0;

	/**
	* add a callback for socket read notification
	* @param fd socket descriptor (file/socket/etc)
	*/
	virtual void registerSocketCallback(
		int fd, boost::function<void()> func) = 0;

	/**
	* remove a I/O read callback
	* @param fd socket descriptor (file/socket/etc)
	*/
	virtual void unregisterSocketCallback(int fd) = 0;

	/// Create the main window with given geometry settings.
	LyXView & createView(unsigned int width, unsigned int height,
		int posx, int posy, int maximized,
		unsigned int iconSizeXY, const std::string & geometryArg);

	///
	LyXView const * currentView() const;

	///
	LyXView * currentView();

	///
	void setCurrentView(LyXView & current_view);

private:
	/// This LyXView is the one receiving Clipboard and Selection
	/// Events
	LyXView * current_view_;

}; // Application

} // namespace frontend

frontend::Application * theApp();
frontend::Application * createApplication(int & argc, char * argv[]);


} // namespace lyx


#endif // LYX_APPLICATION_H
