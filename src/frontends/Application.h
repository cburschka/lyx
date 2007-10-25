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

#include "ColorCode.h"

#include <boost/function.hpp>

#include <string>

namespace lyx {

class BufferView;
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

 Model/View/Controller separation at frontend level in LyX-qt4:

 BufferList (N Buffers)
   |
   Buffer-a
   Buffer-b
   Buffer-c
   Buffer-d

 Application (this is the frontend really, should probably be renamed).
   |
   LyXView-1 (M1 WorkAreas, M1 <= N)
   |  |
   |  <tab-widget>
   |     | (many)
   |     WorkArea-1
   |       |
   |       BufferView <-----------> Buffer-c
   |         |
   |         Cursor
   |
   LyXView-2 (M2 WorkAreas, M2 <= N, M2 independent of M1)
      |
     ...


 1) The Model: \c Buffer

 The Buffer is the in-memory representation of a LyX file format. The
 Buffer does not (should not) have any information on what part of it
 is represented on screen. There is one unique Buffer per opened LyX
 file. A Buffer may or may not be represented on screen; typically, a
 child document does not have an associated BufferView unless the user
 choose to visualize it.


 2) The Controller: \c BufferView / \c Painter \c Cursor

 The BufferView is a tool used by the view (\sa WorkArea) that
 translates a part of the Buffer contents into drawing routines. The
 BufferView asks each inset of the Buffer to draw itself onto the
 screen using the Painter. There can be only one Buffer displayed in
 a BufferView and it is set on construction. Ideally, a BufferView
 should not be able to change the contents of its associated Buffer.
 A BufferView is instanciated and destroyed by a \c WorkArea; it is
 automatically destroyed by the parent WorkArea when its Buffer is
 closed.

 \todo Move all Buffer changing LFUN to LyXFunc or Cursor.
 \todo BufferView::buffer() should only offer const access.

 The \c Painter is just a virtual interface to formalize each kind of
 drawing routines (text, line, rectangle, etc).

 The \c BufferView also contains a Cursor which may or may not be
 visible on screen. The cursor is really just a bookmark to remember
 where the next Buffer insertion/deletion is going to take place.


 3) The View: \c WorkArea (and it's qt4 specialisation GuiWorkArea)

 This contains the real screen area where the drawing is done by the
 Painter. One WorkArea holds one unique \c BufferView. While it could
 be possible that multiple WorkArea share one BufferView, this is not
 something desirable because a BufferView is dependent of the WorkArea
 size.
 The WorkArea also provide a scrollbar which position is translated
 into scrolling command to the inner \c BufferView.

 The WorkArea use the BufferView to translate each keyboard or mouse
 events into terms that the Buffer can understand:
 - insert/delete char
 - select char
 - etc.


 4) The Window: \c LyXView (and its qt4 specialisation \c GuiView)

 This is a full window containing a menubar, toolbars and a central
 widget. A LyXView is in charge of creating and closing a View for a
 given Buffer.
 In the qt4 specialisation, \c GuiView, the central widget is a tab
 widget. Each tab is reverved to the visualisation of one Buffer and
 contains one WorkArea. In the qt4 frontend, one LyXView thus contains
 multiple WorkAreas but this number can limited to one for another
 frontend. The idea is that the kernel should not know how a Buffer
 is displayed on screen; it's the frontend business.
 In the future, we may also have multiple Workareas showing
 simultaneously in the same GuiView (ex: with split window).

 \todo Implement split-window

 In any case, there would be only one WorkArea that gets the focus
 at a time.

 With our current implementation using a QTabWidget, each Tab own its
 own \c WorkArea. Clicking on a tab switch a WorkArea and not really
 a Buffer. LFUN_BUFFER_SWITCH will tell the frontend to search the
 WorkArea associated to this Buffer. The WorkArea is automatically
 created if not already present.

 A WorkArea is connected to the Buffer::closing signal and is thus
 automatically destroyed when its Buffer is closed.

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
	virtual int exec() = 0;

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
	virtual bool getRgbColor(ColorCode col, RGBColor & rgbcol) = 0;

	/** Eg, passing Color_black returns "000000",
	*      passing Color_white returns "ffffff".
	*/
	virtual std::string const hexName(ColorCode col) = 0;

	/**
	* update an altered GUI color
	*/
	virtual void updateColor(ColorCode col) = 0;

	/**
	* add a callback for socket read notification
	* @param fd socket descriptor (file/socket/etc)
	*/
	typedef boost::function<void()> SocketCallback;
	virtual void registerSocketCallback(int fd, SocketCallback func) = 0;

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
