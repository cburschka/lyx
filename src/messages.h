// -*- C++ -*-
/* \file messages.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef MESSAGES_H
#define MESSAGES_H

#include "LString.h"

#include <boost/scoped_ptr.hpp>

///
class Messages {
public:
	///
	Messages();
	///
	Messages(string const & l);
	///
	~Messages();
	///
	string const get(string const & msg) const;
private:
	class Pimpl;
	boost::scoped_ptr<Pimpl> pimpl_;
};

#endif
