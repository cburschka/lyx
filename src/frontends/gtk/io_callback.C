/**
 * \file io_callback.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author João Luis M. Assirati
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

// Too hard to make concept checks work with this file
#ifdef _GLIBCPP_CONCEPT_CHECKS
#undef _GLIBCPP_CONCEPT_CHECKS
#endif

#include "io_callback.h"


io_callback::io_callback(int fd, boost::function<void()> func)
	: func_(func)
{
	conn_ = Glib::signal_io().connect(
		sigc::mem_fun(*this, &io_callback::data_received),
		fd, Glib::IO_IN);
}


bool io_callback::data_received(Glib::IOCondition /*condition*/)
{
	func_();
	return true;
}
