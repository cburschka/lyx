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

#include <boost/scoped_ptr.hpp>

#include <string>

class BufferList;
class BufferView;
class LyXFunc;
class LyXServer;
class LyXServerSocket;
class LyXView;
	
namespace lyx {
namespace frontend {

struct Application_pimpl;
class Clipboard;
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

	int start(std::string const & batch);
	///
	virtual Gui & gui() = 0;
	///
	virtual int const exec() = 0;
	///
	virtual void exit(int status) = 0;

	///
	virtual Clipboard & clipboard() = 0;
	///
	virtual Selection & selection() = 0;

	/// return a suitable serif font name.
	virtual std::string const romanFontName() = 0;

	/// return a suitable sans serif font name.
	virtual std::string const sansFontName() = 0;

	/// return a suitable monospaced font name.
	virtual std::string const typewriterFontName() = 0;

	///
	LyXFunc & lyxFunc();
	LyXFunc const & lyxFunc() const;
	///
	LyXServer & server();
	LyXServer const & server() const;
	///
	LyXServerSocket & socket();
	LyXServerSocket const & socket() const;
	///
	BufferList & bufferList();
	BufferList const & bufferList() const;

	///
	LyXView & createView(unsigned int width, unsigned int height,
		int posx, int posy, bool maximize);
	
	///
	void setBufferView(BufferView * buffer_view);

protected:
	/// This BufferView is the one receiving Clipboard and Selection
	/// Events
	/// FIXME: \todo use Gui::currentView() in the future
	BufferView * buffer_view_;

private:
	/// Application private implementation.
	Application_pimpl * pimpl_;

}; // Application

} // namespace frontend
} // namespace lyx

extern lyx::frontend::Application * theApp;


#endif // LYX_APPLICATION_H
