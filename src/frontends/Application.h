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

class BufferView;
class LyXView;
class LColor_color;
	
namespace lyx {

struct RGBColor;

namespace frontend {

class Clipboard;
class FontLoader;
class Gui;
class Selection;

/// The main application class
/**
There should be only one instance of this class. No Qt object
initialisation should be done before the instanciation of this class.

\todo The work areas handling could be moved to a base virtual class
comon to all frontends.
*/
class Application
{
public:
	///
	Application(int & argc, char ** argv);
	///
	virtual ~Application() {}

	/// Start the main event loop, after executing the given batch
	/// commands.
	int start(std::string const & batch);
	///
	virtual Gui & gui() = 0;
	///
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
	virtual bool getRgbColor(LColor_color col, lyx::RGBColor & rgbcol) = 0;

	/** Eg, passing LColor::black returns "000000",
	*      passing LColor::white returns "ffffff".
	*/
	virtual std::string const hexName(LColor_color col) = 0;

	/**
	* update an altered GUI color
	*/
	virtual void updateColor(LColor_color col) = 0;

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
		int posx, int posy, bool maximize);
	
	///
	void setBufferView(BufferView * buffer_view);

protected:
	/// This BufferView is the one receiving Clipboard and Selection
	/// Events
	/// FIXME: \todo use Gui::currentView() in the future
	BufferView * buffer_view_;

}; // Application

} // namespace frontend

lyx::frontend::Application * createApplication(int & argc, char * argv[]);

} // namespace lyx

extern lyx::frontend::Application * theApp;


#endif // LYX_APPLICATION_H
