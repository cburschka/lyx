/**
 * \file frontend/Application_pimpl.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_APPLICATION_PIMPL_H
#define LYX_APPLICATION_PIMPL_H

#include "bufferlist.h"
#include "funcrequest.h"
#include "lyxserver.h"
#include "lyxsocket.h"

#include <boost/scoped_ptr.hpp>

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

} // namespace frontend
} // namespace lyx

#endif // LYX_APPLICATION_PIMPL_H
