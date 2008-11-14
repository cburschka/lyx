// -*- C++ -*-
/* \file Messages.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MESSAGES_H
#define MESSAGES_H

#include "support/docstring.h"

#include <boost/scoped_ptr.hpp>


namespace lyx {

///
class Messages {
public:
	///
	Messages();
	///
	Messages(std::string const & l);
	///
	~Messages();
	///
	docstring const get(std::string const & msg) const;
private:
	class Pimpl;
	::boost::scoped_ptr<Pimpl> pimpl_;
};


} // namespace lyx

#endif
