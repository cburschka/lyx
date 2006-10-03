/**
 * \file frontend/Application.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Application.h"

#include "Gui.h"
#include "LyXView.h"
#include "WorkArea.h"

#include "bufferlist.h"
#include "funcrequest.h"
#include "LyXAction.h"
#include "lyxfunc.h"
#include "lyxrc.h"
#include "lyxserver.h"
#include "lyxsocket.h"

#include "support/lstrings.h"
#include "support/os.h"
#include "support/package.h"


using lyx::support::package;

namespace lyx {
namespace frontend {

/// The main application class private implementation.
struct Application_pimpl 
{
	///
	BufferList buffer_list_;
	/// our function handler
	boost::scoped_ptr<LyXFunc> lyxfunc_;
	///
	boost::scoped_ptr<LyXServer> lyx_server_;
	///
	boost::scoped_ptr<LyXServerSocket> lyx_socket_;
};


Application::Application(int &, char **)
{
	pimpl_ = new Application_pimpl;
}


LyXFunc & Application::lyxFunc()
{
	return *pimpl_->lyxfunc_.get();
}


LyXFunc const & Application::lyxFunc() const
{
	return *pimpl_->lyxfunc_.get(); 
}


LyXServer & Application::server()
{
	return *pimpl_->lyx_server_.get(); 
}


LyXServer const & Application::server() const 
{
	return *pimpl_->lyx_server_.get(); 
}


LyXServerSocket & Application::socket()
{
	return *pimpl_->lyx_socket_.get();
}


LyXServerSocket const & Application::socket() const
{
	return *pimpl_->lyx_socket_.get();
}


BufferList & Application::bufferList()
{
	return pimpl_->buffer_list_;
}


BufferList const & Application::bufferList() const
{
	return pimpl_->buffer_list_;
}


void Application::setBufferView(BufferView * buffer_view)
{
	buffer_view_ = buffer_view;
}


LyXView & Application::createView(unsigned int width,
								  unsigned int height,
								  int posx, int posy,
								  bool maximize)
{
	// FIXME: please confirm: with unicode, I think initEncoding()
	// is not needed anymore!
	
	// this can't be done before because it needs the Languages object
	//initEncodings();

	int view_id = gui().newView(width, height);
	LyXView & view = gui().view(view_id);

	pimpl_->lyxfunc_.reset(new LyXFunc(&view));

	// FIXME: for now we assume that there is only one LyXView with id = 0.
	/*int workArea_id_ =*/ gui().newWorkArea(width, height, 0);
	//WorkArea * workArea_ = & theApp->gui().workArea(workArea_id_);

	view.init();
	view.setGeometry(width, height, posx, posy, maximize);

	return view;
}


int Application::start(std::string const & batch)
{
	pimpl_->lyx_server_.reset(new LyXServer(pimpl_->lyxfunc_.get(), lyxrc.lyxpipes));
	pimpl_->lyx_socket_.reset(new LyXServerSocket(pimpl_->lyxfunc_.get(), 
		lyx::support::os::internal_path(package().temp_dir() + "/lyxsocket")));

	// handle the batch commands the user asked for
	if (!batch.empty()) {
		pimpl_->lyxfunc_->dispatch(lyxaction.lookupFunc(batch));
	}

	return exec();
}


} // namespace frontend
} // namespace lyx
