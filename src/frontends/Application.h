/**
 * \file frontend/Application.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef APPLICATION_H
#define APPLICATION_H

#include "ColorCode.h"
#include "FuncCode.h"

#include "KeyModifier.h"

#include "support/strfwd.h"

#include <boost/function.hpp>

#include <vector>


namespace lyx {

class BufferView;
class Buffer;
class DispatchResult;
class docstring_list;
class FuncRequest;
class FuncStatus;
class Inset;
class Lexer;
struct RGBColor;

namespace frontend {

class Clipboard;
class FontLoader;
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
   GuiView-1 (one or more in case of split-view mode).
   |  |
   |  <tab-widget-1-1> 
   |  |  |
   |  |  WorkArea-1-1-1 (M1-1 WorkAreas, M1-1 <= N)
   |  |  | |
   |  |  | BufferView <-----------> Buffer-c
   |  |  |   |
   |  |  |   Cursor
   |  |  |
   |  |  WorkArea-1-1-2
   |  |  | |
   |  |  | BufferView <-----------> Buffer-a
   |  |  |   |
   |  |  |   Cursor
   |  |
   |  <tab-widget-1-2> 
   |
   GuiView-2 (one or more in case of split-view mode).
   |  |
   |  <tab-widget-2-1>
   |  |  |
   |  |  WorkArea-2-1-1 (M2-1 WorkAreas, M2-1 <= N, M2-1 independent of M1-1)
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

 \todo Move all Buffer changing LFUN to Buffer::dispatch(),
 LyXFunc::dispatch() or Cursor::dispatch().
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


 4) The Window: \c GuiView

 This is a full window containing a menubar, toolbars and a central
 widget. A GuiView is in charge of creating and closing a View for a
 given Buffer.
 In the qt4 specialisation, \c GuiView, the central widget is a tab
 widget. Each tab is reverved to the visualisation of one Buffer and
 contains one WorkArea. In the qt4 frontend, one GuiView thus contains
 multiple WorkAreas but this number can limited to one for another
 frontend. The idea is that the kernel should not know how a Buffer
 is displayed on screen; it's the frontend business.
 It is also possible to have multiple Workareas showing
 simultaneously in the same GuiView (ex: with split window), thus
 sharing the menubar and toolbars.

 In any case, there should be only one WorkArea that gets the focus
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
	Application() {}
	///
	virtual ~Application() {}

	/// LyX dispatcher: executes lyx actions and does necessary
	/// screen updates depending on results.
	/// This method encapsulates all the LyX command operations.
	/// This is the class of the LyX's "high level event handler".
	/// Every user command is processed here, either invocated from
	/// keyboard or from the GUI. All GUI objects, including buttons and
	/// menus should use this class and never call kernel functions directly.
	virtual DispatchResult const & dispatch(FuncRequest const &) = 0;

	/// LyX dispatcher: executes lyx actions and returns result.
	virtual void dispatch(FuncRequest const &, DispatchResult & dr) = 0;

	///
	virtual FuncStatus getStatus(FuncRequest const & cmd) const = 0;

	/// Load files and restore GUI Session.
	virtual void restoreGuiSession() = 0;

	///
	virtual Buffer const * updateInset(Inset const * inset) const = 0;

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
	* Given col, fills r, g, b in the range 0-255.
	* The function returns true if successful.
	* It returns false on failure and sets r, g, b to 0.
	*/
	virtual bool getRgbColor(ColorCode col, RGBColor & rgbcol) = 0;
	/// Like getRgbColor(), but static and slower
	static bool getRgbColorUncached(ColorCode col, RGBColor & rgbcol);

	/** Eg, passing Color_black returns "000000",
	*      passing Color_white returns "ffffff".
	*/
	virtual std::string const hexName(ColorCode col) = 0;
	
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

	virtual bool searchMenu(FuncRequest const & func,
		docstring_list & names) const = 0;

	/// \return the icon file name for the given action.
	static docstring iconName(FuncRequest const & f, bool unknown);
	/// \return the math icon name for the given command.
	static docstring mathIcon(docstring const & c);

	/// Handle a accented char key sequence
	/// FIXME: this is only needed for LFUN_ACCENT_* in Text::dispatch()
	virtual void handleKeyFunc(FuncCode action) = 0;

	/// Start a long operation with some cancel possibility (button or ESC)
	virtual void startLongOperation() = 0;
	/// This needs to be periodically called to avoid freezing the GUI
	virtual bool longOperationCancelled() = 0;
	/// Stop the long operation mode (i.e., release the GUI)
	virtual void stopLongOperation() = 0;
	/// A started long operation is still in progress ?
	virtual bool longOperationStarted() = 0;

};

/// Return the list of loadable formats.
std::vector<std::string> loadableImageFormats();

} // namespace frontend

frontend::Application * theApp();
frontend::Application * createApplication(int & argc, char * argv[]);
void hideDialogs(std::string const & name, Inset * inset);
/// Set locale correctly using LyXRC::gui_language
void setLocale();

} // namespace lyx


#endif // APPLICATION_H
