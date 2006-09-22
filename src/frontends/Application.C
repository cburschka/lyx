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

#include "funcrequest.h"
#include "LyXAction.h"
#include "lyxrc.h"
#include "LyXView.h"

#include "support/lstrings.h"
#include "support/os.h"
#include "support/package.h"

#include <boost/scoped_ptr.hpp>

using lyx::support::package;

namespace lyx {
namespace frontend {


Application::Application(int &, char **)
{
}


LyXFunc & Application::lyxFunc()
{
	return *lyxfunc_.get(); 
}


LyXFunc const & Application::lyxFunc() const
{
	return *lyxfunc_.get(); 
}


LyXServer & Application::server()
{
	return *lyx_server_.get(); 
}


LyXServer const & Application::server() const 
{
	return *lyx_server_.get(); 
}


LyXServerSocket & Application::socket()
{
	return *lyx_socket_.get();
}


LyXServerSocket const & Application::socket() const
{
	return *lyx_socket_.get();
}


void Application::setBufferView(BufferView * buffer_view)
{
	buffer_view_ = buffer_view;
}


int Application::start(std::string const & batch)
{
	lyx_server_.reset(new LyXServer(lyxfunc_.get(), lyxrc.lyxpipes));
	lyx_socket_.reset(new LyXServerSocket(lyxfunc_.get(), 
		lyx::support::os::internal_path(package().temp_dir() + "/lyxsocket")));

	// handle the batch commands the user asked for
	if (!batch.empty()) {
		lyxfunc_->dispatch(lyxaction.lookupFunc(batch));
	}

	return exec();
}


} // namespace frontend
} // namespace lyx
