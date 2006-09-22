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

#include "lyxfunc.h"
#include "lyxserver.h"
#include "lyxsocket.h"

#include <boost/scoped_ptr.hpp>

#include <string>

class BufferView;
class LyXView;

namespace lyx {
namespace frontend {

//class GuiWorkArea;
class Gui;


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
	Application(int & argc, char ** argv);

	int start(std::string const & batch);
	///
	virtual Gui & gui() = 0;
	///
	virtual int const exec() = 0;
	///
	virtual void exit(int status) = 0;

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
	void setBufferView(BufferView * buffer_view);

protected:
	///
	BufferView * buffer_view_;

	// FIXME: lyxfunc_ should be private. But the actual construction is done in
	// GuiApplication for now.

	/// our function handler
	boost::scoped_ptr<LyXFunc> lyxfunc_;

private:
	///
	boost::scoped_ptr<LyXServer> lyx_server_;
	///
	boost::scoped_ptr<LyXServerSocket> lyx_socket_;

}; // Application

} // namespace frontend
} // namespace lyx

extern lyx::frontend::Application * theApp;


#endif // LYX_APPLICATION_H
