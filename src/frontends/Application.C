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

#include "Application_pimpl.h"
#include "Gui.h"

#include "LyXAction.h"
#include "lyxfunc.h"
#include "lyxrc.h"
#include "LyXView.h"

#include "support/lstrings.h"
#include "support/os.h"
#include "support/package.h"


using lyx::support::package;

namespace lyx {
namespace frontend {

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
